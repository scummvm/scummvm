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
#include "allfiles.h"
#include "debug.h"
#include "graphics.h"
#include "newfatal.h"
#include "sprites.h"
#include "sprbanks.h"
#include "people.h"
#include "sludger.h"
#include "objtypes.h"
#include "region.h"
#include "backdrop.h"
#include "talk.h"
#include "fonttext.h"
#include "statusba.h"
#include "freeze.h"
#include "zbuffer.h"

namespace Sludge {

extern onScreenPerson *allPeople;
extern screenRegion *allScreenRegions;
extern screenRegion *overRegion;
extern speechStruct *speech;
extern inputType input;
#if 0
extern GLuint backdropTextureName;
#endif
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

void shufflePeople();
#if 0
GLuint freezeTextureName = 0;
#endif

void freezeGraphics() {
#if 0
	glViewport(0, 0, realWinWidth, realWinHeight);

	glGenTextures(1, &freezeTextureName);
#endif
	int w = winWidth;
	int h = winHeight;
	if (!NPOT_textures) {
		w = getNextPOT(winWidth);
		h = getNextPOT(winHeight);
	}
#if 0
	glBindTexture(GL_TEXTURE_2D, freezeTextureName);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0, freezeTextureName);
#endif
	// Temporarily disable AA
	int antiAlias = gameSettings.antiAlias;
	gameSettings.antiAlias = 0;

	int x = 0;
	while (x < winWidth) {
		int y = 0;

		if (winWidth - x < realWinWidth) {
			w = winWidth - x;
		} else {
			w = realWinWidth;
		}

		while (y < winHeight) {

			if (winHeight - y < realWinHeight) {
				h = winHeight - y;
			} else {
				h = realWinHeight;
			}
#if 0
			const GLfloat bPMVMatrix[] = {
				2.0f / realWinWidth * cameraZoom, .0, .0, .0,
				.0, 2.0f / realWinHeight * cameraZoom, .0, .0,
				.0, .0, 1.0f, .0,
				-2.0f * (x / realWinWidth) - 1.0f, -2.0f * (y / realWinHeight) - 1.0f, .0, 1.0f

			};
			for (int i = 0; i < 16; i++) {
				aPMVMatrix[i] = bPMVMatrix[i];
			}
			// Render scene
			glDepthMask(GL_TRUE);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);// Clear The Screen
			glDepthMask(GL_FALSE);

			drawBackDrop();// Draw the room
			drawZBuffer(cameraX, cameraY, false);

			glEnable(GL_DEPTH_TEST);

			drawPeople();// Then add any moving characters...

			glDisable(GL_DEPTH_TEST);

			// Copy Our ViewPort To The Texture
			copyTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 0, 0, w, h, freezeTextureName);
#endif
			y += h;
		}
		x += w;
	}

	gameSettings.antiAlias = antiAlias;

#if 0
	glViewport(viewportOffsetX, viewportOffsetY, viewportWidth, viewportHeight);
	setPixelCoords(false);
#endif
}

