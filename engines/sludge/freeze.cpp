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

#include "sludge/allfiles.h"
#include "sludge/backdrop.h"
#include "sludge/cursors.h"
#include "sludge/event.h"
#include "sludge/fonttext.h"
#include "sludge/freeze.h"
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/objtypes.h"
#include "sludge/people.h"
#include "sludge/region.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/speech.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/statusba.h"
#include "sludge/zbuffer.h"

namespace Sludge {

void GraphicsManager::freezeGraphics() {

	int w = _winWidth;
	int h = _winHeight;

	_freezeSurface.create(w, h, *g_sludge->getScreenPixelFormat());

	displayBase();
	_freezeSurface.copyFrom(_renderSurface);
}

bool GraphicsManager::freeze() {
	FrozenStuffStruct *newFreezer = new FrozenStuffStruct;
	if (!checkNew(newFreezer))
		return false;

	// Grab a copy of the current scene
	freezeGraphics();
	newFreezer->backdropSurface.copyFrom(_backdropSurface);
	newFreezer->sceneWidth = _sceneWidth;
	newFreezer->sceneHeight = _sceneHeight;
	newFreezer->cameraX = _cameraX;
	newFreezer->cameraY = _cameraY;
	newFreezer->cameraZoom = _cameraZoom;

	newFreezer->lightMapSurface.copyFrom(_lightMap);
	newFreezer->lightMapNumber = _lightMapNumber;

	newFreezer->parallaxStuff = _parallaxStuff;
	_parallaxStuff = NULL;
	newFreezer->zBufferSprites = _zBuffer->sprites;
	newFreezer->zBufferNumber = _zBuffer->originalNum;
	newFreezer->zPanels = _zBuffer->numPanels;
	_zBuffer->sprites = NULL;
	// resizeBackdrop kills parallax stuff, light map, z-buffer...
	if (!killResizeBackdrop(_winWidth, _winHeight))
		return fatal("Can't create new temporary backdrop buffer");

	// Copy the old scene to the new backdrop
	_backdropSurface.copyFrom(_freezeSurface);
	_backdropExists = true;

	_vm->_peopleMan->freeze(newFreezer);

	StatusStuff *newStatusStuff = new StatusStuff;
	if (!checkNew(newStatusStuff))
		return false;
	newFreezer->frozenStatus = copyStatusBarStuff(newStatusStuff);

	_vm->_regionMan->freeze(newFreezer);
	_vm->_cursorMan->freeze(newFreezer);
	_vm->_speechMan->freeze(newFreezer);
	_vm->_evtMan->freeze(newFreezer);

	newFreezer->next = _frozenStuff;
	_frozenStuff = newFreezer;

	return true;
}

int GraphicsManager::howFrozen() {
	int a = 0;
	FrozenStuffStruct *f = _frozenStuff;
	while (f) {
		a++;
		f = f->next;
	}
	return a;
}

void GraphicsManager::unfreeze(bool killImage) {
	FrozenStuffStruct *killMe = _frozenStuff;

	if (!_frozenStuff)
		return;

	_sceneWidth = _frozenStuff->sceneWidth;
	_sceneHeight = _frozenStuff->sceneHeight;

	_cameraX = _frozenStuff->cameraX;
	_cameraY = _frozenStuff->cameraY;
	_vm->_evtMan->mouseX() = (int)(_vm->_evtMan->mouseX() * _cameraZoom);
	_vm->_evtMan->mouseY() = (int)(_vm->_evtMan->mouseY() * _cameraZoom);
	_cameraZoom = _frozenStuff->cameraZoom;
	_vm->_evtMan->mouseX() = (int)(_vm->_evtMan->mouseX() / _cameraZoom);
	_vm->_evtMan->mouseY() = (int)(_vm->_evtMan->mouseY() / _cameraZoom);

	g_sludge->_peopleMan->resotre(_frozenStuff);
	g_sludge->_regionMan->resotre(_frozenStuff);

	killLightMap();

	_lightMap.copyFrom(_frozenStuff->lightMapSurface);
	_lightMapNumber = _frozenStuff->lightMapNumber;
	if (_lightMapNumber) {
		loadLightMap(_lightMapNumber);
	}

	if (killImage)
		killBackDrop();
	_backdropSurface.copyFrom(_frozenStuff->backdropSurface);
	_backdropExists = true;

	_zBuffer->sprites = _frozenStuff->zBufferSprites;
	killZBuffer();
	_zBuffer->originalNum = _frozenStuff->zBufferNumber;
	_zBuffer->numPanels = _frozenStuff->zPanels;
	if (_zBuffer->numPanels) {
		setZBuffer(_zBuffer->originalNum);
	}

	killParallax();
	_parallaxStuff = _frozenStuff->parallaxStuff;
	_vm->_cursorMan->resotre(_frozenStuff);
	restoreBarStuff(_frozenStuff->frozenStatus);
	_vm->_evtMan->restore(_frozenStuff);
	_vm->_speechMan->restore(_frozenStuff);

	_frozenStuff = _frozenStuff->next;

	// free current frozen screen struct
	if (killMe->backdropSurface.getPixels())
		killMe->backdropSurface.free();
	if (killMe->lightMapSurface.getPixels())
		killMe->lightMapSurface.free();
	delete killMe;
	killMe = NULL;

}

} // End of namespace Sludge
