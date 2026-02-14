# include "QueryPanel.h"
# include "Stage.hpp"

void QueryPanel::setRect(const RectF& rect)
{
	m_rect = rect;
}

const RectF& QueryPanel::rect() const
{
	return m_rect;
}

void QueryPanel::onStageEnter(const Stage& stage)
{
	double allQueryHeight = 0.0;
	for (const auto& query : stage.m_queries) {
		allQueryHeight += query->getPanelHeight() + 5.0;
	}

	const double headerH = (stage.m_isCleared ? 68 : 38);
	m_scrollBar = ScrollBar(allQueryHeight, m_rect.h - headerH,
		RectF{ Arg::topRight(m_rect.rightX(), m_rect.y + headerH), 10.0, m_rect.h - headerH });
}

bool QueryPanel::update(Stage& stage, SingleUseCursorPos& cursorPos, double dt)
{
	const double headerH = (stage.m_isCleared ? 68 : 38);

	m_scrollBar.viewHeight = m_rect.h - headerH;
	m_scrollBar.moveArea = RectF{ Arg::topRight(m_rect.rightX() - 2, m_rect.y + headerH + 2), 10.0, m_rect.h - headerH - 8 };

	const bool cursorInPanel = m_rect.contains(Cursor::PosF());

	m_scrollBar.update(cursorPos, Mouse::Wheel() * (cursorInPanel && cursorPos), dt);

	bool clickInput = false;
	if (cursorPos && cursorInPanel && !stage.m_isSimulationRunning) {
		if (MouseL.down()) {
			m_mousePressPos = Cursor::PosF();
			m_isDragging = false;
			cursorPos.capture();
		}
	}

	if (m_mousePressPos) {
		if (m_mousePressPos->distanceFrom(Cursor::PosF()) > 20) {
			m_isDragging = true;
		}

		if (m_isDragging) {
			m_scrollBar.viewVelocity = -Cursor::DeltaF().y / dt;
		}
	}

	if (MouseL.up()) {
		if (m_mousePressPos) {
			if (!m_isDragging) {
				clickInput = true;
			}
			m_mousePressPos.reset();
			m_isDragging = false;
			cursorPos.release();
		}
	}

	bool executed = false;
	if (cursorInPanel && !stage.m_isSimulationRunning) {
		auto scrollTf = m_scrollBar.createTransformer();
		if (clickInput) {
			double currentY = m_rect.y + headerH;
			for (int32 i = 0; i < stage.m_queries.size(); ++i) {
				const auto& query = stage.m_queries[i];
				const double queryHeight = query->getPanelHeight() + 5;
				RectF queryRect{ m_rect.x + 8, currentY, m_rect.w - 16, queryHeight - 5 };

				if (queryRect.contains(Cursor::PosF())) {
					stage.m_currentQueryIndex = i;
					stage.startSimulation();
					executed = true;
					break;
				}

				currentY += queryHeight;
			}
		}
	}

	if (cursorPos && cursorInPanel) {
		cursorPos.use();
	}

	return executed;
}

void QueryPanel::draw(const Stage& stage) const
{
	const double time = Scene::Time();
	const double headerH = (stage.m_isCleared ? 68 : 38);

	RectF panelBg = m_rect.stretched(2);
	panelBg.rounded(12).draw(ColorF(0.0, 0.4));
	m_rect.rounded(12).draw(ColorF(0.12, 0.14, 0.18, 0.95));
	m_rect.rounded(12).drawFrame(1, ColorF(0.3, 0.35, 0.4, 0.5));

	FontAsset(U"Regular")(U"QUERIES").draw(14, m_rect.pos + Vec2(15, 10), ColorF(0.7, 0.75, 0.8));

	if (stage.m_isCleared) {
		double pulse = 0.8 + 0.2 * Math::Sin(time * 4);
		RectF clearBadge{ m_rect.x + 10, m_rect.y + 38, m_rect.w - 20, 22 };
		clearBadge.rounded(6).draw(ColorF(0.2, 0.7, 0.3, 0.3));
		FontAsset(U"Regular")(U"✓ CLEARED!").drawAt(13, clearBadge.center(), ColorF(0.4, 1.0, 0.5, pulse));
	}

	double currentY = m_rect.y + headerH;
	Rect clipRect = RectF(m_rect.x, currentY - 2, m_rect.w, m_rect.y + m_rect.h - currentY - 2).asRect();

	{
		Graphics2D::SetScissorRect(clipRect);
		RasterizerState rs = RasterizerState::Default2D;
		rs.scissorEnable = true;
		const ScopedRenderStates2D rasterizer{ rs };
		{
			auto scrollTf = m_scrollBar.createTransformer();
			for (int32 i = 0; i < stage.m_queries.size(); ++i) {
				const auto& query = stage.m_queries[i];
				const double queryHeight = query->getPanelHeight() + 5;
				RectF queryRect{ m_rect.x + 8, currentY, m_rect.w - 16, queryHeight - 5 };

				ColorF cardBg = ColorF(0.18, 0.2, 0.24, 0.9);
				ColorF cardFrame = ColorF(0.3, 0.35, 0.4, 0.3);
				if (stage.m_isSimulationRunning && stage.m_currentQueryIndex == i) {
					cardBg = ColorF(0.25, 0.3, 0.45, 0.95);
					cardFrame = ColorF(0.4, 0.5, 0.8, 0.8);
				}

				if (queryRect.mouseOver() && !stage.m_isSimulationRunning) {
					cardBg = cardBg.lerp(ColorF(0.3, 0.35, 0.4), 0.3);
					Cursor::RequestStyle(CursorStyle::Hand);
				}

				queryRect.rounded(8).draw(cardBg);
				queryRect.rounded(8).drawFrame(1, cardFrame);

				Circle badge{ queryRect.x + 18, queryRect.y + 16, 12 };
				ColorF badgeColor = ColorF(0.3, 0.35, 0.4);
				if (i < stage.m_queryCompleted.size() && stage.m_queryCompleted[i]) {
					badgeColor = ColorF(0.3, 0.7, 0.4);
				}
				else if (i < stage.m_queryFailed.size() && stage.m_queryFailed[i]) {
					badgeColor = ColorF(0.7, 0.3, 0.3);
				}
				badge.draw(badgeColor);
				FontAsset(U"Regular")(U"{}"_fmt(i + 1)).drawAt(11, badge.center, Palette::White);

				if (i < stage.m_queryCompleted.size() && stage.m_queryCompleted[i]) {
					Circle checkBg{ queryRect.x + queryRect.w - 18, queryRect.y + 16, 10 };
					checkBg.draw(ColorF(0.2, 0.6, 0.3));
					FontAsset(U"Regular")(U"✓").drawAt(12, checkBg.center, Palette::White);
				}
				else if (i < stage.m_queryFailed.size() && stage.m_queryFailed[i]) {
					Circle failBg{ queryRect.x + queryRect.w - 18, queryRect.y + 16, 10 };
					failBg.draw(ColorF(0.6, 0.25, 0.25));
					FontAsset(U"Regular")(U"✗").drawAt(12, failBg.center, Palette::White);
				}

				query->drawPanelContent(queryRect, stage.m_currentQueryIndex == i and stage.m_isSimulationRunning);
				currentY += queryHeight;
			}
		}
	}

	m_scrollBar.draw();
}
