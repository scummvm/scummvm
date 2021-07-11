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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/hresmgr.h"
#include "saga2/button.h"
#include "saga2/objects.h"
#include "saga2/fontlib.h"

namespace Saga2 {

typedef void *pVOID;
typedef pVOID apVOID[];

extern void playMemSound(uint32 s); // play click # s

/* ======================================================================= *
    Compressed image class
 * ======================================================================= */

void gCompImage::init(void) {
	compImages      = NULL;
	max             = 0;
	min             = 0;
	internalAlloc   = false;
	currentImage    = 0;
	numPtrAlloc     = 0;
	textFont        = &Onyx10Font;  // default
}

gCompImage::gCompImage(gPanelList &list, const Rect16 &box, void *image, uint16 ident,
                       AppFunc *cmd) : gControl(list, box, NULL, ident, cmd) {
	// setup a single image configuration

	init();

	if (image) {
		compImages = (void **)malloc(sizeof(pVOID) * 1); // allocate room for one pointer
		compImages[0] = image;
		internalAlloc   = false;
		numPtrAlloc     = 1;
	}
}

gCompImage::gCompImage(gPanelList &list,
                       const Rect16 &box,
                       uint32 contextID,
                       char a, char b, char c,
                       uint16 resNum, uint16 numImages,
                       uint16 ident,
                       AppFunc *cmd) : gControl(list, box, NULL, ident, cmd) {
	uint16 i, rNum;

	init();

	// init the resource context handle
	hResContext *resContext = resFile->newContext(contextID, "container window resource");

	// setup for a numImages image configuration
	compImages = (void **)malloc(sizeof(void *)*numImages);  // allocate room for numImages pointers

	for (i = 0, rNum = resNum; i < numImages; i++, rNum++) {
		compImages[i] = LoadResource(resContext,
		                               MKTAG(a, b, c, rNum),
		                               " gCompImage ");
	}

	max             = numImages - 1;
	internalAlloc   = true;
	numPtrAlloc     = numImages;

	// get rid of this context
	resFile->disposeContext(resContext);
	resContext = NULL;
}

gCompImage::gCompImage(gPanelList &list, const Rect16 &box, void *image, const char *text, textPallete &pal, uint16 ident,
                       AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	// setup a single image configuration
	init();

	if (!image)
		return;

	compImages = (void **)malloc(sizeof(void *) * 1); // allocate room for one pointer

	compImages[0] = image;
	max             = 0;
	numPtrAlloc     = 1;
	title           = text;
	textFont        = &Onyx10Font;  // >>> this should be dynamic
	textPal         = pal;
}

gCompImage::gCompImage(gPanelList &list, const Rect16 &box, void **images,
                       int16 numRes, int16 initial,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, NULL, ident, cmd) {
	init();

	if (!images)
		return;

	compImages      = images;

	// set up limits
	max             = numRes - 1;
	currentImage    = clamp(min, initial, max);
}

gCompImage::gCompImage(gPanelList &list, const Rect16 &box, void **images,
                       int16 numRes, int16 initial, const char *text, textPallete &pal,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	init();

	if (images) {
		compImages      = images;

		// set up limits
		max             = numRes - 1;
		currentImage    = clamp(min, initial, max);
	}

	title       = text;
	textFont    = &Onyx10Font;  // >>> this should be dynamic
	textPal     = pal;
}

gCompImage::gCompImage(gPanelList &list, const StaticRect &box, void **images,
                       int16 numRes, int16 initial, const char *text, textPallete &pal,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	init();

	if (images) {
		compImages = images;

		// set up limits
		max          = numRes - 1;
		currentImage = clamp(min, initial, max);
	}

	title    = text;
	textFont = &Onyx10Font;  // >>> this should be dynamic
	textPal  = pal;
}


gCompImage::~gCompImage(void) {
	// delete any allocated image pointers
	// for JEFFL: I took out the winklude #ifdefs becuase I belive
	// I fixed the problem that was causing the crash under win32
	// 11-14-95, I should talk to you tommorow. This is note is
	// a precaution

	// if we LoadRes'ed image internally RDispose those
	if (internalAlloc) {
		for (int16 i = 0; i < numPtrAlloc; i++) {
			free(compImages[i]);
		}
	}

	// delete any pointer arrays new'ed
	if (numPtrAlloc > 0) {
		free(compImages);
	}
}

void gCompImage::pointerMove(gPanelMessage &msg) {
	// call the superclass's pointerMove
	gControl::pointerMove(msg);

	notify(gEventMouseMove, (msg.pointerEnter ? enter : 0) | (msg.pointerLeave ? leave : 0));
}

void gCompImage::enable(bool abled) {
	gPanel::enable(abled);
}

void gCompImage::invalidate(Rect16 *) {
	window.update(extent);
}

void gCompImage::draw(void) {
	gPort   &port = window.windowPort;
	Rect16  rect = window.getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port,
	            Point16(0, 0),
	            Rect16(0, 0, rect.width, rect.height));
	pointer.show(port, extent);              // show mouse pointer
}

