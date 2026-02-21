#pragma once

# include <Siv3D.hpp>
# include "HashCache.hpp"
#include <Siv3D/AsyncHTTPTask.hpp>
#include <Siv3D/String.hpp>

// Forward declarations
class Stage;

// ボールの種類
enum class BallKind {
	Small,
	Large,
};

// BallKind に対応する半径を取得
inline double GetBallRadius(BallKind kind) {
	switch (kind) {
	case BallKind::Small: return 9.0;
	case BallKind::Large: return 19.0;
	default: return 9.0;
	}
}

// BallKind に対応する色を取得
inline ColorF GetBallColor(BallKind kind) {
	switch (kind) {
	case BallKind::Small: return ColorF(0.8, 0.5, 0.2);  // オレンジ系
	case BallKind::Large: return ColorF(0.6, 0.3, 0.8);  // 紫系
	default: return ColorF(0.8, 0.5, 0.2);
	}
}

inline void Formatter(FormatData& formatData, const BallKind& value) {
	switch (value) {
	case BallKind::Small: formatData.string += U"Small"; break;
	case BallKind::Large: formatData.string += U"Large"; break;
	default: formatData.string += U"Unknown"; break;
	}
}

// レイヤー順序管理用のオブジェクト識別子
enum class LayerObjectType {
	Edge,
	StartCircle,
	GoalArea,
	PlacedBall,
};

struct LayerObject {
	LayerObjectType type;
	int32 id;

	template<class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(type, id);
	}
	
	bool operator==(const LayerObject&) const = default;
	
	friend void Formatter(FormatData& formatData, const LayerObject& value)
	{
		formatData.string += U"(";
		switch (value.type) {
		case LayerObjectType::Edge: formatData.string += U"Edge"; break;
		case LayerObjectType::StartCircle: formatData.string += U"StartCircle"; break;
		case LayerObjectType::GoalArea: formatData.string += U"GoalArea"; break;
		case LayerObjectType::PlacedBall: formatData.string += U"PlacedBall"; break;
		}
		formatData.string += U", {})"_fmt(value.id);
	}
};

// プレイヤーが配置したボール（インベントリから）
struct PlacedBall {
	Vec2 center;
	BallKind kind;
	bool isLocked = false;

	PlacedBall() = default;

	PlacedBall(const Vec2& center, BallKind kind, bool isLocked = false)
		: center(center), kind(kind), isLocked(isLocked) {}

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(center, kind, isLocked);
	}

	bool isSmall() const { return kind == BallKind::Small; }
	bool isLarge() const { return kind == BallKind::Large; }
};

// Hover / Select 種別
enum class HoverType {
	Point,
	Edge,
	Group,
	StartCircle,
	GoalArea,
	PlacedBall
};

inline void Formatter(FormatData& formatData, const HoverType& value)
{
	switch (value) {
	case HoverType::Point: formatData.string += U"Point"; break;
	case HoverType::Edge: formatData.string += U"Edge"; break;
	case HoverType::Group: formatData.string += U"Group"; break;
	case HoverType::StartCircle: formatData.string += U"StartCircle"; break;
	case HoverType::GoalArea: formatData.string += U"GoalArea"; break;
	case HoverType::PlacedBall: formatData.string += U"PlacedBall"; break;
	default: formatData.string += U"Unknown"; break;
	}
}

struct HoverInfo {
	HoverType type;
	int32 id; // 点・線・StartCircle・GoalArea・PlacedBall のインデックス
	bool operator==(const HoverInfo& other) const = default;
	friend void Formatter(FormatData& formatData, const HoverInfo& value)
	{
		formatData.string += U"({}, {})"_fmt(Format(value.type), value.id);
	}
};

// 選択対象の種類
enum class SelectType {
	Point,
	Group,
	StartCircle,
	GoalArea,
	PlacedBall,
};

inline void Formatter(FormatData& formatData, const SelectType& value)
{
	switch (value) {
	case SelectType::Point: formatData.string += U"Point"; break;
	case SelectType::Group: formatData.string += U"Group"; break;
	case SelectType::StartCircle: formatData.string += U"StartCircle"; break;
	case SelectType::GoalArea: formatData.string += U"GoalArea"; break;
	case SelectType::PlacedBall: formatData.string += U"PlacedBall"; break;
	default: formatData.string += U"Unknown"; break;
	}
}

