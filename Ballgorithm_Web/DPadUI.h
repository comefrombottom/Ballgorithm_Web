#pragma once

# include <Siv3D.hpp>
# include "InputUtils.hpp"

// 十字キーUIクラス
class DPadUI {
public:
	enum class Direction { Up, Down, Left, Right, None };

	DPadUI() = default;

	void setCenter(const Vec2& center) { m_center = center; }
	Vec2 center() const { return m_center; }

	// 更新処理：押されている方向を返す
	Direction update(SingleUseCursorPos& cursorPos);

	// 描画
	void draw() const;

	// 表示状態
	bool isVisible() const { return m_visible; }
	void setVisible(bool visible) { m_visible = visible; }

private:
	Vec2 m_center{ 0, 0 };
	bool m_visible = false;
	Direction m_hoveredDirection = Direction::None;
	Direction m_pressedDirection = Direction::None;
	double m_pressTimer = 0.0;
	double m_repeatTimer = 0.0;
	bool m_firstPress = true;

	static constexpr double ButtonSize = 50.0;
	static constexpr double ButtonGap = 4.0;
	static constexpr double RepeatDelay = 0.4;
	static constexpr double RepeatInterval = 0.05;

	RectF getButtonRect(Direction dir) const;
	bool hitTest(const Vec2& pos, Direction dir) const;
};
