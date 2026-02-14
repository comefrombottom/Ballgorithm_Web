#include "stdafx.h"
#include "StageEditUI.h"

#include "Stage.hpp"
#include "GeometryUtils.hpp"
#include "MyCamera2D.h"

bool StageEditUI::eraseSelection(Stage& stage)
{
	if (m_selectedIDs.empty()) return false;

	// 削除前に PlacedBall のカウントを集計してインベントリに戻す
	for (const auto& s : m_selectedIDs) {
		if (s.type == SelectType::PlacedBall) {
			const auto& ball = stage.m_placedBalls[s.id];
			stage.returnToInventory(ball.kind);
		}
		else if (s.type == SelectType::Group) {
			const auto& group = stage.m_groups.at(s.id);
			for (const auto& placedBallId : group.getAllPlacedBallIds()) {
				const auto& ball = stage.m_placedBalls[placedBallId];
				stage.returnToInventory(ball.kind);
			}
		}
	}

	stage.eraseSelectedPoints(m_selectedIDs.m_ids);
	m_selectedIDs.clear();
	return true;
}


bool StageEditUI::canGroup(const Stage& stage) const
{
	HashSet<int32> addedPointIds;
	int32 selectedCount = 0;
	for (const auto& s : m_selectedIDs) {
		if (s.type == SelectType::Point) {
			addedPointIds.insert(s.id);
		}
		else {
			selectedCount++;
		}
	}

	for (const auto& edge : stage.m_edges) {
		if (addedPointIds.contains(edge.ids[0]) and addedPointIds.contains(edge.ids[1])) {
			selectedCount++;
		}
	}

	return selectedCount >= 2;
}

bool StageEditUI::canUngroup(const Stage& stage) const
{
	if (m_selectedIDs.size() == 1 && m_selectedIDs.begin()->type == SelectType::Group) {
		const auto groupId = m_selectedIDs.begin()->id;
		if (stage.m_groups.contains(groupId) && !stage.m_groups.at(groupId).isLocked) {
			return true;
		}
	}
	return false;
}


bool StageEditUI::canGroupOrUngroup(const Stage& stage) const
{
	return canGroup(stage) || canUngroup(stage);
}


bool StageEditUI::groupSelection(Stage& stage)
{
	if (!canGroup(stage)) return false;
	stage.createGroupFromSelection(m_selectedIDs.m_ids);
	return true;
}

bool StageEditUI::ungroupSelection(Stage& stage)
{
	if (!canUngroup(stage)) return false;
	int32 onlySelectedGroupId = m_selectedIDs.begin()->id;
	auto& group = stage.m_groups.at(onlySelectedGroupId);
	if (group.isLocked) return false;
	for (auto& subGroup : group.m_groups) {
		int32 newGroupId = stage.m_nextGroupId++;
		stage.m_groups[newGroupId] = subGroup;
	}
	stage.m_groups.erase(onlySelectedGroupId);
	m_selectedIDs.clear();
	return true;
}

bool StageEditUI::groupOrUngroup(Stage& stage)
{
	if (canGroup(stage)) {
		return groupSelection(stage);
	}
	else if (canUngroup(stage)) {
		return ungroupSelection(stage);
	}
	return false;
}


void StageEditUI::resetTransientState()
{
	m_selectedIDs.clear();
	m_hoveredInfo.reset();
	m_lineCreateStart.reset();
	m_selectAreaStart.reset();
	m_dragOffset.reset();
	m_didDragMove = false;
	m_selectSingleLine = false;
	m_lastSelectAreaBottomRight.reset();
}

int32 StageEditUI::getOneGridLength() const
{
	return static_cast<int32>(Clamp(Math::Exp2(Round(Math::Log2(4.0 / Graphics2D::GetMaxScaling()))), 1.0, 1.0)) * 5;
}

int32 StageEditUI::getDrawOneGridLength() const
{
	return static_cast<int32>(Clamp(Math::Exp2(Round(Math::Log2(4.0 / Graphics2D::GetMaxScaling()))), 1.0, 16.0)) * 5;
}

bool StageEditUI::isHoveredPoint(const Stage& stage, const Optional<HoverInfo>& hoverInfo, int32 pointId) const
{
	if (not hoverInfo) return false;
	if (hoverInfo->type == HoverType::Point && hoverInfo->id == pointId) return true;
	if (hoverInfo->type == HoverType::Group) return stage.m_groups.at(hoverInfo->id).containsPointRecursive(pointId);
	if (hoverInfo->type == HoverType::Edge) {
		auto& edge = stage.m_edges.at(hoverInfo->id);
		for (auto pid : edge.ids) {
			if (auto topGroupId = stage.findTopGroup(pid)) {
				if (stage.m_groups.at(*topGroupId).containsPointRecursive(pointId)) return true;
			}
			else if (pid == pointId) {
				return true;
			}
		}
	}
	return false;
}

