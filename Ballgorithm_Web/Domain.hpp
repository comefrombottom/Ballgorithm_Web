#pragma once

# include <Siv3D.hpp>
# include "HashCache.hpp"

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
	size_t id;

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
	Circle circle;
	BallKind kind;

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(circle, kind);
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
	size_t id; // 点・線・StartCircle・GoalArea・PlacedBall のインデックス
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
	size_t id;
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
	HashTable<size_t, Vec2> m_points;
	Array<std::array<size_t, 2>> m_edges;
	Array<struct Group> m_groups;
	Array<PlacedBall> m_placedBalls;

	void moveBy(const Vec2& delta)
	{
		for (auto& [id, point] : m_points) {
			point += delta;
		}
		for (auto& placedBall : m_placedBalls) {
			placedBall.circle.center += delta;
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
};

struct Group {
	HashSet<size_t> m_pointIds; // 点のID
	HashSet<size_t> m_placedBallIds; // PlacedBall のID
	HashSet<size_t> m_startCircleIds; // StartCircle のID
	HashSet<size_t> m_goalAreaIds; // GoalArea のID
	Array<Group> m_groups;
	bool isLocked = false;  // グループ解除不可能フラグ

	template <class Archive>
	void SIV3D_SERIALIZE(Archive& archive)
	{
		archive(m_pointIds, m_placedBallIds, m_startCircleIds, m_goalAreaIds, m_groups, isLocked);
	}

	void insert(const Group& group) { m_groups.push_back(group); }
	void insertPointId(size_t pointId) { m_pointIds.insert(pointId); }
	void insertGoalAreaId(size_t goalAreaId) { m_goalAreaIds.insert(goalAreaId); }
	void insertStartCircleId(size_t startCircleId) { m_startCircleIds.insert(startCircleId); }
	void insertPlacedBallId(size_t placedBallId) { m_placedBallIds.insert(placedBallId); }

	size_t size() const { return m_pointIds.size() + m_placedBallIds.size() + m_startCircleIds.size() + m_goalAreaIds.size() + m_groups.size(); }

	size_t getBeginPointId() const {
		if (not m_pointIds.empty()) {
			return *m_pointIds.begin();
		}
		else if (not m_groups.empty()) {
			return (*m_groups.begin()).getBeginPointId();
		}
		throw std::runtime_error("Group is empty, no point ID available.");
	}

	bool containsPointRecursive(size_t pointId) const {
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
	
	bool containsPlacedBallRecursive(size_t placedBallId) const {
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
	
	bool containsStartCircleRecursive(size_t startCircleId) const {
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
	
	bool containsGoalAreaRecursive(size_t goalAreaId) const {
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

	HashSet<size_t> getAllPointIds() const {
		HashSet<size_t> allPointIds = m_pointIds;
		for (const auto& g : m_groups) {
			auto subPointIds = g.getAllPointIds();
			allPointIds.merge(subPointIds);
		}
		return allPointIds;
	}
	
	HashSet<size_t> getAllPlacedBallIds() const {
		HashSet<size_t> allPlacedBallIds = m_placedBallIds;
		for (const auto& g : m_groups) {
			auto subPlacedBallIds = g.getAllPlacedBallIds();
			allPlacedBallIds.merge(subPlacedBallIds);
		}
		return allPlacedBallIds;
	}
	
	HashSet<size_t> getAllStartCircleIds() const {
		HashSet<size_t> allStartCircleIds = m_startCircleIds;
		for (const auto& g : m_groups) {
			auto subStartCircleIds = g.getAllStartCircleIds();
			allStartCircleIds.merge(subStartCircleIds);
		}
		return allStartCircleIds;
	}
	
	HashSet<size_t> getAllGoalAreaIds() const {
		HashSet<size_t> allGoalAreaIds = m_goalAreaIds;
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
	std::array<size_t, 2> ids; // 点のID
	bool isLocked = false;

	[[nodiscard]] size_t& operator[](size_t i) { return ids[i]; }
	[[nodiscard]] const size_t& operator[](size_t i) const { return ids[i]; }
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
	size_t size() const { return m_ids.size(); }
	void insert(const SelectedID& id) { m_ids.insert(id); }
	void erase(const SelectedID& id) { m_ids.erase(id); }
	bool contains(const SelectedID& id) const { return m_ids.contains(id); }
	auto begin() const { return m_ids.begin(); }
	auto end() const { return m_ids.end(); }
	
	// 選択状態のチェック
	bool isSelectedPoint(const Stage& stage, size_t pointId) const;
	bool isSelectedEdge(const Stage& stage, size_t edgeIndex) const;
	bool isSelectedStartCircle(size_t index) const;
	bool isSelectedStartCircle(const Stage& stage, size_t index) const;
	bool isSelectedGoalArea(size_t index) const;
	bool isSelectedGoalArea(const Stage& stage, size_t index) const;
	bool isSelectedPlacedBall(const Stage& stage, size_t index) const;
	
	// 選択オブジェクトの操作
	Optional<Vec2> getBeginPointOfSelectedObjects(const Stage& stage) const;
	Vec2 getBottomRightOfSelectedObjects(const Stage& stage) const;
	bool isMovedSelectedNotInNonEditableArea(const Stage& stage, const Vec2& delta) const;
	void moveSelectedObjects(Stage& stage, const Vec2& delta) const;
	
	// 選択操作
	void selectAllObjects(Stage& stage);
	void selectObjectsByPoints(Stage& stage, const HashSet<size_t>& pointIds);
	void selectPlacedBallsByIds(Stage& stage, const HashSet<size_t>& ballIds);
	void selectObjectsInArea(Stage& stage, const RectF& area);
	
	friend void Formatter(FormatData& formatData, const SelectedIDSet& value)
	{
		formatData.string += Format(value.m_ids);
	}
};
