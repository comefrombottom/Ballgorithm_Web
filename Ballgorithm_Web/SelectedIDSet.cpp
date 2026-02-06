#include "Domain.hpp"
#include "Stage.hpp"

bool SelectedIDSet::isSelectedPoint(const Stage& stage, size_t pointId) const
{
	if (m_ids.contains(SelectedID{ SelectType::Point, pointId })) return true;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			if (stage.m_groups.at(s.id).containsPointRecursive(pointId)) return true;
		}
	}
	return false;
}

bool SelectedIDSet::isSelectedEdge(const Stage& stage, size_t index) const
{
	bool hasEdge_0 = m_ids.contains(SelectedID{ SelectType::Point, stage.m_edges[index][0] });
	bool hasEdge_1 = m_ids.contains(SelectedID{ SelectType::Point, stage.m_edges[index][1] });
	if (hasEdge_0 && hasEdge_1) return true;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			const auto& edge = stage.m_edges[index];
			// エッジの両端点がグループに属しているかチェック
			if (stage.m_groups.at(s.id).containsPointRecursive(edge[0]) ||
				stage.m_groups.at(s.id).containsPointRecursive(edge[1])) {
				return true;
			}
		}
	}
	return false;
}

bool SelectedIDSet::isSelectedStartCircle(size_t index) const
{
	return m_ids.contains(SelectedID{ SelectType::StartCircle, index });
}

bool SelectedIDSet::isSelectedStartCircle(const Stage& stage, size_t index) const
{
	if (m_ids.contains(SelectedID{ SelectType::StartCircle, index })) return true;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			if (stage.m_groups.at(s.id).containsStartCircleRecursive(index)) return true;
		}
	}
	return false;
}

bool SelectedIDSet::isSelectedGoalArea(size_t index) const
{
	return m_ids.contains(SelectedID{ SelectType::GoalArea, index });
}

bool SelectedIDSet::isSelectedGoalArea(const Stage& stage, size_t index) const
{
	if (m_ids.contains(SelectedID{ SelectType::GoalArea, index })) return true;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			if (stage.m_groups.at(s.id).containsGoalAreaRecursive(index)) return true;
		}
	}
	return false;
}

bool SelectedIDSet::isSelectedPlacedBall(const Stage& stage, size_t index) const
{
	if (m_ids.contains(SelectedID{ SelectType::PlacedBall, index })) return true;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			if (stage.m_groups.at(s.id).containsPlacedBallRecursive(index)) return true;
		}
	}
	return false;
}

Optional<Vec2> SelectedIDSet::getBeginPointOfSelectedObjects(const Stage& stage) const
{
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Point) {
			return stage.m_points.at(s.id);
		}
	}
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Group) {
			return stage.getBeginPointOfGroup(stage.m_groups.at(s.id));
		}
	}
	for (const auto& s : m_ids) {
		if (s.type == SelectType::StartCircle) {
			return stage.m_startCircles[s.id].circle.center;
		}
	}
	for (const auto& s : m_ids) {
		if (s.type == SelectType::GoalArea) {
			return stage.m_goalAreas[s.id].rect.pos;
		}
	}
	for (const auto& s : m_ids) {
		if (s.type == SelectType::PlacedBall) {
			return stage.m_placedBalls[s.id].circle.center;
		}
	}
	return none;
}

