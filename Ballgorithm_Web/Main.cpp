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
	});
# endif

	Platform::Web::IndexedDB::Init(U"Ballagorithm");

	Game game;
	FontAsset::Register(U"Regular", FontMethod::MSDF, 30);
	FontAsset::Register(U"Bold", FontMethod::MSDF, 80, Typeface::Bold);
	FontAsset::Register(U"Icon", FontMethod::MSDF, 24, Typeface::Icon_Awesome_Solid);
	Scene::SetBackground(ColorF(0.08, 0.1, 0.14));
	//Scene::SetBackground(Palette::Papayawhip);

	bool subTouchActive = false;
	Vec2 subTouchPos = Vec2::Zero();

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
