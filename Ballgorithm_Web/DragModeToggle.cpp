#include "DragModeToggle.h"

void DragModeToggle::update(SingleUseCursorPos& cursorPos)
{
	const RectF leftRect{ m_rect.x, m_rect.y, m_rect.w * 0.5, m_rect.h };
	const RectF rightRect{ m_rect.x + m_rect.w * 0.5, m_rect.y, m_rect.w * 0.5, m_rect.h };

	if (cursorPos.intersects_use(m_rect)) {
		if (MouseL.down()) {
			if (leftRect.contains(Cursor::PosF())) {
				m_isRangeSelectLeft = true;
			}
			else if (rightRect.contains(Cursor::PosF())) {
				m_isRangeSelectLeft = false;
			}
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}
}

void DragModeToggle::draw() const
{
	const RectF leftRect{ m_rect.x, m_rect.y, m_rect.w * 0.5, m_rect.h };
	const RectF rightRect{ m_rect.x + m_rect.w * 0.5, m_rect.y, m_rect.w * 0.5, m_rect.h };

	m_rect.rounded(6).draw(ColorF(0.1, 0.12, 0.16, 0.9));
	m_rect.rounded(6).drawFrame(1, ColorF(0.3, 0.35, 0.4, 0.6));

	const ColorF activeBg = ColorF(0.3, 0.6, 0.9, 0.5);
	const ColorF inactiveBg = ColorF(0.2, 0.22, 0.26, 0.6);
	leftRect.rounded(6).draw(m_isRangeSelectLeft ? activeBg : inactiveBg);
	rightRect.rounded(6).draw(m_isRangeSelectLeft ? inactiveBg : activeBg);

	const Font& iconFont = FontAsset(U"Icon");
	iconFont(U"\uF0C8").drawAt(14, leftRect.center(), ColorF(0.9));
	iconFont(U"\uF245").drawAt(14, rightRect.center(), ColorF(0.9));
}
