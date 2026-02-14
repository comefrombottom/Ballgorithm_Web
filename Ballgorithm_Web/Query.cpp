# include "Query.hpp"
# include "Stage.hpp"

namespace {
	static bool CheckGoalRequirements(const Stage& stage, const Array<GoalRequirement>& goalRequirements)
	{
		Array<Array<BallKind>> reachedGoalAreas(stage.m_goalAreas.size());
		for (auto [i, r] : Indexed(stage.m_goalAreas)) {
			for (const auto& c : stage.m_startBallsInWorld) {
				if (r.rect.contains(c.body.getPos())) {
					reachedGoalAreas[i].push_back(c.kind);
				}
			}
		}

		if (reachedGoalAreas.size() != goalRequirements.size()) {
			return false;
		}

		for (int32 i = 0; i < goalRequirements.size(); ++i) {
			if (!goalRequirements[i].isSatisfiedBy(reachedGoalAreas[i])) {
				return false;
			}
		}

		return true;
	}

} // End of anonymous namespace

double MultiPhaseQuery::getPanelHeight() const
{
	// フェーズごとに「IN（シーケンス） + OUT（ゴール要件）」を積む
	const double rowHeight = 20.0;

	double total = 12.0; // top padding
	for (int32 phaseIndex = 0; phaseIndex < m_phases.size(); ++phaseIndex) {
		const auto& phase = m_phases[phaseIndex];
		const int32 sequenceLength = phase.releases.size();
		const int32 rows = Max<int32>(1, sequenceLength);

		// IN block height
		total += 34.0 + rowHeight * rows;

		// OUT block height
		int32 maxBallsInGoal = 0;
		for (const auto& req : phase.goalRequirements) {
			maxBallsInGoal = Max(maxBallsInGoal, req.totalCount());
		}
		const double maxIconHeight = Max(14.0, 4.0 + maxBallsInGoal * 14.0);
		total += maxIconHeight + 26.0;

		// Between-phase arrow gap
		if (phaseIndex + 1 < m_phases.size()) {
			total += 2.0;
		}
	}

	return Max(80.0, total);
}

