/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

void GfxCompImage::init() {
	_compImages      = nullptr;
	_max             = 0;
	_min             = 0;
	_internalAlloc   = false;
	_currentImage    = 0;
	_numPtrAlloc     = 0;
	_textFont        = &Onyx10Font;  // default
}

GfxCompImage::GfxCompImage(gPanelList &list, const Rect16 &box, void *image, uint16 ident,
                       AppFunc *cmd) : gControl(list, box, nullptr, ident, cmd) {
	// setup a single image configuration

	init();

	if (image) {
		_compImages = (void **)malloc(sizeof(pVOID) * 1); // allocate room for one pointer
		_compImages[0] = image;
		_internalAlloc   = false;
		_numPtrAlloc     = 1;
	}
}

GfxCompImage::GfxCompImage(gPanelList &list,
                       const Rect16 &box,
                       uint32 contextID,
                       char a, char b, char c,
                       uint16 resNum, uint16 numImages,
                       uint16 ident,
                       AppFunc *cmd) : gControl(list, box, nullptr, ident, cmd) {
	uint16 i, rNum;

	init();

	// init the resource context handle
	hResContext *resContext = resFile->newContext(contextID, "container window resource");

	// setup for a numImages image configuration
	_compImages = (void **)malloc(sizeof(void *)*numImages);  // allocate room for numImages pointers

	for (i = 0, rNum = resNum; i < numImages; i++, rNum++) {
		_compImages[i] = LoadResource(resContext,
		                               MKTAG(a, b, c, rNum),
		                               " GfxCompImage ");
	}

	_max             = numImages - 1;
	_internalAlloc   = true;
	_numPtrAlloc     = numImages;

	// get rid of this context
	resFile->disposeContext(resContext);
	resContext = nullptr;
}

GfxCompImage::GfxCompImage(gPanelList &list, const Rect16 &box, void *image, const char *text, textPallete &pal, uint16 ident,
                       AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	// setup a single image configuration
	init();

	if (!image)
		return;

	_compImages = (void **)malloc(sizeof(void *) * 1); // allocate room for one pointer

	_compImages[0] = image;
	_max             = 0;
	_numPtrAlloc     = 1;
	_title           = text;
	_textFont        = &Onyx10Font;  // >>> this should be dynamic
	_textPal         = pal;
}

GfxCompImage::GfxCompImage(gPanelList &list, const Rect16 &box, void **images,
                       int16 numRes, int16 initial,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, nullptr, ident, cmd) {
	init();

	if (!images)
		return;

	_compImages      = images;

	// set up limits
	_max             = numRes - 1;
	_currentImage    = clamp(_min, initial, _max);
}

GfxCompImage::GfxCompImage(gPanelList &list, const Rect16 &box, void **images,
                       int16 numRes, int16 initial, const char *text, textPallete &pal,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	init();

	if (images) {
		_compImages      = images;

		// set up limits
		_max             = numRes - 1;
		_currentImage    = clamp(_min, initial, _max);
	}

	_title       = text;
	_textFont    = &Onyx10Font;  // >>> this should be dynamic
	_textPal     = pal;
}

GfxCompImage::GfxCompImage(gPanelList &list, const StaticRect &box, void **images,
                       int16 numRes, int16 initial, const char *text, textPallete &pal,
                       uint16 ident, AppFunc *cmd) : gControl(list, box, text, ident, cmd) {
	init();

	if (images) {
		_compImages = images;

		// set up limits
		_max          = numRes - 1;
		_currentImage = clamp(_min, initial, _max);
	}

	_title    = text;
	_textFont = &Onyx10Font;  // >>> this should be dynamic
	_textPal  = pal;
}


GfxCompImage::~GfxCompImage() {
	// delete any allocated image pointers
	// for JEFFL: I took out the winklude #ifdefs because I believe
	// I fixed the problem that was causing the crash under win32
	// 11-14-95, I should talk to you tomorrow. This note is
	// a precaution

	// if we LoadRes'ed image internally RDispose those
	if (_internalAlloc) {
		for (int16 i = 0; i < _numPtrAlloc; i++) {
			free(_compImages[i]);
		}
	}

	// delete any pointer arrays new'ed
	if (_numPtrAlloc > 0) {
		free(_compImages);
	}
}

