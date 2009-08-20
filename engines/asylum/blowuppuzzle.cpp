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
#include "asylum/shared.h"

namespace Asylum {

BlowUpPuzzle::BlowUpPuzzle() {
}

BlowUpPuzzle::~BlowUpPuzzle() {
}

void BlowUpPuzzle::updateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	Shared.getScreen()->setCursor(_cursorResource, _curMouseCursor);
}


// BlowUp Puzzle VCR ---------------------------------------------------------------------------------------------

BlowUpPuzzleVCR::BlowUpPuzzleVCR() {
	_mouseX	            = 0;
	_mouseY			    = 0;
	_leftClickUp	    = false;
    _leftClickDown	    = false;
    _rightClickDown     = false;
	_curMouseCursor	    = 0;
	_cursorStep		    = 1;
	_active			    = false;
    _cursorResource     = Shared.getScene()->getGraphicResource(Shared.getScene()->getResources()->getWorldStats()->grResId[28]);
    _bgResource		    = Shared.getScene()->getGraphicResource(Shared.getScene()->getResources()->getWorldStats()->grResId[0]);
    _tvScreenAnimIdx    = 0;
    _isAccomplished     = false;

    // reset all states
    memset(&_jacksState,0,sizeof(_jacksState));
    memset(&_holesState,0,sizeof(_holesState));
    memset(&_buttonsState,0,sizeof(_buttonsState));
}

BlowUpPuzzleVCR::~BlowUpPuzzleVCR() {
	delete _cursorResource;
    delete _bgResource;
}

void BlowUpPuzzleVCR::openBlowUp() {
	_active = true;
	Shared.getScene()->deactivate();
    // FIXME: decomment this line when stopSfx works properly (it nows stop together SFX and Music
    //Shared.getSound()->stopSfx();

	// Load the graphics palette
	Shared.getScreen()->setPalette(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[29]);
    
    // show blow up puzzle BG
	GraphicFrame *bg = _bgResource->getFrame(0);
	Shared.getScreen()->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	Shared.getScreen()->setCursor(_cursorResource, 0);
	Shared.getScreen()->showCursor();

	_leftClickUp = false;
    _leftClickDown = false;
    _rightClickDown = false;
	_mouseX = _mouseY = 0;
}

