#include "stdafx.h"
#include "MyCamera2D.h"

void MyCamera2D::update(const double deltaTime, const SizeF sceneSize)
{
	{
		const double oldScale = m_scale;
		m_scale = Math::SmoothDamp(m_scale, m_targetScale, m_scaleChangeVelocity, m_parameters.scaleSmoothTime, unspecified, deltaTime);

		if (deltaTime && (m_scale != m_targetScale) && (m_scale == oldScale))
		{
			m_scale = m_targetScale;
		}
	}

	if (m_pointedScale)
	{
		const Vec2 v = m_pointedScale->first - (sceneSize * 0.5);
		m_targetCenter = m_center = (m_pointedScale->second - v / m_scale);
	}
	else
	{
		const Vec2 oldCenter = m_center;
		m_center = Math::SmoothDamp(m_center, m_targetCenter, m_positionChangeVelocity, m_parameters.positionSmoothTime, unspecified, deltaTime);

		if (deltaTime && (m_center != m_targetCenter) && (m_center == oldCenter))
		{
			m_center = m_targetCenter;
		}
	}
}

void MyCamera2D::updateWheel(const SizeF& sceneSize)
{
	if (m_parameters.wheelScaleFactor == 1.0)
	{
		return;
	}

	const double wheel = Mouse::Wheel();

	if (wheel == 0.0)
	{
		return;
	}

	m_positionChangeVelocity = Vec2::Zero();

	if (wheel < 0.0)
	{
		m_targetScale *= m_parameters.wheelScaleFactor;
	}
	else
	{
		m_targetScale /= m_parameters.wheelScaleFactor;
	}

	m_targetScale = Clamp(m_targetScale, m_parameters.minScale, m_parameters.maxScale);

	const auto t1 = Transformer2D{ Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetLocal };
	const auto t2 = Transformer2D{ Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetCamera };

	const Vec2 cursorPos = Cursor::PosF();
	const Vec2 point = (m_center + (cursorPos - (sceneSize * 0.5)) / m_scale);
	m_pointedScale.emplace(cursorPos, point);
}

void MyCamera2D::zoomAt(const Vec2& screenPos, double targetScale, const SizeF& sceneSize)
{
	if (targetScale <= 0.0)
	{
		return;
	}

	m_positionChangeVelocity = Vec2::Zero();

	const double clampedScale = Clamp(targetScale, m_parameters.minScale, m_parameters.maxScale);
	m_targetScale = clampedScale;

	const auto t1 = Transformer2D{ Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetLocal };
	const auto t2 = Transformer2D{ Mat3x2::Identity(), TransformCursor::Yes, Transformer2D::Target::SetCamera };

	const Vec2 point = (m_center + (screenPos - (sceneSize * 0.5)) / m_scale);
	m_pointedScale.emplace(screenPos, point);
}

void MyCamera2D::zoomAtImmediate(const Vec2& screenPos, double targetScale, const SizeF& sceneSize)
{
	if (targetScale <= 0.0)
	{
		return;
	}

	const double clampedScale = Clamp(targetScale, m_parameters.minScale, m_parameters.maxScale);
	const Vec2 screenCenter = sceneSize * 0.5;
	const Vec2 worldPoint = m_center + (screenPos - screenCenter) / m_scale;
	const Vec2 newCenter = worldPoint - (screenPos - screenCenter) / clampedScale;

	m_pointedScale.reset();
	m_positionChangeVelocity = Vec2::Zero();
	m_scaleChangeVelocity = 0.0;
	m_targetScale = m_scale = clampedScale;
	m_targetCenter = m_center = newCenter;
}
