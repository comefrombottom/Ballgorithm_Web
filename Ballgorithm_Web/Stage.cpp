# include "Stage.hpp"
# include "Query.hpp"
# include "GeometryUtils.hpp"
# include "IndexedDB.hpp"

Stage::Stage()
{
}

int32 Stage::addLine(const Line& line, bool isLocked)
{
	int32 id1 = m_nextPointId++;
	int32 id2 = m_nextPointId++;
	m_points[id1] = line.begin;
	m_points[id2] = line.end;
	int32 edgeIndex = m_edges.size();
	m_edges.push_back(Edge{ { id1, id2 }, isLocked });
	m_layerOrder.push_back(LayerObject{ LayerObjectType::Edge, edgeIndex });
	return edgeIndex;
}

int32 Stage::addStartCircle(const StartCircle& startCircle)
{
	int32 index = m_startCircles.size();
	m_startCircles.push_back(startCircle);
	m_layerOrder.push_back(LayerObject{ LayerObjectType::StartCircle, index });
	return index;
}

int32 Stage::addGoalArea(const GoalArea& goalArea)
{
	int32 index = m_goalAreas.size();
	m_goalAreas.push_back(goalArea);
	m_layerOrder.push_back(LayerObject{ LayerObjectType::GoalArea, index });
	return index;
}

int32 Stage::addPlacedBall(const PlacedBall& placedBall)
{
	int32 index = m_placedBalls.size();
	m_placedBalls.push_back(placedBall);
	m_layerOrder.push_back(LayerObject{ LayerObjectType::PlacedBall, index });
	return index;
}

void Stage::bringToFront(LayerObject obj)
{
	// 既存の位置から削除して末尾に追加
	m_layerOrder.remove(obj);
	m_layerOrder.push_back(obj);
}

void Stage::removeFromLayerOrder(LayerObject obj)
{
	m_layerOrder.remove(obj);
}

void Stage::updateLayerOrderAfterRemoval(LayerObjectType type, int32 removedId)
{
	// 削除されたオブジェクトをレイヤー順序から除去
	m_layerOrder.remove(LayerObject{ type, removedId });
	
	// 削除されたIDより大きいIDを持つ同じタイプのオブジェクトのIDを1減らす
	for (auto& obj : m_layerOrder) {
		if (obj.type == type && obj.id > removedId) {
			--obj.id;
		}
	}
}

void Stage::removePlacedBall(int32 index)
{
	if (index >= m_placedBalls.size()) return;
	m_placedBalls.remove_at(index);
	updateLayerOrderAfterRemoval(LayerObjectType::PlacedBall, index);
}

void Stage::createGroup(const Group& group)
{
	if (group.size() >= 2) {
		int32 newGroupId = m_nextGroupId++;
		m_groups[newGroupId] = group;
	}
}

int32 Stage::createLockedGroup(const Group& group)
{
	int32 newGroupId = m_nextGroupId++;
	Group lockedGroup = group;
	lockedGroup.isLocked = true;
	m_groups[newGroupId] = lockedGroup;
	return newGroupId;
}

void Stage::createGroupFromSelection(HashSet<SelectedID>& selectedIDs)
{
	Group newGroup;
	HashSet<int32> addPointIds;
	HashSet<int32> addPlacedBallIds;
	HashSet<int32> addStartCircleIds;
	HashSet<int32> addGoalAreaIds;
	
	for (const auto& s : selectedIDs) {
		if (s.type == SelectType::Point) {
			addPointIds.insert(s.id);
		}
		else if (s.type == SelectType::PlacedBall) {
			addPlacedBallIds.insert(s.id);
		}
		else if (s.type == SelectType::StartCircle) {
			addStartCircleIds.insert(s.id);
		}
		else if (s.type == SelectType::GoalArea) {
			addGoalAreaIds.insert(s.id);
		}
		else if (s.type == SelectType::Group) {
			newGroup.insert(m_groups.at(s.id));
			m_groups.erase(s.id);
		}
	}
	
	// Point を追加
	auto& mut_points = newGroup.m_pointIds;
	for (auto& edge : m_edges) {
		if (addPointIds.contains(edge[0]) and addPointIds.contains(edge[1])) {
			mut_points.insert(edge[0]);
			mut_points.insert(edge[1]);
		}
	}
	
	// PlacedBall を追加
	auto& mut_placedBalls = newGroup.m_placedBallIds;
	for (auto ballId : addPlacedBallIds) {
		mut_placedBalls.insert(ballId);
	}
	
	// StartCircle を追加
	auto& mut_startCircles = newGroup.m_startCircleIds;
	for (auto startCircleId : addStartCircleIds) {
		mut_startCircles.insert(startCircleId);
	}
	
	// GoalArea を追加
	auto& mut_goalAreas = newGroup.m_goalAreaIds;
	for (auto goalAreaId : addGoalAreaIds) {
		mut_goalAreas.insert(goalAreaId);
	}
	
	createGroup(newGroup);
	selectedIDs.clear();
}

