# include "TitleScene.hpp"
# include "Game.hpp"

void TitleScene::update(Game& game)
{
	m_time += Scene::DeltaTime();

	if ((not KeyAlt.pressed() and KeyEnter.down()) || (Cursor::Pos().intersects(Scene::Rect()) and MouseL.down()))
	{
		game.goToNameInput();
	}
}

void TitleScene::draw() const
{
	// 背景
	Rect{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF(0.05, 0.05, 0.1), Arg::bottom = ColorF(0.1, 0.15, 0.25));

	// 装飾的な円
	for (int i = 0; i < 8; ++i)
	{
		double t = m_time * 0.2 + i * Math::TwoPi / 8;
		double r = 150 + 50 * Math::Sin(m_time * 0.5 + i);
		Vec2 pos = Scene::Center() + Vec2(Math::Cos(t), Math::Sin(t)) * 200;
		Circle{ pos, r }.draw(ColorF(0.3, 0.5, 0.8, 0.05));
	}

	// タイトル
	const String title = U"Ballgorithm";
	FontAsset(U"Bold")(title).drawAt(Scene::Center().moveBy(0, -50), ColorF(0.9, 0.95, 1.0));
	
	// サブタイトル的な装飾
	FontAsset(U"Regular")(U"Algorithm Puzzle Game").drawAt(20, Scene::Center().moveBy(0, 20), ColorF(0.6, 0.7, 0.8));

	// Press Start
	double alpha = (Math::Sin(m_time * 3.0) + 1.0) * 0.5 * 0.8 + 0.2;
	FontAsset(U"Regular")(U"Click to Start").drawAt(24, Scene::Center().moveBy(0, 150), ColorF(1.0, alpha));
}
