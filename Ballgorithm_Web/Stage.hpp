#pragma once

# include <Siv3D.hpp>
# include "Debug.hpp"
# include "Domain.hpp"
# include "GeometryUtils.hpp"
# include "Query.hpp"
# include "Inventory.h"

class Game;

// Undo/Redo用のステージスナップショット
struct StageSnapshot {
	HashTable<int32, Vec2> points;
	int32 nextPointId;
	Array<Edge> edges;
	HashTable<int32, Group> groups;
	int32 nextGroupId;
	Array<PlacedBall> placedBalls;
	Array<InventorySlot> inventorySlots;
	Array<LayerObject> layerOrder;
	Array<RectF> nonEditableAreas;
};

class StageSave
{
public:
	String name;
	PointEdgeGroup peg;

	StageSave() = default;
	StageSave(const Stage& stage);

	AsyncHTTPTask createPostTask();

	static String ProcessPostTask(AsyncHTTPTask& task);
	static AsyncHTTPTask CreateGetTask(String shareCode);
	static StageSave ProcessGetTask(AsyncHTTPTask& task);
};

class StageRecord
{
public:
	String m_stageName;

	HashTable<int32, Vec2> m_points;
	Array<Edge> m_edges;
	HashTable<int32, Group> m_groups;
	Array<StartCircle> m_startCircles;
	Array<GoalArea> m_goalAreas;
	Array<PlacedBall> m_placedBalls;
	Array<RectF> m_nonEditableAreas;
	Array<InventorySlot> m_inventorySlots;
	Array<LayerObject> m_layerOrder;

	int32 m_numberOfObjects;
	int32 m_totalLength;

	String m_author;

	String m_blobStr;
	MD5Value m_hash;

	StageRecord() = default;

	StageRecord(const Stage& stage, String author);

	bool isValid() const;

	void intoBlobStr();
	void fromBlobStr();

	void calculateHash();

	void fromJSON(const JSON& json);

	AsyncHTTPTask createPostTask();

	static AsyncHTTPTask CreateGetLeaderboradTask(String stageName);
	static Array<StageRecord> ProcessGetLeaderboardTask(AsyncHTTPTask& task);
};


// ステージ固有のデータとロジックのみを持つクラス
class Stage {
public:
	String m_name = U"Unnamed Stage";
	
	// チュートリアルテキスト（オプション、複数ページ対応）
	Array<String> m_tutorialTexts;

	// ステージ固有データ
	HashTable<int32, Vec2> m_points;
	int32 m_nextPointId = 0;
	Array<Edge> m_edges;
	HashTable<int32, Group> m_groups;
	int32 m_nextGroupId = 0;
	Array<StartCircle> m_startCircles;
	Array<GoalArea> m_goalAreas;
	Array<PlacedBall> m_placedBalls; // プレイヤーが配置したボール
	
	// 編集不可エリア（UI操作で線を作れない/貫通できない領域）
	Array<RectF> m_nonEditableAreas;
	
	// インベントリスロット情報（UIは別で持つ）
	Array<InventorySlot> m_inventorySlots;
	
	// レイヤー順序（後ろほど手前に描画される）
	Array<LayerObject> m_layerOrder;
	
	P2World m_world;
	Array<P2Body> m_linesInWorld;
	Array<Ball> m_startBallsInWorld;
	Array<Line> m_initialLines;
	Array<PlacedBall> m_initialBalls;
	double m_simulationTimeAccumlate = 0.0;
	static constexpr double simulationTimeStep = 1.0 / 60.0;
	bool m_isSimulationRunning = false;
	bool m_isSimulationPaused = false;
	double m_simulationSpeed = 1.0;  // 1.0 = 通常速度, 2.0以上 = 早送り
	std::shared_ptr<Array<std::unique_ptr<IQuery>>> m_queries = std::make_shared<Array<std::unique_ptr<IQuery>>>();
	int32 m_currentQueryIndex = 0;
	
