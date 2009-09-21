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

BlowUpPuzzle::BlowUpPuzzle(Scene *scene): _scene(scene) {
}

BlowUpPuzzle::~BlowUpPuzzle() {
}

// BlowUp Puzzle VCR ---------------------------------------------------------------------------------------------

BlowUpPuzzleVCR::BlowUpPuzzleVCR(Scene *scene): BlowUpPuzzle(scene) {
	_leftClickUp	 = false;
    _leftClickDown	 = false;
    _rightClickDown  = false;
	_active			 = false;
    _bgResource		 = _scene->getGraphicResource(_scene->worldstats()->grResId[0]);
    _tvScreenAnimIdx = 0;
    _isAccomplished  = false;

    // setup cursor
    _cursor = new Cursor(_scene->getResourcePack());

    // reset all states
    memset(&_jacksState,0,sizeof(_jacksState));
    memset(&_holesState,0,sizeof(_holesState));
    memset(&_buttonsState,0,sizeof(_buttonsState));
}

BlowUpPuzzleVCR::~BlowUpPuzzleVCR() {
	delete _cursor;
    delete _bgResource;
}

void BlowUpPuzzleVCR::openBlowUp() {
	_active = true;
	_scene->deactivate();

	_scene->vm()->sound()->stopAllSounds(true);

	// Load the graphics palette
	_scene->vm()->screen()->setPalette(_scene->getResourcePack(), _scene->worldstats()->grResId[29]);
    
    // show blow up puzzle BG
	GraphicFrame *bg = _bgResource->getFrame(0);
	_scene->vm()->screen()->copyToBackBuffer((byte *)bg->surface.pixels, bg->surface.w, 0, 0, bg->surface.w, bg->surface.h);

	// Set mouse cursor
	_cursor->load(_scene->worldstats()->grResId[28]);
	_cursor->show();

	_leftClickUp    = false;
    _leftClickDown  = false;
    _rightClickDown = false;
}

void BlowUpPuzzleVCR::closeBlowUp() {
	_active = false;
	_scene->activate();
}