void BlowUpPuzzleVCR::closeBlowUp() {
	_active = false;
	Shared.getScene()->activate();
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
    case Common::EVENT_RBUTTONDOWN:
		_rightClickDown = true;
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
    Shared.getScreen()->clearGraphicsInQueue();

    if (_rightClickDown) { // quits BlowUp Puzzle
		_rightClickDown = false;
        closeBlowUp();
        // TODO: stop sound fx grResId[47] (TV On sfx)
        Shared.getScene()->enterScene();
    }

    if (_leftClickDown) {
		_leftClickDown = false;
        handleMouseDown();
    }

    if (_leftClickUp) {
		_leftClickUp = false;
        handleMouseUp();
    }

    updateCursorInPolyRegion();

    updateBlackJack();
    updateRedJack();
    updateYellowJack();

    updatePowerButton();
    updateRewindButton();
    updatePlayButton();
    updateStopButton();

    if(_buttonsState[kPower] == kON) {
        Shared.getScreen()->addGraphicToQueue(Shared.getScene()->getResources()->getWorldStats()->grResId[22], _tvScreenAnimIdx, 0, 37, 0, 0, 1);
        Shared.getScreen()->addGraphicToQueue(Shared.getScene()->getResources()->getWorldStats()->grResId[23], _tvScreenAnimIdx++, 238, 22, 0, 0, 1);
        _tvScreenAnimIdx %= 6;
    }

    if(_isAccomplished) {
        Shared.getScreen()->drawGraphicsInQueue();

        int barSize = 0;
        do { 
            Shared.getScreen()->drawWideScreen(barSize);
            barSize += 4;
        } while(barSize < 84);

        // TODO: fade palette to grey
      
        Shared.getVideo()->playVideo(2, kSubtitlesOn);

        _isAccomplished = false;
        _active = false;
        Shared.getScene()->enterScene();
    } else {
        Shared.getScreen()->drawGraphicsInQueue();
    } 
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicJackItem(int resId) {
    GraphicQueueItem jackItemOnHand;

    int jackY = _mouseY;
    if(_mouseY < 356) {
        jackY = 356;
    }

    jackItemOnHand.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[resId];
    jackItemOnHand.frameIdx = 0;
    jackItemOnHand.x = _mouseX - 114;
    jackItemOnHand.y = jackY - 14;
    jackItemOnHand.priority = 1;
    
    return jackItemOnHand;
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicShadowItem() {
    GraphicQueueItem shadowItem;

    int shadowY = (_mouseY - 356) / 4;
    if(_mouseY < 356) {
        shadowY = 0;
    }
    shadowItem.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[30];
    shadowItem.frameIdx = 0;
    shadowItem.x = _mouseX - shadowY;
    shadowItem.y = 450;
    shadowItem.priority = 2;
    
    return shadowItem;
}

void BlowUpPuzzleVCR::updateJack(Jack jack, VCRDrawInfo onTable, VCRDrawInfo pluggedOnRed, VCRDrawInfo pluggedOnYellow, VCRDrawInfo pluggedOnBlack, int resIdOnHand) {
    GraphicQueueItem item;

    switch(_jacksState[jack]){
        case kOnTable:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[onTable.resId];
            item.frameIdx = 0;
            item.x = onTable.x;
            item.y = onTable.y;
            item.priority = 3;
            break;
        case kPluggedOnRed:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[pluggedOnRed.resId];
            item.frameIdx = 0;
            item.x = 329;
            item.y = 407;
            item.priority = 3;
            break;
        case kPluggedOnYellow:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[pluggedOnYellow.resId];
            item.frameIdx = 0;
            item.x = 402;
            item.y = 413;
            item.priority = 3;
            break;
        case kPluggedOnBlack:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[pluggedOnBlack.resId];
            item.frameIdx = 0;
            item.x = 477;
            item.y = 418;
            item.priority = 3;
            break;
        case kOnHand: {
            GraphicQueueItem jackItemOnHand = getGraphicJackItem(resIdOnHand);
            Shared.getScreen()->addGraphicToQueue(jackItemOnHand);

            item = getGraphicShadowItem();
        }
            break;
        default:
            item.resId = 0;
            break;
    }

    if(item.resId != 0)
    {
        Shared.getScreen()->addGraphicToQueue(item);
    }
}

void BlowUpPuzzleVCR::updateBlackJack() {
    VCRDrawInfo onTable;
    onTable.resId = 1;
    onTable.x = 0;
    onTable.y = 411;

    VCRDrawInfo pluggedOnRed;
    pluggedOnRed.resId = 5;

    VCRDrawInfo pluggedOnYellow;
    pluggedOnYellow.resId = 8;

    VCRDrawInfo pluggedOnBlack;
    pluggedOnBlack.resId = 11;

    updateJack(kBlack, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 27);
}

void BlowUpPuzzleVCR::updateRedJack() {
    VCRDrawInfo onTable;
    onTable.resId = 2;
    onTable.x = 76;
    onTable.y = 428;

    VCRDrawInfo pluggedOnRed;
    pluggedOnRed.resId = 4;

    VCRDrawInfo pluggedOnYellow;
    pluggedOnYellow.resId = 7;

    VCRDrawInfo pluggedOnBlack;
    pluggedOnBlack.resId = 10;

    updateJack(kRed, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 25);
}

void BlowUpPuzzleVCR::updateYellowJack() {
    VCRDrawInfo onTable;
    onTable.resId = 3;
    onTable.x = 187;
    onTable.y = 439;

    VCRDrawInfo pluggedOnRed;
    pluggedOnRed.resId = 6;

    VCRDrawInfo pluggedOnYellow;
    pluggedOnYellow.resId = 9;

    VCRDrawInfo pluggedOnBlack;
    pluggedOnBlack.resId = 12;

    updateJack(kYellow, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 26);
}


// common function to set and unset the jack on holes for each type of jack
int BlowUpPuzzleVCR::setJackOnHole(int jackType, JackState plugged) {
    if(!_holesState[plugged-1]) {
        if(_jacksState[jackType-1] == kOnHand) {
            _jacksState[jackType-1] = plugged;
            _holesState[plugged-1] = jackType; // set jack on red
            Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[44]);
        }
    } else if(jackType == 0) {
        jackType = _holesState[plugged-1];
        _jacksState[jackType-1] = kOnHand;
        _holesState[plugged-1] = 0;
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[43]);
        return 0;
    }
    return 1;
}

