# include "Stage.hpp"
# include "Query.hpp"
# include "GeometryUtils.hpp"
# include "IndexedDB.hpp"

Stage::Stage()
{
}

size_t Stage::addLine(const Line& line, bool isLocked)
{
	size_t id1 = m_nextPointId++;
	size_t id2 = m_nextPointId++;
	m_points[id1] = line.begin;
	m_points[id2] = line.end;
	size_t edgeIndex = m_edges.size();
	m_edges.push_back(Edge{ { id1, id2 }, isLocked });
	m_layerOrder.push_back(LayerObject{ LayerObjectType::Edge, edgeIndex });
	return edgeIndex;
}

size_t Stage::addStartCircle(const StartCircle& startCircle)
{
	size_t index = m_startCircles.size();
	m_startCircles.push_back(startCircle);
	m_layerOrder.push_back(LayerObject{ LayerObjectType::StartCircle, index });
	return index;
}

size_t Stage::addGoalArea(const GoalArea& goalArea)
{
	size_t index = m_goalAreas.size();
	m_goalAreas.push_back(goalArea);
	m_layerOrder.push_back(LayerObject{ LayerObjectType::GoalArea, index });
	return index;
}

size_t Stage::addPlacedBall(const PlacedBall& placedBall)
{
	size_t index = m_placedBalls.size();
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

void Stage::updateLayerOrderAfterRemoval(LayerObjectType type, size_t removedId)
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

void Stage::removePlacedBall(size_t index)
{
	if (index >= m_placedBalls.size()) return;
	m_placedBalls.remove_at(index);
	updateLayerOrderAfterRemoval(LayerObjectType::PlacedBall, index);
}

void Stage::createGroup(const Group& group)
{
	if (group.size() >= 2) {
		size_t newGroupId = m_nextGroupId++;
		m_groups[newGroupId] = group;
	}
}

size_t Stage::createLockedGroup(const Group& group)
{
	size_t newGroupId = m_nextGroupId++;
	Group lockedGroup = group;
	lockedGroup.isLocked = true;
	m_groups[newGroupId] = lockedGroup;
	return newGroupId;
}

void Stage::createGroupFromSelection(HashSet<SelectedID>& selectedIDs)
{
	Group newGroup;
	HashSet<size_t> addPointIds;
	HashSet<size_t> addPlacedBallIds;
	HashSet<size_t> addStartCircleIds;
	HashSet<size_t> addGoalAreaIds;
	
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

void Stage::ungroup(size_t groupId)
{
	const auto& group = m_groups.at(groupId);
	for (const auto& g : group.m_groups) {
		size_t newGroupId = m_nextGroupId++;
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
		return m_placedBalls[*group.m_placedBallIds.begin()].circle.center;
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

Optional<size_t> Stage::findTopGroup(size_t pointId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsPointRecursive(pointId)) {
			return groupId;
		}
	}
	return none;
}

Optional<size_t> Stage::findTopGroupForPlacedBall(size_t placedBallId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsPlacedBallRecursive(placedBallId)) {
			return groupId;
		}
	}
	return none;
}

Optional<size_t> Stage::findTopGroupForStartCircle(size_t startCircleId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsStartCircleRecursive(startCircleId)) {
			return groupId;
		}
	}
	return none;
}

Optional<size_t> Stage::findTopGroupForGoalArea(size_t goalAreaId) const
{
	for (const auto& [groupId, group] : m_groups) {
		if (group.containsGoalAreaRecursive(goalAreaId)) {
			return groupId;
		}
	}
	return none;
}

