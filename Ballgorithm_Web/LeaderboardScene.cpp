# include "LeaderboardScene.hpp"
# include "Game.hpp"
# include "Stage.hpp"

namespace {
	constexpr std::array<double, 4> kSimulationSpeeds = { 1.0, 2.0, 4.0, 8.0 };
}

LeaderboardScene::LeaderboardScene()
{
	Camera2DParameters params = m_viewerCamera.getParameters();
	params.minScale = (1.0 / 8.0);
	params.maxScale = 8.0;
	m_viewerCamera.setParameters(params);
}

void LeaderboardScene::enter(Game& game, int32 stageIndex)
{
	m_stageIndex = stageIndex;
	m_viewerActive = false;
	m_records.clear();
	m_hoveredRecordIndex.reset();
	m_selectedRecordIndex.reset();
	m_isLoading = true;
	m_loadFailed = false;

	m_rankingScrollBar = ScrollBar(
		100,
		Scene::Height() - RankingStartY - 20,
		RectF{ Arg::topRight(LeftPanelWidth - 2, RankingStartY), 16, Scene::Height() - RankingStartY - 20 }
	);

	layoutViewerUI();

	// リーダーボード取得開始
	const auto& stage = game.m_stages[stageIndex];
	m_leaderboardTask = StageRecord::CreateGetLeaderboradTask(stage->m_name);
}

void LeaderboardScene::exit()
{
	m_records.clear();
	m_hoveredRecordIndex.reset();
	m_selectedRecordIndex.reset();
	m_viewerActive = false;
	m_isLoading = false;
	m_loadFailed = false;
}

void LeaderboardScene::layoutViewerUI()
{
	const double rightX = LeftPanelWidth;
	const double btnH = 40;
	const double btnY = 10;
	const double gap = 8;

	double x = rightX + 10;
	m_simulationStartButtonRect = RectF{ x, btnY, 80, btnH };
	x += 80 + gap;
	m_simulationPauseButtonRect = RectF{ x, btnY, 80, btnH };
	x += 80 + gap;
	m_simulationStopButtonRect = RectF{ x, btnY, 80, btnH };
	x += 80 + gap;
	m_simulationFastForwardButtonRect = RectF{ x, btnY, 55, btnH };

	const double panelMargin = 10;
	const double panelW = 220;
	const double panelTop = 60;
	const double panelBottomMargin = 80;
	const double panelH = Max(200.0, Scene::Height() - panelTop - panelBottomMargin);
	m_queryPanelRect = RectF{ Scene::Width() - panelMargin - panelW, panelTop, panelW, panelH };
	m_viewerQueryPanel.setRect(m_queryPanelRect);

	const double loadW = m_queryPanelRect.w;
	const double loadH = 40;
	const double loadY = m_queryPanelRect.y + m_queryPanelRect.h + 8;
	m_loadButtonRect = RectF{ m_queryPanelRect.x, loadY, loadW, loadH };
}

void LeaderboardScene::enterViewer(Game& game, int32 recordIndex)
{
	auto& originalStage = *game.m_stages[m_stageIndex].get();

	// 既にビュワーが有効な場合、元のスナップショットに復元してから切り替え
	if (m_viewerActive) {
		if (m_viewerStage.m_isSimulationRunning) {
			m_viewerStage.endSimulation();
		}
	}

	m_selectedRecordIndex = recordIndex;
	m_viewerActive = true;

	// スナップショットを一時ステージに適用（元のステージのクエリを使う）
	m_viewerStage.restoreRecord(m_records[recordIndex]);
	m_viewerStage.m_queries = originalStage.m_queries;
	m_viewerStage.resetQueryProgress();

	m_viewerCamera.setCenter(Vec2{ 150,300 });
	m_viewerCamera.setTargetCenter(Vec2{ 150,300 });
	m_viewerCamera.setScale(0.8);
	m_viewerCamera.setTargetScale(0.8);

	layoutViewerUI();
	m_viewerQueryPanel.onStageEnter(m_viewerStage);
	m_singleQueryMode = false;
	m_viewerStage.m_simulationSpeed = kSimulationSpeeds[m_speedIndex];
}

