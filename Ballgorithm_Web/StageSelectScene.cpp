# include "StageSelectScene.hpp"
# include "Game.hpp"

StageSelectScene::StageSelectScene()
{
	// スクロールバーの初期化（ページ高さは後で更新される）
	m_scrollBar = ScrollBar(
		1000,  // pageHeight（仮）
		Scene::Height() - CardStartY - 20,  // viewHeight
		RectF{ Arg::topRight(Scene::Width() - 2, CardStartY), 16, Scene::Height() - CardStartY - 20 }
	);
}

RectF StageSelectScene::getCardRect(size_t index) const
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
	String title = U"SELECT STAGE";
	
	// タイトル背景
	RectF titleBg{ 0, 30, Scene::Width(), 80 };
	titleBg.draw(ColorF(0.0, 0.3));
	
	// 波打つタイトル文字
	Vec2 basePos{ Scene::Width() / 2.0, 70 };
	double totalWidth = font(title).region(40).w;
	double startX = basePos.x - totalWidth / 2.0;
	
	for (size_t i = 0; i < title.size(); ++i) {
		double offset = Math::Sin(m_titleWave * 3.0 + i * 0.5) * 3;
		double charX = startX + font(title.substr(0, i)).region(40).w;
		ColorF color = HSV{ 200 + i * 10, 0.6, 1.0 };
		font(title.substr(i, 1)).drawAt(40, Vec2{ charX, basePos.y + offset }, color); // drawAtを使用
	}
	
	// サブタイトル
	font(U"↑↓: Select   Enter: Start").draw(14, Arg::center = Vec2{ Scene::Width() / 2.0, 130 }, ColorF(0.7));
}

void StageSelectScene::drawCard(size_t index, const String& name, bool isCleared, bool isSelected, bool isHovered, size_t queryCount, size_t completedCount) const
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

	const auto& stages = game.getStages();
	size_t stageCount = stages.size();

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
	size_t prevSelected = game.getSelectedStageIndex();

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
	for (size_t i = 0; i < stageCount; ++i) {
		RectF rect = getCardRect(i);
		// スクロール位置を考慮したヒットテスト
		RectF screenRect = rect.movedBy(0, -m_scrollBar.viewTop);

		// 画面内にある場合のみホバー判定
		if (screenRect.y + screenRect.h > CardStartY && screenRect.y < Scene::Height()) {
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
	size_t currentSelected = game.getSelectedStageIndex();
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
	size_t selected = game.getSelectedStageIndex();
	
	// 背景描画
	drawBackground();
	
	// スクロール領域のクリッピング
	{
		// スクロールバーのTransformerを使用
		auto transformer = m_scrollBar.createTransformer();
		
		// カード描画
		for (size_t i = 0; i < stages.size(); ++i) {
			const auto& stage = stages[i];
			size_t queryCount = stage->m_queries.size();
			size_t completedCount = 0;
			for (size_t j = 0; j < stage->m_queryCompleted.size(); ++j) {
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
	
	// スクロールバー描画
	m_scrollBar.draw(ColorF(0.5, 0.6, 0.7, 0.6));
}
