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

#ifndef VCRUISE_DETECTION_H
#define VCRUISE_DETECTION_H

#include "engines/advancedDetector.h"

namespace VCruise {

enum VCruiseGameID {
	GID_UNKNOWN	= 0,

	GID_REAH	= 1,
	GID_SCHIZM	= 2,
	GID_AD2044	= 3,
};

enum VCruiseGameFlag {
	VCRUISE_GF_WANT_MP3			= (1 << 0),
	VCRUISE_GF_WANT_OGG_VORBIS	= (1 << 1),
	VCRUISE_GF_NEED_JPEG		= (1 << 2),
	VCRUISE_GF_GENTEE_PACKAGE	= (1 << 3),
	
	VCRUISE_GF_STEAM_LANGUAGES	= (1 << 4),
	VCRUISE_GF_FORCE_LANGUAGE	= (1 << 5),
	
	VCRUISE_GF_WANT_MIDI		= (1 << 6),
};

struct VCruiseGameDescription {
	AD_GAME_DESCRIPTION_HELPERS(desc);

	ADGameDescription desc;

	VCruiseGameID gameID;

	// Specifying the language in the ADGameDescription causes AD to fail to detect the game as a match,
	// so we have to specify it as UNK_LANG and specify the default language here.
	Common::Language defaultLanguage;
};


#define GAMEOPTION_LAUNCH_DEBUG					GUIO_GAMEOPTIONS1
#define GAMEOPTION_FAST_ANIMATIONS				GUIO_GAMEOPTIONS2
#define GAMEOPTION_SKIP_MENU					GUIO_GAMEOPTIONS3
#define GAMEOPTION_INCREASE_DRAG_DISTANCE		GUIO_GAMEOPTIONS4
#define GAMEOPTION_USE_4BIT_GRAPHICS			GUIO_GAMEOPTIONS5
#define GAMEOPTION_PRELOAD_SOUNDS				GUIO_GAMEOPTIONS6
#define GAMEOPTION_FAST_VIDEO_DECODER			GUIO_GAMEOPTIONS7


} // End of namespace VCruise

#endif // VCRUISE_DETECTION_H