struct SelectedID {
	SelectType type;
	int32 id;
	bool operator==(const SelectedID&) const = default;
	friend void Formatter(FormatData& formatData, const SelectedID& value)
	{
		formatData.string += U"({}, {})"_fmt(Format(value.type), value.id);
	}
};

namespace std {
	template<>
	struct hash<SelectedID>
	{
		size_t operator()(const SelectedID& v) const noexcept
		{
			return hash_values(v.type, v.id);
		}
	};
}

struct Group;

struct PointEdgeGroup {
	HashTable<int32, Vec2> m_points;
	Array<std::array<int32, 2>> m_edges;
	Array<struct Group> m_groups;
	Array<PlacedBall> m_placedBalls;

	void moveBy(const Vec2& delta)
	{
		for (auto& [id, point] : m_points) {
			point += delta;
		}
		for (auto& placedBall : m_placedBalls) {
			placedBall.center += delta;
		}
	}

	bool empty() const
	{
		return m_points.empty() && m_edges.empty() && m_groups.empty() && m_placedBalls.empty();
	}

	friend void Formatter(FormatData& formatData, const PointEdgeGroup& value)
	{
		formatData.string += U"Points: ";
		formatData.string += Format(value.m_points);
		formatData.string += U", Edges: ";
		formatData.string += Format(value.m_edges);
		formatData.string += U", Groups: ";
		formatData.string += Format(value.m_groups);
		formatData.string += U", PlacedBalls: ";
		formatData.string += Format(value.m_placedBalls.size());
	}

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(m_points, m_edges, m_groups, m_placedBalls);
	}
};

struct Group {
	HashSet<int32> m_pointIds; // 点のID
	HashSet<int32> m_placedBallIds; // PlacedBall のID
	HashSet<int32> m_startCircleIds; // StartCircle のID
	HashSet<int32> m_goalAreaIds; // GoalArea のID
	Array<Group> m_groups;
	bool isLocked = false;  // グループ解除不可能フラグ

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(m_pointIds, m_placedBallIds, m_startCircleIds, m_goalAreaIds, m_groups, isLocked);
	}

	void insert(const Group& group) { m_groups.push_back(group); }
	void insertPointId(int32 pointId) { m_pointIds.insert(pointId); }
	void insertGoalAreaId(int32 goalAreaId) { m_goalAreaIds.insert(goalAreaId); }
	void insertStartCircleId(int32 startCircleId) { m_startCircleIds.insert(startCircleId); }
	void insertPlacedBallId(int32 placedBallId) { m_placedBallIds.insert(placedBallId); }

	int32 size() const { return m_pointIds.size() + m_placedBallIds.size() + m_startCircleIds.size() + m_goalAreaIds.size() + m_groups.size(); }

	int32 getBeginPointId() const {
		if (not m_pointIds.empty()) {
			return *m_pointIds.begin();
		}
		else if (not m_groups.empty()) {
			return (*m_groups.begin()).getBeginPointId();
		}
		throw std::runtime_error("Group is empty, no point ID available.");
	}

	bool containsPointRecursive(int32 pointId) const {
		if (m_pointIds.contains(pointId)) {
			return true;
		}
		for (const auto& g : m_groups) {
			if (g.containsPointRecursive(pointId)) {
				return true;
			}
		}
		return false;
	}
	
	bool containsPlacedBallRecursive(int32 placedBallId) const {
		if (m_placedBallIds.contains(placedBallId)) {
			return true;
		}
		for (const auto& g : m_groups) {
			if (g.containsPlacedBallRecursive(placedBallId)) {
				return true;
			}
		}
		return false;
	}
	
	bool containsStartCircleRecursive(int32 startCircleId) const {
		if (m_startCircleIds.contains(startCircleId)) {
			return true;
		}
		for (const auto& g : m_groups) {
			if (g.containsStartCircleRecursive(startCircleId)) {
				return true;
			}
		}
		return false;
	}
	
	bool containsGoalAreaRecursive(int32 goalAreaId) const {
		if (m_goalAreaIds.contains(goalAreaId)) {
			return true;
		}
		for (const auto& g : m_groups) {
			if (g.containsGoalAreaRecursive(goalAreaId)) {
				return true;
			}
		}
		return false;
	}

	HashSet<int32> getAllPointIds() const {
		HashSet<int32> allPointIds = m_pointIds;
		for (const auto& g : m_groups) {
			auto subPointIds = g.getAllPointIds();
			allPointIds.merge(subPointIds);
		}
		return allPointIds;
	}
	
	HashSet<int32> getAllPlacedBallIds() const {
		HashSet<int32> allPlacedBallIds = m_placedBallIds;
		for (const auto& g : m_groups) {
			auto subPlacedBallIds = g.getAllPlacedBallIds();
			allPlacedBallIds.merge(subPlacedBallIds);
		}
		return allPlacedBallIds;
	}
	
	HashSet<int32> getAllStartCircleIds() const {
		HashSet<int32> allStartCircleIds = m_startCircleIds;
		for (const auto& g : m_groups) {
			auto subStartCircleIds = g.getAllStartCircleIds();
			allStartCircleIds.merge(subStartCircleIds);
		}
		return allStartCircleIds;
	}
	
	HashSet<int32> getAllGoalAreaIds() const {
		HashSet<int32> allGoalAreaIds = m_goalAreaIds;
		for (const auto& g : m_groups) {
			auto subGoalAreaIds = g.getAllGoalAreaIds();
			allGoalAreaIds.merge(subGoalAreaIds);
		}
		return allGoalAreaIds;
	}

	bool operator==(const Group& other) const = default;

	friend void Formatter(FormatData& formatData, const Group& value)
	{
		formatData.string += U"Points: ";
		formatData.string += Format(value.m_pointIds);
		formatData.string += U", PlacedBalls: ";
		formatData.string += Format(value.m_placedBallIds);
		formatData.string += U", StartCircles: ";
		formatData.string += Format(value.m_startCircleIds);
		formatData.string += U", GoalAreas: ";
		formatData.string += Format(value.m_goalAreaIds);
		formatData.string += U", Groups: ";
		formatData.string += Format(value.m_groups);
		if (value.isLocked) {
			formatData.string += U" [LOCKED]";
		}
	}
};

