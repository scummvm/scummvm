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

#ifndef MTROPOLIS_DETECTION_H
#define MTROPOLIS_DETECTION_H

#include "engines/advancedDetector.h"

namespace MTropolis {

enum MTropolisGameID {
	GID_OBSIDIAN			= 0,
	GID_LEARNING_MTROPOLIS	= 1,
	GID_MTI    				= 2,
	GID_ALBERT1				= 3,
	GID_ALBERT2				= 4,
	GID_ALBERT3				= 5,
	GID_SPQR				= 6,
	GID_STTGS				= 7,
};

// Boot IDs - These can be shared across different variants if the file list and other properties are identical.
// Cross-reference with the game table in mTropolis engine's boot.cpp
enum MTropolisGameBootID {
	MTBOOT_INVALID = 0,

	MTBOOT_OBSIDIAN_RETAIL_MAC_EN,
	MTBOOT_OBSIDIAN_RETAIL_MAC_JP,
	MTBOOT_OBSIDIAN_RETAIL_WIN_EN,
	MTBOOT_OBSIDIAN_RETAIL_WIN_DE_INSTALLED,
	MTBOOT_OBSIDIAN_RETAIL_WIN_DE_DISC,
	MTBOOT_OBSIDIAN_RETAIL_WIN_IT,
	MTBOOT_OBSIDIAN_DEMO_MAC_EN,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_1,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_2,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_3,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_4,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_5,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_6,
	MTBOOT_OBSIDIAN_DEMO_WIN_EN_7,

	MTBOOT_MTI_RETAIL_MAC,
	MTBOOT_MTI_RETAIL_WIN,
	MTBOOT_MTI_DEMO_WIN,

	MTBOOT_ALBERT1_WIN_DE,
	MTBOOT_ALBERT2_WIN_DE,
	MTBOOT_ALBERT3_WIN_DE,
	MTBOOT_SPQR_RETAIL_WIN,
	MTBOOT_SPQR_RETAIL_MAC,

	MTBOOT_STTGS_DEMO_WIN,
};

enum MTGameFlag {
	MTGF_WANT_MPEG_VIDEO = (1 << 0),
	MTGF_WANT_MPEG_AUDIO = (1 << 1),
};

struct MTropolisGameDescription {
	ADGameDescription desc;

	int gameID;
	int gameType;
	MTropolisGameBootID bootID;
};

#define GAMEOPTION_WIDESCREEN_MOD				GUIO_GAMEOPTIONS1
#define GAMEOPTION_DYNAMIC_MIDI					GUIO_GAMEOPTIONS2
#define GAMEOPTION_LAUNCH_DEBUG					GUIO_GAMEOPTIONS3
#define GAMEOPTION_SOUND_EFFECT_SUBTITLES		GUIO_GAMEOPTIONS4
#define GAMEOPTION_AUTO_SAVE_AT_CHECKPOINTS		GUIO_GAMEOPTIONS5
#define GAMEOPTION_ENABLE_SHORT_TRANSITIONS		GUIO_GAMEOPTIONS6

} // End of namespace MTropolis

#endif // MTROPOLIS_DETECTION_H