void GfxCompImage::pointerMove(gPanelMessage &msg) {
	// call the superclass's pointerMove
	gControl::pointerMove(msg);

	notify(kEventMouseMove, (msg._pointerEnter ? kEnter : 0) | (msg._pointerLeave ? kLeave : 0));
}

void GfxCompImage::enable(bool abled) {
	gPanel::enable(abled);
}

void GfxCompImage::invalidate(Rect16 *) {
	_window.update(_extent);
}

void GfxCompImage::draw() {
	gPort   &port = _window._windowPort;
	Rect16  rect = _window.getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port,
	            Point16(0, 0),
	            Rect16(0, 0, rect.width, rect.height));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}

void *GfxCompImage::getCurrentCompImage() {
	if (_compImages) {
		return _compImages[_currentImage];  // return the image pointed to by compImage
	} else {
		return nullptr;
	}
}

// warning! : the number of images has has to be == to the initial number
void GfxCompImage::setImages(void **images) {
	if (images) {
		_compImages = images;
	}
}

void GfxCompImage::setImage(void *image) {
	if (image) {
		_compImages[0] = image;
		_max             = 0;
		_currentImage    = 0;
	}
}

void GfxCompImage::select(uint16 val) {
	setCurrent(val);

	if (getEnabled()) {
		_window.update(_extent);
	}
}

void GfxCompImage::select(uint16 val, const Rect16 &rect) {
	select(val);
	setExtent(rect);
}

