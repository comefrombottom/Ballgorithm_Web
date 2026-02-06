#pragma once

# include <Siv3D.hpp>

class Button {
	RectF m_rect;

public:
	Button(const RectF& rect) : m_rect(rect) {}

	void update()
	{
		if (m_rect.mouseOver()) {
			if (MouseL.down()) {
				m_rect.setPos(Cursor::PosF() - m_rect.size / 2);
			}
		}
	}

	void draw(const ColorF& color) const {
		m_rect.draw(color);
	}
};
