#pragma once
#include <cassert>

namespace koma {
	class Vector2 {
	public:
		int x;
		int y;

		Vector2() :x(0), y(0) {}
		Vector2(int x, int y) : x(x), y(y) {}
		Vector2(int pos) : x(pos / 5), y(pos % 5) {}

		inline unsigned tou();
		inline bool isInside();

		inline bool operator==(const Vector2& v);
		inline bool operator!=(const Vector2& v);
		inline Vector2& operator+=(const Vector2& v);
		inline Vector2 operator+(const Vector2& v)const;
	};

	inline unsigned Vector2::tou() {
		return 5u * x + y;
	}
	inline bool Vector2::isInside() {
		return 0 <= x && x < 9 && 0 <= y && y < 9;
	}

	inline bool Vector2::operator==(const Vector2& v) {
		return x == v.x && y == v.y;
	}
	inline bool Vector2::operator!=(const Vector2& v) {
		return !operator==(v);
	}
	inline Vector2& Vector2::operator+=(const Vector2& v) {
		x += v.x; y += v.y;
		return *this;
	}
	inline Vector2 Vector2::operator+(const Vector2& v)const {
		return Vector2(x + v.x, y + v.y);
	}

}