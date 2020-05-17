#pragma once
#include <array>
#include <string>

class Bitboard {
public:
	static Bitboard genOneposBB(unsigned pos) { Bitboard bb; bb.set(pos); return bb; }
private:
	static const unsigned bbNum = 25;
public:
	Bitboard() : _p(0) {}
	Bitboard(std::uint32_t bb) : _p(bb & 0x1FFFFFFu){}
	std::string toString();

	//bit操作・観測
	unsigned pop_first();//先頭の1のbitを探してindexを返し、0にする
	unsigned find_first()const;	//bitを順に探す 見つからなければbbNumを返す
	unsigned find_next(const unsigned)const;
	unsigned find_last()const;
	unsigned popcount()const;
	bool test(const unsigned pos)const;
	void set(const unsigned pos, const bool value);
	void set(const unsigned pos);
	void reset(const unsigned pos);
	void all_reset();
	bool none()const;//すべて0ならtrue
	bool any()const { return !none(); }
	Bitboard getLineOR()const;//1のある筋を全て1で埋めたBBを返す
	Bitboard getNoFuLines()const;//1のある筋を0でマスクしたBBを返す
	static Bitboard fillOne(const unsigned numofone);//左からnumofone個の1で埋めたBBを返す

	//演算子
	bool operator==(const Bitboard&)const;
	bool operator!=(const Bitboard&)const;
	Bitboard operator&(const Bitboard&)const;
	Bitboard operator|(const Bitboard&)const;
	Bitboard& operator&=(const Bitboard&);
	Bitboard& operator|=(const Bitboard&);
	Bitboard operator~()const;
	size_t size() const { return bbNum; }

	const std::uint32_t& val() const { return _p; }

private:
	uint32_t _p;
	friend class BBkiki;
};

namespace bbmask {
	static const Bitboard Dan1to4{ 0b0111101111011110111101111u };
	static const Bitboard Dan1to3{ 0b0011100111001110011100111u };
	static const Bitboard Dan2to5{ 0b1111011110111101111011110u };
	static const Bitboard Dan3to5{ 0b1110011100111001110011100u };
	static const Bitboard Dan1   { 0b0000100001000010000100001u };
	static const Bitboard Dan2   { 0b0001000010000100001000010u };
	static const Bitboard Dan4   { 0b0100001000010000100001000u };
	static const Bitboard Dan5   { 0b1000010000100001000010000u };
	static const Bitboard AllOne{ 0x1FFFFFFu };
	static const Bitboard AllZero{0};
}


inline bool Bitboard::operator==(const Bitboard& rhs) const {
	return _p == rhs._p;
}

inline bool Bitboard::operator!=(const Bitboard& rhs)const {
	return _p != rhs._p;
}

inline Bitboard Bitboard::operator&(const Bitboard& rhs) const {
	return Bitboard(_p & rhs._p);
}

inline Bitboard Bitboard::operator|(const Bitboard& rhs) const {
	return Bitboard(_p | rhs._p);
}

inline Bitboard& Bitboard::operator&=(const Bitboard& rhs) {
	_p &= rhs._p;
	return *this;
}

inline Bitboard& Bitboard::operator|=(const Bitboard& rhs) {
	_p |= rhs._p;
	return *this;
}

inline Bitboard Bitboard::operator~()const {
	//そのまま反転させると不使用ビットが1になってしまうが、コンストラクタ内でマスクして0になるので大丈夫
	return Bitboard(~_p);
}