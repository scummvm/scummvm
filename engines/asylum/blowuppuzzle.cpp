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
	_leftClickUp	    = false;
    _leftClickDown	    = false;
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

	_leftClickUp = false;
    _leftClickDown = false;
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
		_leftClickUp = true;
		break;
    case Common::EVENT_LBUTTONDOWN:
		_leftClickDown = true;
		break;
	default:
		break;
	}

	if (doUpdate || _leftClickUp || _leftClickDown)
		update();
}

int BlowUpPuzzleVCR::inPolyRegion(int x, int y, int polyIdx) {
    return  x >= BlowUpPuzzleVCRPolies[polyIdx].left && x <= BlowUpPuzzleVCRPolies[polyIdx].right && 
            y >= BlowUpPuzzleVCRPolies[polyIdx].top  && y <= BlowUpPuzzleVCRPolies[polyIdx].bottom;
}

void BlowUpPuzzleVCR::update() {
    // TODO: set states
    if (_leftClickDown) {
		_leftClickDown = false;
        handleMouseDown();
    }

    // TODO: put sound working
    if (_leftClickUp) {
		_leftClickUp = false;
        handleMouseUp();
    }

    updateCursorInPolyRegion();

    updateYellowJack();
    updateRedJack();
    updateBlackJack();   
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
        case kOnHand: {
            // Jack info
            int jackY = _mouseY;
            if(_mouseY < 356) {
                jackY = 356;
            }

            GraphicResource *j = _scene->getGraphicResource(_scene->getResources()->getWorldStats()->grResId[27]);
            GraphicFrame *f = j->getFrame(0);
            
            // FIXME: this must be a special copy to screen function to cut parts of the screen (shouldn't be calculated here
            int newHeight = (480-_mouseY) + 14;
            if (newHeight > f->surface.h) {
                newHeight = f->surface.h;
            }

            _screen->copyRectToScreenWithTransparency((byte *)f->surface.pixels, f->surface.w, _mouseX - 114, jackY - 14, f->surface.w, newHeight);
            
            // Shadow Info
            int shadowY = (_mouseY - 356) / 4;
            if(_mouseY < 356) {
                shadowY = 356;
            }
            grResId = _scene->getResources()->getWorldStats()->grResId[30];
            frameNum = 0;
            x = _mouseX - shadowY;
            y = 450;
            }
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
        case kOnHand:
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
        case kOnHand:
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


void BlowUpPuzzleVCR::updateCursorInPolyRegion() {
    int showCursor = 0;

    if ( _jacksState[kBlack] == kOnHand ) {
        showCursor = 1;
    } else if ( _jacksState[kRed] == 4 ) {
        showCursor = 2;
    } else {
        showCursor = ((_jacksState[kYellow] != 4) - 1) & 3;
    }

    if(showCursor) {
        if(inPolyRegion(_mouseX, _mouseY, kRewindButton)
           || inPolyRegion(_mouseX, _mouseY, kStopButton)
           || inPolyRegion(_mouseX, _mouseY, kPlayButton)
           || inPolyRegion(_mouseX, _mouseY, kRecButton)
           || inPolyRegion(_mouseX, _mouseY, kBlackJack)
           || inPolyRegion(_mouseX, _mouseY, kRedJack)
           || inPolyRegion(_mouseX, _mouseY, kYellowJack)) {
            //updateCursor();
        } else {
            if(_curMouseCursor != 0) { // reset cursor
                _curMouseCursor = 0;
                _cursorStep = 1;
                updateCursor();
            }
        }
    }
}

void BlowUpPuzzleVCR::handleMouseDown() {
    // TODO: Jack hole regions
    if(inPolyRegion(_mouseX, _mouseY, kRedHole)) { // red jack hole

    }

    if(inPolyRegion(_mouseX, _mouseY, kYellowHole)) { // yellow jack hole

    }

    if(inPolyRegion(_mouseX, _mouseY, kBlackHole)) { // black jack hole

    }

    // Get Jacks from Table
    if (inPolyRegion(_mouseX, _mouseY, kBlackJack)) {
        _jacksState[kBlack] = kOnHand;
    } else if (inPolyRegion(_mouseX, _mouseY, kRedJack)) {
        _jacksState[kRed] = kOnHand;
    } else if (inPolyRegion(_mouseX, _mouseY, kYellowJack)) {
        _jacksState[kYellow] = kOnHand;
    }

    // TODO: VCR button regions
}

void BlowUpPuzzleVCR::handleMouseUp() {

}

} // end of namespace Asylum
