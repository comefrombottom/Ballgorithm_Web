#pragma once

# include <Siv3D.hpp>
# include "Debug.hpp"
# include "Domain.hpp"
# include "GeometryUtils.hpp"
# include "Query.hpp"
# include "Inventory.h"
# include "Records.hpp"

class Game;

// Undo/Redo用のステージスナップショット
struct StageSnapshot {
	HashTable<size_t, Vec2> points;
	size_t nextPointId;
	Array<Edge> edges;
	HashTable<size_t, Group> groups;
	size_t nextGroupId;
	Array<PlacedBall> placedBalls;
	Array<InventorySlot> inventorySlots;
	Array<LayerObject> layerOrder;
	Array<RectF> nonEditableAreas;

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(points, nextPointId, edges, groups, nextGroupId, placedBalls, inventorySlots, layerOrder, nonEditableAreas);
	}

	size_t CalculateNumberOfObjects() const;
	double CalculateTotalLength() const;
};

// ステージ固有のデータとロジックのみを持つクラス
class Stage {
public:
	String m_name = U"Unnamed Stage";
	
	// チュートリアルテキスト（オプション、複数ページ対応）
	Array<String> m_tutorialTexts;

	// ステージ固有データ
	HashTable<size_t, Vec2> m_points;
	size_t m_nextPointId = 0;
	Array<Edge> m_edges;
	HashTable<size_t, Group> m_groups;
	size_t m_nextGroupId = 0;
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
	double m_simulationTimeAccumlate = 0.0;
	static constexpr double simulationTimeStep = 1.0 / 60.0;
	bool m_isSimulationRunning = false;
	bool m_isSimulationPaused = false;
	double m_simulationSpeed = 1.0;  // 1.0 = 通常速度, 2.0以上 = 早送り
	Array<std::unique_ptr<IQuery>> m_queries;
	size_t m_currentQueryIndex = 0;
	
	// クエリ達成状況
	Array<bool> m_queryCompleted;
	Array<bool> m_queryFailed;  // クエリ失敗状況
	bool m_isCleared = false;
	
	// カメラ位置（ステージごとに保持）
	Vec2 m_cameraCenter{ 400, 300 };
	double m_cameraScale = 1.0;

	Array<StageRecord> m_snapshotRecords;

	Stage();

	// 各オブジェクトを追加し、追加されたエッジ/オブジェクトのインデックスを返す
	size_t addLine(const Line& line, bool isLocked = false);
	size_t addStartCircle(const StartCircle& startCircle);
	size_t addGoalArea(const GoalArea& goalArea);
	size_t addPlacedBall(const PlacedBall& placedBall);
	
	// エッジのポイントIDを取得
	std::pair<size_t, size_t> getEdgePointIds(size_t edgeIndex) const {
		const auto& edge = m_edges[edgeIndex];
		return { edge[0], edge[1] };
	}
	
	void bringToFront(LayerObject obj);
	void removeFromLayerOrder(LayerObject obj);
	void updateLayerOrderAfterRemoval(LayerObjectType type, size_t removedId);
	void removePlacedBall(size_t index);  // PlacedBallを削除し、レイヤー順序も更新
	
	void createGroup(const Group& group);
	size_t createLockedGroup(const Group& group);  // isLocked=trueでグループ作成、IDを返す
	void createGroupFromSelection(HashSet<SelectedID>& selectedIDs);
	void ungroup(size_t groupId);
	Vec2 getBeginPointOfGroup(const Group& group) const;
	Optional<size_t> findTopGroup(size_t pointId) const;
	Optional<size_t> findTopGroupForPlacedBall(size_t placedBallId) const;
	Optional<size_t> findTopGroupForStartCircle(size_t startCircleId) const;
	Optional<size_t> findTopGroupForGoalArea(size_t goalAreaId) const;
	Group mapGroupIDs(const Group& group, const HashTable<size_t, size_t>& idMapping) const;
	PointEdgeGroup copySelectedObjects(const HashSet<SelectedID>& selectedIDs) const;
	void deltaMoveGroup(const Group& group, const Vec2& deltaMove);
	void eraseSelectedPoints(const HashSet<SelectedID>& selectedIDs);
	void startSimulation();
	bool checkSimulationResult() const;
	void endSimulation();
	double getLowestY() const;
	
	// インベントリ操作
	void addInventorySlot(BallKind ballKind, Optional<size_t> maxCount);
	bool canPlaceFromSlot(size_t index) const;
	void useFromSlot(size_t slotIndex);
	void returnToInventory(BallKind ballKind);
	const Array<InventorySlot>& inventorySlots() const { return m_inventorySlots; }
	
	// クエリ進捗管理
	void resetQueryProgress();
	void markQueryCompleted(size_t queryIndex);
	void markQueryFailed(size_t queryIndex);
	bool isAllQueriesCompleted() const;
	
	// 編集不可エリア操作
	void addNonEditableArea(const RectF& rect) { m_nonEditableAreas.push_back(rect); }
	const Array<RectF>& nonEditableAreas() const { return m_nonEditableAreas; }
	bool isPointInNonEditableArea(const Vec2& pos) const;
	bool isLineAllowedInEditableArea(const Line& line) const;

	// Undo/Redo用スナップショット
	StageSnapshot createSnapshot() const;
	void restoreSnapshot(const StageSnapshot& snapshot);

	AsyncTask<bool> loadAsync();
	AsyncTask<bool> saveAsync() const;

	AsyncTask<bool> loadRecordsAsync();
	AsyncTask<bool> saveRecordsAsync() const;
};

