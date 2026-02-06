#pragma once
#include "InputUtils.hpp"

struct ScrollBar
{
	RectF moveArea;
	Optional<double> dragOffset;

	double viewHeight = 600;
	double pageHeight = 1000;
	double viewTop = 0;
	double viewVelocity = 0;

	static constexpr double oneWheelScroll = 200;
	static constexpr double mouseOverStretch = 5;
	static constexpr double sliderHeightMin = 20;

	double accumulateTime = 0;
	static constexpr double stepTime = 1.0 / 200;
	static constexpr double resistance = 10;

	static constexpr bool smoothStop = false;
	static constexpr double smoothStopStrength = 40;
	static constexpr double smoothDragLimit = 300;

	double sliderWidthTransition = 0;
	double sliderWidthTransitionVel = 0;
	static constexpr double sliderWidthSmoothTime = 0.05;

	ScrollBar() = default;

	ScrollBar(double pageHeight, double viewHeight = Graphics2D::GetRenderTargetSize().y, const RectF& moveArea = RectF{ Arg::topRight(Graphics2D::GetRenderTargetSize().x - 2,2),10,Graphics2D::GetRenderTargetSize().y - 4 })
		: moveArea(moveArea)
		, viewHeight(viewHeight)
		, pageHeight(pageHeight)
	{

	}

	double sliderHeight() const
	{
		return Max(moveArea.h * viewHeight / pageHeight, sliderHeightMin);
	}

	double sliderYPerViewY() const
	{
		return (moveArea.h - sliderHeight()) / (pageHeight - viewHeight);
	}

	double sliderY() const
	{
		return viewTop * sliderYPerViewY();
	}

	double progress0_1() const {
		return viewTop / (pageHeight - viewHeight);
	}

	RectF sliderRect() const
	{
		return RectF(moveArea.x, moveArea.y + sliderY(), moveArea.w, sliderHeight());
	}

	bool existSlider() const
	{
		return viewHeight < pageHeight;
	}

	bool mouseOver(SingleUseCursorPos& cursorPos) const
	{
		if (not cursorPos) {
			return false;
		}
		return cursorPos.intersects_use(sliderRect().stretched(mouseOverStretch));
	}

	bool mouseOver() const
	{
		return sliderRect().stretched(mouseOverStretch).mouseOver();
	}

	Transformer2D createTransformer() const
	{
		return Transformer2D(Mat3x2::Translate(0, -viewTop), TransformCursor::Yes);
	}

	void scrollBy(double h) {
		viewVelocity = resistance * h;
	}

	void scrollAdd(double h) {
		viewVelocity += resistance * h;
	}

	void scrollTopTo(double y) {
		scrollBy(y - viewTop);
	}

	void scrollBottomTo(double y) {
		scrollBy(y - viewTop - viewHeight);
	}

	void scrollCenterTo(double y) {
		scrollBy(y - viewTop - viewHeight / 2);
	}

	void update(double wheel = Mouse::Wheel(), double delta = Scene::DeltaTime())
	{
		if (not existSlider()) {
			viewTop = 0;
			viewVelocity = 0;
			dragOffset.reset();
			sliderWidthTransition = 0;
			sliderWidthTransitionVel = 0;
			return;
		}

		if (dragOffset)
		{
			const double prevTop = viewTop;
			viewTop = (Cursor::PosF().y - *dragOffset) / sliderYPerViewY();

			if (smoothStop) {
				if (viewTop < 0) {
					viewTop = viewTop / (abs(viewTop / smoothDragLimit) + 2);
				}
				else if (double over = viewTop + viewHeight - pageHeight; over > 0) {
					viewTop = pageHeight - viewHeight + over / (abs(over / smoothDragLimit) + 2);
				}
			}

			viewVelocity = (viewTop - prevTop) / delta;
		}

		if (mouseOver() and MouseL.down())
		{
			dragOffset = Cursor::PosF().y - sliderY();
		}
		else if (dragOffset && MouseL.up())
		{
			dragOffset.reset();
		}

		scrollAdd(wheel * oneWheelScroll);


		for (accumulateTime += delta; accumulateTime >= stepTime; accumulateTime -= stepTime)
		{
			if (not dragOffset) {
				viewTop += viewVelocity * stepTime;
			}

			if (viewVelocity != 0)
			{
				viewVelocity += -viewVelocity * stepTime * resistance;
			}

			if (viewTop < 0)
			{
				if (smoothStop) {
					if (not dragOffset) {
						viewTop += -viewTop * smoothStopStrength * stepTime;
					}
				}
				else {
					viewTop = 0;
					viewVelocity = 0;
				}
			}
			else if (viewTop + viewHeight > pageHeight)
			{

				if (smoothStop) {
					if (not dragOffset) {
						viewTop += (pageHeight - viewHeight - viewTop) * smoothStopStrength * stepTime;
					}
				}
				else {
					viewTop = pageHeight - viewHeight;
					viewVelocity = 0;
				}
			}
		}

		sliderWidthTransition = Math::SmoothDamp(sliderWidthTransition, mouseOver() or dragOffset, sliderWidthTransitionVel, sliderWidthSmoothTime);
	}

