#pragma once

# include <Siv3D.hpp>
# include "ScrollBar.h"
# include "InputUtils.hpp"

class Stage;

class QueryPanel {
public:
	QueryPanel() = default;

	void setRect(const RectF& rect);
	const RectF& rect() const;

	void onStageEnter(const Stage& stage);

	// returns true if a query card was clicked (and action executed)
	bool update(Stage& stage, SingleUseCursorPos& cursorPos, double dt);
	void draw(const Stage& stage) const;

private:
	RectF m_rect{ 0, 0, 0, 0 };

	ScrollBar m_scrollBar;
	Optional<Vec2> m_mousePressPos;
	bool m_isDragging = false;
};