double MultiPhaseQuery::drawPanelContent(const RectF& queryRect, bool isActive) const
{
	const double colWidth = 26.0;
	const double rowHeight = 20.0;
	const double labelX = queryRect.x + 38;
	const double startX = labelX + 24;

	// StartCircleの最大数（全フェーズ全イベントから）
	int32 numStartCircles = 0;
	for (const auto& phase : m_phases) {
		for (const auto& release : phase.releases) {
			numStartCircles = Max(numStartCircles, release.startBalls.size());
		}
	}

	// フェーズごとに縦に描画
	double yCursor = queryRect.y + 12;

	for (int32 phaseIndex = 0; phaseIndex < m_phases.size(); ++phaseIndex) {
		const auto& phase = m_phases[phaseIndex];
		const int32 sequenceLength = phase.releases.size();

		// phase header removed

		// --- IN block (release sequence)
		const double inBlockTop = yCursor;
		const double inStartY = inBlockTop + 12;

		// "IN" label
		double inLabelY = inStartY + (Max<int32>(1, sequenceLength) * rowHeight) / 2.0;
		FontAsset(U"Regular")(U"IN").draw(9, Vec2(labelX, inLabelY - 5), ColorF(0.5, 0.55, 0.6));

		// Header a,b,c...
		for (int32 sc = 0; sc < numStartCircles; ++sc) {
			double x = startX + sc * colWidth;
			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + sc))).drawAt(9, Vec2(x + 9, inStartY), ColorF(0.6, 0.7, 0.8));
		}

		// Active highlight for current phase/release
		Optional<int32> activeRow = none;
		if (isActive && phaseIndex == m_phaseIndex && (phase.releases.size() >= 2 or m_phases.size() >= 2)) {
			if (m_nextReleaseIndex == 0) {
				// none
			}
			else if (m_nextReleaseIndex >= phase.releases.size()) {
				activeRow = phase.releases.size() - 1;
			}
			else {
				activeRow = m_nextReleaseIndex - 1;
			}
		}
		const ColorF activeRowBg = ColorF(0.35, 0.45, 0.9, 0.12);
		const ColorF activeRowFrame = ColorF(0.45, 0.6, 1.0, 0.55);

		for (int32 row = 0; row < Max<int32>(1, sequenceLength); ++row) {
			double y = inStartY + 12 + row * rowHeight;

			if (sequenceLength == 0) {
				// no releases: show placeholder
				Circle(startX + 9, y + 4, 4).drawFrame(1, ColorF(0.4));
				break;
			}

			const auto& release = phase.releases[row];
			if (activeRow && row == activeRow.value()) {
				RectF rowRect{ startX - 6, y - 6, numStartCircles * colWidth + 12, rowHeight };
				rowRect.rounded(4).draw(activeRowBg);
				rowRect.rounded(4).drawFrame(1, activeRowFrame);
			}

			for (int32 sc = 0; sc < numStartCircles; ++sc) {
				double x = startX + sc * colWidth;
				if (sc < release.startBalls.size() && release.startBalls[sc].has_value()) {
					const auto& ball = release.startBalls[sc].value();
					double r = ball.radius() * 0.35;
					ColorF color = GetBallColor(ball.kind);
					Circle(x + 9, y + 4, r).draw(color);
					Circle(x + 9, y + 4, r).drawFrame(1, ColorF(1.0, 0.5));
				}
				else {
					Circle(x + 9, y + 4, 4).drawFrame(1, ColorF(0.4));
				}
			}

			if (row + 1 < sequenceLength) {
				const auto& nextRelease = phase.releases[row + 1];
				String delayText;
				if (nextRelease.delay.has_value()) {
					double delayVal = nextRelease.delay.value();
					if (delayVal < 1.0) {
						delayText = U"{}ms"_fmt(static_cast<int>(delayVal * 1000));
					}
					else {
						delayText = U"{}s"_fmt(static_cast<int>(delayVal));
					}
				}
				else {
					delayText = U"∞";
				}

				double arrowX = startX + 9;
				double arrowY = y + 15;
				FontAsset(U"Regular")(U"↓").drawAt(8, Vec2(arrowX, arrowY), ColorF(0.5, 0.55, 0.6));
				FontAsset(U"Regular")(delayText).draw(7, Vec2(arrowX + 8, arrowY - 6), ColorF(0.5, 0.55, 0.6));
			}
		}

		yCursor = inStartY + 12 + Max<int32>(1, sequenceLength) * rowHeight + 10;

		// --- OUT block (goal requirements)
		int32 maxBallsInGoal = 0;
		for (const auto& req : phase.goalRequirements) {
			maxBallsInGoal = Max(maxBallsInGoal, req.totalCount());
		}
		const double maxIconHeight = Max(14.0, 4.0 + maxBallsInGoal * 14.0);

		double reqX = queryRect.x + 38;
		double reqY = yCursor;
		FontAsset(U"Regular")(U"OUT").draw(9, Vec2(reqX - 2, reqY + maxIconHeight / 2.0 - 5), ColorF(0.5, 0.55, 0.6));
		reqX += 24;

		for (int32 j = 0; j < phase.goalRequirements.size(); ++j) {
			const auto& req = phase.goalRequirements[j];
			RectF goalIcon{ reqX, reqY, 18, maxIconHeight };
			ColorF color = ColorF(0.2, 0.5, 0.3, 0.6);
			ColorF frameColor = ColorF(0.3, 0.6, 0.4, 0.5);

			goalIcon.rounded(3).draw(color);
			goalIcon.rounded(3).drawFrame(1, frameColor);

			if (req.isEmpty()) {
				FontAsset(U"Regular")(U"∅").drawAt(10, goalIcon.center(), ColorF(0.5));
			}
			else {
				double totalBallHeight = 0;
				for (const auto& [kind, count] : req.ballCounts) {
					double r = GetBallRadius(kind) * 0.3;
					totalBallHeight += count * (r * 2 + 2);
				}
				totalBallHeight -= 2;

				double ballY = reqY + (maxIconHeight - totalBallHeight) / 2.0;
				for (const auto& [kind, count] : req.ballCounts) {
					for (int32 k = 0; k < count; ++k) {
						double r = GetBallRadius(kind) * 0.3;
						ColorF ballColor = GetBallColor(kind);
						Circle(reqX + 9, ballY + r, r).draw(ballColor);
						ballY += r * 2 + 2;
					}
				}
			}

			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + j))).drawAt(9, Vec2(reqX + 9, reqY + maxIconHeight + 7), ColorF(0.6));
			reqX += 26;
		}

		// Result marker (if decided)
		if (phaseIndex < m_phaseResults.size() && m_phaseResults[phaseIndex].has_value()) {
			const bool ok = m_phaseResults[phaseIndex].value();
			Circle checkBg{ queryRect.x + 38 - 16, reqY + maxIconHeight / 2.0, 7 };
			checkBg.draw(ok ? ColorF(0.2, 0.6, 0.3) : ColorF(0.6, 0.25, 0.25));
			FontAsset(U"Regular")(ok ? U"✓" : U"✗").drawAt(10, checkBg.center, Palette::White);
		}

		yCursor = reqY + maxIconHeight + 26;

		// Between-phase arrow
		if (phaseIndex + 1 < m_phases.size()) {
			const double arrowX = startX + 9;
			const double arrowY = yCursor - 6;
			FontAsset(U"Regular")(U"↓").drawAt(10, Vec2(arrowX, arrowY), ColorF(0.5, 0.55, 0.6));
			yCursor += 2;
		}
	}

	return getPanelHeight();
}

void SampleQuery::startSimulation(Stage& stage)
{
	for (int32 i = 0; i < Min(m_startBalls.size(), stage.m_startCircles.size()); ++i) {
		const auto& state = m_startBalls[i];
		if (not state) continue;
		const auto& pos = stage.m_startCircles[i].circle.center;
		auto circleBody = createCircle(stage.m_world, P2Dynamic, Circle{ pos, state->radius() });
		stage.m_startBallsInWorld.push_back(Ball{ circleBody, state->kind });
	}
}

void SampleQuery::update(Stage& stage, double dt)
{
	// SampleQuery は一度に全てのボールを放出するため、updateでは何もしない
	(void)stage;
	(void)dt;
}

bool SampleQuery::checkSimulationResult(const Stage& stage)
{
	Array<Array<BallKind>> reachedGoalAreas(stage.m_goalAreas.size());
	for (auto [i, r] : Indexed(stage.m_goalAreas)) {
		for (const auto& c : stage.m_startBallsInWorld) {
			if (r.rect.contains(c.body.getPos())) {
				reachedGoalAreas[i].push_back(c.kind);
			}
		}
	}
	if (reachedGoalAreas.size() == m_goalAreaToBeFilled.size()) {
		bool allOk = true;
		for (int32 i = 0; i < m_goalAreaToBeFilled.size(); ++i) {
			const auto& requiredKind = m_goalAreaToBeFilled[i];
			const auto& reachedKinds = reachedGoalAreas[i];
			if (requiredKind) {
				if (reachedKinds.size() == 1 and requiredKind.value() == reachedKinds[0]) {
					// OK
				}
				else { allOk = false; break; }
			}
			else {
				if (reachedKinds.size() == 0) {
					// OK
				}
				else { allOk = false; break; }
			}
		}
		return allOk;
	}
	return false;
}