void BlowUpPuzzleVCR::updateButton(Button button, VCRDrawInfo btON, VCRDrawInfo btDown) {
    GraphicQueueItem item;

    switch(_buttonsState[button]){
        case kON:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[btON.resId];
            item.frameIdx = 0;
            item.x = btON.x;
            item.y = btON.y;
            item.priority = 3;
            break;
        case kDownON:
        case kDownOFF:
            item.resId = Shared.getScene()->getResources()->getWorldStats()->grResId[btDown.resId];
            item.frameIdx = 0;
            item.x = btDown.x;
            item.y = btDown.y;
            item.priority = 3;
            break;
        default:
            item.resId = 0;
            break;
    }

    if(item.resId != 0)
    {
        Shared.getScreen()->addGraphicToQueue(item);
    }
}

void BlowUpPuzzleVCR::updatePowerButton() {
    VCRDrawInfo btON;
    btON.resId = 17;
    btON.x = 512;
    btON.y = 347;

    VCRDrawInfo btDown;
    btDown.resId = 21;
    btDown.x = 506;
    btDown.y = 343;

    updateButton(kPower, btON, btDown);
}

void BlowUpPuzzleVCR::updateRewindButton() {
    VCRDrawInfo btON;
    btON.resId = 14;
    btON.x = 248;
    btON.y = 347;

    VCRDrawInfo btDown;
    btDown.resId = 18;
    btDown.x = 245;
    btDown.y = 344;

    updateButton(kRewind, btON, btDown);
}

void BlowUpPuzzleVCR::updatePlayButton() {
    VCRDrawInfo btON;
    btON.resId = 16;
    btON.x = 401;
    btON.y = 359;

    VCRDrawInfo btDown;
    btDown.resId = 20;
    btDown.x = 391;
    btDown.y = 355;

    updateButton(kPlay, btON, btDown);
}

void BlowUpPuzzleVCR::updateStopButton() {
    VCRDrawInfo btON;
    btON.resId = 15;
    btON.x = 330;
    btON.y = 354;

    VCRDrawInfo btDown;
    btDown.resId = 19;
    btDown.x = 326;
    btDown.y = 350;

    updateButton(kStop, btON, btDown);
}


void BlowUpPuzzleVCR::updateCursorInPolyRegion() {
    int showCursor = 0;

    if ( _jacksState[kBlack] == kOnHand ) {
        showCursor = kBlack+1;
    } else if ( _jacksState[kRed] == kOnHand ) {
        showCursor = kRed+1;
    } else {
        showCursor = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
    }

    if(!showCursor) {
        if(inPolyRegion(_mouseX, _mouseY, kRewindButton)
           || inPolyRegion(_mouseX, _mouseY, kStopButton)
           || inPolyRegion(_mouseX, _mouseY, kPlayButton)
           || inPolyRegion(_mouseX, _mouseY, kPowerButton)
           || inPolyRegion(_mouseX, _mouseY, kBlackJack)
           || inPolyRegion(_mouseX, _mouseY, kRedJack)
           || inPolyRegion(_mouseX, _mouseY, kYellowJack)) {
            updateCursor();
        } else {
            if(inPolyRegion(_mouseX, _mouseY, kRedHole) && _holesState[kPluggedOnRed-1]
                || inPolyRegion(_mouseX, _mouseY, kYellowHole) && _holesState[kPluggedOnYellow-1]
                || inPolyRegion(_mouseX, _mouseY, kBlackHole) && _holesState[kPluggedOnBlack-1]) {
                if(_curMouseCursor != 2) { // reset cursor
                    Shared.getScreen()->showCursor();
                    _curMouseCursor = 2;
                    _cursorStep = 1;
                    updateCursor();
                }
            } else {
                if(_curMouseCursor != 0) { // reset cursor
                    Shared.getScreen()->showCursor();
                    _curMouseCursor = 0;
                    _cursorStep = 1;
                    updateCursor();
                }
            }
        }
    } else {
        Shared.getScreen()->hideCursor();
    }
}

