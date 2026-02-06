# include "StageUI.hpp"
# include "Stage.hpp"
# include "Game.hpp"

// DPadUI implementation
RectF DPadUI::getButtonRect(Direction dir) const {
	const double offset = ButtonSize + ButtonGap;
	switch (dir) {
	case Direction::Up:
		return RectF{ Arg::center = m_center + Vec2{0, -offset}, ButtonSize, ButtonSize };
	case Direction::Down:
		return RectF{ Arg::center = m_center + Vec2{0, offset}, ButtonSize, ButtonSize };
	case Direction::Left:
		return RectF{ Arg::center = m_center + Vec2{-offset, 0}, ButtonSize, ButtonSize };
	case Direction::Right:
		return RectF{ Arg::center = m_center + Vec2{offset, 0}, ButtonSize, ButtonSize };
	default:
		return RectF{};
	}
}

bool DPadUI::hitTest(const Vec2& pos, Direction dir) const {
	return getButtonRect(dir).contains(pos);
}

DPadUI::Direction DPadUI::update(SingleUseCursorPos& cursorPos) {
	if (!m_visible) return Direction::None;

	m_hoveredDirection = Direction::None;
	Direction result = Direction::None;

	// 各方向のヒットテストとホバー検出
	for (Direction dir : { Direction::Up, Direction::Down, Direction::Left, Direction::Right }) {
		if (cursorPos && hitTest(*cursorPos, dir)) {
			m_hoveredDirection = dir;
			cursorPos.use();
			break;
		}
	}

	// マウスダウンで押下開始
	if (MouseL.down() && m_hoveredDirection != Direction::None) {
		m_pressedDirection = m_hoveredDirection;
		m_pressTimer = 0.0;
		m_repeatTimer = 0.0;
		m_firstPress = true;
		result = m_pressedDirection;
	}

	// 押下中の繰り返し処理
	if (MouseL.pressed() && m_pressedDirection != Direction::None) {
		// 押下中も同じボタン上にいるかチェック
		if (m_hoveredDirection == m_pressedDirection) {
			m_pressTimer += Scene::DeltaTime();

			if (m_firstPress) {
				m_firstPress = false;
				// 最初の押下は既にresultに設定済み
			}
			else if (m_pressTimer >= RepeatDelay) {
				m_repeatTimer += Scene::DeltaTime();
				while (m_repeatTimer >= RepeatInterval) {
					m_repeatTimer -= RepeatInterval;
					result = m_pressedDirection;
				}
			}
		}
	}

	// マウスアップで押下終了
	if (MouseL.up()) {
		m_pressedDirection = Direction::None;
		m_pressTimer = 0.0;
		m_repeatTimer = 0.0;
		m_firstPress = true;
	}

	return result;
}

void DPadUI::draw() const {
	if (!m_visible) return;

	auto drawButton = [&](Direction dir, const String& arrow) {
		RectF rect = getButtonRect(dir);
		bool isHovered = (m_hoveredDirection == dir);
		bool isPressed = (m_pressedDirection == dir && MouseL.pressed());

		// 影
		rect.movedBy(2, 2).rounded(8).draw(ColorF(0.0, 0.3));

		// 背景
		ColorF bgColor = ColorF(0.2, 0.25, 0.3, 0.95);
		if (isPressed) {
			bgColor = ColorF(0.4, 0.6, 0.9, 0.95);
		}
		else if (isHovered) {
			bgColor = ColorF(0.3, 0.4, 0.5, 0.95);
		}
		rect.rounded(8).draw(bgColor);

		// 枠
		ColorF frameColor = isPressed ? ColorF(0.6, 0.8, 1.0, 0.9) : ColorF(0.4, 0.5, 0.6, 0.7);
		rect.rounded(8).drawFrame(2, frameColor);

		// 矢印
		ColorF arrowColor = isPressed ? ColorF(1.0) : (isHovered ? ColorF(0.95) : ColorF(0.75));
		FontAsset(U"Regular")(arrow).drawAt(24, rect.center(), arrowColor);
	};

	// 中央のボタン背景（十字の中心部分）
	RectF centerRect{ Arg::center = m_center, ButtonSize * 0.6, ButtonSize * 0.6 };
	centerRect.rounded(6).draw(ColorF(0.15, 0.18, 0.22, 0.8));

	drawButton(Direction::Up, U"▲");
	drawButton(Direction::Down, U"▼");
	drawButton(Direction::Left, U"◀");
	drawButton(Direction::Right, U"▶");
}