void *gCompImage::getCurrentCompImage(void) {
	if (compImages) {
		return compImages[currentImage];  // return the image pointed to by compImage
	} else {
		return NULL;
	}
}

// waring! : the number of images has has to be == to the inital number
void gCompImage::setImages(void **images) {
	if (images) {
		compImages = images;
	}
}

void gCompImage::setImage(void *image) {
	if (image) {
		compImages[0] = image;
		max             = 0;
		currentImage    = 0;
	}
}

void gCompImage::select(uint16 val) {
	setCurrent(val);

	if (getEnabled()) {
		window.update(extent);
	}
}

void gCompImage::select(uint16 val, const Rect16 &rect) {
	select(val);
	setExtent(rect);
}

void gCompImage::setExtent(const Rect16 &rect) {
	// set the new extent
	extent = rect;
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void gCompImage::drawClipped(gPort &port,
                             const Point16 &offset,
                             const Rect16 &r) {
	if (!extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// get the current image
	void *dispImage = getCurrentCompImage();

	// make sure the image is valid
	if (dispImage) {
		// will part of this be drawn on screen?
		if (extent.overlap(r)) {
			// offset the image?
			Point16 pos(extent.x - offset.x,
			            extent.y - offset.y
			           );
			// draw the compressed image
			if (isGhosted()) drawCompressedImageGhosted(port, pos, dispImage);
			else drawCompressedImage(port, pos, dispImage);

			// this could be modified to get the current text coloring
			if (title) {
				Rect16 textRect = extent;
				textRect.x -= offset.x;
				textRect.y -= offset.y;

				writePlaqText(port, textRect, textFont, 0, textPal, selected, title);
			}
		}
	}
}

/* ===================================================================== *
   gCompImageButton class member functions
 * ===================================================================== */

gSpriteImage::gSpriteImage(gPanelList &list, const Rect16 &box, GameObject *object, char,
                           uint16 ident, AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd) {
	// get the prototype for the object
	ProtoObj *proto = object->proto();

	// assign the sprites remapped colors
	object->getColorTranslation(objColors);

	// assing the sprite pointer
	sprPtr = proto->getSprite(object, ProtoObj::objInContainerView).sp;
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void gSpriteImage::drawClipped(gPort &port,
                               const Point16 &offset,
                               const Rect16 &r) {
	if (!extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// if there's a sprite present
	gPixelMap       map;

	//map.size = Point16( extent.height, extent.width );
	map.size = sprPtr->size;

	map.data = (uint8 *)malloc(map.bytes() * sizeof(uint8));
	if (map.data == NULL) return;

	memset(map.data, 0, map.bytes());

	//  Render the sprite into the bitmap image sequence
	ExpandColorMappedSprite(map, sprPtr, objColors);

	port.setMode(drawModeMatte);
	port.bltPixels(map, 0, 0,
	               extent.x - offset.x, extent.y - offset.y,
	               map.size.x, map.size.y);

	free(map.data);
}

/* ===================================================================== *
   gCompImageButton class member functions
 * ===================================================================== */

void gCompButton::loadImages(hResContext *con, hResID res1, hResID res2) {
	if (con) {
		forImage = LoadResource(con, res1, "CBtn fore image");
		resImage = LoadResource(con, res2, "CBtn res image");
		dimImage    = NULL;
	} else {
		forImage    = NULL;
		resImage    = NULL;
		dimImage    = NULL;
	}

	internalAlloc   = true;
	dimmed          = false;
}

void gCompButton::loadImages(hResID contextID, hResID res1, hResID res2) {
	// init the resource context handle
	hResContext *con = resFile->newContext(contextID,
	                                       "container window resource");

	loadImages(con, res1, res2);
	resFile->disposeContext(con);               // get rid of this context
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, hResContext *con, hResID resID1, hResID resID2, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd), extent(box) {
	loadImages(con, resID1, resID2);
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, hResID contextID, hResID resID1, hResID resID2, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd), extent(box) {
	loadImages(contextID, resID1, resID2);
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, hResContext *con, char a, char b, char c, int16 butNum_1, int16 butNum_2, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd), extent(box) {
	loadImages(con, MKTAG(a, b, c, butNum_1), MKTAG(a, b, c, butNum_2));
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, hResID contextID, char a, char b, char c, int16 butNum_1, int16 butNum_2, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd), extent(box) {
	loadImages(contextID, MKTAG(a, b, c, butNum_1), MKTAG(a, b, c, butNum_2));
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, hResContext *con, int16 butNum, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd), extent(box) {
	loadImages(con, MKTAG('B', 'T', 'N', butNum), MKTAG('B', 'T', 'N', butNum + 1));
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		forImage    = images[0];
		resImage    = images[1];
		dimImage    = NULL;
	} else {
		forImage    = NULL;
		resImage    = NULL;
		dimImage    = NULL;
	}

	internalAlloc   = false;
	dimmed          = false;
	extent          = box;
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, const char *text, textPallete &pal, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, 0, 0, text, pal, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		forImage    = images[0];
		resImage    = images[1];
		dimImage    = NULL;
	} else {
		forImage    = NULL;
		resImage    = NULL;
		dimImage    = NULL;
	}

	internalAlloc   = false;
	dimmed          = false;
	extent          = box;
}

gCompButton::gCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, void *newDimImage, bool dimNess, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		forImage    = images[0];
		resImage    = images[1];
	} else {
		forImage    = NULL;
		resImage    = NULL;
	}

	if (newDimImage) {
		dimImage = newDimImage;
	} else {
		dimImage = NULL;
	}

	internalAlloc   = false;
	dimmed          = dimNess;
	extent          = box;
}