void Stage::ungroup(int32 groupId)
{
	const auto& group = m_groups.at(groupId);
	for (const auto& g : group.m_groups) {
		int32 newGroupId = m_nextGroupId++;
		m_groups[newGroupId] = g;
	}
	m_groups.erase(groupId);
}

Vec2 Stage::getBeginPointOfGroup(const Group& group) const
{
	if (not group.m_pointIds.empty()) {
		return m_points.at(group.getBeginPointId());
	}
	else if (not group.m_placedBallIds.empty()) {
		return m_placedBalls[*group.m_placedBallIds.begin()].center;
	}
	else if (not group.m_startCircleIds.empty()) {
		return m_startCircles[*group.m_startCircleIds.begin()].circle.center;
	}
	else if (not group.m_goalAreaIds.empty()) {
		return m_goalAreas[*group.m_goalAreaIds.begin()].rect.pos;
	}
	else if (not group.m_groups.empty()) {
		return getBeginPointOfGroup(*group.m_groups.begin());
	}
	return Vec2(0, 0);
}

Optional<int32> Stage::findTopGroup(int32 pointId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsPointRecursive(pointId)) {
			return groupId;
		}
	}
	return none;
}

Optional<int32> Stage::findTopGroupForPlacedBall(int32 placedBallId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsPlacedBallRecursive(placedBallId)) {
			return groupId;
		}
	}
	return none;
}

Optional<int32> Stage::findTopGroupForStartCircle(int32 startCircleId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsStartCircleRecursive(startCircleId)) {
			return groupId;
		}
	}
	return none;
}

Optional<int32> Stage::findTopGroupForGoalArea(int32 goalAreaId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsGoalAreaRecursive(goalAreaId)) {
			return groupId;
		}
	}
	return none;
}

Group Stage::mapGroupIDs(const Group& group, const HashTable<int32, int32>& idMapping) const
{
	Group newGroup;
	for (auto pointId : group.m_pointIds) {
		newGroup.m_pointIds.insert(idMapping.at(pointId));
	}
	for (const auto& subGroup : group.m_groups) {
		newGroup.insert(mapGroupIDs(subGroup, idMapping));
	}
	return newGroup;
}

PointEdgeGroup Stage::copySelectedObjects(const HashSet<SelectedID>& selectedIDs) const
{
	PointEdgeGroup result;
	HashSet<int32> allSelectedPointIds;
	for (const auto& s : selectedIDs) {
		if (s.type == SelectType::Point) allSelectedPointIds.insert(s.id);
		else if (s.type == SelectType::Group) allSelectedPointIds.merge(m_groups.at(s.id).getAllPointIds());
	}
	for (const auto& edge : m_edges) {
		if (allSelectedPointIds.contains(edge[0]) or allSelectedPointIds.contains(edge[1])) {
			result.m_points[edge[0]] = m_points.at(edge[0]);
			result.m_points[edge[1]] = m_points.at(edge[1]);
			result.m_edges.push_back(edge.ids);
		}
	}
	for (const auto& s : selectedIDs) {
		if (s.type == SelectType::Group) {
			const auto& group = m_groups.at(s.id);
			result.m_groups.push_back(group);
		}
		else if (s.type == SelectType::PlacedBall) {
			result.m_placedBalls.push_back(m_placedBalls[s.id]);
		}
	}
	return result;
}

