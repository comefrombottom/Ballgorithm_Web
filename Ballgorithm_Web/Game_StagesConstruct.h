#pragma once
#include <Siv3D.hpp>
#include "Stage.hpp"



// GoalAreaと受け皿のlineを解除不可能なグループとして追加するヘルパー関数
void addGoalAreaWithContainer(Stage& stage, const RectF& rect, bool isLocked = true)
{
	int32 goalAreaId = stage.addGoalArea({ rect, isLocked });

	// 受け皿のline（左、下、右の3辺）
	int32 edge1 = stage.addLine({ rect.bl(), rect.tl() }, isLocked);  // 左
	int32 edge2 = stage.addLine({ rect.bl(), rect.br() }, isLocked);  // 下
	int32 edge3 = stage.addLine({ rect.br(), rect.tr() }, isLocked);  // 右

	// グループを作成
	Group group;
	group.insertGoalAreaId(goalAreaId);

	// 各エッジのポイントIDをグループに追加
	auto [p1a, p1b] = stage.getEdgePointIds(edge1);
	auto [p2a, p2b] = stage.getEdgePointIds(edge2);
	auto [p3a, p3b] = stage.getEdgePointIds(edge3);
	group.insertPointId(p1a);
	group.insertPointId(p1b);
	group.insertPointId(p2a);
	group.insertPointId(p2b);
	group.insertPointId(p3a);
	group.insertPointId(p3b);

	stage.createLockedGroup(group);
}

void addStartDelayBox(Stage& stage, const Vec2& pos) {
	stage.addNonEditableArea(RectF(pos, Vec2{ 200,200 }).stretched(-1));

	stage.addStartCircle({ Circle{ pos + Vec2{150, 50}, 20}, true });
	stage.addStartCircle({ Circle{ pos + Vec2{150, 150}, 20 }, true });

	const Vec2 tr = pos + Vec2{ 200,0 };
	const Vec2 rc = pos + Vec2{ 200, 100 };
	const Vec2 bl = pos + Vec2{ 0, 180 };
	const Vec2 br = pos + Vec2{ 200, 200 };
	const Vec2 c = pos + Vec2{ 50, 120 };


	stage.addLine(Line(rc, c), true);

	stage.addLine(Line(tr, rc), true);
	stage.addLine(Line(pos, tr), true);
	stage.addLine(Line(pos, bl), true);
	stage.addLine(Line(bl, br), true);
}

void addNonEditableGoal(Stage& stage, const Vec2& pos) {
	const Vec2 goalPos = pos + Vec2{ 100, 50 };

	stage.addNonEditableArea(RectF(pos, Vec2{ 200,150 }).stretched(-1));
	stage.addLine(Line(pos, goalPos), true);
	addGoalAreaWithContainer(stage, RectF{ goalPos, 80, 80 });
}