bool StageEditUI::isHoveredStartCircle(const Stage& stage, int32 index) const
{
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::StartCircle && m_hoveredInfo->id == index) return true;
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::Group) {
		return stage.m_groups.at(m_hoveredInfo->id).containsStartCircleRecursive(index);
	}
	return false;
}

bool StageEditUI::isHoveredGoalArea(const Stage& stage, int32 index) const
{
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::GoalArea && m_hoveredInfo->id == index) return true;
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::Group) {
		return stage.m_groups.at(m_hoveredInfo->id).containsGoalAreaRecursive(index);
	}
	return false;
}

bool StageEditUI::isHoveredPlacedBall(const Stage& stage, int32 index) const
{
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::PlacedBall && m_hoveredInfo->id == index) return true;
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::Group) {
		return stage.m_groups.at(m_hoveredInfo->id).containsPlacedBallRecursive(index);
	}
	return false;
}

bool StageEditUI::isHoveredEdge(const Stage& stage, int32 index) const
{
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::Edge && m_hoveredInfo->id == index) return true;
	if (m_hoveredInfo && m_hoveredInfo->type == HoverType::Group) {
		const auto& edge = stage.m_edges[index];
		return stage.m_groups.at(m_hoveredInfo->id).containsPointRecursive(edge[0])
			|| stage.m_groups.at(m_hoveredInfo->id).containsPointRecursive(edge[1]);
	}
	return false;
}

void StageEditUI::updateHoverInfo(Stage& stage, SingleUseCursorPos& cursorPos)
{
	m_hoveredInfo.reset();
	if (not cursorPos) return;

	for (auto it = stage.m_layerOrder.rbegin(); it != stage.m_layerOrder.rend(); ++it) {
		const auto& obj = *it;

		switch (obj.type) {
		case LayerObjectType::PlacedBall: {
			const auto& ball = stage.m_placedBalls[obj.id];
			if (cursorPos.eval_use([&](Vec2 pos) { return ball.circle.contains(pos); })) {
				if (auto topGroupId = stage.findTopGroupForPlacedBall(obj.id)) {
					m_hoveredInfo = HoverInfo{ HoverType::Group, *topGroupId };
				}
				else {
					m_hoveredInfo = HoverInfo{ HoverType::PlacedBall, obj.id };
				}
				return;
			}
			break;
		}
		case LayerObjectType::StartCircle: {
			const auto& c = stage.m_startCircles[obj.id];
			if (c.isLocked) break;
			if (cursorPos.eval_use([&](Vec2 pos) { return c.circle.contains(pos); })) {
				if (auto topGroupId = stage.findTopGroupForStartCircle(obj.id)) {
					m_hoveredInfo = HoverInfo{ HoverType::Group, *topGroupId };
				}
				else {
					m_hoveredInfo = HoverInfo{ HoverType::StartCircle, obj.id };
				}
				return;
			}
			break;
		}
		case LayerObjectType::GoalArea: {
			const auto& r = stage.m_goalAreas[obj.id];
			if (r.isLocked) break;
			if (cursorPos.eval_use([&](Vec2 pos) { return r.rect.contains(pos); })) {
				if (auto topGroupId = stage.findTopGroupForGoalArea(obj.id)) {
					m_hoveredInfo = HoverInfo{ HoverType::Group, *topGroupId };
				}
				else {
					m_hoveredInfo = HoverInfo{ HoverType::GoalArea, obj.id };
				}
				return;
			}
			break;
		}
		case LayerObjectType::Edge: {
			const auto& edge = stage.m_edges[obj.id];
			if (edge.isLocked) break;
			const Vec2& p1 = stage.m_points.at(edge[0]);
			const Vec2& p2 = stage.m_points.at(edge[1]);

			for (int32 j = 0; j < edge.ids.size(); ++j) {
				auto point_id = edge.ids[edge.ids.size() - 1 - j];
				if (stage.findTopGroup(point_id).has_value()) continue;

				const Vec2& point_pos = stage.m_points.at(point_id);
				if ((point_pos - cursorPos.value()).length() <= POINT_HOVER_THRESHOLD) {
					cursorPos.reset();
					m_hoveredInfo = HoverInfo{ HoverType::Point, point_id };
					return;
				}
			}

			Line line(p1, p2);
			double distance = Geometry2D::Distance(cursorPos.value(), line);
			if (distance <= HOVER_THRESHOLD) {
				cursorPos.reset();
				bool anyPointInGroup = false;
				Optional<int32> groupId;
				for (const auto& e : edge.ids) {
					if (auto topGroup = stage.findTopGroup(e)) {
						anyPointInGroup = true;
						groupId = topGroup;
						break;
					}
				}
				if (anyPointInGroup && groupId) {
					m_hoveredInfo = HoverInfo{ HoverType::Group, *groupId };
				}
				else {
					m_hoveredInfo = HoverInfo{ HoverType::Edge, obj.id };
				}
				return;
			}
			break;
		}
		}
	}
}

