#pragma once

#include <Siv3D.hpp>
#include "Domain.hpp"

// インベントリに入れられるオブジェクトの種類
enum class InventoryObjectKind
{
	Ball,  // ボール（BallKind で種類を指定）
};

struct InventorySlot
{
	InventoryObjectKind kind{};
	BallKind ballKind{};         // kind == Ball の場合、ボールの種類
	Optional<size_t> maxCount;   // none = infinite
	size_t usedCount = 0;

	bool hasRemaining() const
	{
		return (not maxCount) || (usedCount < *maxCount);
	}

	size_t remaining() const
	{
		if (maxCount)
		{
			return *maxCount - usedCount;
		}
		// 無制限の場合は便宜上とても大きな値を返す
		return static_cast<size_t>(std::numeric_limits<uint32>::max());
	}
	
	// ボールスロットを作成するヘルパー
	static InventorySlot CreateBallSlot(BallKind ballKind, Optional<size_t> maxCount = none)
	{
		return InventorySlot{ InventoryObjectKind::Ball, ballKind, maxCount, 0 };
	}
};

// インベントリのUI表示・ヒットテスト機能のみを持つクラス
// データ（スロット情報）はStageで管理される
class InventoryUI
{
	RectF m_barRect;           // 全体バー領域
	double m_slotWidth = 70.0; // 1スロットの幅
	double m_slotHeight = 70.0;
	double m_padding = 10.0;   // スロット間のパディング

	RectF calcDrawRect(const size_t slotCount) const
	{
		// slotCount==0 の場合は「最低幅バー」として扱う
		double totalContentWidth = (slotCount == 0)
			? 0.0
			: (slotCount * m_slotWidth + (slotCount + 1) * m_padding);
		double barWidth = Max(totalContentWidth, 400.0); // 最低幅

		RectF drawRect = m_barRect;
		drawRect.w = barWidth;
		drawRect.x = (Scene::Width() - barWidth) / 2.0;
		return drawRect;
	}

public:
	InventoryUI() = default;
	explicit InventoryUI(const RectF& barRect) : m_barRect(barRect) {}

	void setBarRect(const RectF& barRect) { m_barRect = barRect; }

	// インベントリバー全体の矩形内かどうかを判定
	bool hitTestBar(const Vec2& pos) const
	{
		// 互換用: draw 側の最低幅(400)を使って判定
		return calcDrawRect(0).contains(pos);
	}

	// インベントリバー全体の矩形内かどうかを判定（スロット数に応じた実際の描画矩形）
	bool hitTestBar(const Vec2& pos, const size_t slotCount) const
	{
		return calcDrawRect(slotCount).contains(pos);
	}

	// 画面上のどのスロットかを返す（バー外なら none）
	Optional<size_t> hitTestSlot(const Vec2& pos, size_t slotCount) const
	{
		if (slotCount == 0) return none;

		const RectF drawRect = calcDrawRect(slotCount);
		if (!drawRect.contains(pos)) return none;

		// バーの中央揃え計算
		double totalWidth = slotCount * m_slotWidth + (slotCount + 1) * m_padding;
		double startX = drawRect.x + (drawRect.w - totalWidth) / 2.0;

		if (pos.x < startX || pos.x > startX + totalWidth) return none;

		double localX = pos.x - startX - m_padding;
		if (localX < 0) return none; // 左端パディング

		size_t index = static_cast<size_t>(localX / (m_slotWidth + m_padding));

		// パディング部分をクリックした場合は無効
		double offsetInSlot = localX - index * (m_slotWidth + m_padding);
		if (offsetInSlot > m_slotWidth) return none;

		if (index >= slotCount) return none;
		return index;
	}

	// インベントリバーの描画（スロット情報を外部から受け取る）
	void draw(const Array<InventorySlot>& slots) const
	{
		// スロットが空なら描画しない
		if (slots.empty()) return;

		const RectF drawRect = calcDrawRect(slots.size());
		const double totalContentWidth = slots.size() * m_slotWidth + (slots.size() + 1) * m_padding;

		// 影
		drawRect.movedBy(0, 4).rounded(15).draw(ColorF(0.0, 0.3));

		// バー背景（半透明のダークカラー）
		drawRect.rounded(15).draw(ColorF(0.15, 0.18, 0.22, 0.9));
		drawRect.rounded(15).drawFrame(1, ColorF(0.3, 0.35, 0.4, 0.5));

		// スロット描画開始位置
		double startX = drawRect.x + (drawRect.w - totalContentWidth) / 2.0 + m_padding;
		double startY = drawRect.y + (drawRect.h - m_slotHeight) / 2.0;

		for (size_t i = 0; i < slots.size(); ++i)
		{
			const auto& slot = slots[i];
			RectF r{ startX + i * (m_slotWidth + m_padding), startY, m_slotWidth, m_slotHeight };
			
			bool isHovered = r.mouseOver();
			bool hasRemaining = slot.hasRemaining();
			
			// スロット背景
			ColorF slotBg = ColorF(0.1, 0.12, 0.15);
			if (isHovered && hasRemaining) {
				slotBg = ColorF(0.2, 0.22, 0.25);
			}
			r.rounded(8).draw(slotBg);
			
			// スロット枠
			ColorF frameColor = hasRemaining ? ColorF(0.4, 0.45, 0.5) : ColorF(0.3, 0.3, 0.3);
			if (isHovered && hasRemaining) {
				frameColor = ColorF(0.6, 0.7, 0.8);
			}
			r.rounded(8).drawFrame(1.5, frameColor);

			// アイコン描画（BallKind に基づく）
			if (slot.kind == InventoryObjectKind::Ball)
			{
				double radius = GetBallRadius(slot.ballKind) * 0.9; // 少し大きめに
				// 枠内に収まるようにスケーリング
				if (radius > m_slotWidth * 0.35) radius = m_slotWidth * 0.35;
				
				ColorF color = hasRemaining ? GetBallColor(slot.ballKind) : ColorF(Palette::Darkgray);
				
				// ホバー時のエフェクト
				if (isHovered && hasRemaining) {
					Circle{ r.center(), radius + 2 }.draw(color.withAlpha(0.3));
				}
				
				Circle icon{ r.center(), radius };
				icon.draw(color);
				
				// ボールの光沢
				if (hasRemaining) {
					icon.drawFrame(1.5, ColorF(1.0, 0.3));
				}
			}

			// 残り数バッジ
			if (slot.maxCount)
			{
				int32 remain = static_cast<int32>(*slot.maxCount - slot.usedCount);
				String text = Format(remain);
				
				Vec2 badgePos = r.br() - Vec2(12, 12);
				Circle(badgePos, 10).draw(ColorF(0.2, 0.2, 0.2, 0.8));
				
				ColorF textColor = (remain > 0) ? ColorF(Palette::White) : ColorF(1.0, 0.4, 0.4);
				FontAsset(U"Regular")(text).drawAt(12, badgePos, textColor);
			}
			else
			{
				Vec2 badgePos = r.br() - Vec2(12, 12);
				FontAsset(U"Regular")(U"∞").drawAt(14, badgePos, ColorF(0.6, 0.8, 1.0));
			}
		}
	}
};

// 後方互換性のためのエイリアス（既存のコードがあれば）
using Inventory = InventoryUI;

