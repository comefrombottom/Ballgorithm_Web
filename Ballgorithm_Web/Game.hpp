#pragma once

# include <Siv3D.hpp>
# include "Stage.hpp"
# include "StageUI.hpp"

class StageSelectScene;
class TitleScene;

enum class GameState
{
	Title,
	StageSelect,
	Playing
};

enum class TransitionState
{
	None,
	FadeOut,
	FadeIn
};

class Game {
	Array<std::unique_ptr<Stage>> m_stages;
	HashTable<String, size_t> m_stageNameToIndex;
	std::unique_ptr<StageUI> m_stageUI;
	Optional<size_t> m_currentStageIndex;
	size_t m_selectedStageIndex = 0;
	size_t m_lastStageIndex = 0;
	std::unique_ptr<StageSelectScene> m_stageSelectScene;
	std::unique_ptr<TitleScene> m_titleScene;
	GameState m_state = GameState::Title;

	// 画面遷移用
	GameState m_nextState = GameState::Title;
	TransitionState m_transitionState = TransitionState::None;
	double m_transitionTimer = 0.0;
	static constexpr double TransitionTime = 0.4; // 秒

	void startTransition(GameState nextState);
	void onTransitionFinished();

public:
	Game();
	~Game();
	void selectStage(size_t index);
	void selectNextStage(bool isRepeated = true);
	void selectPrevStage(bool isRepeated = true);
	void enterSelectedStage();
	void exitStage();
	void goToStageSelect();
	void goToNextStage();
	const Array<std::unique_ptr<Stage>>& getStages() const { return m_stages; }
	size_t getSelectedStageIndex() const { return m_selectedStageIndex; }
	void update();
	void draw() const;
};
