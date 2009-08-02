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
    _queueItems.clear();

    if (_leftClickDown) {
		_leftClickDown = false;
        handleMouseDown();
    }

    if (_leftClickUp) {
		_leftClickUp = false;
        handleMouseUp();
    }

    updateBlackJack();
    updateRedJack();
    updateYellowJack();

    updateCursorInPolyRegion();

    if(!_isAccomplished) {
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

void BlowUpPuzzleVCR::updateJack(Jack jack, JackInfo onTable, JackInfo pluggedOnRed, JackInfo pluggedOnYellow, JackInfo pluggedOnBlack, int resIdOnHand) {
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
    JackInfo onTable;
    onTable.resId = 1;
    onTable.x = 0;
    onTable.y = 411;

    JackInfo pluggedOnRed;
    pluggedOnRed.resId = 5;

    JackInfo pluggedOnYellow;
    pluggedOnYellow.resId = 8;

    JackInfo pluggedOnBlack;
    pluggedOnBlack.resId = 11;

    updateJack(kBlack, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 27);
}

void BlowUpPuzzleVCR::updateRedJack() {
    JackInfo onTable;
    onTable.resId = 2;
    onTable.x = 76;
    onTable.y = 428;

    JackInfo pluggedOnRed;
    pluggedOnRed.resId = 4;

    JackInfo pluggedOnYellow;
    pluggedOnYellow.resId = 7;

    JackInfo pluggedOnBlack;
    pluggedOnBlack.resId = 10;

    updateJack(kRed, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 25);
}

void BlowUpPuzzleVCR::updateYellowJack() {
    JackInfo onTable;
    onTable.resId = 3;
    onTable.x = 187;
    onTable.y = 439;

    JackInfo pluggedOnRed;
    pluggedOnRed.resId = 6;

    JackInfo pluggedOnYellow;
    pluggedOnYellow.resId = 9;

    JackInfo pluggedOnBlack;
    pluggedOnBlack.resId = 12;

    updateJack(kYellow, onTable, pluggedOnRed, pluggedOnYellow, pluggedOnBlack, 26);
}


void BlowUpPuzzleVCR::updateCursorInPolyRegion() {
    int showCursor = 0;

    if ( _jacksState[kBlack] == kOnHand ) {
        showCursor = 1;
    } else if ( _jacksState[kRed] == kOnHand ) {
        showCursor = 2;
    } else {
        showCursor = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
    }

    if(!showCursor) {
        if(inPolyRegion(_mouseX, _mouseY, kRewindButton)
           || inPolyRegion(_mouseX, _mouseY, kStopButton)
           || inPolyRegion(_mouseX, _mouseY, kPlayButton)
           || inPolyRegion(_mouseX, _mouseY, kRecButton)
           || inPolyRegion(_mouseX, _mouseY, kBlackJack)
           || inPolyRegion(_mouseX, _mouseY, kRedJack)
           || inPolyRegion(_mouseX, _mouseY, kYellowJack)) {
            updateCursor();
        } else {
            if(_curMouseCursor != 0) { // reset cursor
                _screen->showCursor();
                _curMouseCursor = 0;
                _cursorStep = 1;
                updateCursor();
            }
        }
    } else {
        _screen->hideCursor();
    }

}

void BlowUpPuzzleVCR::handleMouseDown() {
    /*int newState = 1; // v1
    if(_jacksState[kBlack] != kOnHand) {
        if(_jacksState[kRed] == kOnHand) {
            newState = 2;
        } else {
            newState = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
        }
    }*/

    // TODO: put jacks in holes
    if(inPolyRegion(_mouseX, _mouseY, kRedHole)) { 
    }

    if(inPolyRegion(_mouseX, _mouseY, kYellowHole)) { 
    }

    if(inPolyRegion(_mouseX, _mouseY, kBlackHole)) { 
    }

    // Put jacks on table --
    int jackType = 0;
    if(_jacksState[kBlack] == kOnHand) {
        jackType = kBlack+1;
    } else if(_jacksState[kRed] == kOnHand) {
        jackType = kRed+1;
    } else {
        jackType = ((_jacksState[kYellow] != kOnHand) - 1) & 3;
    }
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
}

void BlowUpPuzzleVCR::handleMouseUp() {

}

} // end of namespace Asylum