double SampleQuery::drawPanelContent(const RectF& queryRect, bool isActive) const
{
	// 入力ボール表示
	double ballX = queryRect.x + 38;
	double ballY = queryRect.y + 22;  // ラベル用の上スペースを確保
	
	FontAsset(U"Regular")(U"IN").draw(9, Vec2(ballX, ballY - 2), ColorF(0.5, 0.55, 0.6));
	ballX += 24;
	
	for (auto [k, ball] : Indexed(m_startBalls)) {
		if (ball) {
			double r = ball->radius() * 0.35;
			ColorF color = GetBallColor(ball->kind);
			// OUT 側と同じく「アイコンの中心X = 左端 + 9」になるように揃える
			Circle(ballX + 9, ballY + 6, r).draw(color);
			Circle(ballX + 9, ballY + 6, r).drawFrame(1, ColorF(1.0, 0.5));
			
			// Label（ボールの上に配置）
			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + k))).drawAt(9, Vec2(ballX + 9, ballY - 6), ColorF(0.6));
			
			ballX += 26;
		}
		else {
			// 空の入力を示す
			Circle(ballX + 9, ballY + 6, 4).drawFrame(1, ColorF(0.4));
			
			// Label
			FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + k))).drawAt(9, Vec2(ballX + 9, ballY - 6), ColorF(0.6));
			
			ballX += 26;
		}
	}
	
	// 出力要求表示
	double reqX = queryRect.x + 38;
	double reqY = queryRect.y + 48;
	
	FontAsset(U"Regular")(U"OUT").draw(9, Vec2(reqX - 2, reqY + 2), ColorF(0.5, 0.55, 0.6));
	reqX += 24;
	
	for (int32 j = 0; j < m_goalAreaToBeFilled.size(); ++j) {
		const auto& req = m_goalAreaToBeFilled[j];
		// ゴールエリアアイコン
		RectF goalIcon{ reqX, reqY, 18, 18 };
		ColorF color = ColorF(0.2, 0.5, 0.3, 0.6);
		ColorF frameColor = ColorF(0.3, 0.6, 0.4, 0.5);
		
		goalIcon.rounded(3).draw(color);
		goalIcon.rounded(3).drawFrame(1, frameColor);
		
		if (req) {
			double r = GetBallRadius(req.value()) * 0.35;
			ColorF ballColor = GetBallColor(req.value());
			Circle(reqX + 9, reqY + 9, r).draw(ballColor);
		}
		else {
			FontAsset(U"Regular")(U"∅").drawAt(10, goalIcon.center(), ColorF(0.5));
		}
		
		// Label
		FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + j))).drawAt(9, goalIcon.bottomCenter().movedBy(0, 7), ColorF(0.6));
		
		reqX += 26;
	}
	
	return getPanelHeight();
}

// SequentialQuery implementation

SequentialQuery::SequentialQuery(
	const Array<DelayedBallRelease>& releases,
	const Array<Optional<BallKind>>& goalAreaToBeFilled
)
	: m_releases(releases)
	, m_goalAreaToBeFilled(goalAreaToBeFilled)
{
}

void SequentialQuery::startSimulation(Stage& stage)
{
	// 状態を初期化
	m_nextReleaseIndex = 0;
	m_timeSinceLastRelease = 0.0;
	m_waitingForAllStopped = false;
	
	if (m_releases.empty()) return;
	
	const auto& release = m_releases[0];
	
	if (!release.delay.has_value()) {
		// delay が none = 全ボール停止待ち
		// 最初のイベントの場合は即時放出
		releaseBalls(stage, 0);
		m_nextReleaseIndex = 1;
		m_timeSinceLastRelease = 0.0;
		
		// 次のイベントがあれば待機状態を設定
		if (m_nextReleaseIndex < m_releases.size()) {
			if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
				m_waitingForAllStopped = true;
			}
		}
	}
	else if (release.delay.value() <= 0.0) {
		// delay = 0 は即時放出
		releaseBalls(stage, 0);
		m_nextReleaseIndex = 1;
		m_timeSinceLastRelease = 0.0;
	}
}

void SequentialQuery::update(Stage& stage, double dt)
{
	if (m_nextReleaseIndex >= m_releases.size()) {
		// 全てのボールを放出済み
		return;
	}
	
	const auto& release = m_releases[m_nextReleaseIndex];
	
	if (m_waitingForAllStopped) {
		// 全ボール停止を待機中
		if (areAllBallsStopped(stage)) {
			// 全ボール停止したので次を放出
			releaseBalls(stage, m_nextReleaseIndex);
			m_nextReleaseIndex++;
			m_timeSinceLastRelease = 0.0;
			m_waitingForAllStopped = false;
			
			// 次のイベントの待機モードを設定
			if (m_nextReleaseIndex < m_releases.size()) {
				if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
					m_waitingForAllStopped = true;
				}
			}
		}
	}
	else if (release.delay.has_value()) {
		// 時間遅延を待機中
		m_timeSinceLastRelease += dt;
		
		if (m_timeSinceLastRelease >= release.delay.value()) {
			// 時間経過したので放出
			releaseBalls(stage, m_nextReleaseIndex);
			m_nextReleaseIndex++;
			m_timeSinceLastRelease = 0.0;
			
			// 次のイベントの待機モードを設定
			if (m_nextReleaseIndex < m_releases.size()) {
				if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
					m_waitingForAllStopped = true;
				}
			}
		}
	}
	else {
		// delay が none = 全ボール停止待ち開始
		m_waitingForAllStopped = true;
	}
}

