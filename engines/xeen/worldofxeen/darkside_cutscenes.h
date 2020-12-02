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

#ifndef XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H
#define XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H

#include "xeen/cutscenes.h"

namespace Xeen {

class XeenEngine;

namespace WorldOfXeen {

class DarkSideCutscenes : public Cutscenes {
private:
	Subtitles _subtitles;
	SpriteResource _ball, _claw, _dragon1;
private:
	/**
	 * Initial animation of the Dragon Pharoah moving his hands to the
	 * crystal ball, and then moving them back again
	 * @param fadeIn		If set, fades in screen after first frame
	 */
	bool rubCrystalBall(bool fadeIn = false);

	/**
	 * Animates the dragon pharoah
	 * @param frame		Animation frame number
	 * @param showBall	Flag whether to draw the crystal ball
	 */
	void animatePharoah(int frame, bool showBall = true);

	/**
	 * Shows the Darkside of Xeen title screen
	 */
	bool showDarkSideTitle(bool seenIntro);

	/**
	 * Shows part 1 of the Dark Side intro - up to the point where
	 * Dragon Pharoah ends "contact the Queen"
	 */
	bool showDarkSideIntro1();

	/**
	 * Shows part 2 of the Dark Side intro
	 */
	bool showDarkSideIntro2();

	/**
	 * Shows part 3 of the Dark Side intro
	 */
	bool showDarkSideIntro3();

	/**
	 * Shows the World of Xeen logo from the end of the Dark Side intro
	 */
	bool showWorldOfXeenLogo();

	/**
	 * Shows part 1 of the Dark Side ending, everything up to Corak appearing
	 */
	bool showDarkSideEnding1();

	/**
	 * Shows part 2 of the Dark Side ending, exposition until start of fighting
	 */
	bool showDarkSideEnding2();

	/**
	 * Shows part 3 of the Dark Side ending, the fighting
	 */
	bool showDarkSideEnding3();

	/**
	 * Shows part 4 of the Dark Side ending, the self-destruct and castle explosion
	 */
	bool showDarkSideEnding4();

	/**
	 * Last part of the Dark Side ending, shows the final score and saves the game/
	 */
	 void showDarkSideScore(uint endingScore);

	/**
	 * Shows the Pharaoh ending screen where score text is shown
	 */
	 bool showPharaohEndTextInner(const char *msg1, const char *msg2, const char *msg3);
protected:
	/**
	 * Shows the Pharaoh ending screen where score text is shown
	 */
	bool showPharaohEndText(const char *msg1, const char *msg2 = nullptr, const char *msg3 = nullptr);
public:
	DarkSideCutscenes(XeenEngine *vm) : Cutscenes(vm) {}

	/**
	 * Shows the Darkside of Xeen intro sequence
	 */
	bool showDarkSideIntro(bool seenIntro);

	/**
	 * Shows the Darkside of Xeen ending sequence
	 */
	void showDarkSideEnding(uint endingScore);
};

} // End of namespace WorldOfXeen
} // End of namespace Xeen

#endif /* XEEN_WORLDOFXEEN_DARKSIDE_CUTSCENES_H */