void LeaderboardScene::exitViewer()
{
	m_viewerActive = false;
	m_selectedRecordIndex.reset();
}

void LeaderboardScene::update(Game& game, double dt)
{
	m_cursorPos.init();

	// 非同期タスクの完了チェック
	if (m_isLoading && !m_leaderboardTask.isEmpty() && m_leaderboardTask.isReady())
	{
		m_records = StageRecord::ProcessGetLeaderboardTask(m_leaderboardTask);
		m_isLoading = false;
		m_loadFailed = m_records.empty();

		double pageHeight = m_records.size() * (RankingRowHeight + RankingRowSpacing) + 20;
		m_rankingScrollBar.pageHeight = pageHeight;
	}

	// ランキングとビュワーを同時に更新
	updateRanking(game, dt);
	if (m_viewerActive) {
		updateViewer(game, dt);
	}
}

void LeaderboardScene::draw(const Game& game) const
{
	// 背景
	Rect{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF(0.1, 0.1, 0.2), Arg::bottom = ColorF(0.05, 0.15, 0.25));

	// 左パネル: ランキング
	drawRanking(game);

	// 右パネル: ビュワー
	drawViewer(game);

	// パネル境界線
	Line{ LeftPanelWidth, 0, LeftPanelWidth, Scene::Height() }.draw(2, ColorF(0.3, 0.4, 0.5, 0.6));
}

// ========== Ranking ==========