void Stage::deltaMoveGroup(const Group& group, const Vec2& deltaMove)
{
	auto allPointIds = group.getAllPointIds();
	for (auto pointId : allPointIds) {
		m_points[pointId] += deltaMove;
	}
	
	auto allPlacedBallIds = group.getAllPlacedBallIds();
	for (auto ballId : allPlacedBallIds) {
		m_placedBalls[ballId].center += deltaMove;
	}
	
	auto allStartCircleIds = group.getAllStartCircleIds();
	for (auto startCircleId : allStartCircleIds) {
		m_startCircles[startCircleId].circle.center += deltaMove;
	}
	
	auto allGoalAreaIds = group.getAllGoalAreaIds();
	for (auto goalAreaId : allGoalAreaIds) {
		m_goalAreas[goalAreaId].rect.pos += deltaMove;
	}
}

void Stage::eraseSelectedPoints(const HashSet<SelectedID>& selectedIDs)
{
	HashSet<int32> pointsToErase;
	Array<int32> placedBallsToErase;
	
	for (const auto& s : selectedIDs) {
		if (s.type == SelectType::Point) pointsToErase.insert(s.id);
		else if (s.type == SelectType::Group) {
			auto groupPointIds = m_groups.at(s.id).getAllPointIds();
			for (auto pid : groupPointIds) pointsToErase.insert(pid);
			
			auto groupBallIds = m_groups.at(s.id).getAllPlacedBallIds();
			for (auto bid : groupBallIds) placedBallsToErase.push_back(bid);
			
			m_groups.erase(s.id);
		}
		else if (s.type == SelectType::PlacedBall) {
			placedBallsToErase.push_back(s.id);
		}
	}
	
	// PlacedBall を削除（インデックスが大きい順に削除）
	std::sort(placedBallsToErase.begin(), placedBallsToErase.end(), std::greater<int32>());
	for (auto idx : placedBallsToErase) {
		m_placedBalls.remove_at(idx);
		updateLayerOrderAfterRemoval(LayerObjectType::PlacedBall, idx);
	}
	
	Array<int32> edgesToErase;
	for (int32 i = 0; i < m_edges.size(); ++i) {
		const auto& edge = m_edges[i];
		if (pointsToErase.contains(edge[0]) or pointsToErase.contains(edge[1])) {
			edgesToErase.push_back(i);
			pointsToErase.insert(edge[0]);
			pointsToErase.insert(edge[1]);
		}
	}
	// Edge も大きい順に削除してレイヤー順序を更新
	std::sort(edgesToErase.begin(), edgesToErase.end(), std::greater<int32>());
	for (auto idx : edgesToErase) {
		m_edges.remove_at(idx);
		updateLayerOrderAfterRemoval(LayerObjectType::Edge, idx);
	}
	
	for (auto pointId : pointsToErase) {
		m_points.erase(pointId);
	}
}

