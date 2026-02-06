#pragma once

# include <Siv3D.hpp>
# include "Domain.hpp"
# include "GeometryUtils.hpp"

// クエリで使用するボールの初期状態
struct StartBallState { 
	BallKind kind;
	
	double radius() const { return GetBallRadius(kind); }
};

// シミュレーション中のボール
struct Ball { 
	P2Body body; 
	BallKind kind; 
	
	// ゴールに入ってからの経過時間（秒）。ゴール外なら none
	Optional<double> timeSinceEnteredGoal;
	
	bool isSmall() const { return kind == BallKind::Small; }
	bool isLarge() const { return kind == BallKind::Large; }
};

// 時間遅延付きボール放出イベント
// 各StartCircleから同時にボールを放出する
// delay が none の場合、前のボールが停止するまで待つ
struct DelayedBallRelease {
	Array<Optional<StartBallState>> startBalls;  // 各StartCircleから放出するボール（noneは何も放出しない）
	Optional<double> delay;                       // 前のイベントからの遅延秒数（noneは全ボール停止まで待機）
};

// ゴールエリアに入れるべきボールの要件（複数ボール対応）
// 例: {Small: 1, Large: 2} のような指定が可能
struct GoalRequirement {
	HashTable<BallKind, size_t> ballCounts;  // 各ボール種類の必要個数
	
	// 空の要件（何も入れてはいけない）
	static GoalRequirement empty() { return GoalRequirement{}; }
	
	// 単一ボールの要件
	static GoalRequirement single(BallKind kind) {
		GoalRequirement req;
		req.ballCounts[kind] = 1;
		return req;
	}
	
	// 複数ボールの要件
	static GoalRequirement multi(std::initializer_list<std::pair<BallKind, size_t>> counts) {
		GoalRequirement req;
		for (const auto& [kind, count] : counts) {
			if (count > 0) {
				req.ballCounts[kind] = count;
			}
		}
		return req;
	}
	
	// 要件が空かどうか（何も入れてはいけない）
	bool isEmpty() const { return ballCounts.empty(); }
	
	// 合計ボール数
	size_t totalCount() const {
		size_t total = 0;
		for (const auto& [kind, count] : ballCounts) {
			total += count;
		}
		return total;
	}
	
	// 指定したボール種類の必要個数
	size_t countOf(BallKind kind) const {
		auto it = ballCounts.find(kind);
		return (it != ballCounts.end()) ? it->second : 0;
	}
	
	// 実際に入っているボールが要件を満たすかチェック
	bool isSatisfiedBy(const Array<BallKind>& actualBalls) const {
		// 実際のボール数をカウント
		HashTable<BallKind, size_t> actualCounts;
		for (const auto& kind : actualBalls) {
			actualCounts[kind]++;
		}
		
		// 要件と比較
		// 1. 要件にある全ての種類について、正確な個数が必要
		for (const auto& [kind, requiredCount] : ballCounts) {
			auto it = actualCounts.find(kind);
			size_t actualCount = (it != actualCounts.end()) ? it->second : 0;
			if (actualCount != requiredCount) {
				return false;
			}
		}
		
		// 2. 要件にない種類のボールが入っていないことを確認
		for (const auto& [kind, actualCount] : actualCounts) {
			if (ballCounts.find(kind) == ballCounts.end() && actualCount > 0) {
				return false;
			}
		}
		
		return true;
	}
};

class Stage;

class IQuery {
public:
	virtual ~IQuery() = default;
	virtual void startSimulation(Stage& stage) = 0;
	virtual void update(Stage& stage, double dt) = 0;  // 時間ベースの更新
	virtual bool checkSimulationResult(const Stage& stage) = 0;
	
	// 全てのボールを放出済みかどうか
	virtual bool hasFinishedReleasing() const = 0;
	
	// 表示用情報取得
	virtual Array<Optional<StartBallState>> getStartBalls() const = 0;
	virtual Array<Optional<BallKind>> getGoalRequirements() const = 0;
	
	// クエリパネル描画（クエリカード内の内容を描画）
	// queryRect: クエリカード全体の矩形
	// 戻り値: 描画に使用した高さ（次のクエリの配置に使用）
	virtual double drawPanelContent(const RectF& queryRect, bool isActive) const = 0;
	
	// クエリパネルの必要な高さを取得
	virtual double getPanelHeight() const = 0;
};

class SampleQuery : public IQuery {
	Array<Optional<StartBallState>> m_startBalls;
	Array<Optional<BallKind>> m_goalAreaToBeFilled; // ゴールエリアごとに、どの種類のボールを入れるべきか
public:
	SampleQuery(const Array<Optional<StartBallState>>& startBalls, const Array<Optional<BallKind>>& goalAreaToBeFilled)
		: m_startBalls(startBalls), m_goalAreaToBeFilled(goalAreaToBeFilled) {}
	void startSimulation(Stage& stage) override;
	void update(Stage& stage, double dt) override;  // no-op for SampleQuery
	bool checkSimulationResult(const Stage& stage) override;
	
	// SampleQueryは開始時に全て放出するので常にtrue
	bool hasFinishedReleasing() const override { return true; }
	