// 変数制限のためstagesConstructを分割
void stagestagesConstruct_0(Array<std::unique_ptr<Stage>>& m_stages) {
	// Straight
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Straight";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		// チュートリアルテキストを設定（オプション）
		stage->m_tutorialTexts = {
			U"ダブルクリックで線を引くことができます",
			U"線を引いて、ボールをゴールに導こう！",
			U"準備ができたら右上の「Run」ボタンで実行！"
		};

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}

	// Detour
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Detour";

		stage->addStartCircle({ Circle{ 300, 200, 20 }, true });

		stage->addLine(Line({ 100, 350 }, { 500, 350 }), true);

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}

	// NonEditable
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"NonEditable";

		// 編集不可エリア（ここには線を引けない/貫通できない）
		stage->addNonEditableArea(RectF::FromPoints(Vec2(240, 250), Vec2(580, 480)).stretched(-1));

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));

		stage->m_tutorialTexts = {
			U"赤いエリアには新たに線を引くことが出来ません",
		};


		m_stages.push_back(std::move(stage));
	}

	// Move Camera
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Move Camera";

		stage->addStartCircle({ Circle{ 1200, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 1500, 400, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		// チュートリアルテキストを設定（オプション）
		stage->m_tutorialTexts = {
			U"マウス右ドラッグもしくは二本指でカメラが動かせます",
			U"マウスホイールもしくは二本指で拡大縮小ができます",
			U"右下のボタンでモードを変更できます",
		};

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}


	// Curve
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Curve";

		stage->addNonEditableArea(RectF::FromPoints(Vec2(150, 80), Vec2(600, 600)).stretched(-1));

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });

		stage->addLine(Line({ 90, 400 }, { 380, 400 }), true);
		stage->addLine(Line({ 90, 80 }, { 90, 400 }), true);

		addGoalAreaWithContainer(*stage, RectF{ 550, 500, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}

	// Jump
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Jump";

		// 編集不可エリア（ここには線を引けない/貫通できない）
		stage->addNonEditableArea(RectF::FromPoints(Vec2(240, 80), Vec2(600, 600)).stretched(-1));

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 500, 80, 80 }, true);



		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}

	// Filter
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Filter";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		stage->m_tutorialTexts = {
			// U"クエリが複数あるステージでは同じ構造で全てのクエリを満たす必要があります。",
			U"大きなボールだけをゴールに導こう！"
		};


		m_stages.push_back(std::move(stage));
	}

	// Classification
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Classification";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, none }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// NonEditable (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"NonEditable (2)";

		// 編集不可エリア（ここには線を引けない/貫通できない）
		stage->addNonEditableArea(RectF::FromPoints(Vec2(240, 150), Vec2(370, 480)).stretched(-1));

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));

		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_0_2(Array<std::unique_ptr<Stage>>& m_stages) {

	// Two Paths
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Two Paths";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 300, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// Crossing
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Crossing";

		int32 startCircle1 = stage->addStartCircle({ Circle{ 100, 200, 20 }, true });
		int32 startCircle2 = stage->addStartCircle({ Circle{ 100, 300, 20 }, true });

		Group startGroup;
		startGroup.insertStartCircleId(startCircle1);
		startGroup.insertStartCircleId(startCircle2);
		stage->createLockedGroup(startGroup);

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// Classification (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Classification (2)";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ none, BallKind::Large }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// Crossing (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Crossing (2)";

		int32 startCircle1 = stage->addStartCircle({ Circle{ 100, 200, 20 }, true });
		int32 startCircle2 = stage->addStartCircle({ Circle{ 540, 200, 20 }, true });

		stage->addLine(Line({ 60, 220 }, { 60 + 80, 220 + 20 }), true);
		stage->addLine(Line({ 580, 220 }, { 580 - 80, 220 + 5 }), true);

		//stage->addLine(Line({ 60, 150 }, { 60, 320 }), true);

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -50,150 }, Vec2{ 120,650 }).stretched(-1));



		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 520,150 }, Vec2{ 690,650 }).stretched(-1));

		Group startGroup;
		startGroup.insertStartCircleId(startCircle1);
		startGroup.insertStartCircleId(startCircle2);
		stage->createLockedGroup(startGroup);

		addGoalAreaWithContainer(*stage, RectF{ 60, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 500, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// Crossing (3)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Crossing (3)";

		int32 startCircle1 = stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		int32 startCircle2 = stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		stage->addLine(Line({ 60, 120 }, { 140, 140 }), true);
		stage->addLine(Line({ 60, 220 }, { 140, 240 }), true);

		//stage->addLine(Line({ 60, 150 }, { 60, 320 }), true);

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -50,50 }, Vec2{ 120,650 }).stretched(-1));



		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 520,50 }, Vec2{ 690,650 }).stretched(-1));

		Group startGroup;
		startGroup.insertStartCircleId(startCircle1);
		startGroup.insertStartCircleId(startCircle2);
		stage->createLockedGroup(startGroup);

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	// Collision
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Collision";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{none},
			Array<Optional<BallKind>>{none}
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		stage->m_tutorialTexts = {
			U"インベントリからドラッグでボールを出し、配置することが出来ます。",
		};

		m_stages.push_back(std::move(stage));
	}

	// Hit
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Hit";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 300,80 }, Vec2{ 600,600 }).stretched(-1));
		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 260,80 }, Vec2{ 300,120 }).stretched(-1));

		stage->addPlacedBall(PlacedBall{ Vec2{280,100},BallKind::Small, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 500, 60, 40 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{none},
			Array<Optional<BallKind>>{none}
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		// stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Be Small
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Be Small";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Not
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Not";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_0_3(Array<std::unique_ptr<Stage>>& m_stages) {

	// Obstacle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Obstacle";

		stage->addStartCircle({ Circle{ 200, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none }
		);

		stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Cancel Out
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Cancel Out";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 500, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none },
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none }
		);

		m_stages.push_back(std::move(stage));
	}

	// Slow
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Slow";

		stage->addStartCircle({ Circle{ -200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 00, 225, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 350, 20 }, true });

		stage->addLine(Line({ -240, 100 }, { 240, 400 }), true);

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -240, 80 }, Vec2{ 240, 400 }).stretched(-1));

		stage->addLine(Line({ 440, 500 }, { 540, 500 }), true);
		stage->addLine(Line({ 540, 500 }, { 540, 600 }), true);

		addGoalAreaWithContainer(*stage, RectF{ 540, 600, 80, 80 });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 440, 100 }, Vec2{ 820, 680 }).stretched(-1));

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->m_tutorialTexts = {
			U"上部バーStopボタンの右にあるボタンで、速度を速くできます",
		};

		m_stages.push_back(std::move(stage));
	}

	// Box
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Box";

		stage->addLine(Line(240, 295, 240, 460), true);
		stage->addLine(Line(240, 460, 560, 480), true);
		stage->addLine(Line(560, 480, 560, 160), true);
		stage->addLine(Line(560, 160, 240, 160), true);
		stage->addLine(Line(240, 220, 495, 240), true);
		stage->addLine(Line(495, 240, 530, 240), true);
		stage->addLine(Line(505, 220, 530, 220), true);
		stage->addLine(Line(530, 220, 530, 240), true);
		stage->addLine(Line(560, 260, 550, 295), true);
		stage->addLine(Line(550, 295, 540, 310), true);
		stage->addLine(Line(540, 310, 525, 320), true);
		stage->addLine(Line(240, 295, 410, 315), true);
		stage->addLine(Line(500, 385, 500, 480), true);

		stage->addPlacedBall(PlacedBall{ {505, 210} , BallKind::Small,true });
		//stage->addPlacedBall(PlacedBall{ {0, 210} , BallKind::Small,true });

		stage->addNonEditableArea(RectF{ 240, 160, 320, 320 }.stretched(-1));

		stage->addGoalArea({ RectF{ 500,400,60,80 } ,true });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		// stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, 2);
		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_1(Array<std::unique_ptr<Stage>>& m_stages) {

	// Send Once
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Send Once";

		stage->addStartCircle({ Circle{100,200,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none },
			Array<Optional<BallKind>>{ none }
		);
		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{StartBallState{ BallKind::Large }}, 0.0 },
				{ Array<Optional<StartBallState>>{StartBallState{ BallKind::Large }}, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Large, 1);


		m_stages.push_back(std::move(stage));
	}

	// Send Big Once
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Send Big Once";

		stage->addStartCircle({ Circle{100,200,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);


		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Send Second
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Send Second";
		stage->addStartCircle({ Circle{100,200,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		m_stages.push_back(std::move(stage));
	}

	// Or
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"OR";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}


	// Third Only
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Third Only";
		stage->addStartCircle({ Circle{100,200,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Double Big
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Double Big";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(

			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ none }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);


		m_stages.push_back(std::move(stage));
	}

	// And
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"And";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Nor
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Nor";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Nand
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Nand";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_2(Array<std::unique_ptr<Stage>>& m_stages) {

	// Double
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Double";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none },
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Replication
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Replication";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none },
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	//　Mix
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Mix";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });
		stage->addStartCircle({ Circle{ 100, 300, 20 }, true });
		stage->addStartCircle({ Circle{ 100, 400, 20 }, true });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -50,50 }, Vec2{ 80,950 }).stretched(-1));

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 520,50 }, Vec2{ 690,950 }).stretched(-1));

		addGoalAreaWithContainer(*stage, RectF{ 500, 300, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 450, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 600, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 500, 750, 80, 80 });

		auto query1 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Large }
		);

		auto query2 = std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small, BallKind::Large, BallKind::Small }
		);

		stage->m_queries->push_back(std::move(query1));
		stage->m_queries->push_back(std::move(query2));

		m_stages.push_back(std::move(stage));
	}

	//　Mix Six
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Mix Six";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 500, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 600, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 60, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 160, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 260, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 360, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 460, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 560, 500, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small, BallKind::Small, BallKind::Small, BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Small, BallKind::Small, BallKind::Small }
		);

		m_stages.push_back(std::move(stage));
	}

	// Replication (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Replication (2)";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -400,80 }, Vec2{ -120,650 }).stretched(-1));
		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 520,80 }, Vec2{ 800,650 }).stretched(-1));


		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 });

		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none },
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Replication Two
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Replication Two";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ -100, 350, 20 }, true });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -400,80 }, Vec2{ -120,1050 }).stretched(-1));
		stage->addNonEditableArea(RectF::FromPoints(Vec2{ 620,80 }, Vec2{ 900,1050 }).stretched(-1));


		addGoalAreaWithContainer(*stage, RectF{ 600, 500, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 600, 650, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 600, 800, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 600, 950, 80, 80 });

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none },
			Array<Optional<BallKind>>{ none, none, none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small, BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large, BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small, BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large, BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, 2);
		stage->addInventorySlot(BallKind::Large, 2);

		m_stages.push_back(std::move(stage));
	}

	// And Or
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"And Or";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ -100, 350, 20 }, true });

		stage->addNonEditableArea(RectF::FromPoints(Vec2{ -200,80 }, Vec2{ -120,1050 }).stretched(-1));

		stage->addLine(Line{ 600,500,650,500 });
		stage->addLine(Line{ 600,650,650,650 });
		stage->addLine(Line{ 600,800,650,800 });
		stage->addLine(Line{ 600,950,650,950 });

		addGoalAreaWithContainer(*stage, RectF{ 1100, 800, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 1100, 1100, 80, 80 });



		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none },
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

	// Nand Or
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Nand Or";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ -100, 350, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 1100, 800, 80, 80 });
		addGoalAreaWithContainer(*stage, RectF{ 1100, 1100, 80, 80 });



		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none },
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);
		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

	// Xor
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Xor";

		stage->addStartCircle({ Circle{ -100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ -100, 350, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 1800, 1300, 80, 80 });



		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none },
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, StartBallState{ BallKind::Large } },
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

}