gCompButton::gCompButton(gPanelList &list, const Rect16 &box, void *image, uint16 ident,
                         AppFunc *cmd) : gCompImage(list, box, NULL, ident, cmd)

{
	if (image) {
		forImage    = image;
		resImage    = image;
		dimImage    = NULL;
	} else {
		forImage    = NULL;
		resImage    = NULL;
		dimImage    = NULL;
	}

	internalAlloc   = false;
	dimmed          = false;
	extent          = box;
}

gCompButton::gCompButton(gPanelList &list, const StaticRect &box, void **images, int16 numRes, const char *text, textPallete &pal, uint16 ident, AppFunc *cmd) : gCompImage(list, box, NULL, 0, 0, text, pal, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		forImage = images[0];
		resImage = images[1];
		dimImage = nullptr;
	} else {
		forImage = nullptr;
		resImage = nullptr;
		dimImage = nullptr;
	}

	internalAlloc = false;
	dimmed        = false;
	extent        = box;
}

gCompButton::~gCompButton(void) {
	if (internalAlloc) {
		if (forImage) {
			free(forImage);
			forImage = NULL;
		}

		if (resImage) {
			free(resImage);
			resImage = NULL;
		}

		if (dimImage) {
			free(dimImage);
			dimImage = NULL;
		}
	}
}

