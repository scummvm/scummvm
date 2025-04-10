/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "bagel/hodjnpodj/metagame/boardgame/gtl_data.h"
#include "bagel/hodjnpodj/metagame/bgen/bgen.h"

namespace Bagel {
namespace HodjNPodj {
namespace Metagame {

struct CLUE_LOC {
	int iLocID;
	int iNoteID;
	int iPersonID;
	int iPlaceID;
};

static const char *szGameSounds[MG_SOUND_COUNT] = {
	"meta/sound/PK1.WAV",
	"meta/sound/PK2.WAV",
	"meta/sound/PG1.WAV",
	"meta/sound/PG2.WAV",
	"meta/sound/PG3.WAV",
	"meta/sound/PG4.WAV",
	"meta/sound/PG5.WAV",
	"meta/sound/PG6.WAV",
	"meta/sound/FG1.WAV",
	"meta/sound/FG2.WAV",
	"meta/sound/FG3.WAV",
	"meta/sound/FG4.WAV",
	"meta/sound/FG5.WAV",
	"meta/sound/TG1.WAV",
	"meta/sound/TG2.WAV",
	"meta/sound/TG3.WAV",
	"meta/sound/TG4.WAV",
	"meta/sound/TG5.WAV",
	"meta/sound/TG6.WAV",
	"meta/sound/DF1.WAV",
	"meta/sound/DF2.WAV",
	"meta/sound/DF3.WAV",
	"meta/sound/DF4.WAV",
	"meta/sound/DF5.WAV",
	"meta/sound/GF1B.WAV",
	"meta/sound/GF2B.WAV",
	"meta/sound/GF3B.WAV",
	"meta/sound/CR1.WAV",
	"meta/sound/CR2.WAV",
	"meta/sound/CR3.WAV",
	"meta/sound/CR4.WAV",
	"meta/sound/CR5.WAV",
	"meta/sound/BC1.WAV",
	"meta/sound/BC2.WAV",
	"meta/sound/BC3.WAV",
	"meta/sound/BC4.WAV",
	"meta/sound/BC5.WAV",
	"meta/sound/RR1.WAV",
	"meta/sound/RR2.WAV",
	"meta/sound/RR3.WAV",
	"meta/sound/RR4.WAV",
	"meta/sound/BF1.WAV",
	"meta/sound/BF2.WAV",
	"meta/sound/BF3.WAV",
	"meta/sound/PR1B.WAV",
	"meta/sound/PR2B.WAV",
	"meta/sound/PR3B.WAV",
	"meta/sound/PR4B.WAV",
	"meta/sound/MD1.WAV",
	"meta/sound/MD2.WAV",
	"meta/sound/MD3.WAV",
	"meta/sound/MD4.WAV",
	"meta/sound/WS1.WAV",
	"meta/sound/WS2.WAV",
	"meta/sound/WS3.WAV",
	"meta/sound/WS4.WAV",
	"meta/sound/BQ1.WAV",
	"meta/sound/BQ2.WAV",
	"meta/sound/MK1.WAV",
	"meta/sound/MK6.WAV",
	"meta/sound/AR1.WAV",
	"meta/sound/AR2.WAV",
	"meta/sound/NV1.WAV",
	"meta/sound/NV2.WAV",
	"meta/sound/NV3.WAV",
	"meta/sound/AP1.WAV",
	"meta/sound/AP2.WAV",
	"meta/sound/LF1.WAV",
	"meta/sound/LF2.WAV",
	"meta/sound/EU1.WAV",
	"meta/sound/EU2.WAV",
	"meta/sound/OLN1.WAV",
	"meta/sound/OLN2.WAV",
	"meta/sound/OLN3.WAV",
	"meta/sound/OLN4.WAV",
	"meta/sound/OLN5.WAV",
	"meta/sound/OLN6.WAV",
	"meta/sound/OLN7.WAV",
	"meta/sound/OLN8.WAV",
	"meta/sound/OLN9.WAV",
	"meta/sound/GSPS1.WAV",
	"meta/sound/GSPS2.WAV",
	"meta/sound/GSPS3.WAV",
	"meta/sound/GSPS4.WAV",
	"meta/sound/GSPS5.WAV",
	"meta/sound/GSPS6.WAV",
	"meta/sound/GSPS7.WAV",
	"meta/sound/GSPS8.WAV",
	"meta/sound/UTP1.WAV",
	"meta/sound/UTP2.WAV",
	"meta/sound/UTP3.WAV",
	"meta/sound/UTP4.WAV",
	"meta/sound/UTP5.WAV",
	"meta/sound/UTP6.WAV",
	"meta/sound/UTP7.WAV",
	"meta/sound/UTP8.WAV",
	"meta/sound/TRN1.WAV",
	"meta/sound/TRN2.WAV",
	"meta/sound/TRN3.WAV",
	"meta/sound/TRN4.WAV",
	"meta/sound/TRN5.WAV",
	"meta/sound/TRN6.WAV",
	"meta/sound/TRN7.WAV",
	"meta/sound/TRN8.WAV",
	"meta/sound/TRN9.WAV",
	"meta/sound/TRN10.WAV",
	"meta/sound/TRN11.WAV",
	"meta/sound/TRN12.WAV",
	"meta/sound/BB61.WAV",
	"meta/sound/BB62.WAV",
	"meta/sound/BB63.WAV",
	"meta/sound/BB64.WAV",
	"meta/sound/BB65.WAV",
	"meta/sound/BB66.WAV",
	"meta/sound/BB67.WAV",
	"meta/sound/BB68.WAV",
	"meta/sound/BB69.WAV",
	"meta/sound/BB70.WAV",
	"meta/sound/BB71.WAV",
	"meta/sound/FM1.WAV",
	"meta/sound/FM2.WAV",
	"meta/sound/PM1.WAV",
	"meta/sound/PM2.WAV",
	"meta/sound/WC1.WAV",
	"meta/sound/WC2.WAV",
	"meta/sound/AC1.WAV",
	"meta/sound/AC2.WAV",
	"meta/sound/WT1.WAV",
	"meta/sound/WT2.WAV",
	"meta/sound/WR1.WAV",
	"meta/sound/WR2.WAV",
	"meta/sound/BR1.WAV",
	"meta/sound/BR2.WAV",
	"meta/sound/MM1.WAV",
	"meta/sound/MM2.WAV",
	"meta/sound/PI1.WAV",
	"meta/sound/PI2.WAV",
	"meta/sound/PI3.WAV",
	"meta/sound/HM1.WAV",
	"meta/sound/HM2.WAV"
};

static const CLUE_LOC nClueLocation[NOTE_COUNT] = {
	{ MG_LOC_INN,        NOTE_MANKALA,            NOTE_ICON_CRAB,       NOTE_ICON_INN},
	{ MG_LOC_TOURNAMENT, NOTE_ARCHEROIDS,         NOTE_ICON_ARCHER,     NOTE_ICON_FIELD},
	{ MG_LOC_BOARDING,   NOTE_NOVACANCY,          NOTE_ICON_INNKEEPER,  NOTE_ICON_HOUSE},
	{ MG_LOC_ARTISTCOTT, NOTE_ARTPARTS,           NOTE_ICON_ARTIST,     NOTE_ICON_ARTISTCOTTAGE},
	{ MG_LOC_TEMPLE,     NOTE_LIFE,               NOTE_ICON_PRIEST,     NOTE_ICON_TEMPLE},
	{ MG_LOC_BANDITHIDE, NOTE_EVERYTHINGUNDERSUN, NOTE_ICON_BANDIT,     NOTE_ICON_HIDEOUT},
	{ MG_LOC_FARMHOUSE,  NOTE_FARMHOUSE,          NOTE_ICON_FARMER,     NOTE_ICON_FARM},
	{ MG_LOC_MANSION,    NOTE_MANSION,            NOTE_ICON_ARISTOCRAT, NOTE_ICON_MANSION},
	{ MG_LOC_WOODCUTTER, NOTE_WOODCUTTERSCOTTAGE, NOTE_ICON_WOODCUTTER, NOTE_ICON_CUTTERCOTTAGE},
	{ MG_LOC_POSTOFFICE, NOTE_POSTOFFICE,         NOTE_ICON_POSTMASTER, NOTE_ICON_POSTOFFICE},
	{ MG_LOC_AMPHI,      NOTE_GARFUNKLE,          NOTE_ICON_LEOPOLD,    NOTE_ICON_THEATRE}
};

bool bExitMetaDLL = false;

bool st_bExitDll = false;  // static DLL exit flag

bool bLacksMoney = false;   // 1 - lacks money for eligibility, 0 - doesn't lack money

int nMissTurn = 0;  // 0 - nobody misses turn; 1 - Hodj misses turn; 2 - Podj misses turn;

/*------------------------------------------------------------------------*/

CXodj::~CXodj(void) {
	if (m_xpStrategyInfo != nullptr)
		delete m_xpStrategyInfo;

	if (m_pThemeSound != nullptr) {
		m_pThemeSound->stop();
		delete m_pThemeSound;
		m_pThemeSound = nullptr;
	}
}

/*------------------------------------------------------------------------*/


} // namespace Metagame
} // namespace HodjNPodj
} // namespace Bagel
