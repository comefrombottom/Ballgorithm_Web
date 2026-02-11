#pragma once

# include <Siv3D.hpp>
# include "Debug.hpp"
# include "Domain.hpp"
# include "InputUtils.hpp"
# include "GeometryUtils.hpp"
# include "SimpleWatch.hpp"
# include "Inventory.h"
# include "Stage.hpp"
# include "ScrollBar.h"
# include "MyCamera2D.h"
# include "StageEditUI.h"
# include "QueryPanel.h"
# include "ContextMenu.h"
# include "DPadUI.h"
# include "DragModeToggle.h"
# include "Touches.h"

class Game;
class Stage;

// クリア演出用パーティクル
struct ClearParticle {
	Vec2 pos;
	Vec2 velocity;
	ColorF color;
	double size;
	double life;
	double maxLife;
};

// UI 専用クラス
class StageUI {
public:
	StageUI();

	// 入力更新
	void update(Game& game, Stage& stage, double dt = Scene::DeltaTime());
	// 描画
	void draw(const Stage& stage) const;
	// ステージ切り替え時の初期化
	void onStageEnter(Stage& stage, bool isSameWithLastStage = false);
	void onStageExit(Stage& stage);

private:
	StageEditUI m_editUI;

	PointEdgeGroup m_clipboard;
	RectF m_homeButtonRect{ 20, 20, 50, 50 };
	RectF m_undoButtonRect{ 0, 0, 0, 0 };
	RectF m_redoButtonRect{ 0, 0, 0, 0 };
	RectF m_groupingButtonRect{ 140, 20, 100, 50 };
	RectF m_eraseButtonRect{ 260, 20, 100, 50 };
	RectF m_pasteButtonRect{ 140, 20, 100, 50 };
	RectF m_simulationStartButtonRect{ 380, 20, 80, 50 };
	RectF m_simulationPauseButtonRect{ 470, 20, 80, 50 };
	RectF m_simulationStopButtonRect{ 560, 20, 80, 50 };
	RectF m_simulationFastForwardButtonRect{ 650, 20, 50, 50 };
	RectF m_nextStageButtonRect{ 720, 20, 110, 50 };
	RectF m_queryPanelRect{ 650, 80, 140, 500 };  // クエリパネル領域
	InventoryUI m_inventoryUI{ RectF{ 0, 540, 800, 60 } };

	QueryPanel m_queryPanel;

	// 十字キーUI
	DPadUI m_dpadUI;
	DragModeToggle m_dragModeToggle;

	// コンテキストメニュー
	ContextMenu m_contextMenu;
	
	// 右クリック開始位置（動いていない判定用）
	Optional<Vec2> m_rightClickStartPos;
	static constexpr double RightClickMoveThreshold = 5.0;

	// 左クリック開始位置（動いていない判定用）
	Optional<Vec2> m_leftClickStartPos;
	static constexpr double LeftClickMoveThreshold = 5.0;

	// ドラッグ中のボール情報（インベントリ起点 or ステージ上のPlacedBall起点）
	Optional<DraggingBallInfo> m_draggingBall;

	MyCamera2D m_camera{ Vec2(400, 300), 1.0, CameraControl::Wheel };
	SingleUseCursorPos m_cursorPos;
	SimpleWatch m_timeAfterMouseLDown;
	bool m_isFirstMouseLDown = false;
	Vec2 m_preMouseLDownPos{};
	SimpleWatch m_contextMenuDelayTimer;
	bool m_delayContextMenuDraw = false;

	double m_arrowKeyAccumulate = 0.0;
	Point m_prevArrowKeyInput{ 0, 0 };

	bool m_singleQueryMode = false;

	// タッチ 2本指ジェスチャー（パン/ピンチ）
	bool m_isTwoFingerGesturing = false;
	int32 m_twoFingerId0 = -1;
	int32 m_twoFingerId1 = -1;
	Vec2 m_prevTwoFingerCenter{ 0, 0 };
	double m_prevTwoFingerDistance = 0.0;
	double m_twoFingerBaseScale = 1.0;
	double m_twoFingerBaseDistance = 1.0;

	Array<StageSnapshot> m_undoStack;
	Array<StageSnapshot> m_redoStack;
	static constexpr size_t MAX_UNDO_HISTORY = 50;
	
	// チュートリアルテキスト表示用
	Array<String> m_tutorialTexts;
	size_t m_tutorialPageIndex = 0;
	double m_tutorialDisplayTime = 0.0;
	bool m_tutorialWaitingForClick = false;
	static constexpr double TutorialFadeInTime = 0.3;
	
	// クリア演出用
	bool m_showClearEffect = false;
	double m_clearEffectTime = 0.0;
	Array<ClearParticle> m_clearParticles;
	static constexpr double ClearEffectDuration = 3.0;
	
	void startClearEffect();
	void updateClearEffect(double dt);
	void drawClearEffect() const;

	bool hasSameObjectWithClipboard(const Stage& stage) const;
	void eraseSelection(Stage& stage);
	void onStageEdited(Stage& stage);

	void pasteFromClipboard(Stage& stage);

	void pushUndoState(Stage& stage);
	void undo(Stage& stage);
	void redo(Stage& stage);
	void clearUndoRedoHistory();
};