Vec2 SelectedIDSet::getBottomRightOfSelectedObjects(const Stage& stage) const
{
	double maxX = -Inf<double>;
	double maxY = -Inf<double>;
	
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Point) {
			const Vec2& pos = stage.m_points.at(s.id);
			maxX = Max(maxX, pos.x);
			maxY = Max(maxY, pos.y);
		}
		else if (s.type == SelectType::Group) {
			const auto& group = stage.m_groups.at(s.id);
			auto allPointIds = group.getAllPointIds();
			for (auto pointId : allPointIds) {
				const Vec2& pos = stage.m_points.at(pointId);
				maxX = Max(maxX, pos.x);
				maxY = Max(maxY, pos.y);
			}
			auto allPlacedBallIds = group.getAllPlacedBallIds();
			for (auto ballId : allPlacedBallIds) {
				const auto& ball = stage.m_placedBalls[ballId];
				maxX = Max(maxX, ball.circle.center.x + ball.circle.r);
				maxY = Max(maxY, ball.circle.center.y + ball.circle.r);
			}
			auto allStartCircleIds = group.getAllStartCircleIds();
			for (auto scId : allStartCircleIds) {
				const auto& sc = stage.m_startCircles[scId];
				maxX = Max(maxX, sc.circle.center.x + sc.circle.r);
				maxY = Max(maxY, sc.circle.center.y + sc.circle.r);
			}
			auto allGoalAreaIds = group.getAllGoalAreaIds();
			for (auto gaId : allGoalAreaIds) {
				const auto& ga = stage.m_goalAreas[gaId];
				maxX = Max(maxX, ga.rect.br().x);
				maxY = Max(maxY, ga.rect.br().y);
			}
		}
		else if (s.type == SelectType::StartCircle) {
			const auto& sc = stage.m_startCircles[s.id];
			maxX = Max(maxX, sc.circle.center.x + sc.circle.r);
			maxY = Max(maxY, sc.circle.center.y + sc.circle.r);
		}
		else if (s.type == SelectType::GoalArea) {
			const auto& ga = stage.m_goalAreas[s.id];
			maxX = Max(maxX, ga.rect.br().x);
			maxY = Max(maxY, ga.rect.br().y);
		}
		else if (s.type == SelectType::PlacedBall) {
			const auto& ball = stage.m_placedBalls[s.id];
			maxX = Max(maxX, ball.circle.center.x + ball.circle.r);
			maxY = Max(maxY, ball.circle.center.y + ball.circle.r);
		}
	}
	
	if (maxX > -Inf<double> && maxY > -Inf<double>) {
		return Vec2{ maxX, maxY };
	}
	
	return Vec2{ 0, 0 };
}

bool SelectedIDSet::isMovedSelectedNotInNonEditableArea(const Stage& stage, const Vec2& delta) const {
	// No restriction
	if (stage.nonEditableAreas().empty()) {
		return true;
	}

	// Collect point ids that will move (selected points + points in selected groups)
	HashSet<size_t> movedPointIds;
	for (const auto& s : m_ids) {
		if (s.type == SelectType::Point) {
			movedPointIds.insert(s.id);
		}
		else if (s.type == SelectType::Group) {
			movedPointIds.merge(stage.m_groups.at(s.id).getAllPointIds());
		}
	}

	// Moved point positions must stay editable
	for (auto pid : movedPointIds) {
		const Vec2 newPos = stage.m_points.at(pid) + delta;
		if (stage.isPointInNonEditableArea(newPos)) {
			return false;
		}
	}

	// Any edge incident to a moved point must remain allowed
	if (!movedPointIds.empty()) {
		for (const auto& e : stage.m_edges) {
			if (e.isLocked) continue;

			const bool end0Moved = movedPointIds.contains(e[0]);
			const bool end1Moved = movedPointIds.contains(e[1]);
			if (!(end0Moved || end1Moved)) continue;

			const Vec2 p0 = stage.m_points.at(e[0]) + (end0Moved ? delta : Vec2{ 0, 0 });
			const Vec2 p1 = stage.m_points.at(e[1]) + (end1Moved ? delta : Vec2{ 0, 0 });
			if (!stage.isLineAllowedInEditableArea(Line{ p0, p1 })) {
				return false;
			}
		}
	}

	// Other selected objects must not move their anchors into forbidden area
	for (const auto& s : m_ids) {
		if (s.type == SelectType::StartCircle) {
			const Vec2 newCenter = stage.m_startCircles[s.id].circle.center + delta;
			if (stage.isPointInNonEditableArea(newCenter)) return false;
		}
		else if (s.type == SelectType::GoalArea) {
			const Vec2 newPos = stage.m_goalAreas[s.id].rect.pos + delta;
			if (stage.isPointInNonEditableArea(newPos)) return false;
		}
		else if (s.type == SelectType::PlacedBall) {
			const Vec2 newCenter = stage.m_placedBalls[s.id].circle.center + delta;
			if (stage.isPointInNonEditableArea(newCenter)) return false;
		}
	}

	// Group members for non-point objects
	for (const auto& s : m_ids) {
		if (s.type != SelectType::Group) continue;
		const auto& g = stage.m_groups.at(s.id);

		for (auto sid : g.getAllStartCircleIds()) {
			const Vec2 newCenter = stage.m_startCircles[sid].circle.center + delta;
			if (stage.isPointInNonEditableArea(newCenter)) return false;
		}
		for (auto gid : g.getAllGoalAreaIds()) {
			const Vec2 newPos = stage.m_goalAreas[gid].rect.pos + delta;
			if (stage.isPointInNonEditableArea(newPos)) return false;
		}
		for (auto bid : g.getAllPlacedBallIds()) {
			const Vec2 newCenter = stage.m_placedBalls[bid].circle.center + delta;
			if (stage.isPointInNonEditableArea(newCenter)) return false;
		}
	}

	return true;
}