void BlowUpPuzzleVCR::handleMouseDown() {

    if(_isAccomplished)
        return;

    int jackType = 0;
    if(_jacksState[kBlack] == kOnHand) {
        jackType = kBlack+1;
    } else if(_jacksState[kRed] == kOnHand) {
        jackType = kRed+1;
    } else {
        jackType = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
    }

    // Plug-in jacks
    if(inPolyRegion(_mouseX, _mouseY, kRedHole)) { 
        if(!setJackOnHole(jackType, kPluggedOnRed)) {
            return;
        }
    }
    if(inPolyRegion(_mouseX, _mouseY, kYellowHole)) {
        if(!setJackOnHole(jackType, kPluggedOnYellow)) {
            return;
        }
    }
    if(inPolyRegion(_mouseX, _mouseY, kBlackHole)) { 
        if(!setJackOnHole(jackType, kPluggedOnBlack)) {
            if(_holesState[kBlackHole] != kPluggedOnYellow && _buttonsState[kPower] == 1) { // TODO: check this better
                _buttonsState[kPower]  = kOFF;
                _buttonsState[kStop]   = kOFF;
                _buttonsState[kPlay]   = kOFF;
                _buttonsState[kRewind] = kOFF;
            }
            return;
        }
    }

    // Put jacks on table --
    if(jackType) {
        if (_mouseX >= (uint32)BlowUpPuzzleVCRPolies[kBlackJack].left && _mouseX <= (uint32)BlowUpPuzzleVCRPolies[kYellowJack].right &&
            _mouseY >= (uint32)BlowUpPuzzleVCRPolies[kBlackJack].top  && _mouseY <= (uint32)BlowUpPuzzleVCRPolies[kYellowJack].bottom) {
            _jacksState[jackType-1] = kOnTable;
            Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[50]);
            Shared.getScreen()->showCursor();
        }
        return;
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
    if (inPolyRegion(_mouseX, _mouseY, kRewindButton)) {
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[39]);
        if(!_buttonsState[kRewind]) {
            _buttonsState[kRewind] = kDownON;
            return;
        }
        if(_buttonsState[kRewind] == kON) {
            _buttonsState[kRewind] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_mouseX, _mouseY, kPlayButton)) {
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[39]);
        if(!_buttonsState[kPlay]) {
            _buttonsState[kPlay] = kDownON;
            return;
        }
        if(_buttonsState[kPlay] == kON) {
            _buttonsState[kPlay] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_mouseX, _mouseY, kStopButton)) {
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[39]);
        if(_buttonsState[kStop]) {
            if(_buttonsState[kStop] == kON) {
                _buttonsState[kStop] = kDownOFF;
                return;
            }
        } else {
            _buttonsState[kStop] = kDownON;
            return;
        }
    } else if (inPolyRegion(_mouseX, _mouseY, kPowerButton)) {
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[39]);
        
        if(!_buttonsState[kPower] && _holesState[kPluggedOnBlack-1] == kBlack+1 && _holesState[kPluggedOnRed-1] && _holesState[kPluggedOnYellow-1]) {
            _buttonsState[kPower] = kDownON;
        } else {
            _buttonsState[kPower] = kDownOFF;
        }
    }
}

void BlowUpPuzzleVCR::handleMouseUp() {
    if(_isAccomplished)
        return;

    if(_buttonsState[kPower] == kDownON) {
        // TODO: check if next sound is already playing
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[47]);
        _buttonsState[kPower]  = kON;
        _buttonsState[kStop]   = kON;
        _buttonsState[kPlay]   = kON;
        _buttonsState[kRewind] = kON;
    } else if(_buttonsState[kPower] == kDownOFF) {
        _buttonsState[kPower]  = kOFF;
        _buttonsState[kStop]   = kOFF;
        _buttonsState[kPlay]   = kOFF;
        _buttonsState[kRewind] = kOFF;
        // TODO: stop sound grResId[47]
    }

    if(_buttonsState[kRewind] == kDownOFF) {
        _buttonsState[kRewind] = kON;
        Shared.getSound()->playSfx(Shared.getScene()->getResourcePack(), Shared.getScene()->getResources()->getWorldStats()->grResId[46]);
    } else if(_buttonsState[kRewind] == kDownON) {
        _buttonsState[kRewind] = kOFF;
    }

    if(_buttonsState[kPlay] == kDownOFF) {
        _buttonsState[kPlay] = kON;
        if(_holesState[kPluggedOnRed-1] == kRed+1 && _holesState[kPluggedOnYellow-1] == kYellow+1 && _holesState[kPluggedOnBlack-1] == kBlack+1) {
            // TODO: set game flag 220
            _isAccomplished = true;
        }
    } else if(_buttonsState[kPlay] == kDownON) {
        _buttonsState[kPlay] = kOFF;
    }

    if(_buttonsState[kStop] == kDownOFF) {
        _buttonsState[kStop] = kON;
        return;
    }
    if(_buttonsState[kStop] == kDownON) {
        _buttonsState[kStop] = kOFF;
    }
}

} // end of namespace Asylum
