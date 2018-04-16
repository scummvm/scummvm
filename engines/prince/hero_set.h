/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

namespace Prince {

const int heroSetBack[7] = { 0, 0, 10, 0, 6, 0, 0 };

typedef const char *HeroSetAnimNames[26];

static HeroSetAnimNames heroSet5 = {
	"SL_DIAB.ANI",
	"SR_DIAB.ANI",
	"SU_DIAB.ANI",
	"SD_DIAB.ANI",
	nullptr,
	nullptr,
	"MU_DIAB.ANI",
	"MD_DIAB.ANI",
	"TL_DIAB.ANI",
	"TR_DIAB.ANI",
	"TU_DIAB.ANI",
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr,
	nullptr
};

static HeroSetAnimNames heroSet1 = {
    "SL_HERO1.ANI",
    "SR_HERO1.ANI",
    "SU_HERO1.ANI",
    "SD_HERO1.ANI",
    "ML_HERO1.ANI",
    "MR_HERO1.ANI",
    "MU_HERO1.ANI",
    "MD_HERO1.ANI",
    "TL_HERO1.ANI",
    "TR_HERO1.ANI",
    "TU_HERO1.ANI",
    "TD_HERO1.ANI",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "KSI_KURZ.ANI",
    "KS_WLOSY.ANI"
};

static HeroSetAnimNames heroSet2 = {
    "SL_HERO2.ANI",
    "SR_HERO2.ANI",
    "SU_HERO2.ANI",
    "SD_HERO2.ANI",
    "ML_HERO2.ANI",
    "MR_HERO2.ANI",
    "MU_HERO2.ANI",
    "MD_HERO2.ANI",
    "TL_HERO2.ANI",
    "TR_HERO2.ANI",
    "TU_HERO2.ANI",
    "TD_HERO2.ANI",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "KSI_KU_S.ANI",
    "KS_WLO_S.ANI"
};

static HeroSetAnimNames heroSet3 = {
    "SL_BEAR.ANI",
    "SR_BEAR.ANI",
    "SU_BEAR.ANI",
    "SD_BEAR.ANI",
    "NIED-LEW.ANI",
    "NIED-PRW.ANI",
    "NIED-TYL.ANI",
    "NIED-PRZ.ANI",
    "SL_BEAR.ANI",
    "SR_BEAR.ANI",
    "SU_BEAR.ANI",
    "SD_BEAR.ANI",
    "N_LW-TYL.ANI",
    "N_LW-PRZ.ANI",
    "N_LW-PR.ANI",
    "N_PR-TYL.ANI",
    "N_PR-PRZ.ANI",
    "N_PR-LW.ANI",
    "N_TYL-LW.ANI",
    "N_TYL-PR.ANI",
    "N_TL-PRZ.ANI",
    "N_PRZ-LW.ANI",
    "N_PRZ-PR.ANI",
    "N_PRZ-TL.ANI",
    nullptr,
    nullptr,
};

static HeroSetAnimNames shanSet1 = {
    "SL_SHAN.ANI",
    "SR_SHAN.ANI",
    "SU_SHAN.ANI",
    "SD_SHAN.ANI",
    "ML_SHAN.ANI",
    "MR_SHAN.ANI",
    "MU_SHAN.ANI",
    "MD_SHAN.ANI",
    "TL_SHAN.ANI",
    "TR_SHAN.ANI",
    "TU_SHAN.ANI",
    "TD_SHAN.ANI",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "B1_SHAN.ANI",
    "B2_SHAN.ANI",
};

static HeroSetAnimNames shanSet2 = {
    "SL_SHAN2.ANI",
    "SR_SHAN2.ANI",
    "SU_SHAN.ANI",
    "SD_SHAN2.ANI",
    "ML_SHAN2.ANI",
    "MR_SHAN2.ANI",
    "MU_SHAN.ANI",
    "MD_SHAN2.ANI",
    "TL_SHAN2.ANI",
    "TR_SHAN2.ANI",
    "TU_SHAN.ANI",
    "TD_SHAN2.ANI",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    "B1_SHAN2.ANI",
    "B2_SHAN2.ANI"
};

static HeroSetAnimNames arivSet1 = {
    "SL_ARIV.ANI",
    "SR_ARIV.ANI",
    "SU_ARIV.ANI",
    "SD_ARIV.ANI",
    "ML_ARIV.ANI",
    "MR_ARIV.ANI",
    "MU_ARIV.ANI",
    "MD_ARIV.ANI",
    "TL_ARIV.ANI",
    "TR_ARIV.ANI",
    "TU_ARIV.ANI",
    "TD_ARIV.ANI",
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

const HeroSetAnimNames *heroSetTable[7] = {
	&heroSet1,
	&heroSet2,
	&heroSet3,
	&shanSet1,
	&arivSet1,
	&heroSet5,
	&shanSet2,
};

} // End of namespace Prince