void Stage::startSimulation()
{
	m_isSimulationRunning = true;
	m_world = P2World{ 980 };
	m_linesInWorld.clear();
	m_startBallsInWorld.clear();

	auto saveTask = saveAsync();

	if (isQueriesInitialState()) {


		m_initialLines.clear();
		m_initialBalls.clear();


		// 壁（エッジ）を物理世界に追加
		for (const auto& edge : m_edges) {
			const Vec2& p1 = m_points.at(edge[0]);
			const Vec2& p2 = m_points.at(edge[1]);
			Line line(p1, p2);
			if (edge.isLocked or isLineAllowedInEditableArea(line)) {
				m_initialLines.push_back(line);

				//auto lineBody = createLine(m_world, P2Static, line);
				//m_linesInWorld.push_back(lineBody);
			}
		}

		// プレイヤーが配置したボールを物理世界に追加
		// 種類ごとに maxCount を超えないようにカウント
		HashTable<BallKind, int32> kindCounts;
		for (const auto& placedBall : m_placedBalls) {
			if (placedBall.isLocked) {
				m_initialBalls.push_back(placedBall);
				//P2Body ballBody = createCircle(m_world, P2Dynamic, Circle(placedBall.center, GetBallRadius(placedBall.kind)));
				//m_startBallsInWorld.push_back(Ball{ ballBody, placedBall.kind });
				continue;
			}
			if (isPointInNonEditableArea(placedBall.center)) continue;

			const BallKind kind = placedBall.kind;
			bool withinLimit = true;
			for (const auto& slot : m_inventorySlots) {
				if (slot.kind == InventoryObjectKind::Ball && slot.ballKind == kind && slot.maxCount) {
					withinLimit = (kindCounts[kind] < *slot.maxCount);
					break;
				}
			}
			if (not withinLimit) continue;

			++kindCounts[kind];
			m_initialBalls.push_back(placedBall);
			// P2Body ballBody = createCircle(m_world, P2Dynamic, Circle(placedBall.center, GetBallRadius(placedBall.kind)));
			// m_startBallsInWorld.push_back(Ball{ ballBody, placedBall.kind });
		}

	}

	for (const auto& line : m_initialLines) {
		auto lineBody = createLine(m_world, P2Static, line);
		m_linesInWorld.push_back(lineBody);
	}

	for (const auto& circle : m_initialBalls) {
		P2Body ballBody = createCircle(m_world, P2Dynamic, Circle(circle.center, GetBallRadius(circle.kind)));
		m_startBallsInWorld.push_back(Ball{ ballBody, circle.kind });
	}
	
	// クエリ固有のシミュレーション開始処理
	if (m_currentQueryIndex < m_queries->size()) {
		(*m_queries)[m_currentQueryIndex]->startSimulation(*this);
	}

#if SIV3D_PLATFORM(WEB)
	s3d::Platform::Web::System::AwaitAsyncTask(saveTask);
#endif
}

bool Stage::checkSimulationResult() const
{
	if (m_currentQueryIndex < m_queries->size()) {
		return (*m_queries)[m_currentQueryIndex]->checkSimulationResult(*this);
	}
	return false;
}

void Stage::endSimulation()
{
	m_startBallsInWorld.clear();
	m_linesInWorld.clear();
	m_world = P2World{};
	m_simulationTimeAccumlate = 0.0;
	m_isSimulationRunning = false;
	m_isSimulationPaused = false;
	// m_simulationSpeed = 1.0;  // 速度をリセットしない
}

double Stage::getLowestY() const
{
	double lowestY = -Inf<double>;
	
	// Points (Edges の頂点)
	for (const auto& [id, pos] : m_points) {
		lowestY = Max(lowestY, pos.y);
	}
	
	// StartCircles
	for (const auto& c : m_startCircles) {
		lowestY = Max(lowestY, c.circle.center.y + c.circle.r);
	}
	
	// GoalAreas
	for (const auto& g : m_goalAreas) {
		lowestY = Max(lowestY, g.rect.br().y);
	}
	
	// PlacedBalls
	for (const auto& b : m_placedBalls) {
		lowestY = Max(lowestY, b.center.y + GetBallRadius(b.kind));
	}
	
	return lowestY;
}

void Stage::addInventorySlot(BallKind ballKind, Optional<int32> maxCount)
{
	m_inventorySlots.push_back(InventorySlot::CreateBallSlot(ballKind, maxCount));
}

bool Stage::canPlaceFromSlot(int32 index) const
{
	if (index >= m_inventorySlots.size()) return false;
	return m_inventorySlots[index].hasRemaining();
}

void Stage::useFromSlot(int32 slotIndex)
{
	if (slotIndex >= m_inventorySlots.size()) return;
	auto& slot = m_inventorySlots[slotIndex];
	if (slot.maxCount)
	{
		++slot.usedCount;
	}
}