void StageEditUI::updateDragObject(Stage& stage, SingleUseCursorPos& cursorPos, const std::function<void(Stage&)>& onStageEdited, Optional<DraggingBallInfo>& draggingBall, const OpenContextMenuCallback& openContextMenu)
{
	if (MouseL.down()) {
		// クリック開始位置を記録（メニュー表示用）
		m_clickStartPos = Cursor::PosF();
		
		if (m_hoveredInfo) {
			bool selectSingleLine = false;

			if (m_hoveredInfo->type == HoverType::PlacedBall) {
				int32 placedBallId = m_hoveredInfo->id;

				if (auto topGroupId = stage.findTopGroupForPlacedBall(placedBallId)) {
					SelectedID sid{ SelectType::Group, *topGroupId };
					if (KeyShift.pressed()) {
						if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
					}
					else if (not m_selectedIDs.contains(sid)) {
						m_selectedIDs.clear();
						m_selectedIDs.insert(sid);
					}
					if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
						m_dragOffset = *pos - Cursor::PosF();
						cursorPos.capture();
					}
				}
				else {
					// 単体のPlacedBallの場合
					if (m_selectedIDs.size() > 1 && m_selectedIDs.contains(SelectedID{ SelectType::PlacedBall, placedBallId })) {
						// 複数選択中でこのボールも選択されている場合は従来通り
						stage.bringToFront(LayerObject{ LayerObjectType::PlacedBall, placedBallId });
						if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
							m_dragOffset = *pos - Cursor::PosF();
							cursorPos.capture();
						}
						m_didDragMove = false;
					}
					else if (KeyShift.pressed()) {
						// Shift押下時は選択の追加/削除
						SelectedID sid{ SelectType::PlacedBall, placedBallId };
						if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
						stage.bringToFront(LayerObject{ LayerObjectType::PlacedBall, placedBallId });
						if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
							m_dragOffset = *pos - Cursor::PosF();
							cursorPos.capture();
						}
						m_didDragMove = false;
					}
					else {
						// 単体PlacedBallを掴んだ場合はdraggingBallに移動
						const auto& ball = stage.m_placedBalls[placedBallId];
						Vec2 grabOffset = ball.circle.center - Cursor::PosF();
						// クリック時のスクリーン座標を保存（カメラ変換前の座標が必要なのでワールド座標を保存)
						draggingBall = DraggingBallInfo{ ball.kind, 0, placedBallId, grabOffset, ball.circle.center, Cursor::PosF() };
						
						// ステージからボールを削除（インデックスは保持)
						stage.removePlacedBall(placedBallId);
						
						m_selectedIDs.clear();
						cursorPos.capture();
					}
				}
			}
			else if (m_hoveredInfo->type == HoverType::StartCircle) {
				SelectedID sid{ SelectType::StartCircle, m_hoveredInfo->id };
				if (KeyShift.pressed()) {
					if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
				}
				else if (not m_selectedIDs.contains(sid)) {
					m_selectedIDs.clear();
					m_selectedIDs.insert(sid);
				}
				stage.bringToFront(LayerObject{ LayerObjectType::StartCircle, m_hoveredInfo->id });
				if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
					m_dragOffset = *pos - Cursor::PosF();
					cursorPos.capture();
				}
				m_didDragMove = false;
			}
			else if (m_hoveredInfo->type == HoverType::GoalArea) {
				SelectedID sid{ SelectType::GoalArea, m_hoveredInfo->id };
				if (KeyShift.pressed()) {
					if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
				}
				else if (not m_selectedIDs.contains(sid)) {
					m_selectedIDs.clear();
					m_selectedIDs.insert(sid);
				}
				stage.bringToFront(LayerObject{ LayerObjectType::GoalArea, m_hoveredInfo->id });
				if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
					m_dragOffset = *pos - Cursor::PosF();
					cursorPos.capture();
				}
				m_didDragMove = false;
			}
			else if (m_hoveredInfo->type == HoverType::Point) {
				int32 pointId = m_hoveredInfo->id;
				SelectedID sid{ SelectType::Point, pointId };
				if (KeyShift.pressed()) {
					if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
				}
				else if (not m_selectedIDs.contains(sid)) {
					m_selectedIDs.clear();
					m_selectedIDs.insert(sid);
				}
				else {
					if (m_selectedIDs.contains(sid) && m_selectSingleLine) {
						m_selectedIDs.clear();
						m_selectedIDs.insert(sid);
					}
				}

				if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
					m_dragOffset = *pos - Cursor::PosF();
					cursorPos.capture();
				}
				m_didDragMove = false;
			}
			else if (m_hoveredInfo->type == HoverType::Edge) {
				auto& edge = stage.m_edges[m_hoveredInfo->id];
				Array<int32> insertPointIds;
				Array<int32> insertGroupIds;
				for (auto pointId : edge.ids) {
					if (auto topGroup = stage.findTopGroup(pointId)) {
						if (not insertGroupIds.contains(*topGroup)) insertGroupIds.push_back(*topGroup);
					}
					else {
						if (not insertPointIds.contains(pointId)) insertPointIds.push_back(pointId);
					}
				}
				bool allSelected = true;
				for (const auto& id : insertPointIds) allSelected &= m_selectedIDs.contains(SelectedID{ SelectType::Point, id });
				for (const auto& id : insertGroupIds) allSelected &= m_selectedIDs.contains(SelectedID{ SelectType::Group, id });

				if (KeyShift.pressed()) {
					if (allSelected) {
						for (const auto& id : insertPointIds) m_selectedIDs.erase(SelectedID{ SelectType::Point, id });
						for (const auto& id : insertGroupIds) m_selectedIDs.erase(SelectedID{ SelectType::Group, id });
					}
					else {
						for (const auto& id : insertPointIds) m_selectedIDs.insert(SelectedID{ SelectType::Point, id });
						for (const auto& id : insertGroupIds) m_selectedIDs.insert(SelectedID{ SelectType::Group, id });
					}
				}
				else if (not allSelected) {
					m_selectedIDs.clear();
					for (const auto& id : insertPointIds) m_selectedIDs.insert(SelectedID{ SelectType::Point, id });
					for (const auto& id : insertGroupIds) m_selectedIDs.insert(SelectedID{ SelectType::Group, id });
					if (insertPointIds.size() == 2 && insertGroupIds.empty()) {
						selectSingleLine = true;
					}
				}
				else {
					selectSingleLine = m_selectSingleLine;
				}

				stage.bringToFront(LayerObject{ LayerObjectType::Edge, m_hoveredInfo->id });
				if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
					m_dragOffset = *pos - Cursor::PosF();
					cursorPos.capture();
				}
				m_didDragMove = false;
			}
			else if (m_hoveredInfo->type == HoverType::Group) {
				int32 groupId = m_hoveredInfo->id;
				SelectedID sid{ SelectType::Group, groupId };
				if (KeyShift.pressed()) {
					if (m_selectedIDs.contains(sid)) m_selectedIDs.erase(sid); else m_selectedIDs.insert(sid);
				}
				else if (not m_selectedIDs.contains(sid)) {
					m_selectedIDs.clear();
					m_selectedIDs.insert(sid);
				}
				if (auto pos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
					m_dragOffset = *pos - Cursor::PosF();
					cursorPos.capture();
				}
				m_didDragMove = false;
			}

			m_selectSingleLine = selectSingleLine;
		}
	}

	if (m_dragOffset) {
		Vec2 targetPos = Cursor::PosF() + *m_dragOffset;
		targetPos = Snap(targetPos, getOneGridLength());
		if (auto beginPos = m_selectedIDs.getBeginPointOfSelectedObjects(stage)) {
			Vec2 deltaMove = targetPos - *beginPos;
			if (deltaMove != Vec2{ 0, 0 }) {
				bool canMove = m_selectedIDs.isMovedSelectedNotInNonEditableArea(stage, deltaMove);

				if (canMove) {
					m_selectedIDs.moveSelectedObjects(stage, deltaMove);
					m_didDragMove = true;
				}
				else {
					Vec2 bestDelta{ 0, 0 };
					for (int32_t i = 0; i < 100; i++) {
						double nowLen = (targetPos - *beginPos - bestDelta).length();
						bool allDirEnded = true;
						for (Point dir : { Point{ 0,-1 }, Point{ 1,-1 }, Point{ 1,0 }, Point{ 1,1 }, Point{ 0,1 }, Point{ -1,1 }, Point{ -1,0 }, Point{ -1,-1 } }) {
							Vec2 newDelta = bestDelta + dir * getOneGridLength();
							if ((targetPos - *beginPos - newDelta).length() < nowLen
								&& m_selectedIDs.isMovedSelectedNotInNonEditableArea(stage, newDelta)) {
								bestDelta = newDelta;
								allDirEnded = false;
							}
						}
						if (allDirEnded) break;
					}

					if (bestDelta != Vec2{ 0, 0 }) {
						m_selectedIDs.moveSelectedObjects(stage, bestDelta);
						m_didDragMove = true;
					}
				}
			}
		}
		Cursor::RequestStyle(CursorStyle::ResizeAll);
	}

	if (MouseL.up() && m_dragOffset) {
		m_dragOffset.reset();
		cursorPos.release();
		if (m_didDragMove) {
			onStageEdited(stage);
			// ドラッグ移動した場合はメニューを表示しない
		}
		else {
			// ドラッグしなかった場合（クリック選択のみ）はメニューを表示
			if (!m_selectedIDs.empty() && openContextMenu) {
				// クリックした位置を基準にメニューを表示（ワールド座標）
				openContextMenu(m_clickStartPos, false);
			}
		}
		m_didDragMove = false;
	}
}