Group Stage::mapGroupIDs(const Group& group, const HashTable<size_t, size_t>& idMapping) const
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
	HashSet<size_t> allSelectedPointIds;
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
		m_placedBalls[ballId].circle.center += deltaMove;
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
	HashSet<size_t> pointsToErase;
	Array<size_t> placedBallsToErase;
	
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
	std::sort(placedBallsToErase.begin(), placedBallsToErase.end(), std::greater<size_t>());
	for (auto idx : placedBallsToErase) {
		m_placedBalls.remove_at(idx);
		updateLayerOrderAfterRemoval(LayerObjectType::PlacedBall, idx);
	}
	
	Array<size_t> edgesToErase;
	for (size_t i = 0; i < m_edges.size(); ++i) {
		const auto& edge = m_edges[i];
		if (pointsToErase.contains(edge[0]) or pointsToErase.contains(edge[1])) {
			edgesToErase.push_back(i);
			pointsToErase.insert(edge[0]);
			pointsToErase.insert(edge[1]);
		}
	}
	// Edge も大きい順に削除してレイヤー順序を更新
	std::sort(edgesToErase.begin(), edgesToErase.end(), std::greater<size_t>());
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
	
	// 壁（エッジ）を物理世界に追加
	for (const auto& edge : m_edges) {
		const Vec2& p1 = m_points.at(edge[0]);
		const Vec2& p2 = m_points.at(edge[1]);
		auto lineBody = createLine(m_world, P2Static, Line(p1, p2));
		m_linesInWorld.push_back(lineBody);
	}
	
	// プレイヤーが配置したボールを物理世界に追加
	for (const auto& placedBall : m_placedBalls) {
		P2Body ballBody = createCircle(
			m_world,
			P2Dynamic,
			placedBall.circle
		);
		m_startBallsInWorld.push_back(Ball{ ballBody, placedBall.kind });
	}
	
	// クエリ固有のシミュレーション開始処理
	if (m_currentQueryIndex < m_queries.size()) {
		m_queries[m_currentQueryIndex]->startSimulation(*this);
	}

#if SIV3D_PLATFORM(WEB)
	s3d::Platform::Web::System::AwaitAsyncTask(saveTask);
#endif
}

bool Stage::checkSimulationResult() const
{
	if (m_currentQueryIndex < m_queries.size()) {
		return m_queries[m_currentQueryIndex]->checkSimulationResult(*this);
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
		lowestY = Max(lowestY, b.circle.center.y + b.circle.r);
	}
	
	return lowestY;
}

void Stage::addInventorySlot(BallKind ballKind, Optional<size_t> maxCount)
{
	m_inventorySlots.push_back(InventorySlot::CreateBallSlot(ballKind, maxCount));
}

bool Stage::canPlaceFromSlot(size_t index) const
{
	if (index >= m_inventorySlots.size()) return false;
	return m_inventorySlots[index].hasRemaining();
}

void Stage::useFromSlot(size_t slotIndex)
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
	m_queryCompleted.assign(m_queries.size(), false);
	m_queryFailed.assign(m_queries.size(), false);
	m_currentQueryIndex = 0;
	// m_isCleared = false;
}

void Stage::markQueryCompleted(size_t queryIndex)
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

void Stage::markQueryFailed(size_t queryIndex)
{
	if (queryIndex < m_queryFailed.size()) {
		m_queryFailed[queryIndex] = true;
	}
}

