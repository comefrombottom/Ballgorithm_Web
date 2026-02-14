# include "ContextMenu.h"

namespace {
	String GetContextMenuIcon(ContextMenuItemType type)
	{
		switch (type) {
		case ContextMenuItemType::Copy:
			return U"\uF0C5";
		case ContextMenuItemType::Delete:
			return U"\uF1F8";
		case ContextMenuItemType::Group:
			return U"\uF247";
		case ContextMenuItemType::Ungroup:
			return U"\uF248";
		case ContextMenuItemType::CameraReset:
			return U"\uF05B";
		case ContextMenuItemType::Paste:
			return U"\uF0EA";
		case ContextMenuItemType::Run:
			return U"\uF04B";
		default:
			return U"";
		}
	}
}

void ContextMenu::openWithSelection(const Vec2& pos)
{
	m_isOpen = true;
	m_position = pos;
	m_hoveredIndex = none;
	m_alignRight = false;

	// 選択ありメニュー: Copy, Delete, Group, Ungroup
	m_items.clear();
	m_items.push_back({ ContextMenuItemType::Delete, U"Delete", true });
	m_items.push_back({ ContextMenuItemType::Copy, U"Copy", true });
	m_items.push_back({ ContextMenuItemType::Group, U"Group", true });
	m_items.push_back({ ContextMenuItemType::Ungroup, U"Ungroup", true });
}

void ContextMenu::openWithSelectionAlignRight(const Vec2& pos)
{
	m_isOpen = true;
	m_position = pos;
	m_hoveredIndex = none;
	m_alignRight = true;

	// 選択ありメニュー: Copy, Delete, Group, Ungroup
	m_items.clear();
	m_items.push_back({ ContextMenuItemType::Delete, U"Delete", true });
	m_items.push_back({ ContextMenuItemType::Copy, U"Copy", true });
	m_items.push_back({ ContextMenuItemType::Group, U"Group", true });
	m_items.push_back({ ContextMenuItemType::Ungroup, U"Ungroup", true });
}

void ContextMenu::openWithSelectionClick(const Vec2& pos)
{
	m_isOpen = true;
	m_position = pos;
	m_hoveredIndex = none;
	m_alignRight = false;

	// 選択ありメニュー: Copy, Delete, Group, Ungroup
	m_items.clear();
	m_items.push_back({ ContextMenuItemType::Delete, U"Delete", true });
	m_items.push_back({ ContextMenuItemType::Copy, U"Copy", true });
	m_items.push_back({ ContextMenuItemType::Paste, U"Paste", true });
	m_items.push_back({ ContextMenuItemType::Group, U"Group", true });
	m_items.push_back({ ContextMenuItemType::Ungroup, U"Ungroup", true });
}

void ContextMenu::openWithoutSelection(const Vec2& pos)
{
	m_isOpen = true;
	m_position = pos;
	m_hoveredIndex = none;
	m_alignRight = false;

	// 選択なしメニュー: Camera Reset, Paste, Run
	m_items.clear();
	m_items.push_back({ ContextMenuItemType::CameraReset, U"Camera Reset", true });
	m_items.push_back({ ContextMenuItemType::Paste, U"Paste", true });
	m_items.push_back({ ContextMenuItemType::Run, U"Run", true });
}

void ContextMenu::close()
{
	m_isOpen = false;
	m_items.clear();
	m_hoveredIndex = none;
	m_alignRight = false;
}

Optional<ContextMenuItemType> ContextMenu::update(SingleUseCursorPos& cursorPos, bool hasSelection, bool canGroup, bool canUngroup, bool canPaste, bool canCopy, bool isSimulationRunning)
{
	if (!m_isOpen) return none;

	// 有効/無効状態を更新
	for (auto& item : m_items) {
		switch (item.type) {
		case ContextMenuItemType::Copy:
			item.enabled = canCopy && !isSimulationRunning;
			break;
		case ContextMenuItemType::Delete:
			item.enabled = hasSelection && !isSimulationRunning;
			break;
		case ContextMenuItemType::Group:
			item.enabled = canGroup && !isSimulationRunning;
			break;
		case ContextMenuItemType::Ungroup:
			item.enabled = canUngroup && !isSimulationRunning;
			break;
		case ContextMenuItemType::CameraReset:
			item.enabled = true;
			break;
		case ContextMenuItemType::Paste:
			item.enabled = canPaste && !isSimulationRunning;
			break;
		case ContextMenuItemType::Run:
			item.enabled = !isSimulationRunning;
			break;
		}
	}

	m_hoveredIndex = none;
	const RectF closeRect = getCloseButtonRect();
	const bool isOverClose = closeRect.contains(Cursor::PosF());
	if (isOverClose) {
		if (cursorPos) {
			cursorPos.use();
		}
		Cursor::RequestStyle(CursorStyle::Hand);
		if (MouseL.up()) {
			close();
			return none;
		}
	}

	// メニュー外をクリックで閉じる
	if (MouseL.up() || MouseR.up()) {
		if (!hitTest(Cursor::PosF())) {
			close();
			return none;
		}
	}

	// ホバー検出と選択
	const Vec2 mousePos = Cursor::PosF();
	for (int32 i = 0; i < m_items.size(); ++i) {
		RectF itemRect = getItemRect(i);
		if (itemRect.contains(mousePos)) {
			m_hoveredIndex = i;
			if (cursorPos) {
				cursorPos.use();
			}
			Cursor::RequestStyle(CursorStyle::Hand);

			if (MouseL.up() && m_items[i].enabled) {
				ContextMenuItemType selectedType = m_items[i].type;
				close();
				return selectedType;
			}
			break;
		}
	}

	// メニュー領域内のカーソルを消費
	if (hitTest(Cursor::PosF()) && cursorPos) {
		cursorPos.use();
	}

	return none;
}