void gCompButton::dim(bool enableFlag) {
	if (enableFlag) {
		if (!dimmed) dimmed = true;
	} else {
		if (dimmed) dimmed = false;
	}

	window.update(extent);
}


void gCompButton::deactivate(void) {
	selected = 0;
	window.update(extent);
	gPanel::deactivate();
}

bool gCompButton::activate(gEventType why) {
	selected = 1;
	window.update(extent);

	if (why == gEventKeyDown) { // momentarily depress
		deactivate();
		notify(gEventNewValue, 1);       // notify App of successful hit
	}
	playMemSound(2);
	return false;
}

void gCompButton::pointerMove(gPanelMessage &msg) {
	if (dimmed) return;

	//notify( gEventMouseMove, (msg.pointerEnter ? enter : 0)|(msg.pointerLeave ? leave : 0));
	gCompImage::pointerMove(msg);
}

bool gCompButton::pointerHit(gPanelMessage &) {
	if (dimmed) return false;

	activate(gEventMouseDown);
	return true;
}

void gCompButton::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.

	if (selected) {
		deactivate();                       // give back input focus
		notify(gEventNewValue, 1);       // notify App of successful hit
	} else deactivate();
}

void gCompButton::pointerDrag(gPanelMessage &msg) {
	if (selected != msg.inPanel) {
		selected = msg.inPanel;
		window.update(extent);
	}
}

void gCompButton::enable(bool abled) {
	gPanel::enable(abled);
}

void gCompButton::invalidate(Rect16 *) {
	window.update(extent);
}


void gCompButton::draw(void) {
	gPort   &port = window.windowPort;
	Rect16  rect = window.getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	pointer.show(port, extent);              // show mouse pointer
}

void *gCompButton::getCurrentCompImage(void) {
	if (dimmed) {
		return dimImage;
	} else if (selected) {
		return resImage;
	} else {
		return forImage;
	}
}

/************************************************************************
* gToggleCompButton -- like a gCompButton but toggle on and off.        *
************************************************************************/

gToggleCompButton::gToggleCompButton(gPanelList &list, const Rect16 &box, hResContext *con, char a, char b, char c, int16 butNum_1, int16 butNum_2, uint16 ident,
                                     AppFunc *cmd) : gCompButton(list, box, con, a, b, c, butNum_1, butNum_2, ident, cmd) {

}


gToggleCompButton::gToggleCompButton(gPanelList &list, const Rect16 &box, hResContext *con, int16 butNum, uint16 ident,
                                     AppFunc *cmd) : gCompButton(list, box, con, butNum, ident, cmd) {

}

gToggleCompButton::gToggleCompButton(gPanelList &list, const Rect16 &box, void **images, int16 butRes, uint16 ident,
                                     AppFunc *cmd) : gCompButton(list, box, images, butRes, ident, cmd) {

}

gToggleCompButton::gToggleCompButton(gPanelList &list, const Rect16 &box, void **images, int16 butRes, char *text, textPallete &pal, uint16 ident,
                                     AppFunc *cmd) : gCompButton(list, box, images, butRes, text, pal, ident, cmd) {

}

bool gToggleCompButton::activate(gEventType why) {
	if (why == gEventKeyDown || why == gEventMouseDown) {
//		playSound( MKTAG('C','B','T',3) );

		selected = !selected;
		window.update(extent);
		gPanel::deactivate();
		notify(gEventNewValue, selected);    // notify App of successful hit
		playMemSound(1);
	}
	return false;
}

bool gToggleCompButton::pointerHit(gPanelMessage &) {
	return activate(gEventMouseDown);
}

void gToggleCompButton::select(uint16 val) {
	selected = val;

	setCurrent(val);

	if (getEnabled()) {
		window.update(extent);
	}
}


/************************************************************************
* gOwnerSelCompButton -- like a gCompButton but does not chage the      *
* selector bit                                                          *
************************************************************************/

gOwnerSelCompButton::gOwnerSelCompButton(gPanelList &list, const Rect16 &box, void **images, int16 butRes, uint16 ident,
        AppFunc *cmd) : gCompButton(list, box, images, butRes, ident, cmd) {

}

