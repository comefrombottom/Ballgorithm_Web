# include "NameInputScene.hpp"
# include "Game.hpp"
# include "IndexedDB.hpp"

NameInputScene::NameInputScene()
	: m_textBox(Vec2{ 0, 0 }, 360)
{
	double x = (Scene::Width() - 360) / 2.0;
	double y = Scene::Height() / 2.0 - 20;
	m_textBox = TextBox(Vec2{ x, y }, 360);
	m_textBox.editableText.text = U"";
}

void NameInputScene::update(Game& game)
{
	m_time += Scene::DeltaTime();

	m_textBox.update();
	if (m_textBox.editableText.text.size() > 20)
	{
		m_textBox.editableText.text.resize(20);
		m_textBox.editableText.caretIndex = Min(m_textBox.editableText.caretIndex, 20);
		m_textBox.editableText.selectionEndIndex = Min(m_textBox.editableText.selectionEndIndex, 20);
	}

	if (m_showError && m_prevText != m_textBox.editableText.text)
	{
		m_showError = false;
	}

	m_prevText = m_textBox.editableText.text;

	// Focus the text box automatically
	if (!m_textBox.editableText.isFocused && m_time < 0.5)
	{
		m_textBox.editableText.isFocused = true;
	}

	auto commitName = [&]()
	{
		String name = m_textBox.editableText.text.trimmed();
		name.remove_if([](char32 ch) { return (ch < 0x20) || (ch > 0x7E); });
		if (name.isEmpty())
		{
			m_showError = true;
			return;
		}

		game.m_username = name;

		JSON profile = JSON::Load(U"Ballgorithm/profile.json");
		if (not profile) {
			profile = JSON();
		}
		profile[U"username"] = game.m_username;
		profile.saveMinimum(U"Ballgorithm/profile.json");

# if SIV3D_PLATFORM(WEB)
		Platform::Web::IndexedDB::SaveAsync();
# endif

		game.startTransition(GameState::StageSelect);
	};

	if (not KeyAlt.pressed() and KeyEnter.down() and m_textBox.editableText.editingText.empty())
	{
		commitName();
	}

	RectF okButton{ Arg::center = Vec2{ Scene::Width() / 2.0, Scene::Height() / 2.0 + 60 }, 140, 36 };
	if (okButton.mouseOver())
	{
		Cursor::RequestStyle(CursorStyle::Hand);
	}
	if (okButton.leftClicked())
	{
		commitName();
	}
}

void NameInputScene::draw() const
{
	// 背景
	Rect{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF(0.05, 0.05, 0.1), Arg::bottom = ColorF(0.1, 0.15, 0.25));

	// 装飾的な円
	for (int i = 0; i < 5; ++i)
	{
		double t = m_time * 0.3 + i * Math::TwoPi / 5;
		double r = 100 + 30 * Math::Sin(m_time * 0.5 + i);
		Vec2 pos = Scene::Center() + Vec2(Math::Cos(t), Math::Sin(t)) * 200;
		Circle{ pos, r }.draw(ColorF(0.3, 0.5, 0.8, 0.05));
	}

	const Font& font = FontAsset(U"Regular");

	// タイトル
	font(U"Enter Your Name").drawAt(36, Scene::Center().moveBy(0, -100), ColorF(0.95));

	// TextBox
	m_textBox.draw();
	{
		const String countText = U"{}/20"_fmt(m_textBox.editableText.text.size());
		font(countText).draw(14, Arg::leftCenter = Vec2{ m_textBox.body.rect.x + m_textBox.body.rect.w - 40, m_textBox.body.rect.y - 10 }, ColorF(0.7, 0.8, 0.9, 0.7));
	}

	// OKボタン
	RectF okButton{ Arg::center = Vec2{ Scene::Width() / 2.0, Scene::Height() / 2.0 + 80 }, 140, 36 };
	bool okHovered = okButton.mouseOver();
	okButton.rounded(8).draw(okHovered ? ColorF(0.35, 0.55, 0.8) : ColorF(0.25, 0.35, 0.5));
	okButton.rounded(8).drawFrame(1, ColorF(0.4, 0.5, 0.6, 0.5));
	font(U"OK").draw(18, Arg::leftCenter = Vec2{ okButton.center().x - 12, okButton.center().y }, ColorF(0.95));

	// エラーメッセージ
	if (m_showError)
	{
		font(U"Name cannot be empty").drawAt(18, Scene::Center().moveBy(0, 40), ColorF(1.0, 0.4, 0.4));
	}

	// ヒント
	double alpha = (Math::Sin(m_time * 3.0) + 1.0) * 0.5 * 0.6 + 0.2;
	font(U"Press Enter to confirm").drawAt(20, Scene::Center().moveBy(0, 120), ColorF(0.7, 0.8, 0.9, alpha));

	// ASCII制限メッセージ
	font(U"ASCII characters only | 半角英数字記号のみ").draw(14, Arg::leftCenter = Vec2{ m_textBox.body.rect.x, m_textBox.body.rect.y - 15 }, ColorF(0.7, 0.8, 0.9, 0.7));
}
