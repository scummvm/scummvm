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
 * $URL: https://asylumengine.googlecode.com/svn/trunk/menu.h $
 * $Id: menu.h 236 2009-07-08 19:17:47Z sev.mail $
 *
 */

#ifndef ASYLUM_BLOWUPPUZZLE_H_
#define ASYLUM_BLOWUPPUZZLE_H_

#include "common/events.h"

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

	virtual void handleEvent(Common::Event *event, bool doUpdate);
	virtual void openBlowUp();
	virtual void closeBlowUp();

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

	virtual void update();

}; // end of class BlowUpPuzzle


class BlowUpPuzzleVCR : public BlowUpPuzzle {
public:
    BlowUpPuzzleVCR(Screen *screen, Sound *sound, Scene *scene);
	~BlowUpPuzzleVCR();

	void handleEvent(Common::Event *event, bool doUpdate);
	void openBlowUp();
	void closeBlowUp();
private:
    void update();
}; // end of class BlowUpPuzzleVCR


} // end of namespace Asylum

#endif