void GfxCompImage::setExtent(const Rect16 &rect) {
	// set the new extent
	_extent = rect;
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void GfxCompImage::drawClipped(gPort &port,
                             const Point16 &offset,
                             const Rect16 &r) {
	if (!_extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// get the current image
	void *dispImage = getCurrentCompImage();

	// make sure the image is valid
	if (dispImage) {
		// will part of this be drawn on screen?
		if (_extent.overlap(r)) {
			// offset the image?
			Point16 pos(_extent.x - offset.x,
			            _extent.y - offset.y
			           );
			// draw the compressed image
			if (isGhosted()) drawCompressedImageGhosted(port, pos, dispImage);
			else drawCompressedImage(port, pos, dispImage);

			// this could be modified to get the current text coloring
			if (_title) {
				Rect16 textRect = _extent;
				textRect.x -= offset.x;
				textRect.y -= offset.y;

				writePlaqText(port, textRect, _textFont, 0, _textPal, _selected, _title);
			}
		}
	}
}

/* ===================================================================== *
   GfxSpriteImage class member functions
 * ===================================================================== */

GfxSpriteImage::GfxSpriteImage(gPanelList &list, const Rect16 &box, GameObject *object, char,
                           uint16 ident, AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd) {
	// get the prototype for the object
	ProtoObj *proto = object->proto();

	// assign the sprites remapped colors
	object->getColorTranslation(_objColors);

	// assing the sprite pointer
	_sprPtr = proto->getSprite(object, ProtoObj::kObjInContainerView).sp;
}

// getCurrentCompImage() is virtual function that should return
// the current image to be displayed (to be used across all sub-classes)
void GfxSpriteImage::drawClipped(gPort &port,
                               const Point16 &offset,
                               const Rect16 &r) {
	if (!_extent.overlap(r))    return;

	SAVE_GPORT_STATE(port);

	// if there's a sprite present
	gPixelMap       map;

	//map._size = Point16( extent.height, extent.width );
	map._size = _sprPtr->size;

	map._data = (uint8 *)malloc(map.bytes() * sizeof(uint8));
	if (map._data == nullptr) return;

	memset(map._data, 0, map.bytes());

	//  Render the sprite into the bitmap image sequence
	ExpandColorMappedSprite(map, _sprPtr, _objColors);

	port.setMode(kDrawModeMatte);
	port.bltPixels(map, 0, 0,
	               _extent.x - offset.x, _extent.y - offset.y,
	               map._size.x, map._size.y);

	free(map._data);
}

/* ===================================================================== *
   GfxCompButton class member functions
 * ===================================================================== */

void GfxCompButton::loadImages(hResContext *con, hResID res1, hResID res2) {
	if (con) {
		_forImage = LoadResource(con, res1, "CBtn fore image");
		_resImage = LoadResource(con, res2, "CBtn res image");
		_dimImage    = nullptr;
	} else {
		_forImage    = nullptr;
		_resImage    = nullptr;
		_dimImage    = nullptr;
	}

	_internalAlloc   = true;
	_dimmed          = false;
}

void GfxCompButton::loadImages(hResID contextID, hResID res1, hResID res2) {
	// init the resource context handle
	hResContext *con = resFile->newContext(contextID,
	                                       "container window resource");

	loadImages(con, res1, res2);
	resFile->disposeContext(con);               // get rid of this context
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, hResContext *con, hResID resID1, hResID resID2, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd), _extent(box) {
	loadImages(con, resID1, resID2);
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, hResID contextID, hResID resID1, hResID resID2, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd), _extent(box) {
	loadImages(contextID, resID1, resID2);
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, hResContext *con, char a, char b, char c, int16 butNum_1, int16 butNum_2, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd), _extent(box) {
	loadImages(con, MKTAG(a, b, c, butNum_1), MKTAG(a, b, c, butNum_2));
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, hResID contextID, char a, char b, char c, int16 butNum_1, int16 butNum_2, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd), _extent(box) {
	loadImages(contextID, MKTAG(a, b, c, butNum_1), MKTAG(a, b, c, butNum_2));
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, hResContext *con, int16 butNum, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd), _extent(box) {
	loadImages(con, MKTAG('B', 'T', 'N', butNum), MKTAG('B', 'T', 'N', butNum + 1));
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		_forImage    = images[0];
		_resImage    = images[1];
		_dimImage    = nullptr;
	} else {
		_forImage    = nullptr;
		_resImage    = nullptr;
		_dimImage    = nullptr;
	}

	_internalAlloc   = false;
	_dimmed          = false;
	_extent          = box;
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, const char *text, textPallete &pal, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, 0, 0, text, pal, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		_forImage    = images[0];
		_resImage    = images[1];
		_dimImage    = nullptr;
	} else {
		_forImage    = nullptr;
		_resImage    = nullptr;
		_dimImage    = nullptr;
	}

	_internalAlloc   = false;
	_dimmed          = false;
	_extent          = box;
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, void **images, int16 numRes, void *newDimImage, bool dimNess, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		_forImage    = images[0];
		_resImage    = images[1];
	} else {
		_forImage    = nullptr;
		_resImage    = nullptr;
	}

	if (newDimImage) {
		_dimImage = newDimImage;
	} else {
		_dimImage = nullptr;
	}

	_internalAlloc   = false;
	_dimmed          = dimNess;
	_extent          = box;
}


GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, void *image, uint16 ident,
                         AppFunc *cmd) : GfxCompImage(list, box, nullptr, ident, cmd)

{
	if (image) {
		_forImage    = image;
		_resImage    = image;
		_dimImage    = nullptr;
	} else {
		_forImage    = nullptr;
		_resImage    = nullptr;
		_dimImage    = nullptr;
	}

	_internalAlloc   = false;
	_dimmed          = false;
	_extent          = box;
}

GfxCompButton::GfxCompButton(gPanelList &list, const StaticRect &box, void **images, int16 numRes, const char *text, textPallete &pal, uint16 ident, AppFunc *cmd) : GfxCompImage(list, box, nullptr, 0, 0, text, pal, ident, cmd) {
	if (images[0] && images[1] && numRes == 2) {
		_forImage = images[0];
		_resImage = images[1];
		_dimImage = nullptr;
	} else {
		_forImage = nullptr;
		_resImage = nullptr;
		_dimImage = nullptr;
	}

	_internalAlloc = false;
	_dimmed        = false;
	_extent        = box;
}