void Stage::returnToInventory(BallKind ballKind)
{
	for (auto& slot : m_inventorySlots)
	{
		if (slot.kind == InventoryObjectKind::Ball && slot.ballKind == ballKind && slot.maxCount)
		{
			if (slot.usedCount > 0)
			{
				--slot.usedCount;
			}
			break;
		}
	}
}

void Stage::resetQueryProgress()
{
	m_queryCompleted.assign(m_queries->size(), false);
	m_queryFailed.assign(m_queries->size(), false);
	m_currentQueryIndex = 0;
	// m_isCleared = false;
}

void Stage::markQueryCompleted(int32 queryIndex)
{
	if (queryIndex < m_queryCompleted.size()) {
		m_queryCompleted[queryIndex] = true;
		m_queryFailed[queryIndex] = false;
	}
	
	// 全クエリ達成チェック
	if (isAllQueriesCompleted()) {
		m_isCleared = true;
	}
}

void Stage::markQueryFailed(int32 queryIndex)
{
	if (queryIndex < m_queryFailed.size()) {
		m_queryFailed[queryIndex] = true;
	}
}

bool Stage::isAllQueriesCompleted() const
{
	if (m_queries->empty()) return false;
	for (bool completed : m_queryCompleted) {
		if (!completed) return false;
	}
	return true;
}

bool Stage::isQueriesInitialState() const
{
	for (bool completed : m_queryCompleted) {
		if (completed) return false;
	}
	for (bool failed : m_queryFailed) {
		if (failed) return false;
	}
	return true;
}

StageSnapshot Stage::createSnapshot() const
{
	return StageSnapshot{
		.points = m_points,
		.nextPointId = m_nextPointId,
		.edges = m_edges,
		.groups = m_groups,
		.nextGroupId = m_nextGroupId,
		.placedBalls = m_placedBalls,
		.inventorySlots = m_inventorySlots,
		.layerOrder = m_layerOrder,
		.nonEditableAreas = m_nonEditableAreas
	};
}

void Stage::restoreSnapshot(const StageSnapshot& snapshot)
{
	m_points = snapshot.points;
	m_nextPointId = snapshot.nextPointId;
	m_edges = snapshot.edges;
	m_groups = snapshot.groups;
	m_nextGroupId = snapshot.nextGroupId;
	m_placedBalls = snapshot.placedBalls;
	m_inventorySlots = snapshot.inventorySlots;
	m_layerOrder = snapshot.layerOrder;
	m_nonEditableAreas = snapshot.nonEditableAreas;
}

PointEdgeGroup Stage::getAllSelectableObjectsAsPointEdgeGroup() const
{
	SelectedIDSet allSelectedIDs;
	
	allSelectedIDs.selectAllObjects(*this);

	return copySelectedObjects(allSelectedIDs.m_ids);
}

void Stage::removeAllSelectableObjects()
{
	// すべてのオブジェクトを選択状態にしてから削除
	SelectedIDSet allSelectedIDs;
	allSelectedIDs.selectAllObjects(*this);
	eraseSelectedPoints(allSelectedIDs.m_ids);
}