bool freeze() {
	debugOut("calling freeze()\n");
	frozenStuffStruct *newFreezer = new frozenStuffStruct;
	if (!checkNew(newFreezer))
		return false;

	// Grab a copy of the current scene
	freezeGraphics();
#if 0
	newFreezer -> backdropTextureName = backdropTextureName;
#endif
	int picWidth = sceneWidth;
	int picHeight = sceneHeight;
	if (!NPOT_textures) {
		picWidth = getNextPOT(picWidth);
		picHeight = getNextPOT(picHeight);
	}
#if 0
	newFreezer -> backdropTexture = new GLubyte [picHeight * picWidth * 4];
	if (! checkNew(newFreezer -> backdropTexture)) return false;

	saveTexture(backdropTextureName, newFreezer->backdropTexture);

	backdropTextureName = 0;
#endif
	newFreezer->sceneWidth = sceneWidth;
	newFreezer->sceneHeight = sceneHeight;
	newFreezer->cameraX = cameraX;
	newFreezer->cameraY = cameraY;
	newFreezer->cameraZoom = cameraZoom;
#if 0
	newFreezer -> lightMapTexture = lightMap.data;
	newFreezer -> lightMapTextureName = lightMap.name;
	newFreezer -> lightMapNumber = lightMapNumber;
	lightMap.data = NULL;
	lightMap.name = 0;
	newFreezer -> parallaxStuff = parallaxStuff;
	parallaxStuff = NULL;
	newFreezer -> zBufferImage = zBuffer.tex;
	newFreezer -> zBufferNumber = zBuffer.originalNum;
	newFreezer -> zPanels = zBuffer.numPanels;
	zBuffer.tex = NULL;
#endif
	// resizeBackdrop kills parallax stuff, light map, z-buffer...
	if (!killResizeBackdrop(winWidth, winHeight))
		return fatal("Can't create new temporary backdrop buffer");

	if (!NPOT_textures) {
		picWidth = getNextPOT(sceneWidth);
		picHeight = getNextPOT(sceneHeight);
#if 0
		backdropTexW = (double) sceneWidth / picWidth;
		backdropTexH = (double) sceneHeight / picHeight;
#endif
	}

#if 0
	// Copy the old scene to the new backdrop
	deleteTextures(1, &backdropTextureName);
	backdropTextureName = freezeTextureName;
	backdropExists = true;

	// Free texture memory used by old stuff
	parallaxStuff = newFreezer -> parallaxStuff;
	while (parallaxStuff) {
		deleteTextures(1, &parallaxStuff -> textureName);
		parallaxStuff = parallaxStuff -> next;
	}
	if (newFreezer -> zBufferImage) {
		deleteTextures(1, &zBuffer.texName);
	}
	if (newFreezer -> lightMapTextureName) {
		deleteTextures(1, &newFreezer -> lightMapTextureName);
	}
	if (newFreezer -> backdropTextureName) {
		deleteTextures(1, &newFreezer -> backdropTextureName);
	}
#endif
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

#if 0
extern GLubyte *backdropTexture;
#endif

void unfreeze(bool killImage) {
	frozenStuffStruct *killMe = frozenStuff;

	if (!frozenStuff)
		return;

	sceneWidth = frozenStuff->sceneWidth;
	sceneHeight = frozenStuff->sceneHeight;

	cameraX = frozenStuff->cameraX;
	cameraY = frozenStuff->cameraY;
	input.mouseX = (int) (input.mouseX * cameraZoom);
	input.mouseY = (int) (input.mouseY * cameraZoom);
	cameraZoom = frozenStuff->cameraZoom;
	input.mouseX = (int) (input.mouseX / cameraZoom);
	input.mouseY = (int) (input.mouseY / cameraZoom);
	setPixelCoords(false);

	killAllPeople();
	allPeople = frozenStuff->allPeople;

	killAllRegions();
	allScreenRegions = frozenStuff->allScreenRegions;

	killLightMap();
#if 0
	lightMap.data = frozenStuff -> lightMapTexture;
	lightMap.name = frozenStuff -> lightMapTextureName;
	lightMapNumber = frozenStuff -> lightMapNumber;
	if (lightMapNumber) {
		lightMap.name = 0;
		loadLightMap(lightMapNumber);
	}

	killZBuffer();
	zBuffer.tex = frozenStuff -> zBufferImage;
	zBuffer.originalNum = frozenStuff -> zBufferNumber;
	zBuffer.numPanels = frozenStuff -> zPanels;
	if (zBuffer.numPanels) {
		zBuffer.texName = 0;
		setZBuffer(zBuffer.originalNum);
	}

	killParallax();
	parallaxStuff = frozenStuff -> parallaxStuff;
	reloadParallaxTextures();

	if (killImage) killBackDrop();
	backdropTextureName = frozenStuff -> backdropTextureName;
	if (backdropTexture) delete[] backdropTexture;
	backdropTexture = frozenStuff -> backdropTexture;
	backdropExists = true;
	if (backdropTextureName) {
		backdropTextureName = 0;
		glGenTextures(1, &backdropTextureName);
		glBindTexture(GL_TEXTURE_2D, backdropTextureName);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		if (gameSettings.antiAlias < 0) {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		} else {
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		}

		int picWidth = sceneWidth;
		int picHeight = sceneHeight;
		if (! NPOT_textures) {
			picWidth = getNextPOT(picWidth);
			picHeight = getNextPOT(picHeight);
		}
		// Restore the backdrop
		texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, picWidth, picHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, frozenStuff -> backdropTexture, backdropTextureName);
	}
#endif
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
	delete killMe;
	killMe = NULL;

}

} // End of namespace Sludge
