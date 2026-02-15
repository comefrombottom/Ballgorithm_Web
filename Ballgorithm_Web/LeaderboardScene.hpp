#pragma once

# include <Siv3D.hpp>
# include "InputUtils.hpp"
# include "ScrollBar.h"
# include "QueryPanel.h"
# include "MyCamera2D.h"
# include "StageEditUI.h"
# include "Stage.hpp"

class Game;

class LeaderboardScene {
public:
	LeaderboardScene();

	// ステージインデックスを指定して開く
	void enter(Game& game, int32 stageIndex);
	void exit();

	void update(Game& game, double dt = Scene::DeltaTime());
	void draw(const Game& game) const;

private:
	int32 m_stageIndex = 0;
	bool m_viewerActive = false;  // 解法が選択されビュワーが有効か

	// ランキングデータ
	Array<StageRecord> m_records;
	AsyncHTTPTask m_leaderboardTask;
	bool m_isLoading = false;
	bool m_loadFailed = false;

	// ランキング表UI
	ScrollBar m_rankingScrollBar;
	Optional<int32> m_hoveredRecordIndex;
	SingleUseCursorPos m_cursorPos;

	Optional<Vec2> m_mousePressPos;
	bool m_isDragging = false;

	// 解法閲覧用
	Optional<int32> m_selectedRecordIndex;
	StageSnapshot m_viewerStageSnapshot;  // 閲覧前の元スナップショット（復元用）
	StageEditUI m_viewerEditUI;
	MyCamera2D m_viewerCamera{ Vec2(500, 300), 1.0, CameraControl::Wheel };
	QueryPanel m_viewerQueryPanel;
	bool m_singleQueryMode = false;

	// タッチ 2本指ジェスチャー（パン/ピンチ）
	bool m_isTwoFingerGesturing = false;
	int32 m_twoFingerId0 = -1;
	int32 m_twoFingerId1 = -1;
	Vec2 m_prevTwoFingerCenter{ 0, 0 };
	double m_prevTwoFingerDistance = 0.0;
	double m_twoFingerBaseScale = 1.0;
	double m_twoFingerBaseDistance = 1.0;

	// UI矩形
	RectF m_backButtonRect{ 20, 10, 110, 50 };
	RectF m_simulationStartButtonRect{ 0, 0, 0, 0 };
	RectF m_simulationPauseButtonRect{ 0, 0, 0, 0 };
	RectF m_simulationStopButtonRect{ 0, 0, 0, 0 };
	RectF m_simulationFastForwardButtonRect{ 0, 0, 0, 0 };
	RectF m_queryPanelRect{ 0, 0, 0, 0 };
	RectF m_loadButtonRect{ 0, 0, 0, 0 };

	// レイアウト定数
	static constexpr double LeftPanelWidth = 420.0;
	static constexpr double RankingStartY = 100.0;
	static constexpr double RankingRowHeight = 50.0;
	static constexpr double RankingRowSpacing = 6.0;
	static constexpr double RankingRowWidth = 390.0;

	// ヘルパー
	void updateRanking(Game& game, double dt);
	void drawRanking(const Game& game) const;
	void updateViewer(Game& game, double dt);
	void drawViewer(const Game& game) const;

	void enterViewer(Game& game, int32 recordIndex);
	void exitViewer();
	void layoutViewerUI();

	void runSimulation(bool singleQuery, int32 queryIndex = 0);
};
