#pragma once

# include <Siv3D.hpp>
# include "InputUtils.hpp"

class Stage;
class StageUI;

// コンテキストメニューの項目
enum class ContextMenuItemType {
	Copy,
	Delete,
	Group,
	Ungroup,
	CameraReset,
	Paste,
	Run,
};

struct ContextMenuItem {
	ContextMenuItemType type;
	String label;
	bool enabled = true;
};

// コンテキストメニュークラス
class ContextMenu {
public:
	ContextMenu() = default;

	// 選択ありメニューを開く（Copy, Delete, Group, Ungroup）
	void openWithSelection(const Vec2& pos);
	
	// 選択ありメニューを開く（右揃え：メニューの右上が基準）
	void openWithSelectionAlignRight(const Vec2& pos);

	// 右クリックで選択ありメニューを開く
	void openWithSelectionClick(const Vec2& pos);
	
	// 選択なしメニューを開く（Camera Reset, Paste, Run）
	void openWithoutSelection(const Vec2& pos);

	// メニューを閉じる
	void close();

	// メニューが開いているか
	bool isOpen() const { return m_isOpen; }

	// 更新処理：クリックされた項目を返す
	Optional<ContextMenuItemType> update(SingleUseCursorPos& cursorPos, bool hasSelection, bool canGroup, bool canUngroup, bool canPaste, bool canCopy, bool isSimulationRunning);

	// 描画
	void draw() const;

	// メニュー領域のヒットテスト
	bool hitTest(const Vec2& pos) const;

private:
	bool m_isOpen = false;
	Vec2 m_position{ 0, 0 };
	Array<ContextMenuItem> m_items;
	Optional<int32> m_hoveredIndex;
	bool m_alignRight = false;  // trueの場合、右揃え（右上が基準）

	static constexpr double ItemWidth = 140.0;
	static constexpr double ItemHeight = 32.0;
	static constexpr double Padding = 4.0;
	static constexpr double CornerRadius = 6.0;
	static constexpr double CloseButtonRadius = 13.0;

	RectF getMenuRect() const;
	RectF getCloseButtonRect() const;
	RectF getItemRect(int32 index) const;
};
