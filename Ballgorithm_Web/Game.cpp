# include "Game.hpp"
# include "StageSelectScene.hpp"
# include "TitleScene.hpp"
# include "Game_StagesConstruct.h"

Game::Game()
{
	stagesConstruct(m_stages);

	for (auto& file : FileSystem::DirectoryContents(U"Ballagorithm/Stages")) {
		Deserializer<BinaryReader> deserializer{ file };

		try {
			for (auto& stage : m_stages) {
				if (stage->m_name == FileSystem::BaseName(file)) {
					StageSnapshot snapshot{};
					deserializer(snapshot);
					deserializer(stage->m_queryCompleted);
					deserializer(stage->m_queryFailed);
					deserializer(stage->m_isCleared);
					stage->restoreSnapshot(snapshot);
				}
			}
		}
		catch (...) {
			Console << U"[Error] failed to open the save file '{{}}'"_fmt(file);
			FileSystem::Remove(file);
		};
	}

	m_stageUI = std::make_unique<StageUI>();
	m_stageSelectScene = std::make_unique<StageSelectScene>();
	m_titleScene = std::make_unique<TitleScene>();
}

Game::~Game() = default;

void Game::startTransition(GameState nextState)
{
	if (m_transitionState == TransitionState::None)
	{
		m_nextState = nextState;
		m_transitionState = TransitionState::FadeOut;
		m_transitionTimer = 0.0;
	}
}

void Game::goToNextStage()
{
	if (m_stages.empty()) return;
	if (m_state != GameState::Playing) return;
	if (m_transitionState != TransitionState::None) return;
	if (!m_currentStageIndex) return;

	m_selectedStageIndex = (*m_currentStageIndex + 1) % m_stages.size();

	// ここで onStage
	// + currentStageIndex reset をしてしまうと、FadeOut中に描画するものが無くなり
	// 背景色(クリア色)が見えてしまう。切り替えは onTransitionFinished() に寄せる。
	startTransition(GameState::Playing);
}

void Game::goToStageSelect()
{
	startTransition(GameState::StageSelect);
}

void Game::selectStage(size_t index)
{
	if (index >= m_stages.size()) {
		return;
	}

	m_selectedStageIndex = index;
}

void Game::enterSelectedStage()
{
	if (m_selectedStageIndex >= m_stages.size()) {
		return;
	}

	// 既にPlayingで同じステージなら何もしない
	if (m_state == GameState::Playing && m_currentStageIndex && (*m_currentStageIndex == m_selectedStageIndex)) {
		return;
	}

	startTransition(GameState::Playing);
}

void Game::selectNextStage(bool isRepeated)
{
	if (isRepeated) {
		m_selectedStageIndex = (m_selectedStageIndex + 1) % m_stages.size();
	}
	else {
		if (m_selectedStageIndex + 1 < m_stages.size()) {
			m_selectedStageIndex++;
		}
	}
}

void Game::selectPrevStage(bool isRepeated)
{
	if (isRepeated) {
		m_selectedStageIndex = (m_selectedStageIndex + m_stages.size() - 1) % m_stages.size();
	}
	else {
		if (m_selectedStageIndex > 0) {
			m_selectedStageIndex--;
		}
	}
}

void Game::exitStage()
{
	startTransition(GameState::StageSelect);
}

void Game::onTransitionFinished()
{
	// 状態切り替え時のロジック
	if (m_nextState == GameState::Playing)
	{
		// Playing -> Playing のステージ切り替え
		if (m_state == GameState::Playing && m_currentStageIndex && (*m_currentStageIndex != m_selectedStageIndex))
		{
			m_stageUI->onStageExit(*m_stages[*m_currentStageIndex]);
			m_currentStageIndex.reset();
		}

		// ステージ開始
		if (m_selectedStageIndex < m_stages.size())
		{

			m_currentStageIndex = m_selectedStageIndex;
			m_stageUI->onStageEnter(*m_stages[m_selectedStageIndex], m_lastStageIndex == m_selectedStageIndex);
			m_lastStageIndex = m_selectedStageIndex;
		}
	}
	else if (m_nextState == GameState::StageSelect)
	{
		// ステージ終了（Playing -> StageSelect の場合）
		if (m_state == GameState::Playing && m_currentStageIndex)
		{
			m_stageUI->onStageExit(*m_stages[*m_currentStageIndex]);
			m_currentStageIndex.reset();
		}
	}
	
	// 状態更新
	m_state = m_nextState;
}

