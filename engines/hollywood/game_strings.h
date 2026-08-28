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

#ifndef HOLLYWOOD_GAME_STRINGS_H
#define HOLLYWOOD_GAME_STRINGS_H

#include "common/language.h"

namespace Hollywood {

enum {
	kGameActionCaptionCount = 9
};

/**
 * Text stored in the localized game executables rather than RESOURCE.003.
 *
 * These strings follow the detected game language. ScummVM's translation
 * manager is reserved for launcher and engine UI that is independent of the
 * selected game release.
 */
struct HollywoodGameStrings {
	const char *actionCaptions[kGameActionCaptionCount];
	const char *panelVerbLabels[kGameActionCaptionCount];
	const char *relationTo;
	const char *relationWith;
	const char *inventoryCaption;

	const char *optionsSave;
	const char *optionsLoad;
	const char *optionsQuit;
	const char *optionsPlay;
	const char *optionsMusicOn;
	const char *optionsMusicOff;
	const char *optionsSfxOn;
	const char *optionsSfxOff;
	const char *optionsTestOn;
	const char *optionsTestOff;
	const char *optionsMusicVolume;
	const char *optionsSfxVolume;
	const char *optionsVoiceVolume;
	const char *optionsTextSpeed;
	const char *optionsText;
	const char *optionsVoice;
	const char *optionsBoth;
	const char *optionsQuitPrompt;
	const char *optionsQuitYes;
	const char *optionsQuitNo;

	const char *passageName;
	const char *wideCoffinName;
	const char *changeDrink;
	const char *openTap;
	const char *stopMakingCocktails;
};

const HollywoodGameStrings &getGameStrings(Common::Language language);

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAME_STRINGS_H