void BlowUpPuzzleVCR::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {
	case Common::EVENT_MOUSEMOVE:
		_cursor->setCoords(_ev->mouse.x, _ev->mouse.y);
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

void BlowUpPuzzle::playSound(uint resourceId, bool loop) {
	int volume = _scene->vm()->soundVolume();
	_scene->vm()->sound()->playSound(_scene->getResourcePack(), resourceId, volume, loop, 0, true);
}

int BlowUpPuzzleVCR::inPolyRegion(int x, int y, int polyIdx) {
    return  x >= BlowUpPuzzleVCRPolies[polyIdx].left && x <= BlowUpPuzzleVCRPolies[polyIdx].right && 
            y >= BlowUpPuzzleVCRPolies[polyIdx].top  && y <= BlowUpPuzzleVCRPolies[polyIdx].bottom;
}

void BlowUpPuzzleVCR::update() {
    _scene->vm()->screen()->clearGraphicsInQueue();

    if (_rightClickDown) { // quits BlowUp Puzzle
		_rightClickDown = false;
        closeBlowUp();
        _scene->vm()->sound()->stopSound();
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
        _scene->vm()->screen()->addGraphicToQueue(_scene->worldstats()->grResId[22], _tvScreenAnimIdx, 0, 37, 0, 0, 1);
        _scene->vm()->screen()->addGraphicToQueue(_scene->worldstats()->grResId[23], _tvScreenAnimIdx++, 238, 22, 0, 0, 1);
        _tvScreenAnimIdx %= 6;
    }

    if(_isAccomplished) {
        _scene->vm()->screen()->drawGraphicsInQueue();

        int barSize = 0;
        do { 
            _scene->vm()->screen()->drawWideScreen(barSize);
            barSize += 4;
        } while(barSize < 84);

        // TODO: fade palette to grey
      
        _scene->vm()->video()->playVideo(2, kSubtitlesOn);

        _isAccomplished = false;
        _active = false;
        _scene->enterScene();
    } else {
        _scene->vm()->screen()->drawGraphicsInQueue();
    } 
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicJackItem(int resId) {
    GraphicQueueItem jackItemOnHand;

    int jackY = _cursor->y();
    if(_cursor->y() < 356) {
        jackY = 356;
    }

    jackItemOnHand.resId = _scene->worldstats()->grResId[resId];
    jackItemOnHand.frameIdx = 0;
    jackItemOnHand.x = _cursor->x() - 114;
    jackItemOnHand.y = jackY - 14;
    jackItemOnHand.priority = 1;
    
    return jackItemOnHand;
}

GraphicQueueItem BlowUpPuzzleVCR::getGraphicShadowItem() {
    GraphicQueueItem shadowItem;

    int shadowY = (_cursor->y() - 356) / 4;
    if(_cursor->y() < 356) {
        shadowY = 0;
    }
    shadowItem.resId = _scene->worldstats()->grResId[30];
    shadowItem.frameIdx = 0;
    shadowItem.x = _cursor->x() - shadowY;
    shadowItem.y = 450;
    shadowItem.priority = 2;
    
    return shadowItem;
}

void BlowUpPuzzleVCR::updateJack(Jack jack, VCRDrawInfo onTable, VCRDrawInfo pluggedOnRed, VCRDrawInfo pluggedOnYellow, VCRDrawInfo pluggedOnBlack, int resIdOnHand) {
    GraphicQueueItem item;

    switch(_jacksState[jack]){
        case kOnTable:
            item.resId = _scene->worldstats()->grResId[onTable.resId];
            item.frameIdx = 0;
            item.x = onTable.x;
            item.y = onTable.y;
            item.priority = 3;
            break;
        case kPluggedOnRed:
            item.resId = _scene->worldstats()->grResId[pluggedOnRed.resId];
            item.frameIdx = 0;
            item.x = 329;
            item.y = 407;
            item.priority = 3;
            break;
        case kPluggedOnYellow:
            item.resId = _scene->worldstats()->grResId[pluggedOnYellow.resId];
            item.frameIdx = 0;
            item.x = 402;
            item.y = 413;
            item.priority = 3;
            break;
        case kPluggedOnBlack:
            item.resId = _scene->worldstats()->grResId[pluggedOnBlack.resId];
            item.frameIdx = 0;
            item.x = 477;
            item.y = 418;
            item.priority = 3;
            break;
        case kOnHand: {
            GraphicQueueItem jackItemOnHand = getGraphicJackItem(resIdOnHand);
            _scene->vm()->screen()->addGraphicToQueue(jackItemOnHand);

            item = getGraphicShadowItem();
        }
            break;
        default:
            item.resId = 0;
            break;
    }

    if(item.resId != 0)
        _scene->vm()->screen()->addGraphicToQueue(item);
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
            playSound(_scene->worldstats()->grResId[44]);
        }
    } else if(jackType == 0) {
        jackType = _holesState[plugged-1];
        _jacksState[jackType-1] = kOnHand;
        _holesState[plugged-1] = 0;
        playSound(_scene->worldstats()->grResId[43]);
        return 0;
    }
    return 1;
}