bool SequentialQuery::checkSimulationResult(const Stage& stage)
{
	Array<Array<BallKind>> reachedGoalAreas(stage.m_goalAreas.size());
	for (auto [i, r] : Indexed(stage.m_goalAreas)) {
		for (const auto& c : stage.m_startBallsInWorld) {
			if (r.rect.contains(c.body.getPos())) {
				reachedGoalAreas[i].push_back(c.kind);
			}
		}
	}
	if (reachedGoalAreas.size() == m_goalAreaToBeFilled.size()) {
		bool allOk = true;
		for (int32 i = 0; i < m_goalAreaToBeFilled.size(); ++i) {
			const auto& requiredKind = m_goalAreaToBeFilled[i];
			const auto& reachedKinds = reachedGoalAreas[i];
			if (requiredKind) {
				if (reachedKinds.size() == 1 and requiredKind.value() == reachedKinds[0]) {
					// OK
				}
				else { allOk = false; break; }
			}
			else {
				if (reachedKinds.size() == 0) {
					// OK
				}
				else { allOk = false; break; }
			}
		}
		return allOk;
	}
	return false;
}

Array<Optional<StartBallState>> SequentialQuery::getStartBalls() const
{
	// 表示用に最初の放出イベントのボールを返す
	if (m_releases.empty()) {
		return {};
	}
	return m_releases[0].startBalls;
}

double SequentialQuery::getPanelHeight() const
{
	// 放出シーケンスの長さに基づいて高さを計算
	int32 sequenceLength = m_releases.size();
	
	// 基本高さ + シーケンスの行数 * 行の高さ + 出力エリア
	double rowHeight = 20.0;
	double inputAreaHeight = rowHeight * sequenceLength;
	double outputAreaHeight = 30.0;
	return 35.0 + inputAreaHeight + outputAreaHeight;
}

double SequentialQuery::drawPanelContent(const RectF& queryRect, bool isActive) const
{
	// StartCircleの数を取得（最初の放出イベントから）
	int32 numStartCircles = 0;
	for (const auto& release : m_releases) {
		numStartCircles = Max(numStartCircles, release.startBalls.size());
	}
	
	int32 sequenceLength = m_releases.size();
	
	// StartCircleを横に並べる列幅（OUT の 26px 間隔に合わせる）
	const double colWidth = 26.0;
	const double rowHeight = 20.0;
	const double labelX = queryRect.x + 38;
	const double startX = labelX + 24;  // "IN" ラベルの後（OUT と同じオフセット）
	const double startY = queryRect.y + 18;

	// 現在の段階（ボール放出中/待機中）の行を計算
	Optional<int32> activeRow = none;
	if (m_releases.size() >= 2 and isActive) {
		if (m_nextReleaseIndex == 0) {
			// activeRow = 0;
		}
		else if (m_nextReleaseIndex >= m_releases.size()) {
			activeRow = m_releases.size() - 1;
		}
		else {
			activeRow = m_nextReleaseIndex - 1;
		}
	}

	const ColorF activeRowBg = ColorF(0.35, 0.45, 0.9, 0.12);
	const ColorF activeRowFrame = ColorF(0.45, 0.6, 1.0, 0.55);

	
	// "IN" ラベル（縦方向に中央揃え）
	double inLabelY = startY + (sequenceLength * rowHeight) / 2.0;
	FontAsset(U"Regular")(U"IN").draw(9, Vec2(labelX, inLabelY - 5), ColorF(0.5, 0.55, 0.6));
	
	// ヘッダー行: StartCircleラベル (a, b, c...) を横に並べる
	for (int32 sc = 0; sc < numStartCircles; ++sc) {
		double x = startX + sc * colWidth;
		FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + sc))).drawAt(9, Vec2(x + 9, startY), ColorF(0.6, 0.7, 0.8));
	}
	
	// シーケンスを縦に描画（各放出イベントは横に並ぶ）
	for (int32 row = 0; row < sequenceLength; ++row) {
		double y = startY + 12 + row * rowHeight;
		const auto& release = m_releases[row];

		// 現在段階の行を背景ハイライト
		if (activeRow && row == activeRow.value()) {
			RectF rowRect{ startX - 6, y - 6, numStartCircles * colWidth + 12, rowHeight };
			rowRect.rounded(4).draw(activeRowBg);
			rowRect.rounded(4).drawFrame(1, activeRowFrame);
		}

		
		for (int32 sc = 0; sc < numStartCircles; ++sc) {
			double x = startX + sc * colWidth;
			
			if (sc < release.startBalls.size() && release.startBalls[sc].has_value()) {
				const auto& ball = release.startBalls[sc].value();
				double r = ball.radius() * 0.35;
				ColorF color = GetBallColor(ball.kind);
				Circle(x + 9, y + 4, r).draw(color);
				Circle(x + 9, y + 4, r).drawFrame(1, ColorF(1.0, 0.5));
			}
			else {
				// 空の入力
				Circle(x + 9, y + 4, 4).drawFrame(1, ColorF(0.4));
			}
		}
		
		// 次の行への矢印と遅延テキスト（最後の行以外）
		if (row + 1 < sequenceLength) {
			const auto& nextRelease = m_releases[row + 1];
			
			// 遅延テキスト
			String delayText;
			if (nextRelease.delay.has_value()) {
				double delayVal = nextRelease.delay.value();
				if (delayVal < 1.0) {
					delayText = U"{}ms"_fmt(static_cast<int>(delayVal * 1000));
				}
				else {
					delayText = U"{}s"_fmt(static_cast<int>(delayVal));
				}
			}
			else {
				delayText = U"∞";
			}
			
			// 矢印と遅延テキストを中央に表示
		double arrowX = startX + 9;
			double arrowY = y + 15;
			FontAsset(U"Regular")(U"↓").drawAt(8, Vec2(arrowX, arrowY), ColorF(0.5, 0.55, 0.6));
			FontAsset(U"Regular")(delayText).draw(7, Vec2(arrowX + 8, arrowY - 6), ColorF(0.5, 0.55, 0.6));
		}
	}
	
	// 出力要求表示
	double reqX = queryRect.x + 38;
	double reqY = startY + 12 + sequenceLength * rowHeight + 5;
	
	FontAsset(U"Regular")(U"OUT").draw(9, Vec2(reqX - 2, reqY - 2), ColorF(0.5, 0.55, 0.6));
	reqX += 24;
	
	for (int32 j = 0; j < m_goalAreaToBeFilled.size(); ++j) {
		const auto& req = m_goalAreaToBeFilled[j];
		// ゴールエリアアイコン
		RectF goalIcon{ reqX, reqY, 18, 18 };
		ColorF color = ColorF(0.2, 0.5, 0.3, 0.6);
		ColorF frameColor = ColorF(0.3, 0.6, 0.4, 0.5);
		
		goalIcon.rounded(3).draw(color);
		goalIcon.rounded(3).drawFrame(1, frameColor);
		
		if (req) {
			double r = GetBallRadius(req.value()) * 0.35;
			ColorF ballColor = GetBallColor(req.value());
			Circle(reqX + 9, reqY + 9, r).draw(ballColor);
		}
		else {
			FontAsset(U"Regular")(U"∅").drawAt(10, goalIcon.center(), ColorF(0.5));
		}
		
		// Label
		FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + j))).drawAt(9, goalIcon.bottomCenter().movedBy(0, 7), ColorF(0.6));
		
		reqX += 26;
	}
	
	return getPanelHeight();
}