void SelectedIDSet::moveSelectedObjects(Stage& stage, const Vec2& delta) const
{
	for (const auto& s : m_ids) {
		switch (s.type) {
		case SelectType::Point:
			stage.m_points.at(s.id) += delta;
			break;
		case SelectType::Group: {
			auto& group = stage.m_groups.at(s.id);
			auto pointIds = group.getAllPointIds();
			for (auto pid : pointIds) {
				stage.m_points.at(pid) += delta;
			}
			auto startCircleIds = group.getAllStartCircleIds();
			for (auto cid : startCircleIds) {
				stage.m_startCircles[cid].circle.center += delta;
			}
			auto goalAreaIds = group.getAllGoalAreaIds();
			for (auto gid : goalAreaIds) {
				stage.m_goalAreas[gid].rect.pos += delta;
			}
			auto placedBallIds = group.getAllPlacedBallIds();
			for (auto bid : placedBallIds) {
				stage.m_placedBalls[bid].circle.center += delta;
			}
			break;
		}
		case SelectType::StartCircle:
			stage.m_startCircles[s.id].circle.center += delta;
			break;
		case SelectType::GoalArea:
			stage.m_goalAreas[s.id].rect.pos += delta;
			break;
		case SelectType::PlacedBall:
			stage.m_placedBalls[s.id].circle.center += delta;
			break;
		}
	}
}

void SelectedIDSet::selectAllObjects(Stage& stage)
{
	m_ids.clear();

	// ロックされている対象は Ctrl+A の対象外
	HashSet<size_t> lockedPointIds;
	for (const auto& e : stage.m_edges) {
		if (e.isLocked) {
			lockedPointIds.insert(e[0]);
			lockedPointIds.insert(e[1]);
		}
	}

	for (const auto& [pointId, point] : stage.m_points) {
		if (!lockedPointIds.contains(pointId)) {
			m_ids.insert(SelectedID{ SelectType::Point, pointId });
		}
	}
	for (const auto& [groupId, group] : stage.m_groups) {
		if (!group.isLocked) {
			m_ids.insert(SelectedID{ SelectType::Group, groupId });
		}
	}
	for (size_t i = 0; i < stage.m_startCircles.size(); ++i) {
		if (!stage.m_startCircles[i].isLocked) {
			m_ids.insert(SelectedID{ SelectType::StartCircle, i });
		}
	}
	for (size_t i = 0; i < stage.m_goalAreas.size(); ++i) {
		if (!stage.m_goalAreas[i].isLocked) {
			m_ids.insert(SelectedID{ SelectType::GoalArea, i });
		}
	}
	for (size_t i = 0; i < stage.m_placedBalls.size(); ++i) {
		m_ids.insert(SelectedID{ SelectType::PlacedBall, i });
	}
}

