#pragma once

# include <Siv3D.hpp>
# include "Domain.hpp"
# include "InputUtils.hpp"

class Stage;
class MyCamera2D;

// ドラッグ中のボール情報
struct DraggingBallInfo {
	BallKind kind;
	size_t inventorySlotIndex;
	Optional<size_t> placedBallId;
	Vec2 grabOffset;
	Vec2 dragStartPos;  // ドラッグ開始位置（ワールド座標）
	Vec2 clickScreenPos;  // クリック時のスクリーン座標
};

// コンテキストメニューを開くためのコールバック型
// 引数: メニュー位置（ワールド座標）, 左方向フラグ（trueの場合メニューの右上が基準）
using OpenContextMenuCallback = std::function<void(const Vec2& worldPos, bool alignRight)>;

class StageEditUI
{
public:
	using StageEditedCallback = std::function<void(Stage&)>;

	StageEditUI() = default;

	void resetTransientState();

	// Update world-edit interaction (camera transformer must be active in caller if cursor pos is in world coords)
	void update(Stage& stage, bool isDoubleClicked, SingleUseCursorPos& cursorPos, const MyCamera2D& camera, const StageEditedCallback& onStageEdited, Optional<DraggingBallInfo>& draggingBall, const OpenContextMenuCallback& openContextMenu, bool useRightDragSelect, bool cancelSelectArea);

	// Draw editable world objects (expects caller to have activated camera transformer)
	void drawWorld(const Stage& stage, const MyCamera2D& camera) const;

	const SelectedIDSet& selectedIDs() const { return m_selectedIDs; }
	SelectedIDSet& selectedIDs() { return m_selectedIDs; }

	// edit actions
	bool eraseSelection(Stage& stage);
	bool groupSelection(Stage& stage);
	bool ungroupSelection(Stage& stage);
	bool groupOrUngroup(Stage& stage);

	bool canGroup(const Stage& stage) const;
	bool canUngroup(const Stage& stage) const;
	bool canGroupOrUngroup(const Stage& stage) const;

	// 最後の選択エリアの右下座標を取得（ワールド座標）
	Optional<Vec2> getLastSelectAreaBottomRight() const { return m_lastSelectAreaBottomRight; }
	void clearLastSelectAreaBottomRight() { m_lastSelectAreaBottomRight.reset(); }

private:
	SelectedIDSet m_selectedIDs;
	Optional<HoverInfo> m_hoveredInfo;
	Optional<Vec2> m_lineCreateStart;
	Vec2 m_lineCreateLastPos{};
	Optional<Vec2> m_selectAreaStart;
	Optional<Vec2> m_dragOffset;
	bool m_didDragMove = false;
	bool m_selectSingleLine = false;
	Vec2 m_clickStartPos{};  // クリック開始位置（ワールド座標）
	
	// 選択エリアの右下座標（選択完了時に保存、ワールド座標）
	Optional<Vec2> m_lastSelectAreaBottomRight;

	static constexpr double HOVER_THRESHOLD = 7.5;
	static constexpr double POINT_HOVER_THRESHOLD = 10.0;

	// helpers
	int32 getOneGridLength() const;
	int32 getDrawOneGridLength() const;

	bool isHoveredPoint(const Stage& stage, const Optional<HoverInfo>& hoverInfo, size_t pointId) const;
	bool isHoveredStartCircle(const Stage& stage, size_t index) const;
	bool isHoveredGoalArea(const Stage& stage, size_t index) const;
	bool isHoveredPlacedBall(const Stage& stage, size_t index) const;
	bool isHoveredEdge(const Stage& stage, size_t index) const;

	void updateLineCreateMode(Stage& stage, bool isDoubleClicked, SingleUseCursorPos& cursorPos, const std::function<void(Stage&)>& onStageEdited);
	void updateHoverInfo(Stage& stage, SingleUseCursorPos& cursorPos);
	void updateDragObject(Stage& stage, SingleUseCursorPos& cursorPos, const std::function<void(Stage&)>& onStageEdited, Optional<DraggingBallInfo>& draggingBall, const OpenContextMenuCallback& openContextMenu);
	void updateSelectArea(Stage& stage, SingleUseCursorPos& cursorPos, const OpenContextMenuCallback& openContextMenu, bool useRightDragSelect, bool cancelSelectArea);
};