struct StartCircle { Circle circle; bool isLocked = false; };
struct GoalArea { RectF rect; bool isLocked = false; };

struct Edge {
	std::array<int32, 2> ids; // 点のID
	bool isLocked = false;

	[[nodiscard]] int32& operator[](int32 i) { return ids[i]; }
	[[nodiscard]] const int32& operator[](int32 i) const { return ids[i]; }
	bool operator==(const Edge& other) const = default;
	friend void Formatter(FormatData& formatData, const Edge& value)
	{
		formatData.string += U"({}, {})"_fmt(value.ids[0], value.ids[1]);
	}

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(ids, isLocked);
	}
};

// 選択されたIDのセットを管理する構造体
struct SelectedIDSet {
	HashSet<SelectedID> m_ids;
	
	// HashSet<SelectedID> の基本操作をラップ
	void clear() { m_ids.clear(); }
	bool empty() const { return m_ids.empty(); }
	int32 size() const { return m_ids.size(); }
	void insert(const SelectedID& id) { m_ids.insert(id); }
	void erase(const SelectedID& id) { m_ids.erase(id); }
	bool contains(const SelectedID& id) const { return m_ids.contains(id); }
	auto begin() const { return m_ids.begin(); }
	auto end() const { return m_ids.end(); }
	
	// 選択状態のチェック
	bool isSelectedPoint(const Stage& stage, int32 pointId) const;
	bool isSelectedEdge(const Stage& stage, int32 edgeIndex) const;
	bool isSelectedStartCircle(int32 index) const;
	bool isSelectedStartCircle(const Stage& stage, int32 index) const;
	bool isSelectedGoalArea(int32 index) const;
	bool isSelectedGoalArea(const Stage& stage, int32 index) const;
	bool isSelectedPlacedBall(const Stage& stage, int32 index) const;
	
	// 選択オブジェクトの操作
	Optional<Vec2> getBeginPointOfSelectedObjects(const Stage& stage) const;
	Vec2 getBottomRightOfSelectedObjects(const Stage& stage) const;
	bool isMovedSelectedNotInNonEditableArea(const Stage& stage, const Vec2& delta) const;
	void moveSelectedObjects(Stage& stage, const Vec2& delta) const;
	bool flipHorizontalSelectedObjects(Stage& stage) const;
	
	// 選択操作
	void selectAllObjects(const Stage& stage);
	void selectObjectsByPoints(const Stage& stage, const HashSet<int32>& pointIds);
	void selectPlacedBallsByIds(const Stage& stage, const HashSet<int32>& ballIds);
	void selectObjectsInArea(const Stage& stage, const RectF& area);
	
	friend void Formatter(FormatData& formatData, const SelectedIDSet& value)
	{
		formatData.string += Format(value.m_ids);
	}
};
