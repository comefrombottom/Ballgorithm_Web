# include "StageSelectScene.hpp"
# include "Game.hpp"
# include "IndexedDB.hpp"

StageSelectScene::StageSelectScene()
	: m_usernameTextBox(Vec2{ 0, 0 }, 200)
{
	// スクロールバーの初期化（ページ高さは後で更新される）
	m_scrollBar = ScrollBar(
		1000,  // pageHeight（仮）
		Scene::Height() - CardStartY - 20,  // viewHeight
		RectF{ Arg::topRight(Scene::Width() - 2, CardStartY), 16, Scene::Height() - CardStartY - 20 }
	);
	m_usernameTextBox = TextBox(Vec2{ 15, 5 }, 200);
}

RectF StageSelectScene::getCardRect(int32 index) const
{
	double x = (Scene::Width() - CardWidth) / 2.0;
	double y = CardStartY + index * (CardHeight + CardSpacing);
	return RectF{ x, y, CardWidth, CardHeight };
}

void StageSelectScene::drawBackground() const
{
	// グラデーション背景
	Rect{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF(0.1, 0.1, 0.2), Arg::bottom = ColorF(0.05, 0.15, 0.25));
	
	// 装飾的な円（背景）
	double time = Scene::Time();
	for (int i = 0; i < 5; ++i) {
		double phase = time * 0.3 + i * 1.2;
		double x = Scene::Width() * 0.5 + Math::Sin(phase) * 200;
		double y = Scene::Height() * 0.5 + Math::Cos(phase * 0.7) * 150;
		Circle{ x, y, 80 + i * 20 }.draw(ColorF(0.2, 0.3, 0.5, 0.1));
	}
	
	// グリッドパターン（薄く）
	for (int x = 0; x < Scene::Width(); x += 40) {
		Line{ x, 0, x, Scene::Height() }.draw(1, ColorF(1.0, 0.03));
	}
	for (int y = 0; y < Scene::Height(); y += 40) {
		Line{ 0, y, Scene::Width(), y }.draw(1, ColorF(1.0, 0.03));
	}
}

void StageSelectScene::drawTitle() const
{
	const Font& font = FontAsset(U"Regular");
	const String title = U"SELECT STAGE";

	RectF titleBg{ 0, 30, Scene::Width(), 80 };
	titleBg.draw(ColorF(0.0, 0.25));

	const double titleWidth = font(title).region(40).w;
	const Vec2 titlePos{ Scene::Width() / 2.0 - titleWidth / 2.0, 70 };
	font(title).draw(40, Arg::leftCenter = titlePos, ColorF(0.95));

}

