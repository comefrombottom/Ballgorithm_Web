#pragma once
# include <Siv3D.hpp>

/// @brief シンプルなストップウォッチクラス
/// @details Stopwatchのシンプル版で、pauseなどの機能がない基本的な計測機能のみを提供
/// @note Stopwatchの完全下位互換として設計
class SimpleWatch {
private:
	/// @brief 開始時刻（マイクロ秒）。nulloptの場合は未開始
	Optional<int64> m_startTimeMicrosec;

public:
	/// @brief ストップウォッチを作成します。
	/// @param startImmediately 即座に計測を開始する場合は `StartImmediately::Yes`
	explicit SimpleWatch(StartImmediately startImmediately = StartImmediately::No) {
		if (startImmediately) {
			restart();
		}
	}

	/// @brief 指定した時間だけ経過している状態のストップウォッチを作成します。
	/// @param startTime 経過時間
	explicit SimpleWatch(const Duration& startTime) {
		m_startTimeMicrosec = Time::GetMicrosec() - static_cast<int64>(MicrosecondsF{ startTime }.count());
	}

	/// @brief ストップウォッチが動作中であるかを示します。
	/// @return ストップウォッチが開始されている場合 true, それ以外の場合は false
	[[nodiscard]]
	bool isStarted() const noexcept {
		return m_startTimeMicrosec.has_value();
	}

	/// @brief ストップウォッチを停止し、経過時間を 0 にリセットします。
	void reset() noexcept {
		m_startTimeMicrosec.reset();
	}

	/// @brief 経過時間を 0 にリセットして、ストップウォッチを再び開始します。
	void restart() {
		m_startTimeMicrosec = Time::GetMicrosec();
	}

	/// @brief 経過時間を [ミリ秒] で返します。
	/// @return 経過時間 [ミリ秒]
	[[nodiscard]]
	int32 ms() const {
		return static_cast<int32>(ms64());
	}

	/// @brief 経過時間を [ミリ秒] で返します。
	/// @return 経過時間 [ミリ秒]
	[[nodiscard]]
	int64 ms64() const {
		return us() / 1000LL;
	}

	/// @brief 経過時間を [ミリ秒] で返します。
	/// @return 経過時間 [ミリ秒]
	[[nodiscard]]
	double msF() const {
		return static_cast<double>(us()) / 1000.0;
	}

	/// @brief 経過時間を [秒] で返します。
	/// @return 経過時間 [秒]
	[[nodiscard]]
	int32 s() const {
		return static_cast<int32>(s64());
	}

	/// @brief 経過時間を [秒] で返します。
	/// @return 経過時間 [秒]
	[[nodiscard]]
	int64 s64() const {
		return us() / 1000000LL;
	}

	/// @brief 経過時間を [秒] で返します。
	/// @return 経過時間 [秒]
	[[nodiscard]]
	double sF() const {
		return static_cast<double>(us()) / 1000000.0;
	}

	/// @brief 経過時間を [マイクロ秒] で返します。
	/// @return 経過時間 [マイクロ秒]
	[[nodiscard]]
	int64 us() const {
		if (!m_startTimeMicrosec) {
			return 0;
		}
		return Time::GetMicrosec() - m_startTimeMicrosec.value();
	}

	/// @brief 経過時間を [マイクロ秒] で返します。
	/// @return 経過時間 [マイクロ秒]
	[[nodiscard]]
	int64 us64() const {
		return us();
	}

	/// @brief 経過時間を [マイクロ秒] で返します。
	/// @return 経過時間 [マイクロ秒]
	[[nodiscard]]
	double usF() const {
		return static_cast<double>(us());
	}

	/// @brief 経過時間を返します。
	/// @return 経過時間
	[[nodiscard]]
	Duration elapsed() const {
		return SecondsF{ sF() };
	}
};
