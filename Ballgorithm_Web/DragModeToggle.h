#pragma once

# include <Siv3D.hpp>
# include "InputUtils.hpp"

// 範囲選択 / カメラ移動のドラッグモード切り替え
class DragModeToggle {
public:
	void setRect(const RectF& rect) { m_rect = rect; }
	const RectF& rect() const { return m_rect; }
	bool isRangeSelectLeft() const { return m_isRangeSelectLeft; }

	void update(SingleUseCursorPos& cursorPos);
	void draw() const;

private:
	RectF m_rect{ 0, 0, 220, 28 };
	bool m_isRangeSelectLeft = true;
};
