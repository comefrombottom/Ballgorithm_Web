#pragma once

# include <Siv3D.hpp>
# include "Stage.hpp"
# include "StageUI.hpp"

class StageSelectScene;
class TitleScene;
class LeaderboardScene;

enum class GameState
{
	Title,
	StageSelect,
	Playing,
	Leaderboard
};

enum class TransitionState
{
	None,
	FadeOut,
	FadeIn
};

class Game {
public:
	String m_username;
	Array<std::unique_ptr<Stage>> m_stages;
	HashTable<String, int32> m_stageNameToIndex;
	std::unique_ptr<StageUI> m_stageUI;
	Optional<int32> m_currentStageIndex;
	int32 m_selectedStageIndex = 0;
	int32 m_lastStageIndex = 0;
	std::unique_ptr<StageSelectScene> m_stageSelectScene;
	std::unique_ptr<TitleScene> m_titleScene;
	std::unique_ptr<LeaderboardScene> m_leaderboardScene;
	GameState m_state = GameState::Title;

	// 画面遷移用
	GameState m_nextState = GameState::Title;
	TransitionState m_transitionState = TransitionState::None;
	double m_transitionTimer = 0.0;
	static constexpr double TransitionTime = 0.4; // 秒

	AsyncHTTPTask m_postTask;

	void startTransition(GameState nextState);
	void onTransitionFinished();

	Game();
	~Game();
	void selectStage(int32 index);
	void selectNextStage(bool isRepeated = true);
	void selectPrevStage(bool isRepeated = true);
	void enterSelectedStage();
	void exitStage();
	void goToStageSelect();
	void goToNextStage();
	void enterLeaderboard(int32 stageIndex);
	void exitLeaderboard();
	void loadLeaderboardSolution();
	const Array<std::unique_ptr<Stage>>& getStages() const { return m_stages; }
	int32 getSelectedStageIndex() const { return m_selectedStageIndex; }
	void update();
	void draw() const;
};