bool gOwnerSelCompButton::activate(gEventType why) {
	if (why == gEventKeyDown || why == gEventMouseDown) {
//		selected = !selected;
//		window.update( extent );
		gPanel::deactivate();
		notify(gEventNewValue, selected);    // notify App of successful hit
		playMemSound(2);
	}
	return false;
}

bool gOwnerSelCompButton::pointerHit(gPanelMessage &) {
	return activate(gEventMouseDown);
}

void gOwnerSelCompButton::select(uint16 val) {
	selected = val;

	setCurrent(val);

	if (getEnabled()) {
		window.update(extent);
	}
}

/************************************************************************
* gMultCompButton -- like gCompButton but does any number of images     *
************************************************************************/

gMultCompButton::gMultCompButton(gPanelList &list, const Rect16 &box, hResContext *con, char a, char b, char c, int16 resStart, int16 numRes, int16 initial, uint16 ident,
                                 AppFunc *cmd) : gCompButton(list, box, (hResContext *)NULL, 0, ident, cmd) {
	int16   i, k;


	images = (void **)malloc(sizeof(void *)*numRes);

	for (i = 0, k = resStart; i < numRes; i++, k++) {
		images[i] = LoadResource(con, MKTAG(a, b, c, k), "Multi btn image");
	}

	response = true;
	internalAlloc = true;
	max     = numRes - 1;
	min     = 0;
	current = clamp(min, initial, max);

	extent  = box;
}

gMultCompButton::gMultCompButton(gPanelList &list, const Rect16 &box, void **newImages, int16 numRes, int16 initial, uint16 ident,
                                 AppFunc *cmd) : gCompButton(list, box, (hResContext *)NULL, 0, ident, cmd) {
	if (!newImages) {
		images  = NULL;
		max     = 0;
		min     = 0;
		current = 0;
		return;
	}

	images = newImages;

	response = true;
	internalAlloc = false;
	max     = numRes - 1;
	min     = 0;
	current = initial;

	extent  = box;
}

gMultCompButton::gMultCompButton(gPanelList &list, const Rect16 &box, void **newImages,
                                 int16 numRes, int16 initial, bool hitResponse, uint16 ident,
                                 AppFunc *cmd) : gCompButton(list, box, (hResContext *)NULL, 0, ident, cmd) {
	if (!newImages) {
		images  = NULL;
		max     = 0;
		min     = 0;
		current = 0;
		response = hitResponse;
		return;
	}

	images = newImages;

	response = hitResponse;
	internalAlloc = false;
	max     = numRes - 1;
	min     = 0;
	current = initial;

	extent  = box;
}

gMultCompButton::~gMultCompButton(void) {
	int16   i;

	if (images && internalAlloc) {
		for (i = 0; i <= max; i++) {
			if (images[i]) {
				free(images[i]);
			}
		}

		free(images);
		images = NULL;
	}
}

bool gMultCompButton::activate(gEventType why) {
	if (why == gEventKeyDown || why == gEventMouseDown) {
		if (response) {
			if (++current > max) {
				current = 0;
			}
			window.update(extent);
		}

		gPanel::deactivate();
		notify(gEventNewValue, current);     // notify App of successful hit
		playMemSound(1);
//		playSound( MKTAG('C','B','T',5) );
	}
	return false;
}

bool gMultCompButton::pointerHit(gPanelMessage &) {
	return activate(gEventMouseDown);
}

void *gMultCompButton::getCurrentCompImage(void) {
	return images[current];
}

/* ===================================================================== *
   gSlider class
 * ===================================================================== */

gSlider::gSlider(gPanelList &list, const Rect16 &box, const Rect16 &imageBox,
                 int16 sliderStart, int16 sliderEnd, void **newImages, int16 resStart,
                 int16 initial, uint16 ident,
                 AppFunc *cmd) : gMultCompButton(list, box, newImages, resStart, initial, ident, cmd) {
	int16   calcX;

	imageRect   = imageBox;
	slValMin    = sliderStart;
	slValMax    = sliderEnd;
	slCurrent   = initial;

	// find out the position of the slider
	calcX = (slValMax * 100) / clamp(1, slCurrent, slCurrent);
	calcX = (extent.width * 100) / clamp(1, calcX, calcX);

	imagePosX = clamp(extent.x,
	                  calcX,
	                  extent.width - imageRect.x);
}