GfxCompButton::GfxCompButton(gPanelList &list, const Rect16 &box, AppFunc *cmd) : GfxCompImage(list, box, nullptr, 0, cmd) {
	_forImage    = nullptr;
	_resImage    = nullptr;
	_dimImage    = nullptr;

	_internalAlloc   = false;
	_dimmed          = false;
	_extent          = box;
}

GfxCompButton::~GfxCompButton() {
	if (_internalAlloc) {
		if (_forImage) {
			free(_forImage);
			_forImage = nullptr;
		}

		if (_resImage) {
			free(_resImage);
			_resImage = nullptr;
		}

		if (_dimImage) {
			free(_dimImage);
			_dimImage = nullptr;
		}
	}
}

void GfxCompButton::dim(bool enableFlag) {
	if (enableFlag) {
		if (!_dimmed)
			_dimmed = true;
	} else {
		if (_dimmed)
			_dimmed = false;
	}

	_window.update(_extent);
}


void GfxCompButton::deactivate() {
	_selected = 0;
	_window.update(_extent);
	gPanel::deactivate();
}

bool GfxCompButton::activate(gEventType why) {
	_selected = 1;
	_window.update(_extent);

	if (why == kEventKeyDown) { // momentarily depress
		deactivate();
		notify(kEventNewValue, 1);       // notify App of successful hit
	}
	playMemSound(2);
	return false;
}

void GfxCompButton::pointerMove(gPanelMessage &msg) {
	if (_dimmed)
		return;

	//notify( kEventMouseMove, (msg.pointerEnter ? enter : 0)|(msg.pointerLeave ? leave : 0));
	GfxCompImage::pointerMove(msg);
}

bool GfxCompButton::pointerHit(gPanelMessage &) {
	if (_dimmed)
		return false;

	activate(kEventMouseDown);
	return true;
}

void GfxCompButton::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.

	if (_selected) {
		deactivate();                       // give back input focus
		notify(kEventNewValue, 1);       // notify App of successful hit
	} else deactivate();
}

void GfxCompButton::pointerDrag(gPanelMessage &msg) {
	if (_selected != msg._inPanel) {
		_selected = msg._inPanel;
		_window.update(_extent);
	}
}

void GfxCompButton::enable(bool abled) {
	gPanel::enable(abled);
}

void GfxCompButton::invalidate(Rect16 *) {
	_window.update(_extent);
}


void GfxCompButton::draw() {
	gPort   &port = _window._windowPort;
	Rect16  rect = _window.getExtent();

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port, Point16(0, 0), Rect16(0, 0, rect.width, rect.height));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}

void *GfxCompButton::getCurrentCompImage() {
	if (_dimmed) {
		return _dimImage;
	} else if (_selected) {
		return _resImage;
	} else {
		return _forImage;
	}
}

/************************************************************************
* GfxOwnerSelCompButton -- like a GfxCompButton but does not chage the      *
* selector bit                                                          *
************************************************************************/

GfxOwnerSelCompButton::GfxOwnerSelCompButton(gPanelList &list, const Rect16 &box, void **images, int16 butRes, uint16 ident,
        AppFunc *cmd) : GfxCompButton(list, box, images, butRes, ident, cmd) {

}

bool GfxOwnerSelCompButton::activate(gEventType why) {
	if (why == kEventKeyDown || why == kEventMouseDown) {
//		selected = !selected;
//		window.update( extent );
		gPanel::deactivate();
		notify(kEventNewValue, _selected);    // notify App of successful hit
		playMemSound(2);
	}
	return false;
}

bool GfxOwnerSelCompButton::pointerHit(gPanelMessage &) {
	return activate(kEventMouseDown);
}

void GfxOwnerSelCompButton::select(uint16 val) {
	_selected = val;

	setCurrent(val);

	if (getEnabled()) {
		_window.update(_extent);
	}
}

/************************************************************************
* GfxMultCompButton -- like GfxCompButton but does any number of images     *
************************************************************************/