void StageSelectScene::drawCard(int32 index, const String& name, bool isCleared, bool isSelected, bool isHovered, int32 queryCount, int32 completedCount) const
{
	const Font& font = FontAsset(U"Regular");
	RectF rect = getCardRect(index);
	
	// カードスケールとオフセット適用
	double scale = (index < m_cardScales.size()) ? m_cardScales[index] : 1.0;
	double offsetX = (index < m_cardOffsets.size()) ? m_cardOffsets[index] : 0.0;
	
	Vec2 center = rect.center();
	rect = RectF{ Arg::center = center + Vec2{offsetX, 0}, rect.w * scale, rect.h * scale };
	
	// カード背景色
	ColorF bgColor;
	if (isSelected) {
		bgColor = ColorF(0.3, 0.5, 0.8, 0.9);
	}
	else if (isHovered) {
		bgColor = ColorF(0.25, 0.35, 0.5, 0.85);
	}
	else {
		bgColor = ColorF(0.15, 0.2, 0.3, 0.8);
	}
	
	// カード影
	RectF shadowRect = rect.movedBy(4, 4);
	shadowRect.rounded(12).draw(ColorF(0.0, 0.3));
	
	// カード本体
	rect.rounded(12).draw(bgColor);
	
	// 選択時のボーダー
	if (isSelected) {
		rect.rounded(12).drawFrame(3, ColorF(0.5, 0.8, 1.0));
	}
	else {
		rect.rounded(12).drawFrame(1, ColorF(0.4, 0.5, 0.6, 0.5));
	}
	
	// ステージ番号バッジ
	Circle badge{ rect.x + 25, rect.center().y, 20 };
	badge.draw(isCleared ? ColorF(0.2, 0.7, 0.3) : bgColor);
	badge.drawFrame(2, isCleared ? ColorF(0.3, 0.8, 0.4) : isSelected ? ColorF(0.5, 0.8, 1.0) : ColorF(0.4, 0.5, 0.6, 0.5));
	font(U"{}"_fmt(index + 1)).draw(16, Arg::center = badge.center, Palette::White);
	
	// ステージ名
	double nameX = rect.x + 60;
	font(name).draw(22, Vec2{ nameX, rect.y + 20 }, Palette::White);
	
	// クエリ進捗バー
	if (queryCount > 0) {
		double barX = nameX;
		double barY = rect.y + 55;
		double barWidth = 150;
		double barHeight = 8;
		double progress = static_cast<double>(completedCount) / queryCount;
		
		// バー背景
		RectF{ barX, barY, barWidth, barHeight }.rounded(4).draw(ColorF(0.2, 0.2, 0.3));
		// バー進捗
		if (progress > 0) {
			RectF{ barX, barY, barWidth * progress, barHeight }.rounded(4).draw(
				isCleared ? ColorF(0.3, 0.8, 0.4) : ColorF(0.6, 0.6, 0.2)
			);
		}
		
		// 進捗テキスト
		font(U"{}/{}"_fmt(completedCount, queryCount)).draw(12, Vec2{ barX + barWidth + 10, barY - 2 }, ColorF(0.8));
	}
	
	// クリア済みチェックマーク
	if (isCleared) {
		double checkX = rect.x + rect.w - 40;
		double checkY = rect.center().y;
		Circle checkBg{ checkX, checkY, 18 };
		checkBg.draw(ColorF(0.2, 0.7, 0.3));
		font(U"✓").draw(20, Arg::center = checkBg.center, Palette::White);

		// リーダーボードボタン（チェックマークの左）
		const Font& iconFont = FontAsset(U"Icon");
		double lbX = checkX - 40;
		double lbY = rect.center().y;
		Circle lbBg{ lbX, lbY, 16 };
		bool lbHovered = lbBg.mouseOver();
		lbBg.draw(lbHovered ? ColorF(0.35, 0.55, 0.8) : ColorF(0.25, 0.35, 0.5));
		lbBg.drawFrame(1, lbHovered ? ColorF(0.5, 0.7, 1.0) : ColorF(0.4, 0.5, 0.6, 0.5));
		iconFont(U"\uF091").drawAt(13, lbBg.center, ColorF(0.95));
	}
	
	// ホバー時の光沢エフェクト
	/*if (isHovered) {
		double shineX = rect.x + Math::Fmod(Scene::Time() * 100, rect.w - 25);
		RectF shine{ shineX, rect.y, 25, rect.h };
		shine.draw(ColorF(1.0, 0.1));
	}*/
}

