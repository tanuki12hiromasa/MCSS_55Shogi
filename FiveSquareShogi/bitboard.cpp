#include "stdafx.h"
#include "bitboard.h"
#include <intrin.h>
#include <algorithm>
#include <cassert>


std::string Bitboard::toString() {
	std::string str;
	for (unsigned i = 0; i < size(); i++) {
		str += test(24u - i) ? '1' : '0';
	}
	return str;
}

unsigned Bitboard::pop_first() {
	unsigned long index;
	if (_BitScanForward(&index, _p)) {
		_bittestandreset(reinterpret_cast<long*>(&_p), index);
		return index;
	}
	else {
		return size();
	}
}

unsigned Bitboard::find_first() const {
	unsigned long index;
	if (_BitScanForward(&index, _p)) {
		return index;
	}
	else {
		return size();
	}
}

unsigned Bitboard::find_next(const unsigned first) const {
	//自身のfistまでを0でマスクしたBBのfind_firstを返している
	return operator&(~fillOne(first + 1u)).find_first();
}

unsigned Bitboard::find_last()const {
	unsigned long index;
	if (_BitScanReverse(&index, _p)) {
		return index;
	}
	else {
		return size();
	}
}

unsigned Bitboard::popcount()const {
	return __popcnt(_p);
}

bool Bitboard::test(const unsigned pos)const {
	return _bittest(reinterpret_cast<const long*>(&_p), pos) != 0;
}

void Bitboard::set(const unsigned pos, const bool value) {
	if (value) set(pos);
	else reset(pos);
}

void Bitboard::set(const unsigned pos) {
	assert(pos < 25u);
	_bittestandset(reinterpret_cast<long*>(&_p), pos);
}

void Bitboard::reset(const unsigned pos) {
	assert(pos < 25u);
	_bittestandreset(reinterpret_cast<long*>(&_p), pos);
}

void Bitboard::all_reset() {
	_p = 0u;
}

bool Bitboard::none()const {
	return _p == 0u;
}

Bitboard Bitboard::getLineOR() const {
	Bitboard lines;
	for (unsigned i = 0; i < 5; i++) {
		if ((_p & (0x1FFULL << (i * 5u))) != 0u) {
			lines._p |= (0x1FFULL << (i * 5u));
		}
	}
	return lines;
}

Bitboard Bitboard::getNoFuLines()const {
	Bitboard lines(bbmask::AllOne);
	for (unsigned i = 0; i < 5; i++) {
		if ((_p & (0x1FFULL << (i * 5u))) != 0u) {
			lines._p &= ~(0x1FFULL << (i * 5u));
		}
	}
	return lines;
}

Bitboard Bitboard::fillOne(unsigned index) {
	Bitboard p;
	p._p = (1ULL << std::min(index, 25u)) - 1u;
	return p;
}