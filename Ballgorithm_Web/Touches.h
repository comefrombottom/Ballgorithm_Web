#pragma once

struct TouchRawInfo {
	int32 id;
	Vec2 pos;
};

// タッチ情報を取得する関数
static Array<TouchRawInfo> GetMyTouches() {
# if SIV3D_PLATFORM(WEB)
	Array<TouchRawInfo> result = Array<TouchRawInfo>(EM_ASM_INT({ return window.myTouches.length; }));

	EM_ASM({
		const touches = window.myTouches;

		for (let i = 0; i < touches.length; i++) {
			const touch = touches[i];
			const touchPtr = $0 + i * 24; // TouchRawInfo のサイズに応じて調整

			const adjusted = siv3dAdjustPoint(touch.pageX, touch.pageY);

			setValue(touchPtr, touch.identifier, 'i32');
			setValue(touchPtr + 8, adjusted.x, 'double');
			setValue(touchPtr + 16, adjusted.y, 'double');
		}
		}, result.data());

	for (auto& touch : result)
	{
		touch.pos = Scene::ClientToScene(touch.pos);
	}

	return result;
# else
	return Array<TouchRawInfo>();
# endif
}

struct TouchInfo {
	int32 id;
	Vec2 pos;
	Vec2 deltaPos;
	bool down = false;
	bool up = false;
	bool used = false;
	bool captured = false;

	void capture() {
		captured = true;
		used = true;
	}
	void release() {
		captured = false;
	}
	void use() {
		used = true;
	}
};

class TouchView
{
	using Iter = Array<TouchInfo>::iterator;
	Array<Iter> m_iters;

public:
	TouchView() = default;

	explicit TouchView(Array<TouchInfo>& touches)
	{
		for (auto it = touches.begin(); it != touches.end(); ++it)
		{
			m_iters.push_back(it);
		}
	}

	// ===== range interface =====
	auto begin() { return m_iters.begin(); }
	auto end() { return m_iters.end(); }

	bool isEmpty() const { return m_iters.isEmpty(); }
	explicit operator bool() const { return not isEmpty(); }

	// ===== フィルタ =====
	TouchView downs() const
	{
		TouchView r;
		for (auto it : m_iters)
			if (it->down)
				r.m_iters.push_back(it);
		return r;
	}

	TouchView ups() const
	{
		TouchView r;
		for (auto it : m_iters)
			if (it->up)
				r.m_iters.push_back(it);
		return r;
	}

	TouchView unused() const
	{
		TouchView r;
		for (auto it : m_iters)
			if (not it->used)
				r.m_iters.push_back(it);
		return r;
	}

	template<class Shape>
	TouchView intersects(const Shape& shape) const
	{
		TouchView r;
		for (auto it : m_iters)
			if (shape.intersects(it->pos))
				r.m_iters.push_back(it);
		return r;
	}

	// ===== 集約 =====
	TouchInfo& front() const { return *m_iters.front(); }
	TouchInfo& back()  const { return *m_iters.back(); }

	size_t size() const { return m_iters.size(); }

	// ===== 副作用 =====
	void use() const
	{
		for (auto it : m_iters)
			it->used = true;
	}

	void capture() const
	{
		for (auto it : m_iters)
			it->captured = true;
	}
};

class TouchesType
{
private:
	Array<TouchInfo> m_touches;

public:
	// ===== 更新 =====
	void update(const Array<TouchRawInfo>& addRawTouches = {})
	{
		auto rawTouches = GetMyTouches();

#if not SIV3D_PLATFORM(WEB)
		if (rawTouches.isEmpty() && MouseL.pressed())
		{
			rawTouches.push_back(TouchRawInfo{ 0, Cursor::PosF() });
		}
#endif

		rawTouches.append(addRawTouches);


		// up 状態の削除
		m_touches.remove_if([](const TouchInfo& t) { return t.up; });

		for (auto& t : m_touches)
		{
			t.down = false;
			t.up = true; // 一旦 up にしておく
			if (not t.captured)
				t.used = false;
		}

		// 新しい raw touch を反映
		for (const auto& raw : rawTouches)
		{
			auto it = m_touches.begin();
			for (; it != m_touches.end(); ++it)
			{
				if (it->id == raw.id)
				{
					break;
				}
			}

			if (it != m_touches.end())
			{
				it->deltaPos = raw.pos - it->pos;
				it->pos = raw.pos;
				it->up = false;
			}
			else
			{
				m_touches.push_back(TouchInfo{
					raw.id,
					raw.pos,
					Vec2{0,0},
					true,   // down
					false   // used
				});
			}
		}
	}

	// ===== 生 access =====
	const Array<TouchInfo>& raw() const { return m_touches; }

	explicit operator bool() const { return not m_touches.isEmpty(); }

	Optional<TouchInfo> getTouch(int32 id) const
	{
		for (const auto& t : m_touches)
			if (t.id == id)
				return t;
		return none;
	}

	// ===== View API =====
	TouchView view()
	{
		return TouchView(m_touches);
	}

	TouchView downs()
	{
		return view().downs();
	}

	TouchView ups()
	{
		return view().ups();
	}

	TouchView unused()
	{
		return view().unused();
	}

	template<class Shape>
	TouchView intersects(const Shape& shape)
	{
		return view().intersects(shape);
	}
};

inline TouchesType Touches;




