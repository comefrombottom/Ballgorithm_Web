#pragma once

# include <Siv3D.hpp>
# include <ranges>
# include <optional>
# include <concepts>

// T が std::hash を持つかをチェックするコンセプト
template <typename T>
concept is_std_hashable = requires(const T& val) {
	{ std::hash<T>{}(val) } -> std::same_as<std::size_t>;
};

// ハッシュ値を結合して作るヘルパ
template <class... Args>
size_t hash_values(const Args&... args) {
	size_t seed = 0;
	(..., Hash::Combine(seed, args));
	return seed;
}

// dependent false ユーティリティ
template <class>
inline constexpr bool dependent_false_v = false;

// ハッシュ値をキャッシュするラッパークラス
template <typename T>
class HashCache {
public:
	HashCache() = default;
	explicit HashCache(const T& value) : _value(value), _hash(std::nullopt) {}
	explicit HashCache(T&& value) : _value(std::move(value)), _hash(std::nullopt) {}

	// const 参照
	const T& get() const { return _value; }
	// 非 const 参照（変更時にキャッシュ無効化）
	T& get_mutable() { _hash = std::nullopt; return _value; }

	const T& operator*() const { return _value; }
	const T* operator->() const { return &_value; }

	// ハッシュ値を計算または取得（イテラブル型は各要素のハッシュを組み合わせる）
	std::size_t hash() const {
		if (!_hash.has_value()) {
			if constexpr (is_std_hashable<T>) {
				_hash = std::hash<T>{}(_value);
			}
			else if constexpr (std::ranges::range<T> && is_std_hashable<std::ranges::range_value_t<T>>) {
				std::size_t seed = 0;
				for (const auto& item : _value) {
					Hash::Combine(seed, item);
				}
				_hash = seed;
			}
			else {
				static_assert(dependent_false_v<T>, "Type T is not supported for hashing.");
			}
		}
		return _hash.value();
	}

	bool operator==(const HashCache<T>& other) const {
		return hash() == other.hash() && _value == other._value;
	}

	friend void Formatter(FormatData& formatData, const HashCache<T>& hc) {
		Formatter(formatData, hc._value);
	}

private:
	T _value{};
	mutable std::optional<std::size_t> _hash;
};

// HashCache のための std::hash 特殊化
namespace std {
	template <typename T>
	struct hash<HashCache<T>> {
		std::size_t operator()(const HashCache<T>& hc) const noexcept {
			return hc.hash();
		}
	};
}