	// クエリ達成状況
	Array<bool> m_queryCompleted;
	Array<bool> m_queryFailed;  // クエリ失敗状況
	bool m_isCleared = false;
	
	// カメラ位置（ステージごとに保持）
	Vec2 m_cameraCenter{ 400, 300 };
	double m_cameraScale = 1.0;

	Stage();

	// 各オブジェクトを追加し、追加されたエッジ/オブジェクトのインデックスを返す
	int32 addLine(const Line& line, bool isLocked = false);
	int32 addStartCircle(const StartCircle& startCircle);
	int32 addGoalArea(const GoalArea& goalArea);
	int32 addPlacedBall(const PlacedBall& placedBall);
	
	// エッジのポイントIDを取得
	std::pair<int32, int32> getEdgePointIds(int32 edgeIndex) const {
		const auto& edge = m_edges[edgeIndex];
		return { edge[0], edge[1] };
	}
	
	void bringToFront(LayerObject obj);
	void removeFromLayerOrder(LayerObject obj);
	void updateLayerOrderAfterRemoval(LayerObjectType type, int32 removedId);
	void removePlacedBall(int32 index);  // PlacedBallを削除し、レイヤー順序も更新
	
	void createGroup(const Group& group);
	int32 createLockedGroup(const Group& group);  // isLocked=trueでグループ作成、IDを返す
	void createGroupFromSelection(HashSet<SelectedID>& selectedIDs);
	void ungroup(int32 groupId);
	Vec2 getBeginPointOfGroup(const Group& group) const;
	Optional<int32> findTopGroup(int32 pointId) const;
	Optional<int32> findTopGroupForPlacedBall(int32 placedBallId) const;
	Optional<int32> findTopGroupForStartCircle(int32 startCircleId) const;
	Optional<int32> findTopGroupForGoalArea(int32 goalAreaId) const;
	Group mapGroupIDs(const Group& group, const HashTable<int32, int32>& idMapping) const;
	PointEdgeGroup copySelectedObjects(const HashSet<SelectedID>& selectedIDs) const;
	void deltaMoveGroup(const Group& group, const Vec2& deltaMove);
	void eraseSelectedPoints(const HashSet<SelectedID>& selectedIDs);
	void startSimulation();
	bool checkSimulationResult() const;
	void endSimulation();
	double getLowestY() const;
	
	// インベントリ操作
	void addInventorySlot(BallKind ballKind, Optional<int32> maxCount);
	bool canPlaceFromSlot(int32 index) const;
	void useFromSlot(int32 slotIndex);
	void returnToInventory(BallKind ballKind);
	const Array<InventorySlot>& inventorySlots() const { return m_inventorySlots; }
	
	// クエリ進捗管理
	void resetQueryProgress();
	void markQueryCompleted(int32 queryIndex);
	void markQueryFailed(int32 queryIndex);
	bool isAllQueriesCompleted() const;
	bool isQueriesInitialState() const;
	
	// 編集不可エリア操作
	void addNonEditableArea(const RectF& rect) { m_nonEditableAreas.push_back(rect); }
	const Array<RectF>& nonEditableAreas() const { return m_nonEditableAreas; }
	bool isPointInNonEditableArea(const Vec2& pos) const;
	bool isLineAllowedInEditableArea(const Line& line) const;

	// Undo/Redo用スナップショット
	StageSnapshot createSnapshot() const;
	void restoreSnapshot(const StageSnapshot& snapshot);

	PointEdgeGroup getAllSelectableObjectsAsPointEdgeGroup() const;
	void removeAllSelectableObjects();
	void pastePointEdgeGroup(const PointEdgeGroup& group, SelectedIDSet& selectedIDs);

	int32 CalculateNumberOfObjects() const;
	int32 CalculateTotalLength() const;

	void restoreRecord(const StageRecord& record);

	void save(FilePath path = {}) const;
	void load(FilePath path = {});

	AsyncTask<bool> saveAsync(FilePath path = {}) const;
};

