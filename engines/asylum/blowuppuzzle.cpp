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

BlowUpPuzzle::BlowUpPuzzle(Scene *scene) :
	_scene(scene), _screen(scene->getScreen()), _sound(scene->getSound()), _video(scene->getVideo()) {
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

void BlowUpPuzzle::addGraphicToQueue(uint32 redId, uint32 x, uint32 y, uint32 frameIdx, uint32 flags, uint32 priority) {
    GraphicQueueItem item;
    item.resId = redId;
    item.x = x;
    item.y = y;
    item.frameIdx = frameIdx;
    item.flags = flags;
    item.priority = priority;

    _queueItems.push_back(item);
}

void BlowUpPuzzle::addGraphicToQueue(GraphicQueueItem item) {
    _queueItems.push_back(item);
}

void BlowUpPuzzle::updateGraphicsInQueue() {
    // sort by priority first
    graphicsSelectionSort();
    for(uint i = 0; i < _queueItems.size(); i++) {
        GraphicResource *jack = _scene->getGraphicResource(_queueItems[i].resId);
        GraphicFrame *fra = jack->getFrame(_queueItems[i].frameIdx);
        _screen->copyRectToScreenWithTransparency((byte *)fra->surface.pixels, fra->surface.w, _queueItems[i].x, _queueItems[i].y, fra->surface.w, fra->surface.h);
    }
}

void BlowUpPuzzle::graphicsSelectionSort() {
    uint minIdx;

    for (uint i = 0; i < _queueItems.size() - 1; i++) {
        minIdx = i;

        for (uint j = i + 1; j < _queueItems.size(); j++)
            if (_queueItems[j].priority > _queueItems[i].priority)
                minIdx = j;

        if(i != minIdx)
            swapGraphicItem(i, minIdx);
   }
}

void BlowUpPuzzle::swapGraphicItem(int item1, int item2) {
   GraphicQueueItem temp;
   temp = _queueItems[item1];
   _queueItems[item1] = _queueItems[item2];
   _queueItems[item2] = temp;
} 


// BlowUp Puzzle VCR ---------------------------------------------------------------------------------------------

BlowUpPuzzleVCR::BlowUpPuzzleVCR(Scene *scene) : BlowUpPuzzle(scene) {
	_mouseX	            = 0;
	_mouseY			    = 0;
	_leftClickUp	    = false;
    _leftClickDown	    = false;
    _rightClickDown     = false;
	_curMouseCursor	    = 0;
	_cursorStep		    = 1;
	_active			    = false;
    _cursorResource     = _scene->getGraphicResource(_scene->getResources()->getWorldStats()->grResId[28]);
    _bgResource		    = _scene->getGraphicResource(_scene->getResources()->getWorldStats()->grResId[0]);
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
	_scene->deactivate();
    // FIXME: decomment this line when stopSfx works properly (it nows stop together SFX and Music
    //_sound->stopSfx();

	// Load the graphics palette
	_screen->setPalette(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[29]);
    
    // show blow up puzzle BG
	GraphicFrame *bg = _bgResource->getFrame(0);
	_screen->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	_screen->setCursor(_cursorResource, 0);
	_screen->showCursor();

	_leftClickUp = false;
    _leftClickDown = false;
    _rightClickDown = false;
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
    _queueItems.clear();

    if (_rightClickDown) { // quits BlowUp Puzzle
		_rightClickDown = false;
        closeBlowUp();
        // TODO: stop sound fx grResId[47] (TV On sfx)
        _scene->enterScene();
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
        addGraphicToQueue(_scene->getResources()->getWorldStats()->grResId[22], 0, 37, _tvScreenAnimIdx, 0, 1);
        addGraphicToQueue(_scene->getResources()->getWorldStats()->grResId[23], 238, 22, _tvScreenAnimIdx++, 0, 1);
        _tvScreenAnimIdx %= 6;
    }

    if(_isAccomplished) {
        debug("BlowUpPuzzle ACCOMPLISHED!!");

        updateGraphicsInQueue();

        int barSize = 0;
        do { 
            _screen->drawWideScreen(barSize);
            barSize += 4;
        } while(barSize < 84);

        // TODO: fade palette to grey
      
        _video->playVideo(2, kSubtitlesOn);

        _isAccomplished = false;
        _active = false;
        _scene->enterScene();
    } else {
        updateGraphicsInQueue();
    } 
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicJackItem(int resId) {
    GraphicQueueItem jackItemOnHand;

    int jackY = _mouseY;
    if(_mouseY < 356) {
        jackY = 356;
    }

    jackItemOnHand.resId = _scene->getResources()->getWorldStats()->grResId[resId];
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
    shadowItem.resId = _scene->getResources()->getWorldStats()->grResId[30];
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
            item.resId = _scene->getResources()->getWorldStats()->grResId[onTable.resId];
            item.frameIdx = 0;
            item.x = onTable.x;
            item.y = onTable.y;
            item.priority = 3;
            break;
        case kPluggedOnRed:
            item.resId = _scene->getResources()->getWorldStats()->grResId[pluggedOnRed.resId];
            item.frameIdx = 0;
            item.x = 329;
            item.y = 407;
            item.priority = 3;
            break;
        case kPluggedOnYellow:
            item.resId = _scene->getResources()->getWorldStats()->grResId[pluggedOnYellow.resId];
            item.frameIdx = 0;
            item.x = 402;
            item.y = 413;
            item.priority = 3;
            break;
        case kPluggedOnBlack:
            item.resId = _scene->getResources()->getWorldStats()->grResId[pluggedOnBlack.resId];
            item.frameIdx = 0;
            item.x = 477;
            item.y = 418;
            item.priority = 3;
            break;
        case kOnHand: {
            GraphicQueueItem jackItemOnHand = getGraphicJackItem(resIdOnHand);
            addGraphicToQueue(jackItemOnHand);

            item = getGraphicShadowItem();
        }
            break;
        default:
            item.resId = 0;
            break;
    }

    if(item.resId != 0)
    {
        addGraphicToQueue(item);
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
            _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[44]);
        }
    } else if(jackType == 0) {
        jackType = _holesState[plugged-1];
        _jacksState[jackType-1] = kOnHand;
        _holesState[plugged-1] = 0;
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[43]);
        return 0;
    }
    return 1;
}