void StageEditUI::updateLineCreateMode(Stage& stage, bool isDoubleClicked, SingleUseCursorPos& cursorPos, const std::function<void(Stage&)>& onStageEdited)
{
	if (isDoubleClicked && cursorPos) {
		m_selectedIDs.clear();
		Vec2 start = Snap(*cursorPos, getOneGridLength());
		if (!stage.isPointInNonEditableArea(start)) {
			m_lineCreateStart = start;
			m_lineCreateLastPos = start;
			cursorPos.capture();
		}
		return;
	}

	if (m_lineCreateStart) {
		if (Vec2 newPos = Snap(Cursor::PosF(), getOneGridLength()); stage.isLineAllowedInEditableArea(Line(m_lineCreateStart.value(), newPos))) {
			m_lineCreateLastPos = newPos;
		}
		else {
			for (int32_t i = 0; i < 100; i++) {
				double nowLen = (Cursor::PosF() - m_lineCreateLastPos).length();
				bool allDirEnded = true;
				for (Point dir : { Point{ 0,-1 }, Point{ 1,-1 }, Point{ 1,0 }, Point{ 1,1 }, Point{ 0,1 }, Point{ -1,1 }, Point{ -1,0 }, Point{ -1,-1 } }) {
					Vec2 newPos = m_lineCreateLastPos + dir * getOneGridLength();
					if ((Cursor::PosF() - newPos).length() < nowLen
						&& stage.isLineAllowedInEditableArea(Line(m_lineCreateStart.value(), newPos))) {
						m_lineCreateLastPos = newPos;
						allDirEnded = false;
					}
				}
				if (allDirEnded) break;
			}
		}

		if (MouseL.up() || MouseL.down()) {
			Line line(m_lineCreateStart.value(), Snap(m_lineCreateLastPos, getOneGridLength()));
			if (line.length() >= 5.0) {
				if (stage.isLineAllowedInEditableArea(line)) {
					stage.addLine(line);
					auto& edge = stage.m_edges.back();
					m_selectedIDs.clear();
					m_selectedIDs.insert(SelectedID{ SelectType::Point, edge[0] });
					m_selectedIDs.insert(SelectedID{ SelectType::Point, edge[1] });
					m_selectSingleLine = true;
					onStageEdited(stage);
				}
			}
			m_lineCreateStart.reset();
			cursorPos.release();
		}
	}
}