void DragModeToggle::update(SingleUseCursorPos& cursorPos)
{
	const RectF leftRect{ m_rect.x, m_rect.y, m_rect.w * 0.5, m_rect.h };
	const RectF rightRect{ m_rect.x + m_rect.w * 0.5, m_rect.y, m_rect.w * 0.5, m_rect.h };

	if (cursorPos.intersects_use(m_rect)) {
		if (MouseL.down()) {
			if (leftRect.contains(Cursor::PosF())) {
				m_isRangeSelectLeft = true;
			}
			else if (rightRect.contains(Cursor::PosF())) {
				m_isRangeSelectLeft = false;
			}
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}
}

void DragModeToggle::draw() const
{
	const RectF leftRect{ m_rect.x, m_rect.y, m_rect.w * 0.5, m_rect.h };
	const RectF rightRect{ m_rect.x + m_rect.w * 0.5, m_rect.y, m_rect.w * 0.5, m_rect.h };

	m_rect.rounded(6).draw(ColorF(0.1, 0.12, 0.16, 0.9));
	m_rect.rounded(6).drawFrame(1, ColorF(0.3, 0.35, 0.4, 0.6));

	const ColorF activeBg = ColorF(0.3, 0.6, 0.9, 0.5);
	const ColorF inactiveBg = ColorF(0.2, 0.22, 0.26, 0.6);
	leftRect.rounded(6).draw(m_isRangeSelectLeft ? activeBg : inactiveBg);
	rightRect.rounded(6).draw(m_isRangeSelectLeft ? inactiveBg : activeBg);

	const Font& iconFont = FontAsset(U"Icon");
	iconFont(U"\uF0C8").drawAt(14, leftRect.center(), ColorF(0.9));
	iconFont(U"\uF245").drawAt(14, rightRect.center(), ColorF(0.9));
}

StageUI::StageUI() {
	Camera2DParameters params = m_camera.getParameters();
	params.minScale = (1.0 / 8.0);
	params.maxScale = 8.0;
	m_camera.setParameters(params);

	// === レイアウト（ウィンドウサイズに追従） ===
	{
		const double padding = 20;
		const double btnH = 50;
		const double btnY = 20;
		const double gap = 10;

		double x = padding;
		m_homeButtonRect = RectF{ x, btnY, 110, btnH };
		x += m_homeButtonRect.w + gap;
		m_undoButtonRect = RectF{ x, btnY, 80, btnH };
		x += m_undoButtonRect.w + gap;
		m_redoButtonRect = RectF{ x, btnY, 80, btnH };
		x += m_redoButtonRect.w + gap;
		/*m_groupingButtonRect = RectF{ x, btnY, 110, btnH };
		x += m_groupingButtonRect.w + gap;
		m_eraseButtonRect = RectF{ x, btnY, 110, btnH };
		x += m_eraseButtonRect.w + gap;*/
		m_pasteButtonRect = RectF{ x, btnY, 80, btnH };
		x += m_pasteButtonRect.w + gap;
		m_simulationStartButtonRect = RectF{ x, btnY, 90, btnH };
		x += m_simulationStartButtonRect.w + gap;
		m_simulationPauseButtonRect = RectF{ x, btnY, 90, btnH };
		x += m_simulationPauseButtonRect.w + gap;
		m_simulationStopButtonRect = RectF{ x, btnY, 90, btnH };
		x += m_simulationStopButtonRect.w + gap;
		m_simulationFastForwardButtonRect = RectF{ x, btnY, 60, btnH };

		// query panel: top-right
		const double panelMargin = 20;
		const double panelW = 260;
		const double panelTop = 90;
		const double panelBottomMargin = 120; // inventory分 + nextボタン分を避ける
		const double panelH = Max(260.0, Scene::Height() - panelTop - panelBottomMargin);
		m_queryPanelRect = RectF{ Scene::Width() - panelMargin - panelW, panelTop, panelW, panelH };
		m_queryPanel.setRect(m_queryPanelRect);

		// next stage button: below query panel
		const double nextW = m_queryPanelRect.w;
		const double nextH = 50;
		const double nextY = m_queryPanelRect.y + m_queryPanelRect.h + 10;
		m_nextStageButtonRect = RectF{ m_queryPanelRect.x, nextY, nextW, nextH };
	}

	// インベントリUIの初期位置設定（画面下部）
	// 実際の描画時に中央揃えされるが、ヒットテスト用にY座標と高さを設定しておく
	m_inventoryUI.setBarRect(RectF(0, Scene::Height() - 90, Scene::Width(), 90));

}

void StageUI::onStageEnter(Stage& stage, bool isSameWithLastStage)
{
	// カメラ位置を Stage から復元
	m_camera.setCenter(stage.m_cameraCenter);
	m_camera.setTargetCenter(stage.m_cameraCenter);
	m_camera.setScale(stage.m_cameraScale);
	m_camera.setTargetScale(stage.m_cameraScale);
	
	// 一時的な状態をリセット
	m_editUI.resetTransientState();
	m_draggingBall.reset();
	m_cursorPos.release();
	
	// クリア演出をリセット
	m_showClearEffect = false;
	m_clearEffectTime = 0.0;
	m_clearParticles.clear();
	
	// コンテキストメニューを閉じる
	m_contextMenu.close();
	
	// Undo/Redo履歴をクリアして初期状態を保存

	if (not isSameWithLastStage) {
		clearUndoRedoHistory();
		m_undoStack.push_back(stage.createSnapshot());
	}
	else if (m_undoStack.empty()) {
		m_undoStack.push_back(stage.createSnapshot());
	}
	
	// クエリ進捗を初期化（まだ初期化されていない場合）
	if (stage.m_queryCompleted.size() != stage.m_queries.size()) {
		stage.resetQueryProgress();
	}

	m_queryPanel.setRect(m_queryPanelRect);
	m_queryPanel.onStageEnter(stage);
	
	// チュートリアルテキストを初期化
	m_tutorialTexts = stage.m_tutorialTexts;
	m_tutorialPageIndex = 0;
	m_tutorialDisplayTime = 0.0;
	m_tutorialWaitingForClick = false;
	
	// 十字キーUIを非表示で初期化
	m_dpadUI.setVisible(false);
}

void StageUI::onStageExit(Stage& stage)
{
	// カメラ位置を Stage に保存
	stage.m_cameraCenter = m_camera.getCenter();
	stage.m_cameraScale = m_camera.getScale();
	
	// 一時的な状態をクリア
	m_editUI.resetTransientState();
	m_draggingBall.reset();
	m_cursorPos.release();
	m_singleQueryMode = false;
	
	// 十字キーUIを非表示
	m_dpadUI.setVisible(false);
	
	// コンテキストメニューを閉じる
	m_contextMenu.close();
	
	// Undo/Redo履歴をクリア
	// clearUndoRedoHistory();
}

void StageUI::onStageEdited(Stage& stage)
{
	// ステージが編集されたらUndo用スナップショットを保存
	pushUndoState(stage);

	stage.resetQueryProgress();
}

void StageUI::pasteFromClipboard(Stage& stage)
{
	auto oldClipboard = m_clipboard; // コピー前の状態を保存
	for (int32 i = 0; i < 100; i++) {
		if (hasSameObjectWithClipboard(stage)) {
			m_clipboard.moveBy({ 20,20 });
		}
		else {
			break;
		}
	}

	HashSet<size_t> newSelectedPointIds;
	HashTable<size_t, size_t> pointIdMapping;
	HashSet<size_t> usedNewPointIds;
	HashSet<size_t> usedOldPointIds;
	for (const auto& [oldPointId, pos] : m_clipboard.m_points) {
		size_t newPointId = stage.m_nextPointId++;
		pointIdMapping[oldPointId] = newPointId;
		stage.m_points[newPointId] = pos;
		newSelectedPointIds.insert(newPointId);
	}
	for (const auto& edge : m_clipboard.m_edges) {
		const Vec2 p1 = stage.m_points.at(pointIdMapping.at(edge[0]));
		const Vec2 p2 = stage.m_points.at(pointIdMapping.at(edge[1]));
		Line newLine{ p1, p2 };
		if (!stage.isLineAllowedInEditableArea(newLine)) {
			continue;
		}
		Edge newEdge = { { pointIdMapping.at(edge[0]), pointIdMapping.at(edge[1]) } };
		size_t edgeIndex = stage.m_edges.size();
		stage.m_edges.push_back(newEdge);
		stage.m_layerOrder.push_back(LayerObject{ LayerObjectType::Edge, edgeIndex });
		usedNewPointIds.insert(pointIdMapping.at(edge[0]));
		usedNewPointIds.insert(pointIdMapping.at(edge[1]));
		usedOldPointIds.insert(edge[0]);
		usedOldPointIds.insert(edge[1]);
	}
	// エッジに使われなかったポイントは反映しない（編集不可エリアで弾かれて孤立するケース対策）
	for (const auto& [oldPointId, newPointId] : pointIdMapping) {
		if (!usedNewPointIds.contains(newPointId)) {
			stage.m_points.erase(newPointId);
			newSelectedPointIds.erase(newPointId);
		}
	}
	for (const auto& group : m_clipboard.m_groups) {
		// エッジ制限で消えたポイントを含むグループはスキップ
		bool ok = true;
		for (auto oldPid : group.getAllPointIds()) {
			if (!usedOldPointIds.contains(oldPid)) {
				ok = false;
				break;
			}
		}
		if (ok) {
			stage.createGroup(stage.mapGroupIDs(group, pointIdMapping));
		}
	}

	auto& sel = m_editUI.selectedIDs();
	sel.clear();
	for (const auto& placedBall : m_clipboard.m_placedBalls) {
		// インベントリに残数があるかチェック
		BallKind ballKind = placedBall.kind;
		bool canPlace = false;

		// 対応するスロットを探して残数確認
		for (size_t i = 0; i < stage.inventorySlots().size(); ++i) {
			const auto& slot = stage.inventorySlots()[i];
			if (slot.kind == InventoryObjectKind::Ball && slot.ballKind == ballKind && stage.canPlaceFromSlot(i)) {
				// 配置可能なのでインベントリから使用
				stage.useFromSlot(i);
				canPlace = true;
				break;
			}
		}

		// 配置可能な場合のみステージに追加
		if (canPlace) {
			stage.addPlacedBall(placedBall);
			sel.insert(SelectedID{ SelectType::PlacedBall, stage.m_placedBalls.size() - 1 });
		}
	}

	m_clipboard = oldClipboard;
	sel.selectObjectsByPoints(stage, newSelectedPointIds);
	onStageEdited(stage);
}

void StageUI::eraseSelection(Stage& stage)
{
	if (m_editUI.eraseSelection(stage)) {
		onStageEdited(stage);
	}
}

bool StageUI::hasSameObjectWithClipboard(const Stage& stage) const
{
	if (m_clipboard.m_edges.empty()) return false;

	Array<Line> clipboardLines;
	for (const auto& edge : m_clipboard.m_edges) {
		const Vec2& p1 = m_clipboard.m_points.at(edge[0]);
		const Vec2& p2 = m_clipboard.m_points.at(edge[1]);
		clipboardLines.push_back(Line(p1, p2));
	}



	// Array<PlacedBall> clipboardBalls = m_clipboard.m_placedBalls;

	// ステージ内に同じオブジェクトがすべて存在するかチェック
	for (const auto& edge : stage.m_edges) {
		const Vec2& p1 = stage.m_points.at(edge[0]);
		const Vec2& p2 = stage.m_points.at(edge[1]);
		Line lineInStage(p1, p2);
		// clipboardLines に同じ線分があれば削除
		clipboardLines.remove_if([&](const Line& l) {
			return (l.begin == lineInStage.begin && l.end == lineInStage.end) ||
				   (l.begin == lineInStage.end && l.end == lineInStage.begin);
		});
	}

	//for (const auto& placedBallInStage : stage.m_placedBalls) {
	//	// clipboardBalls に同じボールがあれば削除
	//	clipboardBalls.remove_if([&](const PlacedBall& b) {
	//		return b.kind == placedBallInStage.kind &&
	//			   b.circle.center == placedBallInStage.circle.center &&
	//			   b.circle.r == placedBallInStage.circle.r;
	//	});
	//}

	return clipboardLines.empty(); // && clipboardBalls.empty();
}

void StageUI::update(Game& game, Stage& stage, double dt)
{
	// チュートリアルテキストの表示処理
	if (m_tutorialPageIndex < m_tutorialTexts.size()) {
		m_tutorialDisplayTime += dt;
		
		// フェードイン完了後はクリック待ち
		if (m_tutorialDisplayTime >= TutorialFadeInTime) {
			m_tutorialWaitingForClick = true;
		}
		
		// クリックで次のページへ
		if (m_tutorialWaitingForClick && MouseL.down()) {
			// テキストボックス上でのみ進める（全画面クリック判定を避ける）
			const Font& font = FontAsset(U"Regular");
			const String& text = m_tutorialTexts[m_tutorialPageIndex];
			const int32 fontSize = 18;

			const double boxWidth = Min(600.0, Scene::Width() - 80.0);
			const double boxX = (Scene::Width() - boxWidth) / 2.0;
			const double boxY = 550;

			RectF textRegion = font(text).region(fontSize, Vec2{ 0, 0 });
			const double boxHeight = Max(60.0, textRegion.h + 40);
			RectF boxRect{ boxX, boxY, boxWidth, boxHeight };

			if (m_cursorPos and boxRect.contains(Cursor::PosF())) {
				m_tutorialPageIndex++;
				m_tutorialDisplayTime = TutorialFadeInTime;
				m_tutorialWaitingForClick = false;
				// クリックを消費してステージ操作に影響しないようにする
				m_cursorPos.use();
			}
		}
	}
	
	// ダブルクリック検出（最初に行う）
	bool isDoubleClicked = false;
	if (MouseL.down()) {
		Vec2 cursorPos = Cursor::PosF();
		if (m_isFirstMouseLDown and m_timeAfterMouseLDown.elapsed() < 0.5s and (cursorPos - m_preMouseLDownPos).length() < 20) {
			isDoubleClicked = true;
			m_isFirstMouseLDown = false;
		}
		else {
			m_isFirstMouseLDown = true;
		}
		m_timeAfterMouseLDown.restart();
		m_preMouseLDownPos = cursorPos;
	}

	m_cursorPos.init();

	bool hasTwoFingerTouch = false;
	// === 2本指タッチでカメラ操作（パン/ピンチ）===
	// UI操作と衝突させないため、2本指がある間は cursorPos を消費してステージ編集入力を抑制する
	{
		auto touches = Touches.unused();
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
				m_twoFingerBaseScale = m_camera.getScale();
			}
			else
			{
				// パン：中心の移動分だけカメラ中心を逆方向に動かす
				const Vec2 deltaCenter = center - m_prevTwoFingerCenter;
				if (deltaCenter != Vec2{ 0, 0 })
				{
					// deltaCenter はスクリーン座標なのでワールド座標に変換（scale に依存）
					const Vec2 newCenter = m_camera.getCenter() - (deltaCenter / m_camera.getScale());
					m_camera.setTargetCenter(newCenter);
					m_camera.setCenter(newCenter);
				}

				// ピンチ：2点間距離の比率で scale 更新
				const double safeBaseDist = Max(1.0, m_twoFingerBaseDistance);
				const double ratio = Max(0.01, distance / safeBaseDist);
				const double targetScale = m_twoFingerBaseScale * ratio;
				m_camera.zoomAt(center, targetScale);

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
	
	// 選択状態の変化を検出するために現在の選択状態を保存
	const size_t prevSelectionCount = m_editUI.selectedIDs().size();
	HashSet<SelectedID> prevSelection;
	if (!stage.m_isSimulationRunning) {
		prevSelection = m_editUI.selectedIDs().m_ids;
	}
	
	// コンテキストメニューが開いている場合は先に処理
	if (m_contextMenu.isOpen()) {
		// Group/Ungroup判定
		bool canGroup = m_editUI.canGroup(stage) && !stage.m_isSimulationRunning;
		bool canUngroup = m_editUI.canUngroup(stage) && !stage.m_isSimulationRunning;
		
		bool canPaste = not m_clipboard.empty();
		bool hasSelection = !m_editUI.selectedIDs().empty();
		bool canCopy = hasSelection;
		
		auto selectedItem = m_contextMenu.update(m_cursorPos, hasSelection, canGroup, canUngroup, canPaste, canCopy, stage.m_isSimulationRunning);
		if (selectedItem) {
			switch (*selectedItem) {
			case ContextMenuItemType::Copy:
				m_clipboard = stage.copySelectedObjects(m_editUI.selectedIDs().m_ids);
				break;
			case ContextMenuItemType::Delete:
				eraseSelection(stage);
				break;
			case ContextMenuItemType::Group:
				if (m_editUI.groupOrUngroup(stage)) {
					onStageEdited(stage);
				}
				break;
			case ContextMenuItemType::Ungroup:
				if (m_editUI.groupOrUngroup(stage)) {
					onStageEdited(stage);
				}
				break;
			case ContextMenuItemType::CameraReset:
				m_camera.setTargetCenter(Vec2(400, 300));
				m_camera.setTargetScale(1.0);
				break;
			case ContextMenuItemType::Paste:
				// ペースト処理（既存のCtrl+Vと同じ処理）
				pasteFromClipboard(stage);
				break;
			case ContextMenuItemType::Run:
				if (!stage.m_isSimulationRunning) {
					m_editUI.selectedIDs().clear();
					stage.m_currentQueryIndex = 0;
					m_singleQueryMode = false;
					stage.startSimulation();
				}
				break;
			}
		}
		
		// コンテキストメニューが開いている間は他の処理をスキップ（描画更新のため）
		//if (m_contextMenu.isOpen()) {
		//	m_camera.updateWheel();  // ホイールでの拡大縮小は許可
		//	m_camera.update(dt);
		//	updateClearEffect(dt);
		//	return;
		//}
	}
	
	// 右クリック開始位置を記録
	if (MouseR.down()) {
		m_rightClickStartPos = Cursor::PosF();
	}
	
	// 右クリック解放時：動いていなければコンテキストメニューを開く
	if (MouseR.up() && !stage.m_isSimulationRunning && !m_contextMenu.isOpen() && m_rightClickStartPos) {
		const Vec2 currentPos = Cursor::PosF();
		const double movedDistance = (*m_rightClickStartPos - currentPos).length();
		
		if (movedDistance < RightClickMoveThreshold) {
			// 動いていない場合：常にクリック位置にメニューを表示
			if (m_editUI.selectedIDs().empty()) {
				// 選択なし: Camera Reset, Paste, Run
				m_contextMenu.openWithoutSelection(currentPos);
			}
			else {
				// 選択あり: Copy, Delete, Group, Ungroup
				m_contextMenu.openWithSelection(currentPos);
			}
		}
		m_rightClickStartPos.reset();
	}
	
	// 右クリックをキャンセル（動いた場合）
	if (MouseR.pressed() && m_rightClickStartPos) {
		const Vec2 currentPos = Cursor::PosF();
		const double movedDistance = (*m_rightClickStartPos - currentPos).length();
		if (movedDistance >= RightClickMoveThreshold) {
			// 動いた場合はカメラ移動モードなのでコンテキストメニュー用の位置をリセット
			m_rightClickStartPos.reset();
		}
	}
	
	// インベントリUIの位置更新（ウィンドウサイズ変更対応）
	m_inventoryUI.setBarRect(RectF(0, Scene::Height() - 90, Scene::Width(), 90));

	// ドラッグモード切り替え（範囲選択 / カメラ移動）
	{
		const double toggleW = 140.0;
		const double toggleH = 28.0;
		const double toggleX = 20.0;
		const double toggleY = Scene::Height() - toggleH - 10;
		m_dragModeToggle.setRect(RectF{ toggleX, toggleY, toggleW, toggleH });
		m_dragModeToggle.update(m_cursorPos);
	}
	
	// 十字キーUIの位置更新（クエリパネルの左側）
	{
		const double dpadOffsetX = 100;  // クエリパネルの左端からのオフセット
		const double dpadY = Scene::Height() - 100;  // パネル下部付近
		m_dpadUI.setCenter(Vec2{ m_queryPanelRect.x - dpadOffsetX, dpadY });
	}
	
	// 十字キーUIの表示状態を更新（選択中かつシミュレーション中でない場合に表示）
	bool shouldShowDpad = !m_editUI.selectedIDs().empty() && !stage.m_isSimulationRunning;
	m_dpadUI.setVisible(shouldShowDpad);
	
	// 十字キーUIの更新処理
	if (m_dpadUI.isVisible()) {
		DPadUI::Direction dpadDir = m_dpadUI.update(m_cursorPos);
		if (dpadDir != DPadUI::Direction::None) {
			Vec2 arrowMove{ 0, 0 };
			switch (dpadDir) {
			case DPadUI::Direction::Up:    arrowMove.y = -5; break;
			case DPadUI::Direction::Down:  arrowMove.y = 5;  break;
			case DPadUI::Direction::Left:  arrowMove.x = -5; break;
			case DPadUI::Direction::Right: arrowMove.x = 5;  break;
			default: break;
			}
			
			if (arrowMove != Vec2{ 0, 0 }) {
				for (const auto& s : m_editUI.selectedIDs()) {
					if (s.type == SelectType::Point) { stage.m_points[s.id] += arrowMove; }
					else if (s.type == SelectType::Group) { stage.deltaMoveGroup(stage.m_groups.at(s.id), arrowMove); }
					else if (s.type == SelectType::StartCircle) { stage.m_startCircles[s.id].circle.center += arrowMove; }
					else if (s.type == SelectType::GoalArea) { stage.m_goalAreas[s.id].rect.pos += arrowMove; }
					else if (s.type == SelectType::PlacedBall) { stage.m_placedBalls[s.id].circle.center += arrowMove; }
				}
				onStageEdited(stage);
			}
		}
	}
	
	// インベントリからのドラッグ開始（SingleUseCursorPos を使用）
	// シミュレーション中はインベントリからボールを出せない
	if (MouseL.down() && !stage.m_isSimulationRunning)
	{
		if (auto slotIndex = m_inventoryUI.hitTestSlot(Cursor::PosF(), stage.inventorySlots().size()))
		{
			if (stage.canPlaceFromSlot(*slotIndex))
			{
				const auto& slot = stage.inventorySlots()[*slotIndex];
				if (slot.kind == InventoryObjectKind::Ball) {
					// インベントリから出すボール（まだステージに配置されていない）		
					m_draggingBall = DraggingBallInfo{ slot.ballKind, *slotIndex, none, Vec2::Zero() };
					stage.useFromSlot(*slotIndex);  // 先にインベントリから減らす
					m_cursorPos.capture(); // カーソルをキャッチしてステージ操作を無効化
				}
			}
		}
	}
	
	// ドラッグ終了時の処理
	if (MouseL.up() && m_draggingBall)
	{
		// インベントリバー上でドロップした場合はボールを戻す
		if (m_inventoryUI.hitTestBar(Cursor::PosF())) {
			// インベントリに戻す
			stage.returnToInventory(m_draggingBall->kind);
			// ボールは既に削除済み or まだ追加されていないので何もしない
		}
		else {
			// ステージ上に配置
			auto cameraTf = m_camera.createTransformer();
			// オフセットを適用してスナップ（Cursor::PosF()はカメラ変換下なのでワールド座標）
			Vec2 requestedPos = Snap(Cursor::PosF() + m_draggingBall->grabOffset, 5);

			BallKind ballKind = m_draggingBall->kind;
			double r = GetBallRadius(ballKind);

			auto isPosAllowed = [&](const Vec2& p) {
				// center が non editable に入っていないこと（現状の制約は中心点ベース）
				return !stage.isPointInNonEditableArea(p);
			};

			// インベントリから出したボールは、禁止領域に被ったらインベントリに戻す
			if (!m_draggingBall->placedBallId.has_value()) {
				if (!isPosAllowed(requestedPos)) {
					stage.returnToInventory(ballKind);
					m_draggingBall.reset();
					m_cursorPos.release();
					return;
				}
				// allowed のときのみ配置
				stage.addPlacedBall(PlacedBall{ Circle{ requestedPos, r }, ballKind });
			}
			else {
				// 既にステージ上にあったボールは、他の選択オブジェクト同様に「有効な座標」を探し続ける
				Vec2 bestPos = requestedPos;
				if (!isPosAllowed(bestPos)) {
					// 8方向に 5px ずつ進みながら、カーソルに近づくように探索
					Vec2 current = m_draggingBall->dragStartPos;
					Vec2 bestDelta{ 0, 0 };
					for (int32 i = 0; i < 100; ++i) {
						double nowLen = (requestedPos - (current + bestDelta)).length();
						bool allDirEnded = true;
						for (Point dir : { Point{ 0,-1 }, Point{ 1,-1 }, Point{ 1,0 }, Point{ 1,1 }, Point{ 0,1 }, Point{ -1,1 }, Point{ -1,0 }, Point{ -1,-1 } }) {
							Vec2 newDelta = bestDelta + dir * 5;
							Vec2 cand = Snap(current + newDelta, 5);
							if ((requestedPos - cand).length() < nowLen && isPosAllowed(cand)) {
								bestDelta = newDelta;
								allDirEnded = false;
							}
						}
						if (allDirEnded) break;
					}

					Vec2 cand = Snap(current + bestDelta, 5);
					if (bestDelta != Vec2{ 0, 0 } && isPosAllowed(cand)) {
						bestPos = cand;
					}
					else {
						// 見つからなければ元の位置で保持
						bestPos = Snap(m_draggingBall->dragStartPos, 5);
					}
				}

				stage.addPlacedBall(PlacedBall{ Circle{ bestPos, r }, ballKind });
			}

			// 配置したボールを選択状態にする
			auto& sel = m_editUI.selectedIDs();
			sel.clear();
			sel.insert(SelectedID{ SelectType::PlacedBall, stage.m_placedBalls.size() - 1 });

			onStageEdited(stage);  // 編集検出

			// ステージ上のPlacedBallをクリックして動かさなかった場合のみメニューを表示
			if (m_draggingBall->placedBallId.has_value()) {
				// ドラッグ開始位置とドロップ位置の距離をチェック
				double movedDistance = (requestedPos - m_draggingBall->dragStartPos).length();
				if (movedDistance < 5.0) {  // 5ピクセル以内なら動いていないとみなす
					// クリック位置を基準にメニューを表示（ワールド座標→スクリーン座標）
					const Mat3x2 mat = m_camera.getMat3x2();
					Vec2 screenPos = mat.transformPoint(m_draggingBall->clickScreenPos);
					screenPos += Vec2{ 15, 15 };
					m_contextMenu.openWithSelection(screenPos);
				}
			}
		}
		
		m_draggingBall.reset();
		m_cursorPos.release(); // カーソルを解放
	}
	
	// インベントリバー上にマウスがあるときはcursorPosをuse

	if (stage.inventorySlots().size() > 0 && m_inventoryUI.hitTestSlot(Cursor::PosF(), stage.inventorySlots().size())) {
		m_cursorPos.reset();
	}

	// クエリパネル更新（クリックされたら単独実行モード）
	if (m_queryPanel.update(stage, m_cursorPos, dt)) {
		m_editUI.selectedIDs().clear();
		m_singleQueryMode = true;
	}

	// Ctrl + A で全選択
	if (KeyControl.pressed() and KeyA.down()) {
		m_editUI.selectedIDs().selectAllObjects(stage);
	}
	
	// Ctrl+C / Ctrl+V でコピー＆ペースト
	if (KeyControl.pressed() and KeyC.down()) { m_clipboard = stage.copySelectedObjects(m_editUI.selectedIDs().m_ids); }
	if (KeyControl.pressed() and KeyV.down()) {
		pasteFromClipboard(stage);
	}

	// Ctrl+Z / Ctrl+Y で Undo/Redo
	if (KeyControl.pressed() and KeyZ.down() and not stage.m_isSimulationRunning) {
		undo(stage);
	}
	if (KeyControl.pressed() and KeyY.down() and not stage.m_isSimulationRunning) {
		redo(stage);
	}
	
	// Undo / Redo ボタン（Ctrl+Z / Ctrl+Y と同じ挙動）
	if (!stage.m_isSimulationRunning) {
		if (m_cursorPos.intersects_use(m_undoButtonRect)) {
			if (MouseL.down()) {
				undo(stage);
				stage.resetQueryProgress();
			}
			Cursor::RequestStyle(CursorStyle::Hand);
		}
		if (m_cursorPos.intersects_use(m_redoButtonRect)) {
			if (MouseL.down()) {
				redo(stage);
				stage.resetQueryProgress();
			}
			Cursor::RequestStyle(CursorStyle::Hand);
		}
	}

	// 矢印キーで選択オブジェクトを5単位ずつ移動
	if (not stage.m_isSimulationRunning and not m_editUI.selectedIDs().empty()) {
		Vec2 arrowMove{ 0, 0 };
		if (KeyLeft.down()) arrowMove.x -= 5;
		if (KeyRight.down()) arrowMove.x += 5;
		if (KeyUp.down()) arrowMove.y -= 5;
		if (KeyDown.down()) arrowMove.y += 5;

		Point keyInputDir = Point{ KeyRight.pressed() - KeyLeft.pressed(), KeyDown.pressed() - KeyUp.pressed() };
		if (keyInputDir != m_prevArrowKeyInput) {
			m_arrowKeyAccumulate = 0.0;
		}
		m_prevArrowKeyInput = keyInputDir;

		for (m_arrowKeyAccumulate += dt; m_arrowKeyAccumulate >= 0.5; m_arrowKeyAccumulate -= 0.05) {
			arrowMove += Vec2{ keyInputDir.x * 5, keyInputDir.y * 5 };
		}
		
		if (arrowMove != Vec2{ 0, 0 }) {
			for (const auto& s : m_editUI.selectedIDs()) {
				if (s.type == SelectType::Point) { stage.m_points[s.id] += arrowMove; }
				else if (s.type == SelectType::Group) { stage.deltaMoveGroup(stage.m_groups.at(s.id), arrowMove); }
				else if (s.type == SelectType::StartCircle) { stage.m_startCircles[s.id].circle.center += arrowMove; }
				else if (s.type == SelectType::GoalArea) { stage.m_goalAreas[s.id].rect.pos += arrowMove; }
				else if (s.type == SelectType::PlacedBall) { stage.m_placedBalls[s.id].circle.center += arrowMove; }
			}
			onStageEdited(stage);
		}
	}

	// Home ボタン
	if (m_cursorPos.intersects_use(m_homeButtonRect)) {
		if (MouseL.down()) { game.exitStage(); }
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	// NextStage ボタン（ステージクリア後）
	if (stage.m_isCleared && !stage.m_isSimulationRunning) {
		if (m_cursorPos.intersects_use(m_nextStageButtonRect)) {
			if (MouseL.down()) {
				game.goToNextStage();
				return;
			}
			Cursor::RequestStyle(CursorStyle::Hand);
		}
	}
	
	//// グループ化ボタン
	//if (m_cursorPos.intersects_use(m_groupingButtonRect)) {
	//	if (MouseL.down() and not stage.m_isSimulationRunning) {
	//		if (m_editUI.groupOrUngroup(stage)) {
	//			onStageEdited(stage);
	//		}
	//	}
	//	Cursor::RequestStyle(CursorStyle::Hand);
	//}

	//// Erase ボタン
	//if (m_cursorPos.intersects_use(m_eraseButtonRect)) {
	//	if (MouseL.down() and not stage.m_isSimulationRunning) { 
	//		eraseSelection(stage);
	//	}
	//	Cursor::RequestStyle(CursorStyle::Hand);
	//}
	//if (KeyBackspace.down() or KeyDelete.down()) { 
	//	eraseSelection(stage);
	//}

	// Paste ボタン
	if (m_cursorPos.intersects_use(m_pasteButtonRect)) {
		if (MouseL.down() and not stage.m_isSimulationRunning) {
			pasteFromClipboard(stage);
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}


	// Simulation Start ボタン
	if (m_cursorPos.intersects_use(m_simulationStartButtonRect)) {
		if (MouseL.down()) {
			if (not stage.m_isSimulationRunning) {
				m_editUI.selectedIDs().clear();
				stage.m_currentQueryIndex = 0;
				m_singleQueryMode = false;
				stage.startSimulation();
				// Console << U"Simulation Start!";
			}
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}
	
	// Simulation Pause ボタン
	if (m_cursorPos.intersects_use(m_simulationPauseButtonRect)) {
		if (MouseL.down()) {
			if (stage.m_isSimulationRunning) {
				stage.m_isSimulationPaused = !stage.m_isSimulationPaused;
				// Console << (stage.m_isSimulationPaused ? U"Simulation Paused" : U"Simulation Resumed");
			}
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}
	
	// Simulation Stop ボタン
	if (m_cursorPos.intersects_use(m_simulationStopButtonRect)) {
		if (MouseL.down()) {
			if (stage.m_isSimulationRunning) {
				stage.endSimulation();
				// Console << U"Simulation Stopped";
			}
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}
	
	// Simulation Fast Forward ボタン
	if (m_cursorPos.intersects_use(m_simulationFastForwardButtonRect)) {
		if (MouseL.down()) {
			// 速度を切り替え: 1x -> 2x -> 4x -> 1x
			if (stage.m_simulationSpeed < 1.5) {
				stage.m_simulationSpeed = 2.0;
			}
			else if (stage.m_simulationSpeed < 3.0) {
				stage.m_simulationSpeed = 4.0;
			}
			else {
				stage.m_simulationSpeed = 1.0;
			}
			// Console << U"Simulation Speed: {}x"_fmt(stage.m_simulationSpeed);
		}
		Cursor::RequestStyle(CursorStyle::Hand);
	}

	if (stage.m_isSimulationRunning) {
		if (not stage.m_isSimulationPaused) {
			double fallThreshold = stage.getLowestY() + 100;
			for (auto& c : stage.m_startBallsInWorld) { 
				if (c.body.getPos().y > fallThreshold) {
					c.body.release();
					
				} 
			}
			stage.m_startBallsInWorld.remove_if([](const auto& c) { return c.body.isEmpty(); });

			// 終了判定: 全てのボールが (静止 OR ゴール侵入後1秒経過) AND クエリが全てのボールを放出済み
			bool allFinished = true;
			for (const auto& c : stage.m_startBallsInWorld) { 
				if (c.body.isEmpty()) {
					continue;
				}

				const bool finishedBySleep = (not c.body.isAwake());
				const bool finishedByGoal = (c.timeSinceEnteredGoal && (*c.timeSinceEnteredGoal >= 1.0));
				allFinished &= (finishedBySleep || finishedByGoal);
			}
			
			// クエリが全てのボールを放出済みかチェック
			bool hasFinishedReleasing = true;
			if (stage.m_currentQueryIndex < stage.m_queries.size()) {
				hasFinishedReleasing = stage.m_queries[stage.m_currentQueryIndex]->hasFinishedReleasing();
			}
			
			if (allFinished && hasFinishedReleasing) {
				bool isSuccess = stage.checkSimulationResult();
				size_t completedQueryIndex = stage.m_currentQueryIndex;
				
				// クリア演出用：今回初めてクリアしたかを判定
				bool wasAlreadyCleared = stage.m_isCleared;
				
				if (isSuccess) {
					// Console << U"Query {} Success!"_fmt(completedQueryIndex + 1);
					stage.markQueryCompleted(completedQueryIndex);
					
					// 今回初めて全クエリクリアした場合のみ演出
					if (stage.m_isCleared && !wasAlreadyCleared) {
						// Console << U"★ Stage Cleared! ★";
						startClearEffect();  // クリア演出を開始
					}
				}
				else {
					// Console << U"Query {} Failed."_fmt(completedQueryIndex + 1);
					stage.markQueryFailed(completedQueryIndex);
				}
				
				stage.endSimulation();

				// 単独実行モードの場合は次に進まない
				if (m_singleQueryMode) {
					// Console << U"Single query test completed.";
					m_singleQueryMode = false;
				}
				else {
					// 次の未判定クエリを探す（成功・失敗に関わらず続行）

					if (completedQueryIndex + 1 < stage.m_queries.size()) {
						stage.m_currentQueryIndex = completedQueryIndex + 1;
						stage.startSimulation();
					}
					else {
						// 全クエリの判定が完了
						// Console << U"All queries tested.";
						stage.m_currentQueryIndex = 0;
					}
				}
			}
		}
	}

	// カメラ移動

	auto& CameraMoveInput = m_dragModeToggle.isRangeSelectLeft() ? MouseR : MouseL;

	if (m_cursorPos) {
		if (CameraMoveInput.pressed() or MouseM.pressed()) {
			auto cameraTf = m_camera.createTransformer();
			Vec2 pos = m_camera.getCenter() - Cursor::DeltaF();
			m_camera.setTargetCenter(pos);
			m_camera.setCenter(pos);
		}
		m_camera.updateWheel();
	}


	m_camera.update(dt);

	{
		auto cameraTf = m_camera.createTransformer();


		// シミュレーション中にダブルクリックでストップ
		if (stage.m_isSimulationRunning and m_cursorPos and isDoubleClicked) {
			stage.endSimulation();
			m_cursorPos.use();
		}

		// PrintDebug(Cursor::PosF());

		if (stage.m_isSimulationRunning && !stage.m_isSimulationPaused) {
			// 速度倍率を適用
			for (stage.m_simulationTimeAccumlate += dt * stage.m_simulationSpeed; stage.m_simulationTimeAccumlate >= Stage::simulationTimeStep; stage.m_simulationTimeAccumlate -= Stage::simulationTimeStep) {
				stage.m_world.update(Stage::simulationTimeStep);

				// ゴール侵入判定：ゴールに入ってからの経過時間を更新
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

				// クエリの時間ベース更新（SequentialQuery用）
				if (stage.m_currentQueryIndex < stage.m_queries.size()) {
					stage.m_queries[stage.m_currentQueryIndex]->update(stage, Stage::simulationTimeStep);
				}

				// 遅すぎる場合は強制終了
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
		if (not stage.m_isSimulationRunning) {
			// コンテキストメニューを開くコールバック
			auto openContextMenuCallback = [this](const Vec2& worldPos, bool alignRight) {
				// ワールド座標をスクリーン座標に変換
				const Mat3x2 mat = m_camera.getMat3x2();
				Vec2 screenPos = mat.transformPoint(worldPos);
				// バッファを追加
				if (alignRight) {
					// 左方向にドラッグした場合：メニューの右上が基準
					screenPos += Vec2{ -15, 15 };
					m_contextMenu.openWithSelectionAlignRight(screenPos);
				}
				else {
					// 右方向にドラッグした場合：メニューの左上が基準（従来通り）
					screenPos += Vec2{ 15, 15 };
					m_contextMenu.openWithSelection(screenPos);
				}
			};
			
			m_editUI.update(stage, isDoubleClicked, m_cursorPos, m_camera, [this](Stage& s) { onStageEdited(s); }, m_draggingBall, openContextMenuCallback, !m_dragModeToggle.isRangeSelectLeft(), hasTwoFingerTouch);
		}
				

	}
	
	/*PrintDebug(m_lineCreateStart);
	PrintDebug(m_hoveredInfo);
	PrintDebug(m_clipboard);
	PrintDebug(m_selectedIDs);
	PrintDebug(stage.m_groups);
	PrintDebug(stage.m_edges);
	PrintDebug(stage.m_points.size());*/

	PrintDebug(m_editUI.selectedIDs());

	//PrintDebug(m_selectSingleLine);

	// print balls state
	//for (const auto& c : stage.m_startBallsInWorld) {
	//	Print(Format(U"Ball {}: pos={} vel={} angleV={}", c.body.getPos(), c.body.getVelocity(), c.body.getAngularVelocity()));
	//}
	
	// クリア演出の更新
	updateClearEffect(dt);
}

void StageUI::draw(const Stage& stage) const
{
	// スクリーン座標でインベントリバー上判定（カメラ変換前に計算）
	const Vec2 screenCursorPos = Cursor::PosF();
	const bool isOverInventory = m_inventoryUI.hitTestBar(screenCursorPos);
	const double time = Scene::Time();

	// 背景グラデーション
	Rect{ 0, 0, Scene::Width(), Scene::Height() }
		.draw(Arg::top = ColorF(0.12, 0.14, 0.18), Arg::bottom = ColorF(0.08, 0.1, 0.14));

	{
		auto cameraTf = m_camera.createTransformer();

		// world draw (edit or simulation)
		m_editUI.drawWorld(stage, m_camera);
	}

	// ツールバー背景
	RectF toolbarBg{ 0, 0, Scene::Width(), 80 };
	toolbarBg.draw(ColorF(0.1, 0.12, 0.16, 0.95));
	toolbarBg.drawFrame(0, 2, ColorF(0.2, 0.25, 0.3));

	// ステージ名
	{
		const Font& font = FontAsset(U"Regular");
		const double left = m_simulationFastForwardButtonRect.x + m_simulationFastForwardButtonRect.w + 10;
		const double right = Scene::Width() - 10.0;
		const double w = Max(0.0, right - left);
		RectF nameBg{ left, 18, w, 44 };
		nameBg.rounded(8).draw(ColorF(0.0, 0.25));
		font(stage.m_name).draw(20, Vec2{ nameBg.x + 12, nameBg.y + 10 }, ColorF(0.95));
	}

	auto drawButton = [&](const RectF& rect, const String& text, const String& icon, ColorF baseColor, bool enabled, bool hovered, bool emphasized = false, int32 textSize = 13) {
		ColorF bg = enabled ? baseColor : ColorF(0.25, 0.28, 0.32);
		if (hovered && enabled) {
			bg = baseColor.lerp(ColorF(1.0), 0.2);
		}

		RectF shadow = rect.movedBy(2, 2);
		shadow.rounded(8).draw(ColorF(0.0, 0.3));

		rect.rounded(8).draw(bg);
		rect.rounded(8).drawFrame(1, ColorF(1.0, 0.1));

		ColorF textColor = enabled ? ColorF(1.0) : ColorF(0.5);
		const Font& font = FontAsset(U"Regular");
		const Font& boldFont = FontAsset(U"Bold");
		const Font& iconFont = FontAsset(U"Icon");
		const Font& textFont = emphasized ? boldFont : font;

		double iconWidth = iconFont(icon).region(18).w;

		double width = iconWidth + 8 + textFont(text).region(textSize).w;

		iconFont(icon).draw(18, Arg::leftCenter = Vec2(rect.center().x - width / 2, rect.center().y), textColor);
		textFont(text).draw(textSize, Arg::leftCenter = Vec2(rect.center().x - width / 2 + iconWidth + 8, rect.center().y), textColor);

	};

	// Home
	bool homeHovered = m_homeButtonRect.mouseOver();
	drawButton(m_homeButtonRect, U"Back", U"\uF060", ColorF(0.4, 0.45, 0.5), true, homeHovered);

	// NextStage
	if (stage.m_isCleared && !stage.m_isSimulationRunning) {
		bool nextStageHovered = m_nextStageButtonRect.mouseOver();
		drawButton(m_nextStageButtonRect, U"Next Stage", U"\uF061", Palette::Deepskyblue, true, nextStageHovered, false, 18);
	}

	// Undo/Redo
	{
		const bool enabledBase = !stage.m_isSimulationRunning;
		const bool canUndo = enabledBase && (m_undoStack.size() >= 2);
		const bool canRedo = enabledBase && (!m_redoStack.empty());
		const bool undoHovered = m_undoButtonRect.mouseOver();
		const bool redoHovered = m_redoButtonRect.mouseOver();
		drawButton(m_undoButtonRect, U"Undo", U"\uF0E2", ColorF(0.35, 0.45, 0.65), canUndo, undoHovered);
		drawButton(m_redoButtonRect, U"Redo", U"\uF01E", ColorF(0.35, 0.45, 0.65), canRedo, redoHovered);
	}

	/*
	// Group / Ungroup
	bool canGroupOrUngroup = m_editUI.canGroupOrUngroup(stage);
	bool canUngroup = m_editUI.canUngroup(stage);

	ColorF groupColor = canUngroup ? ColorF(0.6, 0.4, 0.2)
		: (canGroupOrUngroup ? ColorF(0.7, 0.65, 0.2) : ColorF(0.3, 0.32, 0.35));
	bool groupHovered = m_groupingButtonRect.mouseOver();
	drawButton(m_groupingButtonRect, canUngroup ? U"Ungroup" : U"Group", groupColor, canGroupOrUngroup, groupHovered);

	// Delete
	bool eraseEnabled = !m_editUI.selectedIDs().empty();
	bool eraseHovered = m_eraseButtonRect.mouseOver();
	drawButton(m_eraseButtonRect, U"Delete", ColorF(0.8, 0.3, 0.3), eraseEnabled, eraseHovered);
	*/

	// paste button
	bool pasteEnabled = !m_clipboard.empty() && !stage.m_isSimulationRunning;
	bool pasteHovered = m_pasteButtonRect.mouseOver();
	drawButton(m_pasteButtonRect, U"Paste", U"\uF0EA", ColorF(0.4, 0.5, 0.6), pasteEnabled, pasteHovered);

	// Simulation buttons
	bool simRunning = stage.m_isSimulationRunning;
	bool startHovered = m_simulationStartButtonRect.mouseOver();
	drawButton(m_simulationStartButtonRect, U"Run", U"\uF04B", ColorF(0.3, 0.7, 0.4), !simRunning, startHovered);

	bool pauseHovered = m_simulationPauseButtonRect.mouseOver();
	String pauseText = stage.m_isSimulationPaused ? U"Resume" : U"Pause";
	String pauseIcon = stage.m_isSimulationPaused ? U"\uF04B" : U"\uF04C";
	ColorF pauseColor = stage.m_isSimulationPaused ? ColorF(0.7, 0.6, 0.2) : ColorF(0.8, 0.5, 0.2);
	drawButton(m_simulationPauseButtonRect, pauseText, pauseIcon, pauseColor, simRunning, pauseHovered);

	bool stopHovered = m_simulationStopButtonRect.mouseOver();
	drawButton(m_simulationStopButtonRect, U"Stop", U"\uF04D", ColorF(0.7, 0.3, 0.3), simRunning, stopHovered);

	bool ffHovered = m_simulationFastForwardButtonRect.mouseOver();
	String ffText = U"{}x"_fmt(static_cast<int>(stage.m_simulationSpeed));
	ColorF ffColor = stage.m_simulationSpeed > 1.5 ? ColorF(0.3, 0.6, 0.8) : ColorF(0.4, 0.5, 0.6);
	drawButton(m_simulationFastForwardButtonRect, ffText, U"\uF04E", ffColor, true, ffHovered);

	// Inventory bar (no simulation)
	if (!stage.m_isSimulationRunning) {
		m_inventoryUI.draw(stage.inventorySlots());
	}

	// ドラッグモード切り替え UI
	m_dragModeToggle.draw();

	// 編集不可エリアのヒント
	if (!stage.m_isSimulationRunning && !isOverInventory) {
		auto cameraTf = m_camera.createTransformer();
		const Vec2 worldCursor = Cursor::PosF();
		if (stage.isPointInNonEditableArea(worldCursor)) {
			const String msg = U"No-edit area";
			const Font& font = FontAsset(U"Regular");
			const Vec2 pos = worldCursor + Vec2{ 14, 14 };
			RectF bg = font(msg).region(14, pos).stretched(8, 4);
			bg.rounded(4).draw(ColorF(0.0, 0.55));
			bg.rounded(4).drawFrame(1, ColorF(1.0, 0.25));
			font(msg).draw(14, pos, ColorF(0.95));
		}
	}

	// ドラッグ中のボール描画（インベントリ起点）
	if (m_draggingBall)
	{
		const double r = GetBallRadius(m_draggingBall->kind);
		const ColorF color = GetBallColor(m_draggingBall->kind);

		if (isOverInventory) {
			const double drawR = r * 0.8;
			Vec2 ballPos = screenCursorPos + m_draggingBall->grabOffset;
			Circle shadow{ ballPos + Vec2{2, 2}, drawR };
			shadow.draw(ColorF(0.0, 0.3));
			Circle ball{ ballPos, drawR };
			ball.draw(color);
			ball.drawFrame(2.0, ColorF(1.0, 0.9));
		}
		else {
			auto cameraTf = m_camera.createTransformer();
			Vec2 worldCursorPos = Cursor::PosF();
			Vec2 worldPos = Snap(worldCursorPos + m_draggingBall->grabOffset, 5);

			Circle shadow{ worldPos + Vec2(3, 3), r };
			shadow.draw(ColorF(0.0, 0.3));
			Circle ball{ worldPos, r };
			ball.draw(color);
			ball.drawFrame(2.5 / Graphics2D::GetMaxScaling(), ColorF(1.0, 0.9));
		}
	}

	// クエリパネル描画
	m_queryPanel.draw(stage);
	
	// 十字キーUI描画
	m_dpadUI.draw();

	// コンテキストメニュー描画
	if (m_timeAfterMouseLDown.elapsed() > 0.3s) {
		m_contextMenu.draw();
	}

	// 操作説明の表示
	if (!stage.m_isSimulationRunning) {
		const Font& font = FontAsset(U"Regular");
		Vec2 helpPos{ 20, 90 };
		String helpText = U"Double-click: Create Line\nRight-click: Move Camera";
		RectF bgRect = font(helpText).region(14, helpPos).stretched(8, 4);
		bgRect.rounded(4).draw(ColorF(0.0, 0.25));
		font(helpText).draw(14, helpPos, ColorF(0.8, 0.8, 0.9));
	}
	
	// チュートリアルテキストの表示（ノベルゲーム風）
	if (m_tutorialPageIndex < m_tutorialTexts.size()) {
		const Font& font = FontAsset(U"Regular");
		const String& text = m_tutorialTexts[m_tutorialPageIndex];
		
		// アルファ値を計算（フェードイン）
		double alpha = Min(m_tutorialDisplayTime / TutorialFadeInTime, 1.0);
		
		// 画面中央少し下にテキストボックスを表示
		const double boxWidth = Min(600.0, Scene::Width() - 80.0);
		const double boxX = (Scene::Width() - boxWidth) / 2.0;
		const double boxY = 550;
		
		// テキスト領域の計算
		const int32 fontSize = 18;
		RectF textRegion = font(text).region(fontSize, Vec2{ 0, 0 });
		const double boxHeight = Max(60.0, textRegion.h + 40);
		
		// 背景ボックスの描画（ノベルゲーム風の枠）
		RectF boxRect{ boxX, boxY, boxWidth, boxHeight };
		
		// 影
		boxRect.movedBy(4, 4).rounded(12).draw(ColorF(0.0, 0.4 * alpha));
		
		// メインボックス
		boxRect.rounded(12).draw(ColorF(0.08, 0.1, 0.15, 0.95 * alpha));
		boxRect.rounded(12).drawFrame(2, ColorF(0.4, 0.5, 0.7, 0.8 * alpha));
		
		// 内側の装飾線
		boxRect.stretched(-6).rounded(8).drawFrame(1, ColorF(0.3, 0.4, 0.5, 0.4 * alpha));
		
		// テキストの描画（中央揃え）
		Vec2 textPos{ boxX + (boxWidth - textRegion.w) / 2.0, boxY + (boxHeight - textRegion.h) / 2.0 };
		font(text).draw(fontSize, textPos, ColorF(0.95, 0.95, 1.0, alpha));
		
		// クリック待ち中は「▼」を点滅表示
		if (m_tutorialWaitingForClick) {
			const double blinkAlpha = 0.5 + 0.5 * Sin(Scene::Time() * 4.0);
			const String clickHint = U"▼";
			Vec2 hintPos{ boxX + boxWidth - 30, boxY + boxHeight - 25 };
			font(clickHint).draw(14, hintPos, ColorF(0.8, 0.9, 1.0, blinkAlpha * alpha));
		}
		
		// ページ数表示（複数ページの場合）
		if (m_tutorialTexts.size() > 1) {
			const String pageText = U"{}/{}"_fmt(m_tutorialPageIndex + 1, m_tutorialTexts.size());
			Vec2 pagePos{ boxX + 15, boxY + boxHeight - 25 };
			font(pageText).draw(12, pagePos, ColorF(0.6, 0.7, 0.8, alpha));
		}
	}
	
	// クリア演出の描画（最前面）
	drawClearEffect();
}

void StageUI::pushUndoState(Stage& stage)
{
	StageSnapshot snapshot = stage.createSnapshot();
	m_redoStack.clear();
	m_undoStack.push_back(snapshot);
	while (m_undoStack.size() > MAX_UNDO_HISTORY) {
		m_undoStack.pop_front();
	}
}

void StageUI::undo(Stage& stage)
{
	if (m_undoStack.size() < 2) return;
	m_redoStack.push_back(m_undoStack.back());
	m_undoStack.pop_back();
	stage.restoreSnapshot(m_undoStack.back());
	m_editUI.selectedIDs().clear();
	if (!stage.m_isCleared) {
		stage.resetQueryProgress();
	}
}

void StageUI::redo(Stage& stage)
{
	if (m_redoStack.empty()) return;
	StageSnapshot snapshot = m_redoStack.back();
	m_redoStack.pop_back();
	stage.restoreSnapshot(snapshot);
	m_undoStack.push_back(snapshot);
	m_editUI.selectedIDs().clear();
	if (!stage.m_isCleared) {
		stage.resetQueryProgress();
	}
}

void StageUI::clearUndoRedoHistory()
{
	m_undoStack.clear();
	m_redoStack.clear();
}

void StageUI::startClearEffect()
{
	m_showClearEffect = true;
	m_clearEffectTime = 0.0;
	m_clearParticles.clear();
}

void StageUI::updateClearEffect(double dt)
{
	if (!m_showClearEffect) return;
	
	m_clearEffectTime += dt;
	
	// 演出終了判定
	if (m_clearEffectTime >= ClearEffectDuration) {
		m_showClearEffect = false;
	}
}

void StageUI::drawClearEffect() const
{
	if (!m_showClearEffect) return;
	
	const Font& font = FontAsset(U"Regular");
	const double screenW = Scene::Width();
	const double screenH = Scene::Height();
	
	// アニメーション進行度（0.0 ~ 1.0）
	const double progress = Min(m_clearEffectTime / 0.6, 1.0);
	const double easeOut = 1.0 - Pow(1.0 - progress, 3.0); // イージング
	
	// フェードアウト（2秒後から開始）
	const double fadeOutStart = 2.0;
	const double fadeAlpha = (m_clearEffectTime > fadeOutStart) 
		? Max(0.0, 1.0 - (m_clearEffectTime - fadeOutStart) / 1.0) 
		: 1.0;
	
	// 背景オーバーレイ（薄い暗転）
	const double overlayAlpha = 0.3 * easeOut * fadeAlpha;
	Rect{ 0, 0, static_cast<int>(screenW), static_cast<int>(screenH) }
		.draw(ColorF(0.0, 0.05, 0.1, overlayAlpha));
	
	// メインバナー（右から左にスライドイン）
	const double bannerY = screenH / 2.0 - 40;
	const double bannerH = 80;
	const double slideOffset = screenW * (1.0 - easeOut);
	
	// バナー背景（グラデーション）
	RectF banner{ -50 + slideOffset, bannerY, screenW + 100, bannerH };
	banner.draw(Arg::left = ColorF(0.0, 0.1, 0.2, 0.0 * fadeAlpha),
				Arg::right = ColorF(0.05, 0.1, 0.2, 0.95 * fadeAlpha));
	
	// 上下のアクセントライン
	const double lineAlpha = easeOut * fadeAlpha;
	Line{ 0, bannerY, screenW, bannerY }
		.draw(LineStyle::Default, 2, ColorF(0.3, 0.7, 1.0, 0.8 * lineAlpha));
	Line{ 0, bannerY + bannerH, screenW, bannerY + bannerH }
		.draw(LineStyle::Default, 2, ColorF(0.3, 0.7, 1.0, 0.8 * lineAlpha));
	
	// 光のライン（右から左に流れる）	
	if (m_clearEffectTime < 1.5) {
		const double lightProgress = Min(m_clearEffectTime / 1.0, 1.0);
		const double lightX = screenW * (1.0 - lightProgress) + 50;
		const double lightW = 150;
		RectF lightBar{ lightX, bannerY, lightW, bannerH };
		lightBar.draw(Arg::left = ColorF(1.0, 1.0, 1.0, 0.0),
					  Arg::right = ColorF(0.5, 0.8, 1.0, 0.3 * (1.0 - lightProgress)));
	}
	
	// テキスト（右から左にスライド）	
	const double textSlide = screenW * 0.3 * (1.0 - easeOut);
	const Vec2 textCenter{ screenW / 2.0 + textSlide, bannerY + bannerH / 2.0 };
	
	// メインテキスト
	const String clearText = U"STAGE CLEAR";
	const int32 fontSize = 42;
	const double textAlpha = easeOut * fadeAlpha;
	
	// テキストグロー効果
	font(clearText).drawAt(fontSize + 2, textCenter, ColorF(0.3, 0.6, 1.0, 0.3 * textAlpha));
	font(clearText).drawAt(fontSize, textCenter, ColorF(1.0, 1.0, 1.0, textAlpha));
	
	// サブテキスト（遅れて表示）
	if (m_clearEffectTime > 0.4) {
		const double subProgress = Min((m_clearEffectTime - 0.4) / 0.3, 1.0);
		const double subEase = 1.0 - Pow(1.0 - subProgress, 2.0);
		const double subSlide = 100 * (1.0 - subEase);
		const double subAlpha = subEase * fadeAlpha;
		
		const String subText = U"- All Queries Completed -";
		const Vec2 subPos{ screenW / 2.0 + subSlide, bannerY + bannerH + 25 };
		font(subText).drawAt(16, subPos, ColorF(0.7, 0.85, 1.0, subAlpha));
	}
}