bool SequentialQuery::hasFinishedReleasing() const
{
	return m_nextReleaseIndex >= m_releases.size();
}

void SequentialQuery::releaseBalls(Stage& stage, int32 releaseIndex)
{
	if (releaseIndex >= m_releases.size()) return;
	
	const auto& release = m_releases[releaseIndex];
	
	for (int32 sc = 0; sc < release.startBalls.size(); ++sc) {
		if (!release.startBalls[sc].has_value()) {
			continue;
		}
		
		if (sc >= stage.m_startCircles.size()) continue;
		
		const auto& startBall = release.startBalls[sc].value();
		const auto& pos = stage.m_startCircles[sc].circle.center;
		auto circleBody = createCircle(stage.m_world, P2Dynamic, Circle{ pos, startBall.radius() });
		stage.m_startBallsInWorld.push_back(Ball{ circleBody, startBall.kind });
	}
}

bool SequentialQuery::areAllBallsStopped(const Stage& stage) const
{
	if (stage.m_startBallsInWorld.empty()) {
		return true;
	}
	
	for (const auto& ball : stage.m_startBallsInWorld) {
		// 落下して削除されたボール（isEmpty）も停止とみなす
		if (ball.body.isEmpty()) {
			continue;
		}
		if (ball.body.isAwake()) {
			return false;
		}
	}
	return true;
}

// MultiPhaseQuery implementation

MultiPhaseQuery::MultiPhaseQuery(const Array<MultiPhaseQuery::Phase>& phases)
	: m_phases(phases)
{
}

void MultiPhaseQuery::startSimulation(Stage& stage)
{
	m_phaseIndex = 0;
	m_nextReleaseIndex = 0;
	m_timeSinceLastRelease = 0.0;
	m_waitingForAllStopped = false;
	m_phaseResults.assign(m_phases.size(), none);

	if (m_phases.empty()) {
		return;
	}

	startPhase(stage, 0);
}

void MultiPhaseQuery::startPhase(Stage& stage, int32 phaseIndex)
{
	(void)stage;
	m_phaseIndex = phaseIndex;
	m_nextReleaseIndex = 0;
	m_timeSinceLastRelease = 0.0;
	m_waitingForAllStopped = false;

	if (m_phaseIndex >= m_phases.size()) {
		return;
	}

	const auto& phase = m_phases[m_phaseIndex];
	if (phase.releases.empty()) {
		// 放出が無い場合も「停止待ち→ゴール確認」で進める
		m_waitingForAllStopped = true;
		return;
	}

	const auto& release0 = phase.releases[0];
	if (!release0.delay.has_value() || release0.delay.value() <= 0.0) {
		releaseBalls(stage, m_phaseIndex, 0);
		m_nextReleaseIndex = 1;
		m_timeSinceLastRelease = 0.0;

		if (m_nextReleaseIndex < phase.releases.size()) {
			if (!phase.releases[m_nextReleaseIndex].delay.has_value()) {
				m_waitingForAllStopped = true;
			}
		}
	}
}