void Stage::pastePointEdgeGroup(const PointEdgeGroup& m_clipboard, SelectedIDSet& selectedIDs)
{
	HashSet<int32> newSelectedPointIds;
	HashTable<int32, int32> pointIdMapping;
	HashSet<int32> usedNewPointIds;
	HashSet<int32> usedOldPointIds;
	for (const auto& [oldPointId, pos] : m_clipboard.m_points) {
		int32 newPointId = m_nextPointId++;
		pointIdMapping[oldPointId] = newPointId;
		m_points[newPointId] = pos;
		newSelectedPointIds.insert(newPointId);
	}
	for (const auto& edge : m_clipboard.m_edges) {
		const Vec2 p1 = m_points.at(pointIdMapping.at(edge[0]));
		const Vec2 p2 = m_points.at(pointIdMapping.at(edge[1]));
		Line newLine{ p1, p2 };
		if (!isLineAllowedInEditableArea(newLine)) {
			continue;
		}
		Edge newEdge = { { pointIdMapping.at(edge[0]), pointIdMapping.at(edge[1]) } };
		int32 edgeIndex = m_edges.size();
		m_edges.push_back(newEdge);
		m_layerOrder.push_back(LayerObject{ LayerObjectType::Edge, edgeIndex });
		usedNewPointIds.insert(pointIdMapping.at(edge[0]));
		usedNewPointIds.insert(pointIdMapping.at(edge[1]));
		usedOldPointIds.insert(edge[0]);
		usedOldPointIds.insert(edge[1]);
	}
	// エッジに使われなかったポイントは反映しない（編集不可エリアで弾かれて孤立するケース対策）
	for (const auto& [oldPointId, newPointId] : pointIdMapping) {
		if (!usedNewPointIds.contains(newPointId)) {
			m_points.erase(newPointId);
			newSelectedPointIds.erase(newPointId);
		}
	}
	for (const auto& group : m_clipboard.m_groups) {
		// エッジ制限で消えたポイントを含むグループはスキップ
		bool ok = true;
		for (auto oldPid : group.getAllPointIds()) {
			if (!usedOldPointIds.contains(oldPid)) {
				ok = false;
				break;
			}
		}
		if (ok) {
			createGroup(mapGroupIDs(group, pointIdMapping));
		}
	}

	auto& sel = selectedIDs;
	sel.clear();
	for (const auto& placedBall : m_clipboard.m_placedBalls) {
		// インベントリに残数があるかチェック
		BallKind ballKind = placedBall.kind;
		bool canPlace = false;

		// 対応するスロットを探して残数確認
		for (int32 i = 0; i < inventorySlots().size(); ++i) {
			const auto& slot = inventorySlots()[i];
			if (slot.kind == InventoryObjectKind::Ball && slot.ballKind == ballKind && canPlaceFromSlot(i)) {
				// 配置可能なのでインベントリから使用
				useFromSlot(i);
				canPlace = true;
				break;
			}
		}

		// 配置可能な場合のみステージに追加
		if (canPlace) {
			addPlacedBall(placedBall);
			sel.insert(SelectedID{ SelectType::PlacedBall, static_cast<int32>(m_placedBalls.size()) - 1 });
		}
	}

	sel.selectObjectsByPoints(*this, newSelectedPointIds);
}

bool Stage::isPointInNonEditableArea(const Vec2& pos) const
{
	for (const auto& r : m_nonEditableAreas) {
		if (r.contains(pos)) {
			return true;
		}
	}
	return false;
}

bool Stage::isLineAllowedInEditableArea(const Line& line) const
{
	if (m_nonEditableAreas.empty()) {
		return true;
	}

	if (isPointInNonEditableArea(line.begin) || isPointInNonEditableArea(line.end)) {
		return false;
	}

	for (const auto& rect : m_nonEditableAreas) {
		if (rect.intersects(line)) {
			return false;
		}
	}

	return true;
}


void Stage::save(FilePath path) const
{
	auto task = saveAsync(path);
#if SIV3D_PLATFORM(WEB)
	s3d::Platform::Web::System::AwaitAsyncTask(task).value_or(false);
#endif
}

void Stage::load(FilePath path, bool restoreClearFlag)
{
	if (path.isEmpty()) {
		path = U"Ballgorithm/V2Stages/{}.bin"_fmt(m_name);
	}

	if (!FileSystem::Exists(path)) {
		return;
	}

	{
		Deserializer<BinaryReader> deserializer{ path };
		int32 version;
		deserializer(version);
		if (version == 2)
		{
			PointEdgeGroup peg;
			deserializer(peg);
			bool isCleared;
			deserializer(isCleared);
			if (restoreClearFlag) {
				m_isCleared = isCleared;
			}
			removeAllSelectableObjects();
			SelectedIDSet sid;
			pastePointEdgeGroup(peg, sid);
		}
	}
}