void ContextMenu::draw() const
{
	if (!m_isOpen) return;

	const RectF menuRect = getMenuRect();
	const RectF closeRect = getCloseButtonRect();
	const bool isCloseHovered = closeRect.mouseOver();

	// 影
	menuRect.movedBy(3, 3).rounded(CornerRadius).draw(ColorF(0.0, 0.4));

	// 背景
	menuRect.rounded(CornerRadius).draw(ColorF(0.15, 0.18, 0.22, 0.98));
	menuRect.rounded(CornerRadius).drawFrame(1, ColorF(0.3, 0.35, 0.4));

	const Vec2 closeCenter = closeRect.center();
	const ColorF closeBg = isCloseHovered ? ColorF(0.8, 0.3, 0.3, 0.9) : ColorF(0.2, 0.22, 0.26, 0.9);
	const ColorF closeFrame = isCloseHovered ? ColorF(1.0, 0.6, 0.6, 0.9) : ColorF(0.5, 0.55, 0.6, 0.7);
	Circle(closeCenter, CloseButtonRadius).draw(closeBg);
	Circle(closeCenter, CloseButtonRadius).drawFrame(1.0, closeFrame);

	// 各項目の描画
	const Font& font = FontAsset(U"Regular");
	const Font& iconFont = FontAsset(U"Icon");
	iconFont(U"\uF00d").draw(14, Arg::center = closeCenter, ColorF(0.95));
	for (int32 i = 0; i < m_items.size(); ++i) {
		const auto& item = m_items[i];
		RectF itemRect = getItemRect(i);

		// ホバー時のハイライト
		if (m_hoveredIndex == i && item.enabled) {
			itemRect.stretched(-2, 0).rounded(4).draw(ColorF(0.3, 0.5, 0.7, 0.6));
		}

		// テキスト & アイコン
		ColorF textColor = item.enabled ? ColorF(0.95) : ColorF(0.5);
		if (item.type == ContextMenuItemType::Delete && item.enabled) {
			textColor = ColorF(0.95, 0.35, 0.35);
		}

		const String icon = GetContextMenuIcon(item.type);
		double contentX = itemRect.x + 10;
		if (!icon.isEmpty()) {
			iconFont(icon).drawAt(16, Vec2{ contentX + 8, itemRect.center().y }, textColor);
			contentX += 22;
		}
		font(item.label).draw(14, Arg::leftCenter = Vec2{ contentX, itemRect.center().y }, textColor);
	}
}

bool ContextMenu::hitTest(const Vec2& pos) const
{
	if (!m_isOpen) return false;
	return getMenuRect().contains(pos) || getCloseButtonRect().contains(pos);
}

RectF ContextMenu::getCloseButtonRect() const
{
	const RectF menuRect = getMenuRect();
	const Vec2 center = menuRect.tr() + Vec2{ -CloseButtonRadius, -CloseButtonRadius - 6.0 };
	return RectF{ Arg::center = center, CloseButtonRadius * 2.0, CloseButtonRadius * 2.0 };
}

RectF ContextMenu::getMenuRect() const
{
	const double width = ItemWidth + Padding * 2;
	const double height = Padding * 2 + ItemHeight * m_items.size();
	
	// 基準位置を計算
	Vec2 adjustedPos = m_position;
	
	// 右揃えの場合、メニューの右上が基準
	if (m_alignRight) {
		adjustedPos.x -= width;
	}
	
	// 画面端の調整
	if (adjustedPos.x + width > Scene::Width()) {
		adjustedPos.x = Scene::Width() - width;
	}
	if (adjustedPos.x < 0) {
		adjustedPos.x = 0;
	}
	if (adjustedPos.y + height > Scene::Height()) {
		adjustedPos.y = Scene::Height() - height;
	}
	if (adjustedPos.y < 0) {
		adjustedPos.y = 0;
	}
	
	return RectF{ adjustedPos, width, height };
}

RectF ContextMenu::getItemRect(int32 index) const
{
	const RectF menuRect = getMenuRect();
	return RectF{
		menuRect.x + Padding,
		menuRect.y + Padding + ItemHeight * index,
		ItemWidth,
		ItemHeight
	};
}