void BlowUpPuzzleVCR::updateButton(Button button, VCRDrawInfo btON, VCRDrawInfo btDown) {
    GraphicQueueItem item;

    switch(_buttonsState[button]){
        case kON:
            item.resId = _scene->worldstats()->grResId[btON.resId];
            item.frameIdx = 0;
            item.x = btON.x;
            item.y = btON.y;
            item.priority = 3;
            break;
        case kDownON:
        case kDownOFF:
            item.resId = _scene->worldstats()->grResId[btDown.resId];
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
        _scene->vm()->screen()->addGraphicToQueue(item);

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
        if(inPolyRegion(_cursor->x(), _cursor->y(), kRewindButton)
           || inPolyRegion(_cursor->x(), _cursor->y(), kStopButton)
           || inPolyRegion(_cursor->x(), _cursor->y(), kPlayButton)
           || inPolyRegion(_cursor->x(), _cursor->y(), kPowerButton)
           || inPolyRegion(_cursor->x(), _cursor->y(), kBlackJack)
           || inPolyRegion(_cursor->x(), _cursor->y(), kRedJack)
           || inPolyRegion(_cursor->x(), _cursor->y(), kYellowJack)) {
            _cursor->animate();
        } else {
            if(inPolyRegion(_cursor->x(), _cursor->y(), kRedHole) && _holesState[kPluggedOnRed-1]
                || inPolyRegion(_cursor->x(), _cursor->y(), kYellowHole) && _holesState[kPluggedOnYellow-1]
                || inPolyRegion(_cursor->x(), _cursor->y(), kBlackHole) && _holesState[kPluggedOnBlack-1]) {
                if(_cursor->currentFrame() != 2) { // reset cursor
                    _cursor->show();
                    _cursor->set(2);
                    _cursor->animate();
                }
            } else {
                if(_cursor->currentFrame() != 0) { // reset cursor
                    _cursor->show();
                    _cursor->set(0);
                    _cursor->animate();
                }
            }
        }
    } else {
        _cursor->hide();
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
    if(inPolyRegion(_cursor->x(), _cursor->y(), kRedHole)) {
        if(!setJackOnHole(jackType, kPluggedOnRed)) {
            return;
        }
    }
    if(inPolyRegion(_cursor->x(), _cursor->y(), kYellowHole)) {
        if(!setJackOnHole(jackType, kPluggedOnYellow)) {
            return;
        }
    }
    if(inPolyRegion(_cursor->x(), _cursor->y(), kBlackHole)) {
        if(!setJackOnHole(jackType, kPluggedOnBlack)) {
            if(_holesState[kBlackHole] != kPluggedOnYellow && _buttonsState[kPower] == 1) {
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
        if (_cursor->x() >= (uint32)BlowUpPuzzleVCRPolies[kBlackJack].left && _cursor->x() <= (uint32)BlowUpPuzzleVCRPolies[kYellowJack].right &&
            _cursor->y() >= (uint32)BlowUpPuzzleVCRPolies[kBlackJack].top  && _cursor->y() <= (uint32)BlowUpPuzzleVCRPolies[kYellowJack].bottom) {
            _jacksState[jackType-1] = kOnTable;
            playSound(_scene->worldstats()->grResId[50]);
            _cursor->show();
        }
        return;
    }

    // Get Jacks from Table
    if (inPolyRegion(_cursor->x(), _cursor->y(), kBlackJack)) {
        _jacksState[kBlack] = kOnHand;
    } else if (inPolyRegion(_cursor->x(), _cursor->y(), kRedJack)) {
        _jacksState[kRed] = kOnHand;
    } else if (inPolyRegion(_cursor->x(), _cursor->y(), kYellowJack)) {
        _jacksState[kYellow] = kOnHand;
    }

    // VCR button regions
    if (inPolyRegion(_cursor->x(), _cursor->y(), kRewindButton)) {
    	playSound(_scene->worldstats()->grResId[39]);
        if(!_buttonsState[kRewind]) {
            _buttonsState[kRewind] = kDownON;
            return;
        }
        if(_buttonsState[kRewind] == kON) {
            _buttonsState[kRewind] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_cursor->x(), _cursor->y(), kPlayButton)) {
    	playSound(_scene->worldstats()->grResId[39]);
        if(!_buttonsState[kPlay]) {
            _buttonsState[kPlay] = kDownON;
            return;
        }
        if(_buttonsState[kPlay] == kON) {
            _buttonsState[kPlay] = kDownOFF;
            return;
        }
    } else if (inPolyRegion(_cursor->x(), _cursor->y(), kStopButton)) {
    	playSound(_scene->worldstats()->grResId[39]);
        if(_buttonsState[kStop]) {
            if(_buttonsState[kStop] == kON) {
                _buttonsState[kStop] = kDownOFF;
                return;
            }
        } else {
            _buttonsState[kStop] = kDownON;
            return;
        }
    } else if (inPolyRegion(_cursor->x(), _cursor->y(), kPowerButton)) {
    	playSound(_scene->worldstats()->grResId[39]);
        
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
    	playSound(_scene->worldstats()->grResId[47], true);
        _buttonsState[kPower]  = kON;
        _buttonsState[kStop]   = kON;
        _buttonsState[kPlay]   = kON;
        _buttonsState[kRewind] = kON;
    } else if(_buttonsState[kPower] == kDownOFF) {
        _buttonsState[kPower]  = kOFF;
        _buttonsState[kStop]   = kOFF;
        _buttonsState[kPlay]   = kOFF;
        _buttonsState[kRewind] = kOFF;
        _scene->vm()->sound()->stopSound();
    }

    if(_buttonsState[kRewind] == kDownOFF) {
        _buttonsState[kRewind] = kON;
        playSound(_scene->worldstats()->grResId[46]);
    } else if(_buttonsState[kRewind] == kDownON) {
        _buttonsState[kRewind] = kOFF;
    }

    if(_buttonsState[kPlay] == kDownOFF) {
        _buttonsState[kPlay] = kON;
        if(_holesState[kPluggedOnRed-1] == kRed+1 && _holesState[kPluggedOnYellow-1] == kYellow+1 && _holesState[kPluggedOnBlack-1] == kBlack+1) {
            _scene->vm()->setGameFlag(220);
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
