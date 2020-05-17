#include "stdafx.h"
#include "koma.h"

namespace koma {
	bool isSenteKoma(Koma koma) {
		return static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_Min) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_Max);
	}
	bool isGoteKoma(Koma koma) {
		return static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_Min) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_Max);
	}

	bool isPromotable(Koma koma) {
		return (static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_Hi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_Fu) ||
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_Hi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_Fu));
	}
	bool isPromoted(Koma koma) {
		return (static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::s_nHi) &&
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::s_nFu) ||
			static_cast<std::uint8_t>(koma) >= static_cast<std::uint8_t>(Koma::g_nFu)) &&
			static_cast<std::uint8_t>(koma) <= static_cast<std::uint8_t>(Koma::g_nHi);
	}
	Koma promote(Koma koma) {
		assert(isPromotable(koma));
		return static_cast<Koma>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Koma::Nari));
	}
	Koma dispromote(Koma koma) {
		if (isPromoted(koma)) {
			return static_cast<Koma>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::Nari));
		}
		else return koma;
	}

	Koma MposToKoma(const Position pos) {
		assert(pos >= Position::SQm_Min && pos <= Position::SQm_Max);
		return (pos < Position::m_gFu) ?
			static_cast<Koma>(static_cast<std::uint8_t>(pos) - static_cast<std::uint8_t>(m_sFu) + static_cast<std::uint8_t>(Koma::s_Fu)) :
			static_cast<Koma>(static_cast<std::uint8_t>(pos) - static_cast<std::uint8_t>(m_gFu) + static_cast<std::uint8_t>(Koma::g_Fu));
	}

	Mochigoma MposToMochi(const Position pos) {
		assert(pos >= Position::SQm_Min && pos <= Position::SQm_Max);
		return (pos < Position::m_gFu) ?
			static_cast<Mochigoma>(pos - m_sFu) :
			static_cast<Mochigoma>(pos - m_gFu);
	}

	Position KomaToMpos(Koma koma) {
		if (isPromoted(koma)) { koma = dispromote(koma); }
		return (static_cast<int>(koma) < static_cast<int>(Koma::g_Min)) ?
			static_cast<Position>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::s_Fu) + static_cast<std::uint8_t>(Position::m_gFu)) :
			static_cast<Position>(static_cast<std::uint8_t>(koma) - static_cast<std::uint8_t>(Koma::g_Fu) + static_cast<std::uint8_t>(Position::m_sFu));
	}
	Position MochiToMpos(Mochigoma koma, bool sente) {
		if (sente) {
			return static_cast<Position>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Position::m_sFu));
		}
		else {
			return static_cast<Position>(static_cast<std::uint8_t>(koma) + static_cast<std::uint8_t>(Position::m_gFu));
		}
	}
}