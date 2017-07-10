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
#include "sludge/graphics.h"
#include "sludge/newfatal.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/people.h"
#include "sludge/sludge.h"
#include "sludge/sludger.h"
#include "sludge/objtypes.h"
#include "sludge/region.h"
#include "sludge/backdrop.h"
#include "sludge/talk.h"
#include "sludge/fonttext.h"
#include "sludge/statusba.h"
#include "sludge/freeze.h"
#include "sludge/zbuffer.h"

namespace Sludge {

extern onScreenPerson *allPeople;
extern screenRegion *allScreenRegions;
extern screenRegion *overRegion;
extern speechStruct *speech;
extern inputType input;
extern Graphics::Surface backdropSurface;
extern Graphics::Surface renderSurface;
extern parallaxLayer *parallaxStuff;
extern int lightMapNumber, zBufferNumber;
extern eventHandlers *currentEvents;
extern personaAnimation *mouseCursorAnim;
extern int mouseCursorFrameNum;
extern int cameraX, cameraY;
extern unsigned int sceneWidth, sceneHeight;
extern float cameraZoom;
extern zBufferData zBuffer;
extern bool backdropExists;
frozenStuffStruct *frozenStuff = NULL;
extern unsigned int sceneWidth, sceneHeight;
Graphics::Surface freezeSurface;

void shufflePeople();

void freezeGraphics() {

	int w = winWidth;
	int h = winHeight;

	freezeSurface.create(w, h, *g_sludge->getScreenPixelFormat());

	// Temporarily disable AA
#if 0
	int antiAlias = gameSettings.antiAlias;
	gameSettings.antiAlias = 0;
#endif
	displayBase();
	freezeSurface.copyFrom(renderSurface);
#if 0
	gameSettings.antiAlias = antiAlias;
#endif
}

bool freeze() {
	frozenStuffStruct *newFreezer = new frozenStuffStruct;
	if (!checkNew(newFreezer))
		return false;

	// Grab a copy of the current scene
	freezeGraphics();
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;
	if (!NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
	newFreezer->backdropSurface.copyFrom(backdropSurface);
	newFreezer->sceneWidth = sceneWidth;
	newFreezer->sceneHeight = sceneHeight;
	newFreezer->cameraX = cameraX;
	newFreezer->cameraY = cameraY;
	newFreezer->cameraZoom = cameraZoom;

	newFreezer->lightMapSurface.copyFrom(lightMap);
	newFreezer->lightMapNumber = lightMapNumber;

	newFreezer->parallaxStuff = parallaxStuff;
	parallaxStuff = NULL;
	newFreezer->zBufferSprites = zBuffer.sprites;
	newFreezer->zBufferNumber = zBuffer.originalNum;
	newFreezer->zPanels = zBuffer.numPanels;
	zBuffer.sprites = NULL;
	// resizeBackdrop kills parallax stuff, light map, z-buffer...
	if (!killResizeBackdrop(winWidth, winHeight))
		return fatal("Can't create new temporary backdrop buffer");

	// Copy the old scene to the new backdrop
	backdropSurface.copyFrom(freezeSurface);
	backdropExists = true;

	newFreezer->allPeople = allPeople;
	allPeople = NULL;

	statusStuff *newStatusStuff = new statusStuff;
	if (!checkNew(newStatusStuff))
		return false;
	newFreezer->frozenStatus = copyStatusBarStuff(newStatusStuff);

	newFreezer->allScreenRegions = allScreenRegions;
	allScreenRegions = NULL;
	overRegion = NULL;

	newFreezer->mouseCursorAnim = mouseCursorAnim;
	newFreezer->mouseCursorFrameNum = mouseCursorFrameNum;
	mouseCursorAnim = makeNullAnim();
	mouseCursorFrameNum = 0;

	newFreezer->speech = speech;
	initSpeech();

	newFreezer->currentEvents = currentEvents;
	currentEvents = new eventHandlers;
	if (!checkNew(currentEvents))
		return false;
	memset(currentEvents, 0, sizeof(eventHandlers));

	newFreezer->next = frozenStuff;
	frozenStuff = newFreezer;

	return true;
}

int howFrozen() {
	int a = 0;
	frozenStuffStruct *f = frozenStuff;
	while (f) {
		a++;
		f = f->next;
	}
	return a;
}

void unfreeze(bool killImage) {
	frozenStuffStruct *killMe = frozenStuff;

	if (!frozenStuff)
		return;

	sceneWidth = frozenStuff->sceneWidth;
	sceneHeight = frozenStuff->sceneHeight;

	cameraX = frozenStuff->cameraX;
	cameraY = frozenStuff->cameraY;
	input.mouseX = (int)(input.mouseX * cameraZoom);
	input.mouseY = (int)(input.mouseY * cameraZoom);
	cameraZoom = frozenStuff->cameraZoom;
	input.mouseX = (int)(input.mouseX / cameraZoom);
	input.mouseY = (int)(input.mouseY / cameraZoom);
	setPixelCoords(false);

	killAllPeople();
	allPeople = frozenStuff->allPeople;

	killAllRegions();
	allScreenRegions = frozenStuff->allScreenRegions;

	killLightMap();

	lightMap.copyFrom(frozenStuff->lightMapSurface);
	lightMapNumber = frozenStuff->lightMapNumber;
	if (lightMapNumber) {
		loadLightMap(lightMapNumber);
	}

	if (killImage)
		killBackDrop();
	backdropSurface.copyFrom(frozenStuff->backdropSurface);
	backdropExists = true;

	zBuffer.sprites = frozenStuff->zBufferSprites;
	killZBuffer();
	zBuffer.originalNum = frozenStuff->zBufferNumber;
	zBuffer.numPanels = frozenStuff->zPanels;
	if (zBuffer.numPanels) {
		setZBuffer(zBuffer.originalNum);
	}

	killParallax();
	parallaxStuff = frozenStuff->parallaxStuff;
	reloadParallaxTextures();

	deleteAnim(mouseCursorAnim);
	mouseCursorAnim = frozenStuff->mouseCursorAnim;
	mouseCursorFrameNum = frozenStuff->mouseCursorFrameNum;

	restoreBarStuff(frozenStuff->frozenStatus);

	delete currentEvents;
	currentEvents = frozenStuff->currentEvents;
	killAllSpeech();
	delete speech;

	speech = frozenStuff->speech;
	frozenStuff = frozenStuff->next;

	overRegion = NULL;

	// free current frozen screen struct
	if (killMe->backdropSurface.getPixels())
		killMe->backdropSurface.free();
	if (killMe->lightMapSurface.getPixels())
		killMe->lightMapSurface.free();
	delete killMe;
	killMe = NULL;

}

} // End of namespace Sludge
