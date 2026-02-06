#pragma once

# include <Siv3D.hpp>

// 一つのオブジェクトに一回だけ CursorPos が使えるようにするためのラッパークラス
class SingleUseCursorPos {
	bool m_notHovered = true;
	bool m_isCaptured = false; // カーソル位置がキャッチされているかどうか
public:
	SingleUseCursorPos() = default;

	bool has_value() const { return m_notHovered; }
	operator bool() const { return has_value(); }

	Vec2 operator*() const {
		if (m_notHovered) {
			return Cursor::PosF();
		}
		throw std::runtime_error("Cursor position is not set.");
	}

	bool isCaptured() const { return m_isCaptured; }

	void init() {
		if (m_isCaptured) {
			return;
		}
		m_notHovered = true;
	}

	Vec2 value() const {
		if (m_notHovered) {
			return Cursor::PosF();
		}
		throw std::runtime_error("Cursor position is not set or has already been used.");
	}

	void reset() { m_notHovered = false; }

	Vec2 use() {
		if (m_notHovered) {
			m_notHovered = false;
			return Cursor::PosF();
		}
		throw std::runtime_error("Cursor position has already been used or is not set.");
	}

	template <typename T>
	bool intersects_use(const T& body) {
		if (m_notHovered) {
			bool result = body.intersects(Cursor::PosF());
			if (result) {
				m_notHovered = false;
			}
			return result;
		}
		return false;
	}

	template <typename F>
	bool eval_use(F&& func) {
		if (m_notHovered) {
			bool result = func(Cursor::PosF());
			if (result) {
				m_notHovered = false;
			}
			return result;
		}
		return false;
	}

	void capture() {
		m_notHovered = false;
		m_isCaptured = true;
	}

	void release() { m_isCaptured = false; }
};
