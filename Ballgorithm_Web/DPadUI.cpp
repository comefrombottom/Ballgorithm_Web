#include "DPadUI.h"

RectF DPadUI::getButtonRect(Direction dir) const {
	const double offset = ButtonSize + ButtonGap;
	switch (dir) {
	case Direction::Up:
		return RectF{ Arg::center = m_center + Vec2{0, -offset}, ButtonSize, ButtonSize };
	case Direction::Down:
		return RectF{ Arg::center = m_center + Vec2{0, offset}, ButtonSize, ButtonSize };
	case Direction::Left:
		return RectF{ Arg::center = m_center + Vec2{-offset, 0}, ButtonSize, ButtonSize };
	case Direction::Right:
		return RectF{ Arg::center = m_center + Vec2{offset, 0}, ButtonSize, ButtonSize };
	default:
		return RectF{};
	}
}

bool DPadUI::hitTest(const Vec2& pos, Direction dir) const {
	return getButtonRect(dir).contains(pos);
}

DPadUI::Direction DPadUI::update(SingleUseCursorPos& cursorPos) {
	if (!m_visible) return Direction::None;

	m_hoveredDirection = Direction::None;
	Direction result = Direction::None;

	// 各方向の領域を包む矩形内なら、最も近いボタンをホバー扱いにする
	const RectF upRect = getButtonRect(Direction::Up);
	const RectF downRect = getButtonRect(Direction::Down);
	const RectF leftRect = getButtonRect(Direction::Left);
	const RectF rightRect = getButtonRect(Direction::Right);
	const double minX = Min({ upRect.x, downRect.x, leftRect.x, rightRect.x });
	const double minY = Min({ upRect.y, downRect.y, leftRect.y, rightRect.y });
	const double maxX = Max({ upRect.x + upRect.w, downRect.x + downRect.w, leftRect.x + leftRect.w, rightRect.x + rightRect.w });
	const double maxY = Max({ upRect.y + upRect.h, downRect.y + downRect.h, leftRect.y + leftRect.h, rightRect.y + rightRect.h });
	const RectF groupRect{ minX, minY, (maxX - minX), (maxY - minY) };
	if (cursorPos && groupRect.contains(*cursorPos)) {
		double bestDistance = Math::Inf;
		for (Direction dir : { Direction::Up, Direction::Down, Direction::Left, Direction::Right }) {
			const Vec2 center = getButtonRect(dir).center();
			const double distance = center.distanceFrom(*cursorPos);
			if (distance < bestDistance) {
				bestDistance = distance;
				m_hoveredDirection = dir;
			}
		}
		cursorPos.use();
	}

	// マウスダウンで押下開始
	if (MouseL.down() && m_hoveredDirection != Direction::None) {
		m_pressedDirection = m_hoveredDirection;
		m_pressTimer = 0.0;
		m_repeatTimer = 0.0;
		m_firstPress = true;
		result = m_pressedDirection;
	}

	// 押下中の繰り返し処理
	if (MouseL.pressed() && m_pressedDirection != Direction::None) {
		// 押下中も同じボタン上にいるかチェック
		if (m_hoveredDirection == m_pressedDirection) {
			m_pressTimer += Scene::DeltaTime();

			if (m_firstPress) {
				m_firstPress = false;
				// 最初の押下は既にresultに設定済み
			}
			else if (m_pressTimer >= RepeatDelay) {
				m_repeatTimer += Scene::DeltaTime();
				while (m_repeatTimer >= RepeatInterval) {
					m_repeatTimer -= RepeatInterval;
					result = m_pressedDirection;
				}
			}
		}
	}

	// マウスアップで押下終了
	if (MouseL.up()) {
		m_pressedDirection = Direction::None;
		m_pressTimer = 0.0;
		m_repeatTimer = 0.0;
		m_firstPress = true;
	}

	return result;
}

void DPadUI::draw() const {
	if (!m_visible) return;

	auto drawButton = [&](Direction dir, const String& arrow) {
		RectF rect = getButtonRect(dir);
		bool isHovered = (m_hoveredDirection == dir);
		bool isPressed = (m_pressedDirection == dir && MouseL.pressed());

		// 影
		rect.movedBy(2, 2).rounded(8).draw(ColorF(0.0, 0.3));

		// 背景
		ColorF bgColor = ColorF(0.2, 0.25, 0.3, 0.95);
		if (isPressed) {
			bgColor = ColorF(0.4, 0.6, 0.9, 0.95);
		}
		else if (isHovered) {
			bgColor = ColorF(0.3, 0.4, 0.5, 0.95);
		}
		rect.rounded(8).draw(bgColor);

		// 枠
		ColorF frameColor = isPressed ? ColorF(0.6, 0.8, 1.0, 0.9) : ColorF(0.4, 0.5, 0.6, 0.7);
		rect.rounded(8).drawFrame(2, frameColor);

		// 矢印
		ColorF arrowColor = isPressed ? ColorF(1.0) : (isHovered ? ColorF(0.95) : ColorF(0.75));
		FontAsset(U"Regular")(arrow).drawAt(24, rect.center(), arrowColor);
	};

	// 中央のボタン背景（十字の中心部分）
	RectF centerRect{ Arg::center = m_center, ButtonSize * 0.6, ButtonSize * 0.6 };
	centerRect.rounded(6).draw(ColorF(0.15, 0.18, 0.22, 0.8));

	drawButton(Direction::Up, U"▲");
	drawButton(Direction::Down, U"▼");
	drawButton(Direction::Left, U"◀");
	drawButton(Direction::Right, U"▶");
}