void *gSlider::getCurrentCompImage(void) {
	int16   val;
	int32   index;

	val = getSliderLenVal();

	// max == number of images in array indexing;

	index = val / clamp(1, max + 1, max + 1);

	index = slCurrent / clamp(1, index, index);

	index = clamp(0, index, max);

	return images[index];
}

int16 gSlider::getSliderLenVal(void) {
	int16   val = 0;

	if (slValMin < 0 && slValMax < 0) {
		val = slValMax - slValMin;
	} else if (slValMin < 0 && slValMax >= 0) {
		val = ABS(slValMin) + slValMax;
	} else if (slValMin >= 0 && slValMax < 0) {
		val = ABS(slValMax) - slValMin;
	} else if (slValMin >= 0 && slValMax >= 0) {
		val = slValMax - slValMin;
	}

	return val;
}

void gSlider::draw(void) {
	gPort   &port   = window.windowPort;
	Point16 offset  = Point16(0, 0);

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	pointer.hide(port, extent);              // hide mouse pointer
	drawClipped(port, offset, Rect16(0, 0, imageRect.width, imageRect.height));
	pointer.show(port, extent);              // show mouse pointer
}


inline int16 quantizedVolume(uint16 trueVolume) {
	int16 quantized = trueVolume & 0xFFF8;
	quantized += (quantized / 16);
	return quantized;
}

void gSlider::drawClipped(gPort &port,
                          const Point16 &offset,
                          const Rect16 &r) {
	void *dispImage = getCurrentCompImage();
	if (dispImage) {
		if (extent.overlap(r)) {
			Point16 pos(imagePosX - offset.x,
			            extent.y - offset.y
			           );
			if (isGhosted()) drawCompressedImageGhosted(port, pos, dispImage);
			else drawCompressedImage(port, pos, dispImage);
		}
	}
}

bool gSlider::activate(gEventType why) {
	if (why == gEventKeyDown || why == gEventMouseDown) {
		selected = 1;
		window.update(extent);
		gPanel::deactivate();
		notify(gEventNewValue, slCurrent);   // notify App of successful hit
	}
	return false;
}

void gSlider::deactivate(void) {
	selected = 0;
	window.update(extent);
	gPanel::deactivate();
}

bool gSlider::pointerHit(gPanelMessage &msg) {
	// update the image index
	updateSliderIndexes(msg.pickPos);

	// redraw the control should any visual change hath occured
	window.update(extent);

	activate(gEventMouseDown);
	return true;
}

void gSlider::pointerMove(gPanelMessage &msg) {
	if (selected) {
		// update the image index
		updateSliderIndexes(msg.pickPos);

		// redraw the control should any visual change hath occured
		window.update(extent);

		notify(gEventMouseMove, slCurrent);
	}
}

void gSlider::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.
	if (selected) {
		deactivate();                       // give back input focus
		notify(gEventNewValue, slCurrent);       // notify App of successful hit
	} else deactivate();
}

void gSlider::pointerDrag(gPanelMessage &msg) {
	// update the image index
	updateSliderIndexes(msg.pickPos);

	notify(gEventNewValue, slCurrent);       // notify App of successful hit
	// redraw the control should any visual change hath occured
	window.update(extent);
}

void gSlider::updateSliderIndexes(Point16 &pos) {
	pos.x = quantizedVolume(pos.x);
	// get x position units
	int32   unit    = (extent.width * 100) / clamp(1, pos.x, extent.width);

	// find the ratio and get the current slider value
	slCurrent       = (slValMax * 100) / clamp(1, unit, unit);

	// update the image position index
	imagePosX           = clamp(extent.x,
	                            pos.x,
	                            extent.width - imageRect.x);
}

} // end of namespace Saga2