void MultiPhaseQuery::update(Stage& stage, double dt)
{
	if (m_phaseIndex >= m_phases.size()) {
		return;
	}

	const auto& phase = m_phases[m_phaseIndex];

	// 1) 現フェーズの放出更新
	if (m_nextReleaseIndex < phase.releases.size()) {
		const auto& nextRelease = phase.releases[m_nextReleaseIndex];

		if (m_waitingForAllStopped) {
			if (areAllBallsStopped(stage)) {
				releaseBalls(stage, m_phaseIndex, m_nextReleaseIndex);
				m_nextReleaseIndex++;
				m_timeSinceLastRelease = 0.0;
				m_waitingForAllStopped = false;

				if (m_nextReleaseIndex < phase.releases.size()) {
					if (!phase.releases[m_nextReleaseIndex].delay.has_value()) {
						m_waitingForAllStopped = true;
					}
				}
			}
		}
		else if (nextRelease.delay.has_value()) {
			m_timeSinceLastRelease += dt;
			if (m_timeSinceLastRelease >= nextRelease.delay.value()) {
				releaseBalls(stage, m_phaseIndex, m_nextReleaseIndex);
				m_nextReleaseIndex++;
				m_timeSinceLastRelease = 0.0;

				if (m_nextReleaseIndex < phase.releases.size()) {
					if (!phase.releases[m_nextReleaseIndex].delay.has_value()) {
						m_waitingForAllStopped = true;
					}
				}
			}
		}
		else {
			m_waitingForAllStopped = true;
		}

		return;
	}

	// 2) フェーズ放出完了後: 全停止を待ってからゴール確認→次フェーズへ
	if (!areAllBallsStopped(stage)) {
		return;
	}

	if (!m_phaseResults[m_phaseIndex].has_value()) {
		m_phaseResults[m_phaseIndex] = checkGoalForPhase(stage, m_phaseIndex);
	}

	// 次フェーズへ
	const int32 nextPhase = m_phaseIndex + 1;
	if (nextPhase < m_phases.size()) {
		startPhase(stage, nextPhase);
	}
	else {
		m_phaseIndex = m_phases.size();
	}
}

bool MultiPhaseQuery::checkGoalForPhase(const Stage& stage, int32 phaseIndex) const
{
	if (phaseIndex >= m_phases.size()) {
		return false;
	}
	return CheckGoalRequirements(stage, m_phases[phaseIndex].goalRequirements);
}

bool MultiPhaseQuery::checkSimulationResult(const Stage& stage)
{
	(void)stage;
	if (m_phases.empty()) {
		return false;
	}

	// 未確定が残っている場合は、結果はまだ確定しない（false扱い）
	for (const auto& r : m_phaseResults) {
		if (!r.has_value()) {
			return false;
		}
		if (r.value() == false) {
			return false;
		}
	}
	return true;
}

bool MultiPhaseQuery::hasFinishedReleasing() const
{
	if (m_phases.empty()) {
		return true;
	}
	if (m_phaseIndex >= m_phases.size()) {
		return true;
	}
	return false;
}

Array<Optional<StartBallState>> MultiPhaseQuery::getStartBalls() const
{
	if (m_phases.empty() || m_phases[0].releases.empty()) {
		return {};
	}
	return m_phases[0].releases[0].startBalls;
}

Array<Optional<BallKind>> MultiPhaseQuery::getGoalRequirements() const
{
	if (m_phases.empty()) {
		return {};
	}

	// 互換性のため、最初のフェーズの最初のボール種類のみを返す
	Array<Optional<BallKind>> result;
	for (const auto& req : m_phases[0].goalRequirements) {
		if (req.isEmpty()) {
			result.push_back(none);
		}
		else {
			result.push_back(req.ballCounts.begin()->first);
		}
	}
	return result;
}

void MultiPhaseQuery::releaseBalls(Stage& stage, int32 phaseIndex, int32 releaseIndex)
{
	if (phaseIndex >= m_phases.size()) return;
	const auto& releases = m_phases[phaseIndex].releases;
	if (releaseIndex >= releases.size()) return;

	const auto& release = releases[releaseIndex];
	for (int32 sc = 0; sc < release.startBalls.size(); ++sc) {
		if (!release.startBalls[sc].has_value()) {
			continue;
		}
		if (sc >= stage.m_startCircles.size()) continue;
		const auto& startBall = release.startBalls[sc].value();
		const auto& pos = stage.m_startCircles[sc].circle.center;
		auto circleBody = createCircle(stage.m_world, P2Dynamic, Circle{ pos, startBall.radius() });
		stage.m_startBallsInWorld.push_back(Ball{ circleBody, startBall.kind });
	}
}

bool MultiPhaseQuery::areAllBallsStopped(const Stage& stage) const
{
	if (stage.m_startBallsInWorld.empty()) {
		return true;
	}

	for (const auto& ball : stage.m_startBallsInWorld) {
		if (ball.body.isEmpty()) {
			continue;
		}
		if (ball.body.isAwake()) {
			return false;
		}
	}
	return true;
}

// MultiGoalSequentialQuery implementation

MultiGoalSequentialQuery::MultiGoalSequentialQuery(
	const Array<DelayedBallRelease>& releases,
	const Array<GoalRequirement>& goalRequirements
)
	: m_releases(releases)
	, m_goalRequirements(goalRequirements)
{
}

void MultiGoalSequentialQuery::startSimulation(Stage& stage)
{
	// 状態を初期化
	m_nextReleaseIndex = 0;
	m_timeSinceLastRelease = 0.0;
	m_waitingForAllStopped = false;
	
	if (m_releases.empty()) return;
	
	const auto& release = m_releases[0];
	
	if (!release.delay.has_value()) {
		// delay が none = 全ボール停止待ち
		// 最初のイベントの場合は即時放出
		releaseBalls(stage, 0);
		m_nextReleaseIndex = 1;
		m_timeSinceLastRelease = 0.0;
		
		// 次のイベントがあれば待機状態を設定
		if (m_nextReleaseIndex < m_releases.size()) {
			if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
				m_waitingForAllStopped = true;
			}
		}
	}
	else if (release.delay.value() <= 0.0) {
		// delay = 0 は即時放出
		releaseBalls(stage, 0);
		m_nextReleaseIndex = 1;
		m_timeSinceLastRelease = 0.0;
	}
}