GfxMultCompButton::GfxMultCompButton(gPanelList &list, const Rect16 &box, hResContext *con, char a, char b, char c, int16 resStart, int16 numRes, int16 initial, uint16 ident,
                                 AppFunc *cmd) : GfxCompButton(list, box, (hResContext *)nullptr, 0, ident, cmd) {
	int16   i, k;


	_images = (void **)malloc(sizeof(void *)*numRes);

	for (i = 0, k = resStart; i < numRes; i++, k++) {
		_images[i] = LoadResource(con, MKTAG(a, b, c, k), "Multi btn image");
	}

	_response = true;
	_internalAlloc = true;
	_max     = numRes - 1;
	_min     = 0;
	_current = clamp(_min, initial, _max);

	_extent  = box;
}

GfxMultCompButton::GfxMultCompButton(gPanelList &list, const Rect16 &box, void **newImages, int16 numRes, int16 initial, uint16 ident,
                                 AppFunc *cmd) : GfxCompButton(list, box, (hResContext *)nullptr, 0, ident, cmd) {
	if (!newImages) {
		_images  = nullptr;
		_max     = 0;
		_min     = 0;
		_current = 0;
		_response = false;
		return;
	}

	_images = newImages;

	_response = true;
	_internalAlloc = false;
	_max     = numRes - 1;
	_min     = 0;
	_current = initial;

	_extent  = box;
}

GfxMultCompButton::GfxMultCompButton(gPanelList &list, const Rect16 &box, void **newImages,
                                 int16 numRes, int16 initial, bool hitResponse, uint16 ident,
                                 AppFunc *cmd) : GfxCompButton(list, box, (hResContext *)nullptr, 0, ident, cmd) {
	if (!newImages) {
		_images  = nullptr;
		_max     = 0;
		_min     = 0;
		_current = 0;
		_response = hitResponse;
		return;
	}

	_images = newImages;

	_response = hitResponse;
	_internalAlloc = false;
	_max     = numRes - 1;
	_min     = 0;
	_current = initial;

	_extent  = box;
}

GfxMultCompButton::~GfxMultCompButton() {
	int16   i;

	if (_images && _internalAlloc) {
		for (i = 0; i <= _max; i++) {
			if (_images[i]) {
				free(_images[i]);
			}
		}

		free(_images);
		_images = nullptr;
	}
}

bool GfxMultCompButton::activate(gEventType why) {
	if (why == kEventKeyDown || why == kEventMouseDown) {
		if (_response) {
			if (++_current > _max) {
				_current = 0;
			}
			_window.update(_extent);
		}

		gPanel::deactivate();
		notify(kEventNewValue, _current);     // notify App of successful hit
		playMemSound(1);
//		playSound( MKTAG('C','B','T',5) );
	}
	return false;
}

bool GfxMultCompButton::pointerHit(gPanelMessage &) {
	return activate(kEventMouseDown);
}

void *GfxMultCompButton::getCurrentCompImage() {
	return _images[_current];
}

/* ===================================================================== *
   GfxSlider class
 * ===================================================================== */

GfxSlider::GfxSlider(gPanelList &list, const Rect16 &box, const Rect16 &imageBox,
                 int16 sliderStart, int16 sliderEnd, void **newImages, int16 resStart,
                 int16 initial, uint16 ident,
                 AppFunc *cmd) : GfxMultCompButton(list, box, newImages, resStart, initial, ident, cmd) {
	int16   calcX;

	_imageRect   = imageBox;
	_slValMin    = sliderStart;
	_slValMax    = sliderEnd;
	_slCurrent   = initial;

	// find out the position of the slider
	calcX = (_slValMax * 100) / clamp(1, _slCurrent, _slCurrent);
	calcX = (_extent.width * 100) / clamp(1, calcX, calcX);

	_imagePosX = clamp(_extent.x,
	                  calcX,
	                  _extent.width - _imageRect.x);
}