void LeaderboardScene::updateRanking(Game& game, double dt)
{
	// Back ボタン
	if (m_cursorPos.intersects_use(m_backButtonRect)) {
		if (MouseL.down()) {
			if (m_viewerActive) {
				if (m_viewerStage.m_isSimulationRunning) {
					m_viewerStage.endSimulation();
				}
				exitViewer();
			}
			game.exitLeaderboard();
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	const bool cursorInRanking = (Cursor::PosF().x < LeftPanelWidth);
	m_rankingScrollBar.update(m_cursorPos, cursorInRanking ? Mouse::Wheel() : 0.0, dt);

	if (m_cursorPos) {
		if (MouseL.down() && cursorInRanking) {
			m_mousePressPos = Cursor::PosF();
			m_isDragging = false;
			m_cursorPos.capture();
		}
	}

	if (m_mousePressPos) {
		if (m_mousePressPos->distanceFrom(Cursor::PosF()) > 20) {
			m_isDragging = true;
		}
		if (m_isDragging) {
			m_rankingScrollBar.viewVelocity = -Cursor::DeltaF().y / dt;
		}
	}

	bool clickInput = false;
	if (MouseL.up()) {
		if (m_mousePressPos) {
			if (!m_isDragging) {
				clickInput = true;
			}
			m_mousePressPos.reset();
			m_isDragging = false;
			m_cursorPos.release();
		}
	}

	// ホバー・クリック
	m_hoveredRecordIndex.reset();
	if (!m_isLoading && !m_records.empty()) {
		auto scrollTf = m_rankingScrollBar.createTransformer();
		for (int32 i = 0; i < m_records.size(); ++i) {
			double x = (LeftPanelWidth - RankingRowWidth) / 2.0;
			double y = RankingStartY + i * (RankingRowHeight + RankingRowSpacing);
			RectF rowRect{ x, y, RankingRowWidth, RankingRowHeight };

			if (rowRect.mouseOver()) {
				m_hoveredRecordIndex = i;
				Cursor::RequestStyle(CursorStyle::Hand);

				if (clickInput) {
					enterViewer(game, i);
				}
			}
		}
	}
}

void LeaderboardScene::drawRanking(const Game& game) const
{
	const Font& font = FontAsset(U"Regular");
	const Font& iconFont = FontAsset(U"Icon");

	// 左パネル背景
	RectF{ 0, 0, LeftPanelWidth, Scene::Height() }
		.draw(ColorF(0.08, 0.1, 0.15, 0.95));

	// ツールバー
	RectF toolbarBg{ 0, 0, LeftPanelWidth, 70 };
	toolbarBg.draw(ColorF(0.1, 0.12, 0.16, 0.95));
	toolbarBg.drawFrame(0, 2, ColorF(0.2, 0.25, 0.3));

	// Back ボタン
	{
		bool hovered = m_backButtonRect.mouseOver();
		ColorF bg = hovered ? ColorF(0.5, 0.55, 0.6) : ColorF(0.4, 0.45, 0.5);
		m_backButtonRect.movedBy(2, 2).rounded(8).draw(ColorF(0.0, 0.3));
		m_backButtonRect.rounded(8).draw(bg);
		m_backButtonRect.rounded(8).drawFrame(1, ColorF(1.0, 0.1));
		double iconW = iconFont(U"\uF060").region(18).w;
		double totalW = iconW + 8 + font(U"Back").region(13).w;
		iconFont(U"\uF060").draw(18, Arg::leftCenter = Vec2(m_backButtonRect.center().x - totalW / 2, m_backButtonRect.center().y), ColorF(1.0));
		font(U"Back").draw(13, Arg::leftCenter = Vec2(m_backButtonRect.center().x - totalW / 2 + iconW + 8, m_backButtonRect.center().y), ColorF(1.0));
	}

	// ステージ名
	if (m_stageIndex < game.m_stages.size()) {
		const String& stageName = game.m_stages[m_stageIndex]->m_name;
		font(stageName).draw(14, Vec2{ 140, 25 }, ColorF(0.95));
		font(U"Leaderboard").draw(11, Vec2{ 140, 48 }, ColorF(0.6));
	}

	// ローディング
	if (m_isLoading) {
		font(U"Loading...").draw(16, Arg::center = Vec2{ LeftPanelWidth / 2.0, Scene::Height() / 2.0 }, ColorF(0.7));
		return;
	}

	if (m_loadFailed) {
		font(U"No records found.").draw(16, Arg::center = Vec2{ LeftPanelWidth / 2.0, Scene::Height() / 2.0 }, ColorF(0.6));
		return;
	}

	// ヘッダー行
	{
		double x = (LeftPanelWidth - RankingRowWidth) / 2.0;
		double y = RankingStartY - 22;
		font(U"#").draw(11, Vec2{ x + 10, y }, ColorF(0.6));
		font(U"Player").draw(11, Vec2{ x + 40, y }, ColorF(0.6));
		font(U"Obj").draw(11, Vec2{ x + 260, y }, ColorF(0.6));
		font(U"Len").draw(11, Vec2{ x + 330, y }, ColorF(0.6));
	}

	// ランキング行
	Rect clipRect{ 0, static_cast<int>(RankingStartY) - 5, static_cast<int>(LeftPanelWidth), Scene::Height() - static_cast<int>(RankingStartY) - 15 };
	{
		Graphics2D::SetScissorRect(clipRect);
		RasterizerState rs = RasterizerState::Default2D;
		rs.scissorEnable = true;
		const ScopedRenderStates2D rasterizer{ rs };

		auto scrollTf = m_rankingScrollBar.createTransformer();

		for (int32 i = 0; i < m_records.size(); ++i) {
			double x = (LeftPanelWidth - RankingRowWidth) / 2.0;
			double y = RankingStartY + i * (RankingRowHeight + RankingRowSpacing);
			RectF rowRect{ x, y, RankingRowWidth, RankingRowHeight };

			bool isSelected = m_selectedRecordIndex && *m_selectedRecordIndex == i;
			bool isHovered = m_hoveredRecordIndex && *m_hoveredRecordIndex == i;
			ColorF bgColor = isSelected ? ColorF(0.3, 0.45, 0.65, 0.9) : isHovered ? ColorF(0.25, 0.35, 0.5, 0.85) : ColorF(0.15, 0.2, 0.3, 0.8);

			rowRect.movedBy(3, 3).rounded(8).draw(ColorF(0.0, 0.25));
			rowRect.rounded(8).draw(bgColor);
			rowRect.rounded(8).drawFrame(1, isSelected ? ColorF(0.6, 0.8, 1.0, 0.8) : isHovered ? ColorF(0.5, 0.7, 1.0, 0.7) : ColorF(0.3, 0.4, 0.5, 0.4));

			// 順位
			Circle badge{ rowRect.x + 22, rowRect.center().y, 14 };
			ColorF badgeColor = (i == 0) ? ColorF(0.85, 0.7, 0.2) : (i == 1) ? ColorF(0.65, 0.65, 0.7) : (i == 2) ? ColorF(0.7, 0.5, 0.3) : ColorF(0.3, 0.35, 0.4);
			badge.draw(badgeColor);
			font(U"{}"_fmt(i + 1)).draw(12, Arg::center = badge.center, Palette::White);

			// プレイヤー名
			font(m_records[i].m_author).draw(14, Arg::leftCenter = Vec2{ rowRect.x + 40, rowRect.center().y }, ColorF(0.95));

			// オブジェクト数
			font(U"{}"_fmt(m_records[i].m_numberOfObjects)).draw(12, Arg::leftCenter = Vec2{ rowRect.x + 260, rowRect.center().y }, ColorF(0.8));

			// 長さ
			font(U"{}"_fmt(m_records[i].m_totalLength)).draw(12, Arg::leftCenter = Vec2{ rowRect.x + 330, rowRect.center().y }, ColorF(0.8));
		}
	}

	m_rankingScrollBar.draw(ColorF(0.5, 0.6, 0.7, 0.6));
}

// ========== Viewer ==========

void LeaderboardScene::updateViewer(Game& game, double dt)
{
	auto& stage = m_viewerStage;


	bool hasTwoFingerTouch = false;
	// === 2本指タッチでカメラ操作（パン/ピンチ）===
	// UI操作と衝突させないため、2本指がある間は cursorPos を消費してステージ編集入力を抑制する
	{
		auto touches = Touches.unused().presseds();
		if (touches.size() >= 2)
		{
			hasTwoFingerTouch = true;
			TouchInfo& t0 = touches.front();
			TouchInfo& t1 = touches.back();

			const Vec2 center = (t0.pos + t1.pos) * 0.5;
			const double distance = (t0.pos - t1.pos).length();

			const bool shouldStart = (!m_isTwoFingerGesturing)
				|| (m_twoFingerId0 != t0.id)
				|| (m_twoFingerId1 != t1.id);

			if (shouldStart)
			{
				m_isTwoFingerGesturing = true;
				m_twoFingerId0 = t0.id;
				m_twoFingerId1 = t1.id;
				m_prevTwoFingerCenter = center;
				m_prevTwoFingerDistance = Max(10.0, distance);
				m_twoFingerBaseDistance = Max(10.0, distance);
				m_twoFingerBaseScale = m_viewerCamera.getScale();
			}
			else
			{
				// パン：中心の移動分だけカメラ中心を逆方向に動かす
				const Vec2 deltaCenter = center - m_prevTwoFingerCenter;
				if (deltaCenter != Vec2{ 0, 0 })
				{
					// deltaCenter はスクリーン座標なのでワールド座標に変換（scale に依存）
					const Vec2 newCenter = m_viewerCamera.getCenter() - (deltaCenter / m_viewerCamera.getScale());
					m_viewerCamera.setTargetCenter(newCenter);
					m_viewerCamera.setCenter(newCenter);
				}

				// ピンチ：2点間距離の比率で scale 更新
				const double safeBaseDist = Max(1.0, m_twoFingerBaseDistance);
				const double ratio = Max(0.01, distance / safeBaseDist);
				const double targetScale = m_twoFingerBaseScale * ratio;
				m_viewerCamera.zoomAtImmediate(center, targetScale);

				m_prevTwoFingerCenter = center;
				m_prevTwoFingerDistance = Max(1.0, distance);
			}

			// 2本指ジェスチャー中は他の入力を抑制
			t0.use();
			t1.use();
			// m_cursorPos.use();
		}
		else
		{
			m_isTwoFingerGesturing = false;
			m_twoFingerId0 = -1;
			m_twoFingerId1 = -1;
			m_prevTwoFingerDistance = 0.0;
			m_twoFingerBaseDistance = 1.0;
		}
	}

	// クエリパネル更新
	if (m_viewerQueryPanel.update(stage, m_cursorPos, dt)) {
		m_singleQueryMode = true;
	}

	// Run ボタン
	if (m_cursorPos.intersects_use(m_simulationStartButtonRect)) {
		if (MouseL.down() && !stage.m_isSimulationRunning) {
			stage.m_currentQueryIndex = 0;
			m_singleQueryMode = false;
			stage.startSimulation();
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	// Pause ボタン
	if (m_cursorPos.intersects_use(m_simulationPauseButtonRect)) {
		if (MouseL.down() && stage.m_isSimulationRunning) {
			stage.m_isSimulationPaused = !stage.m_isSimulationPaused;
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	// Stop ボタン
	if (m_cursorPos.intersects_use(m_simulationStopButtonRect)) {
		if (MouseL.down() && stage.m_isSimulationRunning) {
			stage.endSimulation();
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	// Fast Forward ボタン
	if (m_cursorPos.intersects_use(m_simulationFastForwardButtonRect)) {
		if (MouseL.down()) {
			m_speedIndex = (m_speedIndex + 1) % static_cast<int32>(kSimulationSpeeds.size());
			stage.m_simulationSpeed = kSimulationSpeeds[m_speedIndex];
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	//// Load ボタン
	//if (m_cursorPos.intersects_use(m_loadButtonRect)) {
	//	if (MouseL.down() && !stage.m_isSimulationRunning) {
	//		// 現在のスナップショット（他者の解法）をそのまま残してPlayingへ遷移
	//		if (stage.m_isSimulationRunning) {
	//			stage.endSimulation();
	//		}
	//		stage.resetQueryProgress();
	//		game.loadLeaderboardSolution();
	//		return;
	//	}
	//	Cursor::RequestStyle(CursorStyle::Hand);
	//}

	// シミュレーション更新
	if (stage.m_isSimulationRunning && !stage.m_isSimulationPaused) {
		double fallThreshold = stage.getLowestY() + 100;
		for (auto& c : stage.m_startBallsInWorld) {
			if (c.body.getPos().y > fallThreshold) {
				c.body.release();
			}
		}
		stage.m_startBallsInWorld.remove_if([](const auto& c) { return c.body.isEmpty(); });

		bool allFinished = true;
		for (const auto& c : stage.m_startBallsInWorld) {
			if (c.body.isEmpty()) continue;
			const bool finishedBySleep = !c.body.isAwake();
			const bool finishedByGoal = (c.timeSinceEnteredGoal && (*c.timeSinceEnteredGoal >= 1.0));
			allFinished &= (finishedBySleep || finishedByGoal);
		}

		bool hasFinishedReleasing = true;
		if (stage.m_currentQueryIndex < stage.m_queries->size()) {
			hasFinishedReleasing = (*stage.m_queries)[stage.m_currentQueryIndex]->hasFinishedReleasing();
		}

		if (allFinished && hasFinishedReleasing) {
			bool isSuccess = stage.checkSimulationResult();
			int32 completedQueryIndex = stage.m_currentQueryIndex;

			if (isSuccess) {
				stage.markQueryCompleted(completedQueryIndex);
			}
			else {
				stage.markQueryFailed(completedQueryIndex);
			}

			stage.endSimulation();

			if (m_singleQueryMode) {
				m_singleQueryMode = false;
			}
			else {
				if (completedQueryIndex + 1 < stage.m_queries->size()) {
					stage.m_currentQueryIndex = completedQueryIndex + 1;
					stage.startSimulation();
				}
				else {
					stage.m_currentQueryIndex = 0;
				}
			}
		}
	}

	// カメラ
	const bool cursorInViewer = (Cursor::PosF().x >= LeftPanelWidth);
	if (m_cursorPos && cursorInViewer) {
		if (MouseR.pressed() || MouseM.pressed()) {
			auto cameraTf = m_viewerCamera.createTransformer();
			Vec2 pos = m_viewerCamera.getCenter() - (MouseR.down() ? Vec2::Zero() : Cursor::DeltaF());
			m_viewerCamera.setTargetCenter(pos);
			m_viewerCamera.setCenter(pos);
		}
		m_viewerCamera.updateWheel();
	}
	m_viewerCamera.update(dt);

	// 物理シミュレーションステップ
	if (stage.m_isSimulationRunning && !stage.m_isSimulationPaused) {
		auto cameraTf = m_viewerCamera.createTransformer();
		for (stage.m_simulationTimeAccumlate += dt * stage.m_simulationSpeed;
			stage.m_simulationTimeAccumlate >= Stage::simulationTimeStep;
			stage.m_simulationTimeAccumlate -= Stage::simulationTimeStep)
		{
			stage.m_world.update(Stage::simulationTimeStep);

			for (auto& b : stage.m_startBallsInWorld) {
				if (b.body.isEmpty()) {
					b.timeSinceEnteredGoal = none;
					continue;
				}
				const Vec2 pos = b.body.getPos();
				bool inGoal = false;
				for (const auto& g : stage.m_goalAreas) {
					if (g.rect.contains(pos)) {
						inGoal = true;
						break;
					}
				}
				if (inGoal) {
					if (b.timeSinceEnteredGoal) {
						*b.timeSinceEnteredGoal += Stage::simulationTimeStep;
					}
					else {
						b.timeSinceEnteredGoal = 0.0;
					}
				}
				else {
					b.timeSinceEnteredGoal = none;
				}
			}

			if (stage.m_currentQueryIndex < stage.m_queries->size()) {
				(*stage.m_queries)[stage.m_currentQueryIndex]->update(stage, Stage::simulationTimeStep);
			}

			for (auto& c : stage.m_startBallsInWorld) {
				Vec2 vel = c.body.getVelocity();
				double angleV = c.body.getAngularVelocity();
				if (abs(vel.y) < 0.001 and abs(vel.x) < 2 and abs(angleV) < 0.1) {
					c.body.setVelocity({ 0,0 });
					c.body.setAngularVelocity(0);
				}
			}
		}
	}
}

void LeaderboardScene::drawViewer(const Game& game) const
{
	auto& stage = m_viewerStage;
	const Font& font = FontAsset(U"Regular");
	const Font& iconFont = FontAsset(U"Icon");

	const double rightX = LeftPanelWidth;
	const double rightW = Scene::Width() - LeftPanelWidth;

	if (!m_viewerActive) {
		// ビュワー未選択時のプレースホルダー
		font(U"← Select a record to view").draw(16, Arg::center = Vec2{ rightX + rightW / 2.0, Scene::Height() / 2.0 }, ColorF(0.4));
		return;
	}

	// 右パネル背景
	RectF{ rightX, 0, rightW, Scene::Height() }
		.draw(Arg::top = ColorF(0.12, 0.14, 0.18), Arg::bottom = ColorF(0.08, 0.1, 0.14));

	// ワールド描画（右パネル内にクリップ）
	{
		Rect viewerClip{ static_cast<int>(rightX), 0, static_cast<int>(rightW), Scene::Height() };
		Graphics2D::SetScissorRect(viewerClip);
		RasterizerState rs = RasterizerState::Default2D;
		rs.scissorEnable = true;
		const ScopedRenderStates2D rasterizer{ rs };

		auto cameraTf = m_viewerCamera.createTransformer();
		m_viewerEditUI.drawWorld(stage, m_viewerCamera);
	}

	// ツールバー
	RectF toolbarBg{ rightX, 0, rightW, 55 };
	toolbarBg.draw(ColorF(0.1, 0.12, 0.16, 0.95));
	toolbarBg.drawFrame(0, 2, ColorF(0.2, 0.25, 0.3));

	// ボタン描画ラムダ
	auto drawButton = [&](const RectF& rect, const String& text, const String& icon, ColorF baseColor, bool enabled, bool hovered, int32 textSize = 12) {
		ColorF bg = enabled ? baseColor : ColorF(0.25, 0.28, 0.32);
		if (hovered && enabled) {
			bg = baseColor.lerp(ColorF(1.0), 0.2);
		}
		rect.movedBy(2, 2).rounded(6).draw(ColorF(0.0, 0.3));
		rect.rounded(6).draw(bg);
		rect.rounded(6).drawFrame(1, ColorF(1.0, 0.1));
		ColorF textColor = enabled ? ColorF(1.0) : ColorF(0.5);
		double iconW = iconFont(icon).region(16).w;
		double totalW = iconW + 6 + font(text).region(textSize).w;
		iconFont(icon).draw(16, Arg::leftCenter = Vec2(rect.center().x - totalW / 2, rect.center().y), textColor);
		font(text).draw(textSize, Arg::leftCenter = Vec2(rect.center().x - totalW / 2 + iconW + 6, rect.center().y), textColor);
	};

	// 作者名
	if (m_selectedRecordIndex && *m_selectedRecordIndex < m_records.size()) {
		const auto& record = m_records[*m_selectedRecordIndex];
		font(U"{} 's solution"_fmt(record.m_author)).draw(18, Arg::leftCenter=Vec2{ m_simulationFastForwardButtonRect.rightX() + 10, 30 }, ColorF(0.95));
	}

	// シミュレーションボタン
	bool simRunning = stage.m_isSimulationRunning;
	drawButton(m_simulationStartButtonRect, U"Run", U"\uF04B", ColorF(0.3, 0.7, 0.4), !simRunning, m_simulationStartButtonRect.mouseOver());

	String pauseText = stage.m_isSimulationPaused ? U"Resume" : U"Pause";
	String pauseIcon = stage.m_isSimulationPaused ? U"\uF04B" : U"\uF04C";
	ColorF pauseColor = stage.m_isSimulationPaused ? ColorF(0.7, 0.6, 0.2) : ColorF(0.8, 0.5, 0.2);
	drawButton(m_simulationPauseButtonRect, pauseText, pauseIcon, pauseColor, simRunning, m_simulationPauseButtonRect.mouseOver());

	drawButton(m_simulationStopButtonRect, U"Stop", U"\uF04D", ColorF(0.7, 0.3, 0.3), simRunning, m_simulationStopButtonRect.mouseOver());

	String ffText = U"{}x"_fmt(static_cast<int>(kSimulationSpeeds[m_speedIndex]));
	ColorF ffColor = m_speedIndex != 0 ? ColorF(0.3, 0.6, 0.8) : ColorF(0.4, 0.5, 0.6);
	drawButton(m_simulationFastForwardButtonRect, ffText, U"\uF04E", ffColor, true, m_simulationFastForwardButtonRect.mouseOver());

	// クエリパネル
	m_viewerQueryPanel.draw(stage);

	// Load ボタン
	/*{
		bool enabled = !simRunning;
		bool hovered = m_loadButtonRect.mouseOver();
		drawButton(m_loadButtonRect, U"Load Solution", U"\uF019", ColorF(0.3, 0.5, 0.7), enabled, hovered, 14);
	}*/
}