AsyncTask<bool> Stage::saveAsync(FilePath path) const
{
	if (path.isEmpty()) {
		path = U"Ballgorithm/V2Stages/{}.bin"_fmt(m_name);
	}

	{
		Serializer<BinaryWriter> serializer{ path };
		int32 version = 2;
		serializer(version);
		serializer(getAllSelectableObjectsAsPointEdgeGroup());
		serializer(m_isCleared);
	}

#if SIV3D_PLATFORM(WEB)
	return s3d::Platform::Web::IndexedDB::SaveAsync();
#else
	return AsyncTask<bool>([]() { return true; });
#endif
}

int32 Stage::CalculateNumberOfObjects() const
{
	return m_edges.count_if([](const Edge& edge) { return !edge.isLocked; }) + m_placedBalls.count_if([](const PlacedBall& ball) { return !ball.isLocked; });
}

int32 Stage::CalculateTotalLength() const
{
	double sum = 0;
	for (const auto& edge : m_edges) {
		if (!edge.isLocked)
		{
			const Vec2& p1 = m_points.at(edge[0]);
			const Vec2& p2 = m_points.at(edge[1]);
			sum += p1.distanceFrom(p2);
		}
	}
	return sum;
}

void Stage::restoreRecord(const StageRecord& record)
{
	m_name = record.m_stageName;
	m_points = record.m_points;
	m_edges = record.m_edges;
	m_groups = record.m_groups;
	m_startCircles = record.m_startCircles;
	m_goalAreas = record.m_goalAreas;
	m_placedBalls = record.m_placedBalls;
	m_nonEditableAreas = record.m_nonEditableAreas;
}

# include ".SECRET"

StageRecord::StageRecord(const Stage& stage, String author)
{
	m_stageName = stage.m_name;
	m_author = author;
	m_points = stage.m_points;
	m_edges = stage.m_edges;
	m_groups = stage.m_groups;
	m_startCircles = stage.m_startCircles;
	m_goalAreas = stage.m_goalAreas;
	m_placedBalls = stage.m_placedBalls;
	m_nonEditableAreas = stage.m_nonEditableAreas;
	m_inventorySlots = stage.m_inventorySlots;
	m_layerOrder = stage.m_layerOrder;
	m_numberOfObjects = stage.CalculateNumberOfObjects();
	m_totalLength = stage.CalculateTotalLength();
}

bool StageRecord::isValid() const
{
	return !m_stageName.isEmpty() && m_hash != MD5Value();
}

void StageRecord::intoBlobStr()
{
	Serializer<MemoryWriter> archive;
	int32 version = 2;
	archive(version);
	archive(m_points, m_edges, m_groups, m_startCircles, m_goalAreas, m_placedBalls, m_nonEditableAreas, m_inventorySlots, m_layerOrder);
	m_blobStr = archive->getBlob().base64Str();
}

void StageRecord::fromBlobStr()
{
	Deserializer<MemoryReader> archive{ Base64::Decode(m_blobStr) };
	int32 version;
	archive(version);
	if (version == 2)
	{
		archive(m_points, m_edges, m_groups, m_startCircles, m_goalAreas, m_placedBalls, m_nonEditableAreas, m_inventorySlots, m_layerOrder);
	}
}

void StageRecord::calculateHash()
{
	static constexpr int32 version = 2;
	try {
		const std::string secret{ SIV3D_OBFUSCATE(SECRET_KEY) };
		m_hash = MD5::FromText(String(U"{};{};{};{};{};{};{}"_fmt(m_stageName, m_numberOfObjects, m_totalLength, m_blobStr, version, Unicode::Widen(secret), m_author)).toUTF8());
	}
	catch (...) {
		m_hash = MD5Value{};
	}
}

void StageRecord::fromJSON(const JSON& json)
{
	try {
		m_stageName = json[U"stagename"].getString();
		m_author = json[U"username"].getString();
		m_numberOfObjects = json[U"sc1"].get<int32>();
		m_totalLength = json[U"sc2"].get<int32>();
		m_blobStr = json[U"data"].getString();
		int32 version = json[U"version"].get<int32>();
		if (version == 2)
		{
			calculateHash();
			MD5Value sig;
			{
				auto md5Str = json[U"sig"].getString();
				std::array<uint8, 16> md5Array;
				// MD5値(String)をuint8のarrayに変換
				for (int i = 0; i < 16; i++) {
					md5Array[i] = ParseInt<uint8>(md5Str.substrView(i * 2, 2), Arg::radix = 16);
				}
				sig = MD5Value{ md5Array };
			}
			// Console << sig;
			// Console << m_hash;
			if (sig == m_hash) {
				fromBlobStr();
			}
			else {
				m_hash = MD5Value{};
			}
		}
	}
	catch (...) {
		m_hash = MD5Value{};
	}
}