void *GfxSlider::getCurrentCompImage() {
	int16   val;
	int32   index;

	val = getSliderLenVal();

	// max == number of images in array indexing;

	index = val / clamp(1, _max + 1, _max + 1);

	index = _slCurrent / clamp(1, index, index);

	index = clamp(0, index, _max);

	return _images[index];
}

int16 GfxSlider::getSliderLenVal() {
	int16   val = 0;

	if (_slValMin < 0 && _slValMax < 0) {
		val = _slValMax - _slValMin;
	} else if (_slValMin < 0 && _slValMax >= 0) {
		val = ABS(_slValMin) + _slValMax;
	} else if (_slValMin >= 0 && _slValMax < 0) {
		val = ABS(_slValMax) - _slValMin;
	} else if (_slValMin >= 0 && _slValMax >= 0) {
		val = _slValMax - _slValMin;
	}

	return val;
}

void GfxSlider::draw() {
	gPort   &port   = _window._windowPort;
	Point16 offset  = Point16(0, 0);

	SAVE_GPORT_STATE(port);                  // save pen color, etc.
	g_vm->_pointer->hide(port, _extent);              // hide mouse pointer
	drawClipped(port, offset, Rect16(0, 0, _imageRect.width, _imageRect.height));
	g_vm->_pointer->show(port, _extent);              // show mouse pointer
}


inline int16 quantizedVolume(uint16 trueVolume) {
	int16 quantized = trueVolume & 0xFFF8;
	quantized += (quantized / 16);
	return quantized;
}

void GfxSlider::drawClipped(gPort &port,
                          const Point16 &offset,
                          const Rect16 &r) {
	void *dispImage = getCurrentCompImage();
	if (dispImage) {
		if (_extent.overlap(r)) {
			Point16 pos(_imagePosX - offset.x,
			            _extent.y - offset.y
			           );
			if (isGhosted()) drawCompressedImageGhosted(port, pos, dispImage);
			else drawCompressedImage(port, pos, dispImage);
		}
	}
}

bool GfxSlider::activate(gEventType why) {
	if (why == kEventKeyDown || why == kEventMouseDown) {
		_selected = 1;
		_window.update(_extent);
		gPanel::deactivate();
		notify(kEventNewValue, _slCurrent);   // notify App of successful hit
	}
	return false;
}

void GfxSlider::deactivate() {
	_selected = 0;
	_window.update(_extent);
	gPanel::deactivate();
}

bool GfxSlider::pointerHit(gPanelMessage &msg) {
	// update the image index
	updateSliderIndexes(msg._pickPos);

	// redraw the control should any visual change hath occurred
	_window.update(_extent);

	activate(kEventMouseDown);
	return true;
}

void GfxSlider::pointerMove(gPanelMessage &msg) {
	if (_selected) {
		// update the image index
		updateSliderIndexes(msg._pickPos);

		// redraw the control should any visual change hath occurred
		_window.update(_extent);

		notify(kEventMouseMove, _slCurrent);
	}
}

void GfxSlider::pointerRelease(gPanelMessage &) {
	//  We have to test selected first because deactivate clears it.
	if (_selected) {
		deactivate();                       // give back input focus
		notify(kEventNewValue, _slCurrent);       // notify App of successful hit
	} else deactivate();
}

void GfxSlider::pointerDrag(gPanelMessage &msg) {
	// update the image index
	updateSliderIndexes(msg._pickPos);

	notify(kEventNewValue, _slCurrent);       // notify App of successful hit
	// redraw the control should any visual change hath occurred
	_window.update(_extent);
}

void GfxSlider::updateSliderIndexes(Point16 &pos) {
	pos.x = quantizedVolume(pos.x);
	// get x position units
	int32   unit    = (_extent.width * 100) / clamp(1, pos.x, _extent.width);

	// find the ratio and get the current slider value
	_slCurrent       = (_slValMax * 100) / clamp(1, unit, unit);

	// update the image position index
	_imagePosX           = clamp(_extent.x,
	                            pos.x,
	                            _extent.width - _imageRect.x);
}

} // end of namespace Saga2