void stagestagesConstruct_3(Array<std::unique_ptr<Stage>>& m_stages) {

	// Change Second
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Change Second";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,250,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,250,80,80 });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
		}
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Change First
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Change First";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,250,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,250,80,80 });

		stage->addNonEditableArea(RectF{ 500,400,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,400,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Large, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Same
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Same";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,300,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,300,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);

		m_stages.push_back(std::move(stage));
	}

	// Type Count
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Type Count";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,300,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,300,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Large, 1} })
		}
		);


		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Large, 2} })
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Large, 2} })
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Large, 1} })
		}
		);

		// stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 2);

		m_stages.push_back(std::move(stage));
	}

	// Different
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Different";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 500,300,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 500,300,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ none }
		);

		// stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 2);

		m_stages.push_back(std::move(stage));
	}

	// Xor (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Xor (2)";

		stage->addStartCircle({ Circle{100,100,20},true });

		stage->addNonEditableArea(RectF{ 600,500,80,80 }.stretched(-1));
		addGoalAreaWithContainer(*stage, RectF{ 600,500,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none }, 0.0 },
		},
			Array<Optional<BallKind>>{ none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 2);

		m_stages.push_back(std::move(stage));
	}

}

void stagestagesConstruct_4(Array<std::unique_ptr<Stage>>& m_stages) {

	// Branch
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Branch";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Branch Three
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Branch Three";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500, 700, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Large, 2);

		m_stages.push_back(std::move(stage));
	}

	//Branch Any
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Branch Any";

		stage->addStartCircle({ Circle{ 100, 200, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 500, 400, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500, 550, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);


		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Change Route
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Change Route";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		m_stages.push_back(std::move(stage));
	}

	// Switch
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Switch";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small,0} }),
				GoalRequirement::multi({ {BallKind::Small,1} }),
				GoalRequirement::multi({ {BallKind::Large,2} })
		}
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Change Route Any
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Change Route Any";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Large, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Toggle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Toggle";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Catch And Release
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Catch And Release";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
		},
			Array<Optional<BallKind>>{ none, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		m_stages.push_back(std::move(stage));
	}

	// Big Toggle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Big Toggle";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 400, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 400, 80, 160 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4} }),
							GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4} }),
							GoalRequirement::multi({ {BallKind::Small, 6} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 7} }),
							GoalRequirement::multi({ {BallKind::Small, 6} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Branch Repeat
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Branch Repeat";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 450, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 450, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, none }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
							{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
							{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
							{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
		}
		);


		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	// Send Once Repeat
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Send Once Repeat";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		// GoalAreaと受け皿をグループ化
		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
		}
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	{}

	// Send Second Repeat
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Send Second Repeat";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 450, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
		}
		);

		stage->addInventorySlot(BallKind::Large, 1);

		m_stages.push_back(std::move(stage));
	}

	{}

}