void StageEditUI::updateSelectArea(Stage& stage, SingleUseCursorPos& cursorPos, const OpenContextMenuCallback& openContextMenu, bool useRightDragSelect, bool cancelSelectArea)
{
	if (cancelSelectArea && m_selectAreaStart) {
		m_selectAreaStart.reset();
		m_lastSelectAreaBottomRight.reset();
		cursorPos.release();
		return;
	}
	const bool selectDown = useRightDragSelect ? MouseR.down() : MouseL.down();
	const bool selectUp = useRightDragSelect ? MouseR.up() : MouseL.up();
	if (selectDown && cursorPos) {
		m_selectAreaStart = *cursorPos;
		cursorPos.capture();
	}
	if (selectUp && m_selectAreaStart) {
		Vec2 startPos = m_selectAreaStart.value();
		Vec2 endPos = Cursor::PosF();
		Vec2 tl = Math::Min(startPos, endPos);
		Vec2 br = Math::Max(startPos, endPos);
		RectF area = RectF{ tl, br - tl };
		auto oldSelectedIDs = m_selectedIDs;
		m_selectedIDs.selectObjectsInArea(stage, area);

		double len = (br - tl).length();
		if (len > 20 && m_selectedIDs.empty()) {
			m_selectedIDs = oldSelectedIDs;
			m_lastSelectAreaBottomRight.reset();
		}
		else if (len > 20 && !m_selectedIDs.empty()) {
			// 選択エリアの右下座標を保存（ワールド座標）
			m_lastSelectAreaBottomRight = br;
			m_selectSingleLine = false;
			// エリア選択完了時にメニューを表示
			if (openContextMenu) {
				// 終了位置を基準にする
				// 開始位置より終了位置が左にある場合はalignRight=true（メニューの右上が基準）
				bool alignRight = (endPos.x < startPos.x);
				openContextMenu(endPos, alignRight);
			}
		}
		else {
			m_lastSelectAreaBottomRight.reset();
			m_selectSingleLine = false;
		}

		m_selectAreaStart.reset();
		cursorPos.release();
	}
}