void Game::update()
{
	double dt = Scene::DeltaTime();

	// 遷移更新
	if (m_transitionState != TransitionState::None)
	{
		m_transitionTimer += dt;
		
		if (m_transitionState == TransitionState::FadeOut)
		{
			if (m_transitionTimer >= TransitionTime)
			{
				// フェードアウト完了 -> 状態切り替え -> フェードイン開始
				onTransitionFinished();
				m_transitionState = TransitionState::FadeIn;
				m_transitionTimer = 0.0;
			}
		}
		else if (m_transitionState == TransitionState::FadeIn)
		{
			if (m_transitionTimer >= TransitionTime)
			{
				// フェードイン完了
				m_transitionState = TransitionState::None;
				m_transitionTimer = 0.0;
			}
		}
	}

	// フェードアウト中は入力を受け付けない（更新を止める）
	if (m_transitionState == TransitionState::FadeOut)
	{
		return;
	}

	switch (m_state)
	{
	case GameState::Title:
		m_titleScene->update(*this);
		break;
	case GameState::StageSelect:
		m_stageSelectScene->update(*this, dt);
		break;
	case GameState::Playing:
		if (m_currentStageIndex) {
			m_stageUI->update(*this, *m_stages[*m_currentStageIndex], dt);
		}
		break;
	}
}

void Game::draw() const
{
	switch (m_state)
	{
	case GameState::Title:
		m_titleScene->draw();
		break;
	case GameState::StageSelect:
		m_stageSelectScene->draw(*this);
		break;
	case GameState::Playing:
		if (m_currentStageIndex) {
			m_stageUI->draw(*m_stages[*m_currentStageIndex]);
		}
		break;
	}

	// フェード描画
	if (m_transitionState != TransitionState::None)
	{
		const double t = Min(m_transitionTimer / TransitionTime, 1.0);
		const double w = Scene::Width();
		const double h = Scene::Height();
		const double offset = h * 0.4; // 斜めの傾き具合
		const double rectWidth = w + offset * 2; // 画面を覆うのに十分な幅

		// 濃い紺色でおしゃれに
		const ColorF transitionColor(0.05, 0.08, 0.12);

		if (m_transitionState == TransitionState::FadeOut)
		{
			// 右から左へ入ってくる（閉じる）
			// EaseOut: 素早く入ってきてゆっくり止まる
			double easeT = EaseOutCubic(t);
			
			// 左端のX座標: 画面右外 -> 画面左端（画面を覆う）
			double startX = w;
			double endX = -offset;
			double currentX = Math::Lerp(startX, endX, easeT);

			Array<Vec2> points = {
				Vec2(currentX + offset, 0),
				Vec2(currentX + offset + rectWidth, 0),
				Vec2(currentX + rectWidth, h),
				Vec2(currentX, h)
			};
			Polygon(points).draw(transitionColor);
		}
		else // FadeIn
		{
			// 右から左へ出ていく（開く）
			// EaseIn: ゆっくり動き出して素早く去る
			double easeT = EaseInCubic(t);

			// 左端のX座標: 画面左端 -> 画面左外（画面を開ける）
			double startX = -offset;
			double endX = -offset - w - offset; // 完全に通り過ぎる位置
			double currentX = Math::Lerp(startX, endX, easeT);

			Array<Vec2> points = {
				Vec2(currentX + offset, 0),
				Vec2(currentX + offset + rectWidth, 0),
				Vec2(currentX + rectWidth, h),
				Vec2(currentX, h)
			};
			Polygon(points).draw(transitionColor);
		}
	}
}