void stagestagesConstruct_5(Array<std::unique_ptr<Stage>>& m_stages) {

	// Or Way
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Or Way";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });


		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 550, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 550, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);


		m_stages.push_back(std::move(stage));
	}


	// And Way
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"And Way";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });


		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 550, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 550, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ none, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, 1);


		m_stages.push_back(std::move(stage));
	}

	// Or Way Repeat
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Or Way Repeat";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 600, 100, 20 }, true });


		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 650, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 200 - 40, 650, 80, 80 }, true);

		addGoalAreaWithContainer(*stage, RectF{ 400 - 40, 650, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Large, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Large, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 1} }),
		}
		);

		// 2回目以降 00 

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		// 2回目以降 01

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		// 2回目以降 10

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		// 2回目以降 11
		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, none, StartBallState{ BallKind::Large } }, none },
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 2} }),
				GoalRequirement::multi({ {BallKind::Large, 2} }),
		}
		);

		stage->addInventorySlot(BallKind::Large, 2);


		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_5_2(Array<std::unique_ptr<Stage>>& m_stages) {

	// Decoder
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Decoder";

		stage->addStartCircle({ Circle{000,100,20},true });

		addGoalAreaWithContainer(*stage, RectF{ 500,600,60,60 });
		addGoalAreaWithContainer(*stage, RectF{ 600,600,60,60 });
		addGoalAreaWithContainer(*stage, RectF{ 700,600,60,60 });
		addGoalAreaWithContainer(*stage, RectF{ 800,600,60,60 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small, none, none, none }
		);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ none, BallKind::Small, none, none }
		);
		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ none, none, BallKind::Small, none }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ none, none, none, BallKind::Small }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

	// Encoder
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Encoder";

		stage->addStartCircle({ Circle{000,100,20},true });
		stage->addStartCircle({ Circle{100,100,20},true });
		stage->addStartCircle({ Circle{200,100,20},true });
		stage->addStartCircle({ Circle{300,100,20},true });

		addGoalAreaWithContainer(*stage, RectF{ 500,600,60,60 });
		addGoalAreaWithContainer(*stage, RectF{ 600,600,60,60 });
		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none, none, none },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small }, none, none },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none, StartBallState{ BallKind::Small }, none },
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SampleQuery>(
			Array<Optional<StartBallState>>{ none, none, none, StartBallState{ BallKind::Small } },
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

	// Xor And
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Xor And";

		stage->addStartCircle({ Circle{00,100,20},true });

		addGoalAreaWithContainer(*stage, RectF{ 500,480,60,60 });
		addGoalAreaWithContainer(*stage, RectF{ 500,600,60,60 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Large, BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none }
		},
			Array<Optional<BallKind>>{ BallKind::Small, BallKind::Large }
		);

		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);

		m_stages.push_back(std::move(stage));
	}

	// Change Not
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Change Not";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 200 - 40,480,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, 0.0 },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);


		m_stages.push_back(std::move(stage));
	}

	// Xor (3)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Xor (3)";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 200 - 40,480,80,80 });

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large }
		);

		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, 0.0 },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Small }
		);


		stage->addInventorySlot(BallKind::Small, 1);
		stage->addInventorySlot(BallKind::Large, 1);


		m_stages.push_back(std::move(stage));
	}

	// increment
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"increment";

		stage->addStartCircle({ Circle{ 0, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 500,480,80,80 });

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0},{BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 1} }),
					}
				},
		}
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 1} }),
					}
				},
		}
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0},{BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0},{BallKind::Large, 2} }),
					}
				},
		}
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2},{BallKind::Large, 0} }),
					}
				},
		}
		);


		stage->addInventorySlot(BallKind::Small, none);
		stage->addInventorySlot(BallKind::Large, none);


		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_5_3(Array<std::unique_ptr<Stage>>& m_stages) {

	// Queue
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Queue";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 550, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 550, 80, 80 }, true);


		(*stage->m_queries) << std::make_unique<SequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
		},
			Array<Optional<BallKind>>{ BallKind::Large, none }
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
		}
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Queue (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Queue (2)";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 550, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 550, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
						GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
						GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
						GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				}
		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Small Queue
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Small Queue";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 450, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 450, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 0} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
						GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
						GoalRequirement::multi({ {BallKind::Small, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
						GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				}
		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Small Queue (2)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Small Queue (2)";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 550, 80, 80 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 550, 80, 80 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
						GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
						GoalRequirement::multi({ {BallKind::Small, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
						GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				}
		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Queue (3)
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Queue (3)";

		stage->addStartCircle({ Circle{ 100, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 300, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 80, 400, 160, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 80, 400, 160, 160 }, true);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
		}
		);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ none, StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
						GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
						GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
						GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 4} }),
						GoalRequirement::multi({ {BallKind::Large, 4} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 5} }),
						GoalRequirement::multi({ {BallKind::Large, 5} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 6} }),
						GoalRequirement::multi({ {BallKind::Large, 6} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 7} }),
						GoalRequirement::multi({ {BallKind::Large, 7} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 8} }),
						GoalRequirement::multi({ {BallKind::Large, 8} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 9} }),
						GoalRequirement::multi({ {BallKind::Large, 9} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none }, none },
					},
					Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 10} }),
						GoalRequirement::multi({ {BallKind::Large, 10} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_6(Array<std::unique_ptr<Stage>>& m_stages) {


	// Self Toggle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Self Toggle";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 400, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 400, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 2);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 4} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 4} }),
							GoalRequirement::multi({ {BallKind::Large, 4} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Big Self Toggle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Big Self Toggle";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 400, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 400, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 2);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2},{BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1},{BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2},{BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 3},{BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4} ,{BallKind::Large, 2}}),
							GoalRequirement::multi({ {BallKind::Small, 3},{BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4},{BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Small, 6},{BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 7},{BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Small, 6},{BallKind::Large, 3} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Self Toggles
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Self Toggles";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500 - 40, 500, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 4);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} })
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} })
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} })
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} })
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 4} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} })
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 4} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} })

					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 5} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} })

					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 5} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} })

					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 6} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} })

					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 6} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} })

					}
				},
		}
		);

		m_stages.push_back(std::move(stage));
	}

	{}

	// 2Bit Counter
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"2Bit Counter";

		stage->addStartCircle({ Circle{ 0, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });
		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500 - 40, 500, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 4);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 1} }),
				GoalRequirement::multi({ {BallKind::Small, 1} }),
		}
		);

		(*stage->m_queries) << std::make_unique<MultiGoalSequentialQuery>(
			Array<DelayedBallRelease>{
				{ Array<Optional<StartBallState>>{ none, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large }, none, none }, none },
				{ Array<Optional<StartBallState>>{ none,StartBallState{ BallKind::Small },StartBallState{ BallKind::Small } }, none },
		},
			Array<GoalRequirement>{
			GoalRequirement::multi({ {BallKind::Small, 0} }),
				GoalRequirement::multi({ {BallKind::Small, 0} }),
		}
		);

		m_stages.push_back(std::move(stage));
	}

	{}

	// Small Toggle
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Small Toggle";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 400, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 400, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 2);
		stage->addInventorySlot(BallKind::Small, 2);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2} }),
							GoalRequirement::multi({ {BallKind::Small, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Small, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 3} }),
							GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4} }),
							GoalRequirement::multi({ {BallKind::Small, 3} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 4} }),
							GoalRequirement::multi({ {BallKind::Small, 4} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}

	// Self Toggle Any
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Self Toggle (2)";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 2);
		stage->addInventorySlot(BallKind::Small, 2);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,1} }),
							GoalRequirement::multi({ {BallKind::Small, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,1 } }),
							GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,1 } }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2}, {BallKind::Large,1 } }),
							GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,1 } }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2}, {BallKind::Large,1 } }),
							GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,2 } }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2}, {BallKind::Large,2 } }),
							GoalRequirement::multi({ {BallKind::Small, 1}, {BallKind::Large,2 } }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Small } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Small, 2}, {BallKind::Large,2 } }),
							GoalRequirement::multi({ {BallKind::Small, 2}, {BallKind::Large,2 } }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}


	// Four Rotate
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Four Rotate";

		stage->addStartCircle({ Circle{ 400, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 700 - 40, 500, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 6);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},
		}
		);

		m_stages.push_back(std::move(stage));
	}

	{}

	// Three Rotate
	{
		auto stage = std::make_unique<Stage>();
		stage->m_name = U"Three Rotate";

		stage->addStartCircle({ Circle{ 200, 100, 20 }, true });

		addGoalAreaWithContainer(*stage, RectF{ 100 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 300 - 40, 500, 80, 160 }, true);
		addGoalAreaWithContainer(*stage, RectF{ 500 - 40, 500, 80, 160 }, true);

		stage->addInventorySlot(BallKind::Large, 4);

		(*stage->m_queries) << std::make_unique<MultiPhaseQuery>(
			Array<MultiPhaseQuery::Phase>{
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ none }, none },
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 0} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 1} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 2} }),
					}
				},
				{
					Array<DelayedBallRelease>{
						{ Array<Optional<StartBallState>>{ StartBallState{ BallKind::Large } }, none },
					},
						Array<GoalRequirement>{
						GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
							GoalRequirement::multi({ {BallKind::Large, 3} }),
					}
				},

		}
		);

		m_stages.push_back(std::move(stage));
	}
}

void stagestagesConstruct_7(Array<std::unique_ptr<Stage>>& m_stages) {

}

void stagestagesConstruct_8(Array<std::unique_ptr<Stage>>& m_stages) {

}

void stagesConstruct(Array<std::unique_ptr<Stage>>& m_stages) {
	stagestagesConstruct_0(m_stages);
	stagestagesConstruct_0_2(m_stages);
	stagestagesConstruct_0_3(m_stages);
	stagestagesConstruct_1(m_stages);
	stagestagesConstruct_2(m_stages);
	stagestagesConstruct_3(m_stages);
	stagestagesConstruct_4(m_stages);
	stagestagesConstruct_5(m_stages);
	stagestagesConstruct_5_2(m_stages);
	stagestagesConstruct_5_3(m_stages);

	stagestagesConstruct_6(m_stages);
	stagestagesConstruct_7(m_stages);
	stagestagesConstruct_8(m_stages);
}