bool Stage::isAllQueriesCompleted() const
{
	if (m_queries.empty()) return false;
	for (bool completed : m_queryCompleted) {
		if (!completed) return false;
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

void Stage::save() const
{
	auto task = saveAsync();
#if SIV3D_PLATFORM(WEB)
	s3d::Platform::Web::System::AwaitAsyncTask(task).value_or(false);
#endif
}

AsyncTask<bool> Stage::saveAsync() const
{
	{
		Serializer<BinaryWriter> serializer{ U"Ballgorithm/Stages/{}.bin"_fmt(m_name) };

		serializer(createSnapshot());
		serializer(m_queryCompleted);
		serializer(m_queryFailed);
		serializer(m_isCleared);
	}

#if SIV3D_PLATFORM(WEB)
	return s3d::Platform::Web::IndexedDB::SaveAsync();
#else
	return AsyncTask<bool>([]() { return true; });
#endif
}

int32 StageSnapshot::CalculateNumberOfObjects() const
{
	return edges.count_if([](const Edge& edge) { return !edge.isLocked; }) + placedBalls.size();
}

int32 StageSnapshot::CalculateTotalLength() const
{
	double sum = 0;
	for (const auto& edge : edges) {
		if (!edge.isLocked)
		{
			const Vec2& p1 = points.at(edge[0]);
			const Vec2& p2 = points.at(edge[1]);
			sum += p1.distanceFrom(p2);
		}
	}
	return sum;
}

# include ".SECRET"

StageRecord::StageRecord(const Stage& stage, String author)
{
	if (!stage.isAllQueriesCompleted()) {
		return;
	}
	//const std::string secret{ SIV3D_OBFUSCATE(SECRET_KEY) };
	m_stageName = stage.m_name;
	m_snapshot = stage.createSnapshot();
	m_numberOfObjects = m_snapshot.CalculateNumberOfObjects();
	m_totalLength = m_snapshot.CalculateTotalLength();
	m_author = author;
}

bool StageRecord::isValid() const
{
	return !m_stageName.isEmpty() && m_hash != MD5Value();
}

void StageRecord::calculateHash()
{
	try {
		const std::string secret{ SIV3D_OBFUSCATE(SECRET_KEY) };
		if (m_blobStr.isEmpty()) {
			Serializer<BinaryWriter> serializer{ U"Temp/Ballgorithm/record.bin" };
			serializer(m_snapshot);
			serializer->close();
			m_blobStr = Blob{ U"Temp/Ballgorithm/record.bin" }.base64Str();
		}
		m_hash = MD5::FromText(String(U"{}{}{}{}{}{}"_fmt(m_author, m_stageName, m_numberOfObjects, m_totalLength, m_blobStr, Unicode::Widen(secret))).toUTF8());
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
			Deserializer<MemoryReader> deserializer{ Base64::Decode(m_blobStr) };
			deserializer(m_snapshot);
		}
		else {
			m_hash = MD5Value{};
		}
	}
	catch (...) {
		m_hash = MD5Value{};
	}
}

void StageRecord::createPostTask()
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = Unicode::Widen(url);

	if (!m_author.all([](char32 c) { return IsASCII(c) && !IsControl(c); }))
	{
		m_author = U"?";
	}

	calculateHash();

	JSON json{};
	json[U"sc1"] = m_numberOfObjects;
	json[U"sc2"] = m_totalLength;
	json[U"username"] = m_author;
	json[U"stagename"] = m_stageName;
	json[U"data"] = m_blobStr;
	json[U"sig"] = m_hash.asString();

	Console << json;

	auto code = json.formatUTF8Minimum();

	m_postTask = SimpleHTTP::PostAsync(requestURL, {}, code.data(), code.length() * sizeof(std::string::value_type), U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

String StageRecord::processPostTask()
{
	try {
		const auto& response = m_postTask.getResponse();

		if (!response.isOK()) {
			return {};
		}

		JSON json = m_postTask.getAsJSON();
		m_shareCode = json[U"id"].getString();

		return m_shareCode;
	}
	catch (...) {
		return {};
	}
}

AsyncHTTPTask StageRecord::createGetTask(String shareCode)
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = U"{}?code={}"_fmt(Unicode::Widen(url), shareCode);
	return SimpleHTTP::GetAsync(requestURL, {}, U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

AsyncHTTPTask StageRecord::createGetLeaderboradTask(String stageName)
{
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = U"{}?leaderboard={}"_fmt(Unicode::Widen(url), stageName);
	return SimpleHTTP::GetAsync(requestURL, {}, U"Temp/Ballgorithm/{}.json"_fmt(Time::GetMillisecSinceEpoch()));
}

StageRecord StageRecord::processGetTask(AsyncHTTPTask& task)
{
	try {
		const auto& response = task.getResponse();
		if (!response.isOK()) {
			return {};
		}

		StageRecord record;
		
		record.fromJSON(task.getAsJSON());

		return record;
	}
	catch (...) {
		return {};
	}
}

Array<StageRecord> StageRecord::processGetLeaderboardTask(AsyncHTTPTask& task)
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
