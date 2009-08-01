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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef ASYLUM_BLOWUPPUZZLE_H_
#define ASYLUM_BLOWUPPUZZLE_H_

#include "common/events.h"
#include "common/rect.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/graphics.h"

namespace Asylum {

class Scene;
class Screen;
class Sound;

class BlowUpPuzzle {
public:
	BlowUpPuzzle(Screen *screen, Sound *sound, Scene *scene);
	virtual ~BlowUpPuzzle();

    virtual void handleEvent(Common::Event *event, bool doUpdate){};
    virtual void openBlowUp(){};
    virtual void closeBlowUp(){};

    bool isActive() { return _active; }
protected:
	Common::Event *_ev;

	Screen *_screen;
	Sound  *_sound;
    Scene  *_scene;

	uint32	_mouseX;
	uint32	_mouseY;
	uint32	_curMouseCursor;
	int32	_cursorStep;
	bool    _leftClick;
	bool    _active;

    GraphicResource *_bgResource;
	GraphicResource *_cursorResource;

	void updateCursor();

    virtual void update() {};

}; // end of class BlowUpPuzzle






const Common::Rect BlowUpPuzzleVCRPolies[10] = {
    Common::Rect(0x0F7, 0x157, 0x13A, 0x183),
    Common::Rect(0x14B, 0x15C, 0x17B, 0x18B),
    Common::Rect(0x18C, 0x161, 0x1D2, 0x18F),
    Common::Rect(0x1FB, 0x156, 0x233, 0x185),
    Common::Rect(0x154, 0x196, 0x173, 0x1AF),
    Common::Rect(0x19A, 0x19B, 0x1B9, 0x1B3),
    Common::Rect(0x1E3, 0x1A0, 0x202, 0x1BC),
    Common::Rect(0x0,   0x19B, 0x3C,  0x1E0),
    Common::Rect(0x4C,  0x1AC, 0x0A0, 0x1E0),
    Common::Rect(0x0BB, 0x1B7, 0x0F0, 0x1E0)
};

class BlowUpPuzzleVCR : public BlowUpPuzzle {
public:
    BlowUpPuzzleVCR(Screen *screen, Sound *sound, Scene *scene);
	~BlowUpPuzzleVCR();

	void handleEvent(Common::Event *event, bool doUpdate);
	void openBlowUp();
	void closeBlowUp();
private:
    enum Jack {
		kBlack  = 0,
		kRed    = 1,
		kYellow = 2
	};

    enum JackState {
		kOnTable            = 0,
		kPluggedOnRed       = 1,
		kPluggedOnYellow    = 2,
		kPluggedOnBlack     = 3,
        kShowShadows        = 4
	};

    int _jacksState[3];
    int _buttonsState[4];
    int _tvScreenAnimIdx;
    int _isAccomplished;
    // TODO: members for playing sound

    /*GraphicResource *_blackJack;
    GraphicResource *_redJack;
    GraphicResource *_yellowJack;*/

    int inPolyRegion(int x, int y, int polyIdx);

    void update();

    void updateBlackJack();
    void updateRedJack();
    void updateYellowJack();

    void isCursorInPolyRegion();
}; // end of class BlowUpPuzzleVCR


} // end of namespace Asylum

#endif
