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

#include "asylum/blowuppuzzle.h"
#include "asylum/respack.h"
#include "asylum/graphics.h"

namespace Asylum {

BlowUpPuzzle::BlowUpPuzzle(Screen *screen, Sound *sound, Scene *scene) :
	_screen(screen), _sound(sound), _scene(scene) {
}

BlowUpPuzzle::~BlowUpPuzzle() {
}

void BlowUpPuzzle::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	_screen->setCursor(_cursorResource, _curMouseCursor);
}

// BlowUp Puzzle VCR

BlowUpPuzzleVCR::BlowUpPuzzleVCR(Screen *screen, Sound *sound, Scene *scene) : BlowUpPuzzle(screen, sound, scene) {
	_mouseX	            = 0;
	_mouseY			    = 0;
	_leftClick		    = false;
	_curMouseCursor	    = 0;
	_cursorStep		    = 1;
	_active			    = false;
    _cursorResource     = _scene->getGraphicResource(_scene->getResources()->getWorldStats()->grResId[28]);
    _bgResource		    = _scene->getGraphicResource(_scene->getResources()->getWorldStats()->grResId[0]);
    _tvScreenAnimIdx    = 0;
    _isAccomplished     = 0;

    // reset all states
    memset(&_jacksState,0,sizeof(_jacksState));
    memset(&_buttonsState,0,sizeof(_buttonsState));
}

BlowUpPuzzleVCR::~BlowUpPuzzleVCR() {
	delete _cursorResource;
    delete _bgResource;
}

void BlowUpPuzzleVCR::openBlowUp() {
	_active = true;
	_scene->deactivate();
    // FIXME: decomment this line when stopSfx works properly (it nows stop together SFX and Music
    //_sound->stopSfx();

	// Load the graphics palette
	_screen->setPalette(_scene->getResourcePack(), 183);
	
    // show blow up puzzle BG
	GraphicFrame *bg = _bgResource->getFrame(0);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	_leftClick = false;
	_mouseX = _mouseY = 0;
}

void BlowUpPuzzleVCR::closeBlowUp() {
	_active = false;
	_scene->activate();
}

void BlowUpPuzzleVCR::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
		break;
	case Common::EVENT_LBUTTONUP:
		_leftClick = true;
		break;
	default:
		break;
	}

	if (doUpdate || _leftClick)
		update();
}

void BlowUpPuzzleVCR::update() {
    //updateCursor();

    if (_leftClick) {
		_leftClick = false;
    }

    // VCR Jacks State
    // 0 -> on table
    // 1 -> pluged in red hole
    // 2 -> pluged in yellow hole
    // 3 -> pluged in black hole
    // 4 -> show shadows
    updateBlackJack();
    updateRedJack();
    updateYellowJack();
}

void BlowUpPuzzleVCR::updateBlackJack() {
    int grResId = -1;
    int frameNum = 0;
    int x = 0;
    int y = 0;

    switch(_jacksState[kBlack]){
        case kOnTable:
            grResId = _scene->getResources()->getWorldStats()->grResId[1];
            frameNum = 0;
            x = 0;
            y = 411;
            break;
        case kPluggedOnRed:
            break;
        case kPluggedOnYellow:
            break;
        case kPluggedOnBlack:
            break;
        case kShowShadows:
            break;
        default:
            break;
    }

    if(grResId != -1)
    {
        GraphicResource *jack = _scene->getGraphicResource(grResId);
        GraphicFrame *fra = jack->getFrame(0);
        _screen->copyRectToScreenWithTransparency((byte *)fra->surface.pixels, fra->surface.w, x, y, fra->surface.w, fra->surface.h);
    }
}

void BlowUpPuzzleVCR::updateRedJack() {
    int grResId = -1;
    int frameNum = 0;
    int x = 0;
    int y = 0;

    switch(_jacksState[kRed]){
        case kOnTable:
            grResId = _scene->getResources()->getWorldStats()->grResId[2];
            frameNum = 0;
            x = 76;
            y = 428;
            break;
        case kPluggedOnRed:
            break;
        case kPluggedOnYellow:
            break;
        case kPluggedOnBlack:
            break;
        case kShowShadows:
            break;
        default:
            break;
    }

    if(grResId != -1)
    {
        GraphicResource *jack = _scene->getGraphicResource(grResId);
        GraphicFrame *fra = jack->getFrame(0);
        _screen->copyRectToScreenWithTransparency((byte *)fra->surface.pixels, fra->surface.w, x, y, fra->surface.w, fra->surface.h);
    }
}

void BlowUpPuzzleVCR::updateYellowJack() {
    int grResId = -1;
    int frameNum = 0;
    int x = 0;
    int y = 0;

    switch(_jacksState[kYellow]){
        case kOnTable:
            grResId = _scene->getResources()->getWorldStats()->grResId[3];
            frameNum = 0;
            x = 187;
            y = 439;
            break;
        case kPluggedOnRed:
            break;
        case kPluggedOnYellow:
            break;
        case kPluggedOnBlack:
            break;
        case kShowShadows:
            break;
        default:
            break;
    }

    if(grResId != -1)
    {
        GraphicResource *jack = _scene->getGraphicResource(grResId);
        GraphicFrame *fra = jack->getFrame(0);
        _screen->copyRectToScreenWithTransparency((byte *)fra->surface.pixels, fra->surface.w, x, y, fra->surface.w, fra->surface.h);
    }
}

} // end of namespace Asylum