void MultiGoalSequentialQuery::update(Stage& stage, double dt)
{
	if (m_nextReleaseIndex >= m_releases.size()) {
		return;
	}
	
	const auto& release = m_releases[m_nextReleaseIndex];
	
	if (m_waitingForAllStopped) {
		if (areAllBallsStopped(stage)) {
			releaseBalls(stage, m_nextReleaseIndex);
			m_nextReleaseIndex++;
			m_timeSinceLastRelease = 0.0;
			m_waitingForAllStopped = false;
			
			if (m_nextReleaseIndex < m_releases.size()) {
				if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
					m_waitingForAllStopped = true;
				}
			}
		}
	}
	else if (release.delay.has_value()) {
		m_timeSinceLastRelease += dt;
		
		if (m_timeSinceLastRelease >= release.delay.value()) {
			releaseBalls(stage, m_nextReleaseIndex);
			m_nextReleaseIndex++;
			m_timeSinceLastRelease = 0.0;
			
			if (m_nextReleaseIndex < m_releases.size()) {
				if (!m_releases[m_nextReleaseIndex].delay.has_value()) {
					m_waitingForAllStopped = true;
				}
			}
		}
	}
	else {
		m_waitingForAllStopped = true;
	}
}

bool MultiGoalSequentialQuery::checkSimulationResult(const Stage& stage)
{
	// 各ゴールエリアに入っているボールを収集
	Array<Array<BallKind>> reachedGoalAreas(stage.m_goalAreas.size());
	for (auto [i, r] : Indexed(stage.m_goalAreas)) {
		for (const auto& c : stage.m_startBallsInWorld) {
			if (r.rect.contains(c.body.getPos())) {
				reachedGoalAreas[i].push_back(c.kind);
			}
		}
	}
	
	if (reachedGoalAreas.size() != m_goalRequirements.size()) {
		return false;
	}
	
	// 各ゴールエリアの要件をチェック
	for (int32 i = 0; i < m_goalRequirements.size(); ++i) {
		if (!m_goalRequirements[i].isSatisfiedBy(reachedGoalAreas[i])) {
			return false;
		}
	}
	
	return true;
}

bool MultiGoalSequentialQuery::hasFinishedReleasing() const
{
	return m_nextReleaseIndex >= m_releases.size();
}

Array<Optional<StartBallState>> MultiGoalSequentialQuery::getStartBalls() const
{
	// 表示用に最初の放出イベントのボールを返す
	if (m_releases.empty()) {
		return {};
	}
	return m_releases[0].startBalls;
}

Array<Optional<BallKind>> MultiGoalSequentialQuery::getGoalRequirements() const
{
	// 互換性のため、最初のボール種類のみを返す
	Array<Optional<BallKind>> result;
	for (const auto& req : m_goalRequirements) {
		if (req.isEmpty()) {
			result.push_back(none);
		}
		else {
			// 最初のボール種類を返す
			result.push_back(req.ballCounts.begin()->first);
		}
	}
	return result;
}

double MultiGoalSequentialQuery::getPanelHeight() const
{
	// 入力シーケンスの長さ
	int32 sequenceLength = m_releases.size();
	
	// 出力エリアの高さを計算（複数ボール表示用に増加）
	int32 maxBallsInGoal = 0;
	for (const auto& req : m_goalRequirements) {
		maxBallsInGoal = Max(maxBallsInGoal, req.totalCount());
	}
	
	double rowHeight = 20.0;
	double inputAreaHeight = rowHeight * sequenceLength;
	
	// 出力エリア: ゴールアイコンの高さ + ラベル用スペース
	double maxIconHeight = Max(14.0, 4.0 + maxBallsInGoal * 14.0);
	double outputAreaHeight = maxIconHeight + 20.0;  // アイコン高さ + ラベル分
	
	return 35.0 + inputAreaHeight + outputAreaHeight;
}