	void update(SingleUseCursorPos& cursorPos, double wheel = Mouse::Wheel(), double delta = Scene::DeltaTime())
	{
		if (not existSlider()) {
			viewTop = 0;
			viewVelocity = 0;
			dragOffset.reset();
			sliderWidthTransition = 0;
			sliderWidthTransitionVel = 0;
			return;
		}

		if (dragOffset)
		{
			const double prevTop = viewTop;
			viewTop = (Cursor::PosF().y - *dragOffset) / sliderYPerViewY();

			if (smoothStop) {
				if (viewTop < 0) {
					viewTop = viewTop / (abs(viewTop / smoothDragLimit) + 2);
				}
				else if (double over = viewTop + viewHeight - pageHeight; over > 0) {
					viewTop = pageHeight - viewHeight + over / (abs(over / smoothDragLimit) + 2);
				}
			}

			viewVelocity = (viewTop - prevTop) / delta;
		}

		bool over = mouseOver(cursorPos);

		if (over and MouseL.down())
		{
			dragOffset = Cursor::PosF().y - sliderY();
			cursorPos.capture();
		}
		else if (dragOffset && MouseL.up())
		{
			dragOffset.reset();
			cursorPos.release();
		}

		scrollAdd(wheel * oneWheelScroll);


		for (accumulateTime += delta; accumulateTime >= stepTime; accumulateTime -= stepTime)
		{
			if (not dragOffset) {
				viewTop += viewVelocity * stepTime;
			}

			if (viewVelocity != 0)
			{
				viewVelocity += -viewVelocity * stepTime * resistance;
			}

			if (viewTop < 0)
			{
				if (smoothStop) {
					if (not dragOffset) {
						viewTop += -viewTop * smoothStopStrength * stepTime;
					}
				}
				else {
					viewTop = 0;
					viewVelocity = 0;
				}
			}
			else if (viewTop + viewHeight > pageHeight)
			{

				if (smoothStop) {
					if (not dragOffset) {
						viewTop += (pageHeight - viewHeight - viewTop) * smoothStopStrength * stepTime;
					}
				}
				else {
					viewTop = pageHeight - viewHeight;
					viewVelocity = 0;
				}
			}
		}

		sliderWidthTransition = Math::SmoothDamp(sliderWidthTransition, over or dragOffset, sliderWidthTransitionVel, sliderWidthSmoothTime);
	}

	void draw(const ColorF& color = Palette::Dimgray) const
	{
		if (not existSlider()) return;

		double w = moveArea.w * (sliderWidthTransition * 0.5 + 0.5);

		double topY = moveArea.y + sliderY();
		double bottomY = topY + sliderHeight();
		topY = Clamp(topY, moveArea.y, moveArea.bottomY() - w);
		bottomY = Clamp(bottomY, moveArea.y + w, moveArea.bottomY());

		RectF(moveArea.x - w + moveArea.w, topY, w, bottomY - topY).rounded(moveArea.w / 2).draw(color);
	}

};