void StageEditUI::update(Stage& stage, bool isDoubleClicked, SingleUseCursorPos& cursorPos, const MyCamera2D& /*camera*/, const StageEditedCallback& onStageEdited, Optional<DraggingBallInfo>& draggingBall, const OpenContextMenuCallback& openContextMenu, bool useRightDragSelect, bool cancelSelectArea)
{
	updateLineCreateMode(stage, isDoubleClicked, cursorPos, onStageEdited);
	updateHoverInfo(stage, cursorPos);
	updateDragObject(stage, cursorPos, onStageEdited, draggingBall, openContextMenu);
	updateSelectArea(stage, cursorPos, openContextMenu, useRightDragSelect, cancelSelectArea);
	PrintDebug(m_selectSingleLine);
}

void StageEditUI::drawWorld(const Stage& stage, const MyCamera2D& camera) const
{
	// simulation mode draw: draw same world objects, except edit-only visuals
	if (stage.m_isSimulationRunning) {
		// non-editable areas
		if (!stage.nonEditableAreas().empty()) {
			for (const auto& r : stage.nonEditableAreas()) {
				r.rounded(6).draw(ColorF(0.15, 0.08, 0.08, 0.25));
				r.rounded(6).drawFrame(2.0 / Graphics2D::GetMaxScaling(), ColorF(0.9, 0.35, 0.35, 0.7));
			}
		}

		// layer ordered draw (no selection/hover effects)
		for (auto it = stage.m_layerOrder.begin(); it != stage.m_layerOrder.end(); ++it) {
			const auto& obj = *it;
			switch (obj.type) {
			case LayerObjectType::GoalArea: {
				int32 i = obj.id;
				const auto& r = stage.m_goalAreas[i];
				ColorF color = ColorF(0.2, 0.65, 0.3, 0.5);
				ColorF frameColor = ColorF(0.3, 0.75, 0.4, 0.7);
				r.rect.draw(color);
				r.rect.drawFrame(2.0 / Graphics2D::GetMaxScaling(), frameColor);
				FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + i))).drawAt(14.0 / Graphics2D::GetMaxScaling(), r.rect.center(), ColorF(1.0));
				break;
			}
			case LayerObjectType::StartCircle: {
				int32 i = obj.id;
				const auto& c = stage.m_startCircles[i];
				ColorF color = ColorF(0.2, 0.65, 0.3, 0.5);
				ColorF frameColor = ColorF(0.3, 0.75, 0.4, 0.7);
				c.circle.draw(color);
				c.circle.drawFrame(2.0 / Graphics2D::GetMaxScaling(), frameColor);
				FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + i))).drawAt(14.0 / Graphics2D::GetMaxScaling(), c.circle.center, ColorF(1.0));
				break;
			}
			case LayerObjectType::PlacedBall: {
				// while simulating, placed balls should be shown as physical bodies (startBallsInWorld)
				break;
			}
			case LayerObjectType::Edge: {
				int32 i = obj.id;
				const auto& edge = stage.m_edges[i];
				const Vec2& p1 = stage.m_points.at(edge[0]);
				const Vec2& p2 = stage.m_points.at(edge[1]);
				ColorF lineColor = ColorF(0.6, 0.65, 0.7);
				if (edge.isLocked) { lineColor *= 0.6; }
				Line(p1 + Vec2{ 2, 2 }, p2 + Vec2{ 2, 2 }).draw(3.0 / Graphics2D::GetMaxScaling(), ColorF(0.0, 0.2));
				Line(p1, p2).draw(2.5 / Graphics2D::GetMaxScaling(), lineColor);
				break;
			}
			}
		}

		// simulation balls
		for (const auto& ball : stage.m_startBallsInWorld) {
			ColorF ballColor = GetBallColor(ball.kind);
			ball.body.draw(ballColor);

			Circle circle(ball.body.getPos(), GetBallRadius(ball.kind));
			double angle = ball.body.getAngle();
			HSV color = HSV(GetBallColor(ball.kind));
			color.s *= 0.8;
			circle.drawPie(angle, Math::HalfPi, color);
			circle.drawPie(angle + Math::Pi, Math::HalfPi, color);
		}
		return;
	}

	// non-editable areas
	if (!stage.nonEditableAreas().empty()) {
		for (const auto& r : stage.nonEditableAreas()) {
			r.rounded(6).draw(ColorF(0.15, 0.08, 0.08, 0.25));
			r.rounded(6).drawFrame(2.0 / Graphics2D::GetMaxScaling(), ColorF(0.9, 0.35, 0.35, 0.7));
		}
	}

	// grid points
	{
		int32 oneGridLength = getDrawOneGridLength();
		Point tl = Floor(camera.getRegion().tl() / oneGridLength).asPoint();
		Point br = Ceil(camera.getRegion().br() / oneGridLength).asPoint();
		for (const auto& p : step(tl, br - tl + Point::One())) {
			if (IsEven(p.x) and IsEven(p.y)) {
				RectF(Arg::center = p * oneGridLength, 2 / Graphics2D::GetMaxScaling()).draw(ColorF(1, 0.25));
			}
			else {
				RectF(Arg::center = p * oneGridLength, 2 / Graphics2D::GetMaxScaling()).draw(ColorF(1, 0.15));
			}
		}
	}

	// layer ordered draw (edit mode assumes !simulation)
	for (auto it = stage.m_layerOrder.begin(); it != stage.m_layerOrder.end(); ++it) {
		const auto& obj = *it;
		switch (obj.type) {
		case LayerObjectType::GoalArea: {
			int32 i = obj.id;
			const auto& r = stage.m_goalAreas[i];
			ColorF color = ColorF(0.2, 0.65, 0.3, 0.5);
			ColorF frameColor = ColorF(0.3, 0.75, 0.4, 0.7);
			if (m_selectedIDs.isSelectedGoalArea(stage, i)) {
				color = ColorF(0.3, 0.6, 1.0, 0.5);
				frameColor = ColorF(0.4, 0.7, 1.0, 0.9);
			}
			else if (isHoveredGoalArea(stage, i)) {
				color = ColorF(1.0, 0.6, 0.2, 0.5);
				frameColor = ColorF(1.0, 0.7, 0.3, 0.9);
			}
			r.rect.draw(color);
			r.rect.drawFrame(2.0 / Graphics2D::GetMaxScaling(), frameColor);
			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + i))).drawAt(14.0 / Graphics2D::GetMaxScaling(), r.rect.center(), ColorF(1.0));
			break;
		}
		case LayerObjectType::StartCircle: {
			int32 i = obj.id;
			const auto& c = stage.m_startCircles[i];
			ColorF color = ColorF(0.2, 0.65, 0.3, 0.5);
			ColorF frameColor = ColorF(0.3, 0.75, 0.4, 0.7);
			if (m_selectedIDs.isSelectedStartCircle(stage, i)) {
				color = ColorF(0.3, 0.6, 1.0, 0.5);
				frameColor = ColorF(0.4, 0.7, 1.0, 0.9);
			}
			else if (isHoveredStartCircle(stage, i)) {
				color = ColorF(1.0, 0.6, 0.2, 0.5);
				frameColor = ColorF(1.0, 0.7, 0.3, 0.9);
			}
			c.circle.draw(color);
			c.circle.drawFrame(2.0 / Graphics2D::GetMaxScaling(), frameColor);
			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + i))).drawAt(14.0 / Graphics2D::GetMaxScaling(), c.circle.center, ColorF(1.0));
			break;
		}
		case LayerObjectType::PlacedBall: {
			int32 i = obj.id;
			const auto& placedBall = stage.m_placedBalls[i];
			ColorF color = GetBallColor(placedBall.kind);
			ColorF frameColor = ColorF(1.0, 1.0, 1.0, 0.8);

			if (m_selectedIDs.isSelectedPlacedBall(stage, i)) {
				color = ColorF(0.3, 0.6, 1.0, 0.9);
				frameColor = ColorF(0.5, 0.8, 1.0);
			}
			else if (isHoveredPlacedBall(stage, i)) {
				color = ColorF(1.0, 0.6, 0.2, 0.9);
				frameColor = ColorF(1.0, 0.8, 0.4);
			}

			Circle shadow{ placedBall.circle.center + Vec2{3, 3}, placedBall.circle.r };
			shadow.draw(ColorF(0.0, 0.3));

			placedBall.circle.draw(color);
			placedBall.circle.drawFrame(2.5 / Graphics2D::GetMaxScaling(), frameColor);
			break;
		}
		case LayerObjectType::Edge: {
			int32 i = obj.id;
			const auto& edge = stage.m_edges[i];
			const Vec2& p1 = stage.m_points.at(edge[0]);
			const Vec2& p2 = stage.m_points.at(edge[1]);
			ColorF lineColor = ColorF(0.6, 0.65, 0.7);
			if (edge.isLocked) { lineColor *= 0.6; }
			if (m_selectedIDs.isSelectedEdge(stage, i)) { lineColor = ColorF(0.4, 0.7, 1.0); }
			else if (isHoveredEdge(stage, i)) { lineColor = ColorF(1.0, 0.7, 0.3); }

			Line(p1 + Vec2{ 2, 2 }, p2 + Vec2{ 2, 2 }).draw(3.0 / Graphics2D::GetMaxScaling(), ColorF(0.0, 0.2));
			Line(p1, p2).draw(2.5 / Graphics2D::GetMaxScaling(), lineColor);

			auto drawEdgeEndpointHighlight = [&](int32 pointId, int32 otherPointId) {
				const bool isPointSelected = m_selectedIDs.isSelectedPoint(stage, pointId);
				const bool isPointHovered = isHoveredPoint(stage, m_hoveredInfo, pointId);
				if (!isPointSelected && !isPointHovered) {
					return;
				}

				const Vec2 pointPos = stage.m_points.at(pointId);
				const Vec2 otherPointPos = stage.m_points.at(otherPointId);
				const Vec2 direction = otherPointPos - pointPos;
				const double length = direction.length();
				if (length <= 0.0) {
					return;
				}

				const double segmentLength = Min(10.0, length);
				const Vec2 segmentEnd = pointPos + (direction / length) * segmentLength;
				const ColorF highlightColor = isPointSelected ? ColorF(0.4, 0.7, 1.0) : ColorF(1.0, 0.7, 0.3);
				Line(pointPos, segmentEnd).draw(3.0 / Graphics2D::GetMaxScaling(), highlightColor);
			};

			drawEdgeEndpointHighlight(edge[0], edge[1]);
			drawEdgeEndpointHighlight(edge[1], edge[0]);

			for (auto j : step(2)) {
				int32 pointId = edge[j];
				int32 otherPointId = edge[1 - j];
				Vec2 pointPos = stage.m_points.at(pointId);

				bool isInGroup = stage.findTopGroup(pointId).has_value();

				ColorF pointColor = ColorF(0.6, 0.65, 0.7);
				if (m_selectedIDs.isSelectedPoint(stage, pointId)) {
					pointColor = ColorF(0.4, 0.7, 1.0);
					if (std::none_of(m_selectedIDs.begin(), m_selectedIDs.end(), [&](const SelectedID& s) { return s.type == SelectType::Group; })
						&& std::count_if(m_selectedIDs.begin(), m_selectedIDs.end(), [&](const SelectedID& s) { return s.type == SelectType::Point; }) == 1) {
						Vec2 otherPointPos = stage.m_points.at(otherPointId);
						Vec2 direction = (pointPos - otherPointPos);
						if ((direction - Floor(direction)).isZero()) {
							Point direction_int = Floor(direction).asPoint();
							bool right = (pointPos.x >= otherPointPos.x);
							String coordText = U"{}/{}"_fmt(direction_int.x, direction_int.y);
							Vec2 textPos = right ? pointPos + Vec2(20, -25) : pointPos + Vec2(-20 - FontAsset(U"Regular")(coordText).region(13).w, -25);
							RectF textBg = FontAsset(U"Regular")(coordText).region(13, textPos);
							textBg = textBg.stretched(4, 2);
							textBg.rounded(4).draw(ColorF(0.1, 0.12, 0.15, 0.9));
							if (right) { FontAsset(U"Regular")(coordText).draw(13, pointPos + Vec2(24, -23), ColorF(0.9)); }
							else { FontAsset(U"Regular")(coordText).draw(13, pointPos + Vec2(-24 - FontAsset(U"Regular")(coordText).region(13).w, -23), ColorF(0.9)); }
						}
					}
				}
				else if (isHoveredPoint(stage, m_hoveredInfo, pointId)) {
					pointColor = ColorF(1.0, 0.7, 0.3);
				}

				if (not edge.isLocked && not isInGroup) {
					//Circle(pointPos, 10.0).draw(ColorF(0.15, 0.18, 0.22, 0.8));
					Circle(pointPos, 10.0).drawFrame(2.5 / Graphics2D::GetMaxScaling(), pointColor);
				}
			}
			break;
		}
		}
	}

	if (m_lineCreateStart) {
		Vec2 start = m_lineCreateStart.value();
		Vec2 end = Snap(m_lineCreateLastPos, getOneGridLength());
		Line(start, end).draw(2.5 / Graphics2D::GetMaxScaling(), ColorF(0.7, 0.75, 0.8, 0.8));
		Circle(start, 10.0).drawFrame(2.5 / Graphics2D::GetMaxScaling(), ColorF(0.7, 0.75, 0.8));
		Circle(end, 10.0).drawFrame(2.5 / Graphics2D::GetMaxScaling(), ColorF(0.7, 0.75, 0.8));
		Vec2 direction = (end - start);
		if ((direction - Floor(direction)).isZero()) {
			Point direction_int = Floor(direction).asPoint();
			bool right = (end.x >= start.x);
			const String coordText = U"{}/{}"_fmt(direction_int.x, direction_int.y);
			if (right) { FontAsset(U"Regular")(coordText).draw(13, end + Vec2(20, -20), ColorF(0.7)); }
			else { FontAsset(U"Regular")(coordText).draw(13, end + Vec2(-20 - FontAsset(U"Regular")(coordText).region(13).w, -20), ColorF(0.7)); }
		}
	}
	if (m_selectAreaStart) {
		Vec2 tl = Math::Min(m_selectAreaStart.value(), Cursor::PosF());
		Vec2 br = Math::Max(m_selectAreaStart.value(), Cursor::PosF());
		RectF area = RectF{ tl, br - tl };
		area.draw(ColorF(0.4, 0.6, 1.0, 0.15));
		area.drawFrame(2 / Graphics2D::GetMaxScaling(), ColorF(0.5, 0.7, 1.0, 0.7));
	}
}