void SelectedIDSet::selectObjectsByPoints(Stage& stage, const HashSet<size_t>& pointIds)
{
	HashSet<size_t> group_candidates;
	for (auto pointId : pointIds) {
		if (auto topGroupId = stage.findTopGroup(pointId)) group_candidates.insert(*topGroupId);
		else m_ids.insert(SelectedID{ SelectType::Point, pointId });
	}
	for (const auto& groupId : group_candidates) {
		const auto& group = stage.m_groups.at(groupId);
		bool allMembersSelected = true;
		auto allPointIdsInGroup = group.getAllPointIds();
		for (auto& memberId : allPointIdsInGroup) {
			if (not pointIds.contains(memberId)) { allMembersSelected = false; break; }
		}
		if (allMembersSelected) m_ids.insert(SelectedID{ SelectType::Group, groupId });
	}
}

void SelectedIDSet::selectPlacedBallsByIds(Stage& stage, const HashSet<size_t>& ballIds)
{
	HashSet<size_t> group_candidates;
	for (auto ballId : ballIds) {
		if (auto topGroupId = stage.findTopGroupForPlacedBall(ballId)) group_candidates.insert(*topGroupId);
		else m_ids.insert(SelectedID{ SelectType::PlacedBall, ballId });
	}
	for (const auto& groupId : group_candidates) {
		const auto& group = stage.m_groups.at(groupId);
		bool allMembersSelected = true;
		auto allBallIdsInGroup = group.getAllPlacedBallIds();
		for (auto& memberId : allBallIdsInGroup) {
			if (not ballIds.contains(memberId)) { allMembersSelected = false; break; }
		}
		if (allMembersSelected) m_ids.insert(SelectedID{ SelectType::Group, groupId });
	}
}