	Array<Optional<StartBallState>> getStartBalls() const override { return m_startBalls; }
	Array<Optional<BallKind>> getGoalRequirements() const override { return m_goalAreaToBeFilled; }
	
	double drawPanelContent(const RectF& queryRect, bool isActive) const override;
	double getPanelHeight() const override { return 80.0; }  // 75.0 -> 80.0
};

// 各StartCircleから時間を置いて順にボールを放出するクエリ
// delayがnoneの場合、全てのボールが停止するまで待機してから次を放出
class SequentialQuery : public IQuery {
public:
	// releases: 放出シーケンス（各イベントで全StartCircleから一斉放出）
	// goalAreaToBeFilled: ゴール条件
	SequentialQuery(
		const Array<DelayedBallRelease>& releases,
		const Array<Optional<BallKind>>& goalAreaToBeFilled
	);

	void startSimulation(Stage& stage) override;
	void update(Stage& stage, double dt) override;
	bool checkSimulationResult(const Stage& stage) override;
	
	// 全てのボールを放出済みかどうか
	bool hasFinishedReleasing() const override;
	
	Array<Optional<StartBallState>> getStartBalls() const override;
	Array<Optional<BallKind>> getGoalRequirements() const override { return m_goalAreaToBeFilled; }
	
	double drawPanelContent(const RectF& queryRect, bool isActive) const override;
	double getPanelHeight() const override;

private:
	Array<DelayedBallRelease> m_releases;  // 放出シーケンス
	Array<Optional<BallKind>> m_goalAreaToBeFilled;
	
	// シミュレーション状態
	size_t m_nextReleaseIndex = 0;          // 次の放出インデックス
	double m_timeSinceLastRelease = 0.0;    // 最後の放出からの経過時間
	bool m_waitingForAllStopped = false;    // 全ボール停止待ちかどうか
	
	void releaseBalls(Stage& stage, size_t releaseIndex);
	bool areAllBallsStopped(const Stage& stage) const;
};

// 1つのクエリ内で「放出フェーズ → ゴール確認フェーズ」を複数回繰り返すクエリ
// 全てのゴール確認フェーズがOKならクリア
class MultiPhaseQuery : public IQuery {
public:
	struct Phase {
		Array<DelayedBallRelease> releases;
		Array<GoalRequirement> goalRequirements;
	};

	explicit MultiPhaseQuery(const Array<Phase>& phases);

	void startSimulation(Stage& stage) override;
	void update(Stage& stage, double dt) override;
	bool checkSimulationResult(const Stage& stage) override;

	bool hasFinishedReleasing() const override;

	Array<Optional<StartBallState>> getStartBalls() const override;
	Array<Optional<BallKind>> getGoalRequirements() const override;

	double drawPanelContent(const RectF& queryRect, bool isActive) const override;
	double getPanelHeight() const override;

private:
	Array<Phase> m_phases;

	// フェーズ進行状態
	size_t m_phaseIndex = 0;
	size_t m_nextReleaseIndex = 0;
	double m_timeSinceLastRelease = 0.0;
	bool m_waitingForAllStopped = false;

	// フェーズごとのゴール確認結果
	Array<Optional<bool>> m_phaseResults;

	void startPhase(Stage& stage, size_t phaseIndex);
	void releaseBalls(Stage& stage, size_t phaseIndex, size_t releaseIndex);
	bool areAllBallsStopped(const Stage& stage) const;
	bool checkGoalForPhase(const Stage& stage, size_t phaseIndex) const;
};

// SequentialQueryを発展させ、ゴールに複数のボールを指定できるクエリ
// 例: ゴールエリアAに{Small: 1, Large: 2}のような複数ボール要件を設定可能
class MultiGoalSequentialQuery : public IQuery {
public:
	// releases: 放出シーケンス（各イベントで全StartCircleから一斉放出）
	// goalRequirements: ゴールエリアごとの複数ボール要件
	MultiGoalSequentialQuery(
		const Array<DelayedBallRelease>& releases,
		const Array<GoalRequirement>& goalRequirements
	);

	void startSimulation(Stage& stage) override;
	void update(Stage& stage, double dt) override;
	bool checkSimulationResult(const Stage& stage) override;
	
	bool hasFinishedReleasing() const override;
	
	Array<Optional<StartBallState>> getStartBalls() const override;
	Array<Optional<BallKind>> getGoalRequirements() const override;
	
	// 複数ボール要件を取得（新規メソッド）
	const Array<GoalRequirement>& getMultiGoalRequirements() const { return m_goalRequirements; }
	
	double drawPanelContent(const RectF& queryRect, bool isActive) const override;
	double getPanelHeight() const override;

private:
	Array<DelayedBallRelease> m_releases;
	Array<GoalRequirement> m_goalRequirements;
	
	// シミュレーション状態
	size_t m_nextReleaseIndex = 0;
	double m_timeSinceLastRelease = 0.0;
	bool m_waitingForAllStopped = false;
	
	void releaseBalls(Stage& stage, size_t releaseIndex);
	bool areAllBallsStopped(const Stage& stage) const;
};
