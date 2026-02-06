#pragma once

# include <Siv3D.hpp>

inline P2Body createLine(P2World& world, P2BodyType bodyType, const Line& line, OneSided oneSided = OneSided::No, const P2Material& material = {}, const P2Filter& filter = {})
{
	Vec2 worldPos = line.center();
	return world.createLine(bodyType, worldPos, line.movedBy(-worldPos), oneSided, material, filter);
}

inline P2Body createCircle(P2World& world, P2BodyType bodyType, const Circle& circle, const P2Material& material = {}, const P2Filter& filter = {})
{
	Vec2 worldPos = circle.center;
	return world.createCircle(bodyType, worldPos, Circle(Vec2(0, 0), circle.r), material, filter);
}

inline Vec2 Snap(const Vec2& pos, double snapSize = 5.0)
{
	return Round(pos / snapSize) * snapSize;
}
