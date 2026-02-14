#pragma once

# include <Siv3D.hpp>
# include "ScrollBar.h"
# include "InputUtils.hpp"

class Game;

class StageSelectScene {
	double m_titleWave = 0.0;  // タイトルアニメーション用
	Array<double> m_cardScales;  // カードのスケールアニメーション
	Array<double> m_cardOffsets;  // カードの横オフセット
	Optional<size_t> m_hoveredIndex;  // ホバー中のカード
	Optional<size_t> m_hoveredLeaderboardIndex;  // ホバー中のリーダーボードボタン
	ScrollBar m_scrollBar;  // スクロールバー

	double m_arrowKeyAccumulate = 0.0;
	int32 m_prevKeyInput = 0;

	SingleUseCursorPos m_cursorPos;  // カーソル位置（スクロールバー用）

	Optional<Vec2> m_mousePressPos;  // マウス押下位置（ドラッグ判定用）
	bool m_isDragging = false;

	static constexpr double CardWidth = 360.0;
	static constexpr double CardHeight = 100.0;
	static constexpr double CardSpacing = 20.0;
	static constexpr double CardStartY = 180.0;
	
	RectF getCardRect(size_t index) const;
	void drawCard(size_t index, const String& name, bool isCleared, bool isSelected, bool isHovered, size_t queryCount, size_t completedCount) const;
	void drawBackground() const;
	void drawTitle() const;
	
public:
	StageSelectScene();
	void update(Game& game, double dt = Scene::DeltaTime());
	void draw(const Game& game) const;
};