void SelectedIDSet::selectObjectsInArea(Stage& stage, const RectF& area)
{
	Array<size_t> areaPointIds;
	Array<size_t> areaStartCircleIds;
	Array<size_t> areaGoalAreaIds;
	Array<size_t> areaPlacedBallIds;
	HashSet<size_t> group_candidates;

	for (const auto& edge : stage.m_edges) {
		if (edge.isLocked) continue;
		for (auto pid : edge.ids) {
			if (area.intersects(stage.m_points.at(pid))) {
				if (auto topGroupId = stage.findTopGroup(pid)) group_candidates.insert(*topGroupId);
				else areaPointIds.push_back(pid);
			}
		}
	}
	for (size_t i = 0; i < stage.m_startCircles.size(); ++i) {
		if (stage.m_startCircles[i].isLocked) continue;
		if (area.intersects(stage.m_startCircles[i].circle.center)) {
			// グループに属している場合はグループ候補に追加
			if (auto topGroupId = stage.findTopGroupForStartCircle(i)) {
				group_candidates.insert(*topGroupId);
			}
			else {
				areaStartCircleIds.push_back(i);
			}
		}
	}
	for (size_t i = 0; i < stage.m_goalAreas.size(); ++i) {
		if (stage.m_goalAreas[i].isLocked) continue;
		if (area.intersects(stage.m_goalAreas[i].rect.center())) {
			// グループに属している場合はグループ候補に追加
			if (auto topGroupId = stage.findTopGroupForGoalArea(i)) {
				group_candidates.insert(*topGroupId);
			}
			else {
				areaGoalAreaIds.push_back(i);
			}
		}
	}
	for (size_t i = 0; i < stage.m_placedBalls.size(); ++i) {
		if (area.intersects(stage.m_placedBalls[i].circle.center)) {
			// グループに属している場合はグループ候補に追加
			if (auto topGroupId = stage.findTopGroupForPlacedBall(i)) {
				group_candidates.insert(*topGroupId);
			}
			else {
				areaPlacedBallIds.push_back(i);
			}
		}
	}
	Array<size_t> areaGroupIds;
	for (const auto& groupId : group_candidates) {
		const auto& group = stage.m_groups.at(groupId);
		bool allMembersInArea = true;
		
		// ポイントのチェック
		auto allPointIdsInGroup = group.getAllPointIds();
		for (auto& memberId : allPointIdsInGroup) {
			if (not area.contains(stage.m_points.at(memberId))) { allMembersInArea = false; break; }
		}
		
		// StartCircle のチェック
		if (allMembersInArea) {
			auto allStartCircleIdsInGroup = group.getAllStartCircleIds();
			for (auto& memberId : allStartCircleIdsInGroup) {
				if (not area.contains(stage.m_startCircles[memberId].circle.center)) { allMembersInArea = false; break; }
			}
		}
		
		// GoalArea のチェック
		if (allMembersInArea) {
			auto allGoalAreaIdsInGroup = group.getAllGoalAreaIds();
			for (auto& memberId : allGoalAreaIdsInGroup) {
				if (not area.contains(stage.m_goalAreas[memberId].rect.center())) { allMembersInArea = false; break; }
			}
		}
		
		// PlacedBall のチェック
		if (allMembersInArea) {
			auto allPlacedBallIdsInGroup = group.getAllPlacedBallIds();
			for (auto& memberId : allPlacedBallIdsInGroup) {
				if (not area.contains(stage.m_placedBalls[memberId].circle.center)) { allMembersInArea = false; break; }
			}
		}
		
		if (allMembersInArea) areaGroupIds.push_back(groupId);
	}
	if (KeyShift.pressed()) {
		bool allSelected = true;
		for (auto& id : areaPointIds) allSelected &= m_ids.contains(SelectedID{ SelectType::Point, id });
		for (auto& id : areaGroupIds) allSelected &= m_ids.contains(SelectedID{ SelectType::Group, id });
		for (auto& id : areaStartCircleIds) allSelected &= m_ids.contains(SelectedID{ SelectType::StartCircle, id });
		for (auto& id : areaGoalAreaIds) allSelected &= m_ids.contains(SelectedID{ SelectType::GoalArea, id });
		for (auto& id : areaPlacedBallIds) allSelected &= m_ids.contains(SelectedID{ SelectType::PlacedBall, id });
		if (allSelected) {
			for (auto& id : areaPointIds) m_ids.erase(SelectedID{ SelectType::Point, id });
			for (auto& id : areaGroupIds) m_ids.erase(SelectedID{ SelectType::Group, id });
			for (auto& id : areaStartCircleIds) m_ids.erase(SelectedID{ SelectType::StartCircle, id });
			for (auto& id : areaGoalAreaIds) m_ids.erase(SelectedID{ SelectType::GoalArea, id });
			for (auto& id : areaPlacedBallIds) m_ids.erase(SelectedID{ SelectType::PlacedBall, id });
		}
		else {
			for (auto& id : areaPointIds) m_ids.insert(SelectedID{ SelectType::Point, id });
			for (auto& id : areaGroupIds) m_ids.insert(SelectedID{ SelectType::Group, id });
			for (auto& id : areaStartCircleIds) m_ids.insert(SelectedID{ SelectType::StartCircle, id });
			for (auto& id : areaGoalAreaIds) m_ids.insert(SelectedID{ SelectType::GoalArea, id });
			for (auto& id : areaPlacedBallIds) m_ids.insert(SelectedID{ SelectType::PlacedBall, id });
		}
	}
	else {
		m_ids.clear();
		for (auto& id : areaPointIds) m_ids.insert(SelectedID{ SelectType::Point, id });
		for (auto& id : areaGroupIds) m_ids.insert(SelectedID{ SelectType::Group, id });
		for (auto& id : areaStartCircleIds) m_ids.insert(SelectedID{ SelectType::StartCircle, id });
		for (auto& id : areaGoalAreaIds) m_ids.insert(SelectedID{ SelectType::GoalArea, id });
		for (auto& id : areaPlacedBallIds) m_ids.insert(SelectedID{ SelectType::PlacedBall, id });
	}
}