AsyncHTTPTask StageRecord::createPostTask()
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = Unicode::Widen(url);

	if (!m_author.all([](char32 c) { return IsASCII(c) && !IsControl(c); }))
	{
		m_author = U"?";
	}

	calculateHash();

	JSON json{};
	json[U"version"] = 2;
	json[U"sc1"] = m_numberOfObjects;
	json[U"sc2"] = m_totalLength;
	json[U"username"] = m_author;
	json[U"stagename"] = m_stageName;
	json[U"data"] = m_blobStr;
	json[U"sig"] = m_hash.asString();
	json[U"persistent"] = true;

	//Console << json;

	auto code = json.formatUTF8Minimum();

	return SimpleHTTP::PostAsync(requestURL, {}, code.data(), code.length() * sizeof(std::string::value_type), U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

AsyncHTTPTask StageRecord::CreateGetLeaderboradTask(String stageName)
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = U"{}?leaderboard={}"_fmt(Unicode::Widen(url), stageName);
	return SimpleHTTP::GetAsync(requestURL, {}, U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

Array<StageRecord> StageRecord::ProcessGetLeaderboardTask(AsyncHTTPTask& task)
{
	Array<StageRecord> records;

	try {
		const auto& response = task.getResponse();
		if (!response.isOK()) {
			return records;
		}

		JSON json = task.getAsJSON();

		for (const auto& item : json[U"records"]) {
			records.push_back(StageRecord());
			records.back().fromJSON(item.value);
			if (!records.back().isValid()) {
				records.pop_back();
			}
		}

		return records;
	}
	catch (...) {
		return records;
	}
}

StageSave::StageSave(const Stage& stage)
{
	name = stage.m_name;
	peg = stage.getAllSelectableObjectsAsPointEdgeGroup();
}

AsyncHTTPTask StageSave::createPostTask()
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = Unicode::Widen(url);


	Serializer<MemoryWriter> archive;
	int32 version = 2;
	archive(version);
	archive(name);
	archive(peg);
	auto blobStr = archive->getBlob().base64Str();

	JSON json{};
	json[U"version"] = 2;
	json[U"stagename"] = name;
	json[U"data"] = blobStr;
	json[U"persistent"] = false;

	auto code = json.formatUTF8Minimum();

	return SimpleHTTP::PostAsync(requestURL, {}, code.data(), code.length() * sizeof(std::string::value_type), U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

AsyncHTTPTask StageSave::CreateGetTask(String shareCode)
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = U"{}?code={}"_fmt(Unicode::Widen(url), shareCode);
	return SimpleHTTP::GetAsync(requestURL, {}, U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

String StageSave::ProcessPostTask(AsyncHTTPTask& task)
{
	try {
		const auto& response = task.getResponse();

		if (!response.isOK()) {
			return U"";
		}

		if (task.isFile())
		{
			JSON json = task.getAsJSON();
			FileSystem::Remove(task.getFilePath());
			return json[U"id"].getString();
		}
		else
		{
			return U"";
		}
	}
	catch (...) {
		return {};
	}
}

StageSave StageSave::ProcessGetTask(AsyncHTTPTask& task)
{
	try {
		const auto& response = task.getResponse();
		if (!response.isOK()) {
			return {};
		}

		Deserializer<BinaryReader> deserializer(task.getFilePath());

		int32 version;
		deserializer(version);
		StageSave save;
		if (version == 2)
		{
			deserializer(save.name);
			deserializer(save.peg);
		}
		
		return save;
	}
	catch (...) {
		return {};
	}
}