void BlowUpPuzzleVCR::updateButton(Button button, VCRDrawInfo btON, VCRDrawInfo btDown) {
    GraphicQueueItem item;

    switch(_buttonsState[button]){
        case kON:
            item.resId = _scene->getResources()->getWorldStats()->grResId[btON.resId];
            item.frameIdx = 0;
            item.x = btON.x;
            item.y = btON.y;
            item.priority = 3;
            break;
        case kDownON:
        case kDownOFF:
            item.resId = _scene->getResources()->getWorldStats()->grResId[btDown.resId];
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
        addGraphicToQueue(item);
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
                    _screen->showCursor();
                    _curMouseCursor = 2;
                    _cursorStep = 1;
                    updateCursor();
                }
            } else {
                if(_curMouseCursor != 0) { // reset cursor
                    _screen->showCursor();
                    _curMouseCursor = 0;
                    _cursorStep = 1;
                    updateCursor();
                }
            }
        }
    } else {
        _screen->hideCursor();
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
            _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[50]);
            _screen->showCursor();
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
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[39]);
        if(!_buttonsState[kRewind]) {
            _buttonsState[kRewind] = kDownON;
            return;
        }
        if(_buttonsState[kRewind] == kON) {
            _buttonsState[kRewind] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_mouseX, _mouseY, kPlayButton)) {
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[39]);
        if(!_buttonsState[kPlay]) {
            _buttonsState[kPlay] = kDownON;
            return;
        }
        if(_buttonsState[kPlay] == kON) {
            _buttonsState[kPlay] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_mouseX, _mouseY, kStopButton)) {
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[39]);
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
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[39]);
        
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
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[47]);
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
        _sound->playSfx(_scene->getResourcePack(), _scene->getResources()->getWorldStats()->grResId[46]);        
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