double MultiGoalSequentialQuery::drawPanelContent(const RectF& queryRect, bool isActive) const
{
	// StartCircleの数を取得
	int32 numStartCircles = 0;
	for (const auto& release : m_releases) {
		numStartCircles = Max(numStartCircles, release.startBalls.size());
	}
	
	int32 sequenceLength = m_releases.size();
	
	const double colWidth = 26.0;
	const double rowHeight = 20.0;
	const double labelX = queryRect.x + 38;
	const double startX = labelX + 24;
	const double startY = queryRect.y + 12;
	
	// 現在の段階（ボール放出中/待機中）の行を計算
	Optional<int32> activeRow = none;
	if (m_releases.size() >= 2 and isActive) {
		if (m_nextReleaseIndex == 0) {
			// activeRow = 0;
		}
		else if (m_nextReleaseIndex >= m_releases.size()) {
			activeRow = m_releases.size() - 1;
		}
		else {
			activeRow = m_nextReleaseIndex - 1;
		}
	}
	
	const ColorF activeRowBg = ColorF(0.35, 0.45, 0.9, 0.12);
	const ColorF activeRowFrame = ColorF(0.45, 0.6, 1.0, 0.55);
	
	// "IN" ラベル
	double inLabelY = startY + (sequenceLength * rowHeight) / 2.0;
	FontAsset(U"Regular")(U"IN").draw(9, Vec2(labelX, inLabelY - 5), ColorF(0.5, 0.55, 0.6));
	
	// ヘッダー行: StartCircleラベル
	for (int32 sc = 0; sc < numStartCircles; ++sc) {
		double x = startX + sc * colWidth;
		FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'a' + sc))).drawAt(9, Vec2(x + 9, startY), ColorF(0.6, 0.7, 0.8));
	}
	
	// シーケンスを縦に描画
	for (int32 row = 0; row < sequenceLength; ++row) {
		double y = startY + 12 + row * rowHeight;
		const auto& release = m_releases[row];
		
		// 現在段階の行を背景ハイライト
		if (activeRow && row == activeRow.value()) {
			RectF rowRect{ startX - 6, y - 6, numStartCircles * colWidth + 12, rowHeight };
			rowRect.rounded(4).draw(activeRowBg);
			rowRect.rounded(4).drawFrame(1, activeRowFrame);
		}
		
		for (int32 sc = 0; sc < numStartCircles; ++sc) {
			double x = startX + sc * colWidth;
			
			if (sc < release.startBalls.size() && release.startBalls[sc].has_value()) {
				const auto& ball = release.startBalls[sc].value();
				double r = ball.radius() * 0.35;
				ColorF color = GetBallColor(ball.kind);
				Circle(x + 9, y + 4, r).draw(color);
				Circle(x + 9, y + 4, r).drawFrame(1, ColorF(1.0, 0.5));
			}
			else {
				// 空の入力
				Circle(x + 9, y + 4, 4).drawFrame(1, ColorF(0.4));
			}
		}
		
		// 次の行への矢印と遅延テキスト（最後の行以外）
		if (row + 1 < sequenceLength) {
			const auto& nextRelease = m_releases[row + 1];
			
			String delayText;
			if (nextRelease.delay.has_value()) {
				double delayVal = nextRelease.delay.value();
				if (delayVal < 1.0) {
					delayText = U"{}ms"_fmt(static_cast<int>(delayVal * 1000));
				}
				else {
					delayText = U"{}s"_fmt(static_cast<int>(delayVal));
				}
			}
			else {
				delayText = U"∞";
			}
			
		double arrowX = startX + 9;
			double arrowY = y + 15;
			FontAsset(U"Regular")(U"↓").drawAt(8, Vec2(arrowX, arrowY), ColorF(0.5, 0.55, 0.6));
			FontAsset(U"Regular")(delayText).draw(7, Vec2(arrowX + 8, arrowY - 6), ColorF(0.5, 0.55, 0.6));
		}
	}
	
	// 出力要求表示（複数ボール対応）
	double reqX = queryRect.x + 38;
	double reqY = startY + 12 + sequenceLength * rowHeight + 5;
	
	// 全ゴールの最大アイコン高さを事前計算
	int32 maxBallsInGoal = 0;
	for (const auto& req : m_goalRequirements) {
		maxBallsInGoal = Max(maxBallsInGoal, req.totalCount());
	}
	double maxIconHeight = Max(14.0, 4.0 + maxBallsInGoal * 14.0);
	
	// "OUT" ラベル（アイコンの縦中央に配置）
	FontAsset(U"Regular")(U"OUT").draw(9, Vec2(reqX - 2, reqY + maxIconHeight / 2.0 - 5), ColorF(0.5, 0.55, 0.6));
	reqX += 24;
	
	for (int32 j = 0; j < m_goalRequirements.size(); ++j) {
		const auto& req = m_goalRequirements[j];
		
		// ゴールエリアの枠サイズを計算（全て同じ高さに揃える）
		RectF goalIcon{ reqX, reqY, 18, maxIconHeight };
		
		ColorF color = ColorF(0.2, 0.5, 0.3, 0.6);
		ColorF frameColor = ColorF(0.3, 0.6, 0.4, 0.5);
		
		goalIcon.rounded(3).draw(color);
		goalIcon.rounded(3).drawFrame(1, frameColor);
		
		if (req.isEmpty()) {
			FontAsset(U"Regular")(U"∅").drawAt(10, goalIcon.center(), ColorF(0.5));
		}
		else {
			// 複数ボールを縦に並べて描画（中央揃え）
			double totalBallHeight = 0;
			for (const auto& [kind, count] : req.ballCounts) {
				double r = GetBallRadius(kind) * 0.3;
				totalBallHeight += count * (r * 2 + 2);
			}
			totalBallHeight -= 2;  // 最後の余白を除去
			
			double ballY = reqY + (maxIconHeight - totalBallHeight) / 2.0;
			for (const auto& [kind, count] : req.ballCounts) {
				for (int32 k = 0; k < count; ++k) {
					double r = GetBallRadius(kind) * 0.3;
					ColorF ballColor = GetBallColor(kind);
					Circle(reqX + 9, ballY + r, r).draw(ballColor);
					ballY += r * 2 + 2;
				}
			}
		}
		
		// Label（アイコンの下に配置）
		FontAsset(U"Regular")(U"{}"_fmt(static_cast<char32>(U'A' + j))).drawAt(9, Vec2(reqX + 9, reqY + maxIconHeight + 7), ColorF(0.6));
		
		reqX += 26;
	}
	
	return getPanelHeight();
}

void MultiGoalSequentialQuery::releaseBalls(Stage& stage, int32 releaseIndex)
{
	if (releaseIndex >= m_releases.size()) return;
	
	const auto& release = m_releases[releaseIndex];
	
	for (int32 sc = 0; sc < release.startBalls.size(); ++sc) {
		if (!release.startBalls[sc].has_value()) {
			continue;
		}
		
		if (sc >= stage.m_startCircles.size()) continue;
		
		const auto& startBall = release.startBalls[sc].value();
		const auto& pos = stage.m_startCircles[sc].circle.center;
		auto circleBody = createCircle(stage.m_world, P2Dynamic, Circle{ pos, startBall.radius() });
		stage.m_startBallsInWorld.push_back(Ball{ circleBody, startBall.kind });
	}
}

bool MultiGoalSequentialQuery::areAllBallsStopped(const Stage& stage) const
{
	if (stage.m_startBallsInWorld.empty()) {
		return true;
	}
	
	for (const auto& ball : stage.m_startBallsInWorld) {
		if (ball.body.isEmpty()) {
			continue;
		}
		if (ball.body.isAwake()) {
			return false;
		}
	}
	return true;
}
