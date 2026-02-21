# include <Siv3D.hpp>
# include "Game.hpp"
# include "Touches.h"
# include "IndexedDB.hpp"

# if SIV3D_PLATFORM(WEB)
EM_JS(void, setupMultiTouchHandler, (), {
	// グローバル変数を定義
	window.myTouches = [];

// タッチイベントの処理を設定
const canvas = Module['canvas'];

function updateTouches(e) {
  window.myTouches = Array.from(e.touches);
  //e.preventDefault(); // 任意：スクロール防止など
}

canvas.addEventListener("touchstart", updateTouches, false);
canvas.addEventListener("touchmove", updateTouches, false);
canvas.addEventListener("touchend", updateTouches, false);
	});


# endif


void Main()
{
	Window::Resize(1280, 720);

# if SIV3D_PLATFORM(WEB)
	System::SetTerminationTriggers(UserAction::NoAction);

	setupMultiTouchHandler();

	EM_ASM({
		let keyDownEvent = null;
		let timeoutId = null;

		addEventListener("keydown", function(event) {
			if (!event.isTrusted) {
				return;
			}
			keyDownEvent = event;
		});

		addEventListener("keyup", function(event) {
			if (!event.isTrusted) {
				return;
			}
			const keyUpEvent = event;
			if (keyDownEvent.timeStamp == keyUpEvent.timeStamp) {
				clearTimeout(timeoutId);
				dispatchEvent(keyDownEvent);
				timeoutId = setTimeout(function() {
					dispatchEvent(keyUpEvent);
					timeoutId = null;
				}, 100);
			}
		});

		{
			let lastTouchendTime = 0;
			addEventListener("touchend", () => {
				lastTouchendTime = Date.now();
			});
			for (const eventType of["mousedown", "mouseup", "mousemove"]) {
				addEventListener(eventType, e => {
					// touchend の約 50ms 後にマウスイベントのエミュレーションが発生する
					// それの誤検知を防ぐため、 touchend の 100ms 後までマウスイベントを無視する
					if (Date.now() - lastTouchendTime < 100) {
						e.stopImmediatePropagation();
					}
				}, true);
			}
		}
	});

	Platform::Web::IndexedDB::Init(U"Ballgorithm");
# endif

	Game game;
	FontAsset::Register(U"Regular", FontMethod::MSDF, 30);
	FontAsset::Register(U"Bold", FontMethod::MSDF, 80, Typeface::Bold);
	FontAsset::Register(U"Icon", FontMethod::MSDF, 24, Typeface::Icon_Awesome_Solid);
	Scene::SetBackground(ColorF(0.08, 0.1, 0.14));
	//Scene::SetBackground(Palette::Papayawhip);

	bool subTouchActive = false;
	Vec2 subTouchPos = Vec2::Zero();

# if SIV3D_PLATFORM(WEB)
	AsyncHTTPTask getTask;
	auto params = Platform::Web::System::GetURLParameters();
	if (params.contains(U"share"))
	{
		getTask = StageSave::CreateGetTask(params[U"share"]);
	}
# endif

	JSON profile = JSON::Load(U"Ballgorithm/profile.json");

	if (not profile) {
		profile = JSON();
	}

	if (profile.contains(U"username") && profile[U"username"].isString()) {
		game.m_username = profile[U"username"].getString();
	}

# if SIV3D_PLATFORM(WEB)
	Platform::Web::IndexedDB::SaveAsync();
	if (!getTask.isEmpty())
	{
		auto asyncTask = Platform::Web::SimpleHTTP::CreateAsyncTask(getTask);
		Platform::Web::System::AwaitAsyncTask(asyncTask);
		StageSave save = StageSave::ProcessGetTask(getTask);
		if (game.m_stageNameToIndex.contains(save.name)) {
			auto index = game.m_stageNameToIndex[save.name];
			auto& stage = *game.m_stages[index];
			stage.removeAllSelectableObjects();
			SelectedIDSet sid;
			stage.pastePointEdgeGroup(save.peg, sid);
			game.selectStage(index);
			game.enterSelectedStage();
		}
	}
# endif
	
	while (System::Update())
	{
		ClearPrint();

		if (KeyR.down())
		{
			subTouchActive = true;
			subTouchPos = Cursor::PosF();
		}
		if (KeyR.up())
		{
			subTouchActive = false;
		}
		if(KeyT.pressed())
		{
			subTouchPos += Vec2(1, 1);
		}

		Array<TouchRawInfo> additionalTouches;
		if (subTouchActive)
		{
			additionalTouches.push_back(TouchRawInfo{ 999, subTouchPos });
		}

		Touches.update(additionalTouches);

		for (const auto& touch : Touches.raw())
		{
			Print << U"Touch: " << touch.id << U" Position: " << touch.pos << U" Up: " << touch.up << U" Down: " << touch.down;
		}

		if (KeyF11.down())
		{
			Scene::SetResizeMode(ResizeMode::Keep);
			Window::SetFullscreen(not Window::GetState().fullscreen);
		}

		game.update();
		game.draw();

		if (subTouchActive)
		{
			Circle{subTouchPos, 10}.drawFrame(0, 3, Palette::Orange);
		}
	}
}
