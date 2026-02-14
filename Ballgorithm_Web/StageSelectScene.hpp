#pragma once

# include <Siv3D.hpp>
# include "ScrollBar.h"
# include "InputUtils.hpp"
# include "TextBox.h"

class Game;

class StageSelectScene {
	double m_titleWave = 0.0;  // タイトルアニメーション用
	Array<double> m_cardScales;  // カードのスケールアニメーション
	Array<double> m_cardOffsets;  // カードの横オフセット
	Optional<int32> m_hoveredIndex;  // ホバー中のカード
	Optional<int32> m_hoveredLeaderboardIndex;  // ホバー中のリーダーボードボタン
	ScrollBar m_scrollBar;  // スクロールバー

	double m_arrowKeyAccumulate = 0.0;
	int32 m_prevKeyInput = 0;

	SingleUseCursorPos m_cursorPos;  // カーソル位置（スクロールバー用）

	Optional<Vec2> m_mousePressPos;  // マウス押下位置（ドラッグ判定用）
	bool m_isDragging = false;

	bool m_isEditingUsername = false;
	mutable TextBox m_usernameTextBox;

	bool m_showResetConfirm = false;

	static constexpr double CardWidth = 360.0;
	static constexpr double CardHeight = 80.0;
	static constexpr double CardSpacing = 20.0;
	static constexpr double CardStartY = 180.0;
	
	RectF getCardRect(int32 index) const;
	void drawCard(int32 index, const String& name, bool isCleared, bool isSelected, bool isHovered, int32 queryCount, int32 completedCount) const;
	void drawBackground() const;
	void drawTitle() const;
	
public:
	StageSelectScene();
	void update(Game& game, double dt = Scene::DeltaTime());
	void draw(const Game& game) const;
};
