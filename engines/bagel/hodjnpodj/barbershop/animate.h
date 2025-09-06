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

#ifndef HODJNPODJ_BARBERSHOP_ANIMATE_H
#define HODJNPODJ_BARBERSHOP_ANIMATE_H

#include "bagel/hodjnpodj/hnplibs/sprite.h"

namespace Bagel {
namespace HodjNPodj {
namespace Barbershop {

//
// animation sounds
//
#define CLOWN_WAV       ".\\sound\\horns.wav"
#define SPACESHIP_WAV   ".\\sound\\spaceshp.wav"
#define SPACERAY_WAV    ".\\sound\\spaceray.wav"
#define CUT_WAV         ".\\sound\\haircut.wav"
#define BRAT_WAV        ".\\sound\\doneyet.wav"
#define LOLLI_A_WAV     ".\\sound\\lollipop.wav"
#define LOLLI_B_WAV     ".\\sound\\hairpop.wav"


//
// Clown animation
//
#define CLOWN_BMP       ".\\art\\gum.bmp"

#define CLOWN_FRAMES    24          // Clown morph frames

#define CLOWN_BMP_HEI   96          // demensions of clown art
#define CLOWN_BMP_WID   110

#define CLOWN_LEFT      16          // demensions of clown rect
#define CLOWN_TOP       27
#define CLOWN_RIG       (CLOWN_LEFT + CLOWN_BMP_HEI)
#define CLOWN_BOT       (CLOWN_TOP + CLOWN_BMP_WID)

#define CLOWN_TIME_SLICE    1 // millisecs, pause between animation cells

//
// UFO beam up animation
//
#define UFOA_BMP        ".\\art\\ufo1a.bmp"
#define UFOB_BMP        ".\\art\\ufo1b.bmp"

#define UFOA_FRAMES     16              // UFOa frames
#define UFOB_FRAMES     14              // UFOa frames

#define UFO_BMP_HEI     198             // demensions of UFO art
#define UFO_BMP_WID     123

#define UFO_LEFT        134             // demensions of UFO rect
#define UFO_TOP         24
#define UFO_RIG         (UFO_LEFT + UFO_BMP_HEI)
#define UFO_BOT         (UFO_TOP + UFO_BMP_WID)

#define CAR_LEFT        136             // demensions of race car rect
#define CAR_TOP         116
#define CAR_RIG         228
#define CAR_BOT         145

#define UFOA_TIME_SLICE 1       // millisecs, pause between animation cells
#define UFOB_TIME_SLICE 100     // millisecs, pause between animation cells
#define UFO_PAUSE       1000    // wait a second after stealing the car
//
// Brat effect
//
#define BRAT_LEFT       405
#define BRAT_TOP        24
#define BRAT_RIG        463
#define BRAT_BOT        62

//
// Lollipop effect
//
#define LOLLI_LEFT      472
#define LOLLI_TOP       52
#define LOLLI_RIG       483
#define LOLLI_BOT       59

//
// Haircut effect
//
#define CUT_LEFT        563 //355
#define CUT_TOP         26  //120
#define CUT_RIG         625 //418
#define CUT_BOT         178 //154

class CAnimate {
private:    // variables
	CSprite     *m_pSprite;
	CRect       m_cClownRect;
	CRect       m_cUFORect;
	CRect       m_cCarRect;
	CRect       m_cBratRect;
	CRect       m_cLolliRect;
	CRect       m_cCutRect;
	CSound      *m_pSound;

public:     // functions
	CAnimate(CSound*);
	~CAnimate();

	bool    Clown(CDC*, CPoint);
	bool    UFO(CDC*, CPoint);
	bool    Brat(CPoint);
	bool    Lollipop(CPoint);
	bool    Haircut(CPoint);
};

} // namespace Barbershop
} // namespace HodjNPodj
} // namespace Bagel

#endif