void StageSelectScene::update(Game& game, double dt)
{
	m_cursorPos.init();

	// ユーザー名編集処理
	if (m_isEditingUsername)
	{
		auto commitUsername = [&]()
		{
			String newName = m_usernameTextBox.editableText.text.trimmed();
			newName.remove_if([](char32 ch) { return (ch < 0x20) || (ch > 0x7E); });
			if (newName.isEmpty())
			{
				newName = game.m_username;
			}
			else
			{
				game.m_username = newName;

				JSON profile = JSON::Load(U"Ballgorithm/profile.json");
				if (not profile) {
					profile = JSON();
				}
				profile[U"username"] = game.m_username;
				profile.saveMinimum(U"Ballgorithm/profile.json");

# if SIV3D_PLATFORM(WEB)
				Platform::Web::IndexedDB::SaveAsync();
# endif
			}

			m_isEditingUsername = false;
			m_usernameTextBox.editableText.isFocused = false;
		};

		m_usernameTextBox.update();
		if (m_usernameTextBox.editableText.text.size() > 20)
		{
			m_usernameTextBox.editableText.text.resize(20);
			m_usernameTextBox.editableText.caretIndex = Min(m_usernameTextBox.editableText.caretIndex, 20);
			m_usernameTextBox.editableText.selectionEndIndex = Min(m_usernameTextBox.editableText.selectionEndIndex, 20);
		}

		if (not KeyAlt.pressed() and KeyEnter.down() and m_usernameTextBox.editableText.editingText.empty())
		{
			commitUsername();
		}

		if (MouseL.down() && not m_usernameTextBox.body.rect.contains(Cursor::PosF()))
		{
			commitUsername();
		}

		if (KeyEscape.down())
		{
			m_isEditingUsername = false;
			m_usernameTextBox.editableText.isFocused = false;
		}

		return;
	}

	// Editボタン
	{
		const Font& font = FontAsset(U"Regular");
		double usernameWidth = font(game.m_username).region(18).w;
		RectF editBtn{ 20 + usernameWidth + 10, 5, 50, 28 };
		if (editBtn.mouseOver())
		{
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		if (editBtn.leftClicked())
		{
			m_isEditingUsername = true;
			m_usernameTextBox.editableText.text = game.m_username;
			m_usernameTextBox.editableText.isFocused = true;
			m_usernameTextBox.editableText.selectAll();
			return;
		}
	}

	const auto& stages = game.getStages();
	int32 stageCount = stages.size();

	// タイトルアニメーション更新
	m_titleWave += dt;

	// カードスケール配列の初期化
	if (m_cardScales.size() != stageCount) {
		m_cardScales.resize(stageCount, 1.0);
		m_cardOffsets.resize(stageCount, 0.0);
	}

	// スクロールバーのページ高さを更新
	double pageHeight = CardStartY + stageCount * (CardHeight + CardSpacing) + 20;
	m_scrollBar.pageHeight = pageHeight;

	// 現在の選択インデックスを保存（変更検知用）
	int32 prevSelected = game.getSelectedStageIndex();

	// キーボード入力
	if (KeyDown.down()) {
		game.selectNextStage();
	}
	if (KeyUp.down()) {
		game.selectPrevStage();
	}

	// 長押し処理
	int32 keyInput = (KeyDown.pressed() - KeyUp.pressed());

	if (keyInput != m_prevKeyInput) {
		m_arrowKeyAccumulate = 0.0;
	}

	m_prevKeyInput = keyInput;

	if (keyInput == 1) {
		for (m_arrowKeyAccumulate += dt; m_arrowKeyAccumulate >= 0.5; m_arrowKeyAccumulate -= 0.05) {
			game.selectNextStage(false);
		}
	}

	if (keyInput == -1) {
		for (m_arrowKeyAccumulate += dt; m_arrowKeyAccumulate >= 0.5; m_arrowKeyAccumulate -= 0.05) {
			game.selectPrevStage(false);
		}
	}



	if (not KeyAlt.pressed() and KeyEnter.down()) {
		game.enterSelectedStage();
	}

	// スクロールバー更新
	m_scrollBar.update(m_cursorPos, Mouse::Wheel(), dt);

	if (m_cursorPos) {
		if (MouseL.down()) {
			m_mousePressPos = Cursor::PosF();
			m_isDragging = false;
			m_cursorPos.capture();
		}
	}

	if (m_mousePressPos) {
		if (m_mousePressPos->distanceFrom(Cursor::PosF()) > 20) {
			m_isDragging = true;
		}

		if (m_isDragging) {
			m_scrollBar.viewVelocity = -Cursor::DeltaF().y / dt;
		}
	}

	bool clickInput = false;
	if (MouseL.up()) {
		if (not m_isDragging) {
			clickInput = true;
		}
		m_mousePressPos.reset();
		m_isDragging = false;
		m_cursorPos.release();
	}


	// ホバー検出とマウスクリック
	m_hoveredIndex.reset();
	m_hoveredLeaderboardIndex.reset();
	for (int32 i = 0; i < stageCount; ++i) {
		RectF rect = getCardRect(i);
		// スクロール位置を考慮したヒットテスト
		RectF screenRect = rect.movedBy(0, -m_scrollBar.viewTop);

		// 画面内にある場合のみホバー判定
		if (screenRect.y + screenRect.h > CardStartY && screenRect.y < Scene::Height()) {
			// リーダーボードボタンのヒットテスト（クリアステージのみ）
			if (stages[i]->m_isCleared) {
				double scale = (i < m_cardScales.size()) ? m_cardScales[i] : 1.0;
				double offsetX = (i < m_cardOffsets.size()) ? m_cardOffsets[i] : 0.0;
				Vec2 center = screenRect.center();
				RectF scaledRect = RectF{ Arg::center = center + Vec2{offsetX, 0}, screenRect.w * scale, screenRect.h * scale };
				double checkX = scaledRect.x + scaledRect.w - 40;
				double lbX = checkX - 40;
				double lbY = scaledRect.center().y;
				Circle lbCircle{ lbX, lbY, 16 };
				if (lbCircle.contains(Cursor::PosF())) {
					m_hoveredLeaderboardIndex = i;
					Cursor::RequestStyle(CursorStyle::Hand);
					if (clickInput) {
						game.enterLeaderboard(i);
					}
					continue;  // カード全体のクリックと競合させない
				}
			}

			if (screenRect.mouseOver()) {
				m_hoveredIndex = i;
				Cursor::RequestStyle(CursorStyle::Hand);

				if (clickInput) {
					if (game.getSelectedStageIndex() == i) {
						game.enterSelectedStage();
					}
					else {
						game.selectStage(i);
					}
				}
			}
		}
	}


	

	// 選択が変更された場合、選択カードが見えるようにスクロール
	int32 currentSelected = game.getSelectedStageIndex();
	if (prevSelected != currentSelected) {
		RectF cardRect = getCardRect(currentSelected);
		// 上にはみ出ている場合
		if (cardRect.y < m_scrollBar.viewTop + CardStartY) {
			m_scrollBar.scrollTopTo(cardRect.y - CardStartY);
		}
		// 下にはみ出ている場合
		else if (cardRect.y + CardHeight > m_scrollBar.viewTop + m_scrollBar.viewHeight + CardStartY) {
			m_scrollBar.scrollBottomTo(cardRect.y + CardHeight - CardStartY);
		}
	}
}

void StageSelectScene::draw(const Game& game) const
{
	const auto& stages = game.getStages();
	int32 selected = game.getSelectedStageIndex();
	int32 clearedCount = 0;
	for (const auto& stage : stages)
	{
		if (stage->m_isCleared)
		{
			++clearedCount;
		}
	}
	
	// 背景描画
	drawBackground();

	// スクロール領域のクリッピング
	{
		// スクロールバーのTransformerを使用
		auto transformer = m_scrollBar.createTransformer();
		
		// カード描画
		for (int32 i = 0; i < stages.size(); ++i) {
			const auto& stage = stages[i];
			int32 queryCount = stage->m_queries.size();
			int32 completedCount = 0;
			for (int32 j = 0; j < stage->m_queryCompleted.size(); ++j) {
				if (stage->m_queryCompleted[j]) ++completedCount;
			}
			
			// 画面外のカードは描画スキップ
			RectF rect = getCardRect(i);
			if (rect.y + rect.h < m_scrollBar.viewTop + CardStartY - 50 || 
				rect.y > m_scrollBar.viewTop + m_scrollBar.viewHeight + CardStartY + 50) {
				continue;
			}
			
			bool isHovered = m_hoveredIndex && *m_hoveredIndex == i;
			drawCard(i, stage->m_name, stage->m_isCleared, i == selected, isHovered, queryCount, completedCount);
		}
	}
	
	// タイトル背景で上部を覆う（スクロールしたカードを隠す）
	RectF titleMask{ 0, 0, Scene::Width(), CardStartY - 20 };
	titleMask.draw(Arg::top = ColorF(0.1, 0.1, 0.2), Arg::bottom = ColorF(0.1, 0.1, 0.2, 0.95));
	
	// タイトル描画（カードの上に）
	drawTitle();


	// クリアステージ数表示
	{
		const Font& font = FontAsset(U"Regular");
		const String progressText = U"Cleared: {}/{}"_fmt(clearedCount, stages.size());
		font(progressText).draw(18, Arg::rightCenter = Vec2{ Scene::Width() - 20, 20 }, ColorF(0.85, 0.9, 0.95));
	}

	
	// スクロールバー描画
	m_scrollBar.draw(ColorF(0.5, 0.6, 0.7, 0.6));

	// ユーザー名表示
	{
		const Font& font = FontAsset(U"Regular");

		if (m_isEditingUsername)
		{
			m_usernameTextBox.draw();
			{
				const String countText = U"{}/20"_fmt(m_usernameTextBox.editableText.text.size());
				font(countText).draw(12, Arg::leftCenter = Vec2{ m_usernameTextBox.body.rect.x + m_usernameTextBox.body.rect.w - 30, m_usernameTextBox.body.rect.y - 6 }, ColorF(0.7, 0.8, 0.9, 0.7));
			}
		}
		else
		{
			font(game.m_username).draw(18, Arg::leftCenter = Vec2{ 20, 19 }, ColorF(0.85, 0.9, 0.95));

			double usernameWidth = font(game.m_username).region(18).w;
			RectF editBtn{ 20 + usernameWidth + 10, 5, 50, 28 };
			bool hovered = editBtn.mouseOver();
			editBtn.rounded(6).draw(hovered ? ColorF(0.3, 0.45, 0.65) : ColorF(0.2, 0.3, 0.45));
			editBtn.rounded(6).drawFrame(1, ColorF(0.4, 0.5, 0.6, 0.5));
			font(U"Edit").draw(14, Arg::center = editBtn.center(), ColorF(0.9));
		}
	}
}
