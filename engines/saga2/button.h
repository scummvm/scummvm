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

#ifndef SAGA2_BUTTON_H
#define SAGA2_BUTTON_H

#include "saga2/panel.h"
#include "saga2/cmisc.h"
#include "saga2/floating.h"
#include "saga2/idtypes.h"

//#include "saga2/sprite.h"

namespace Saga2 {

struct Sprite;
class hResContext;

/* ======================================================================= *
    Misc. Structs and classes
 * ======================================================================= */

struct StaticTextPallete {
	uint8 dlPen;
	uint8 urPen;
	uint8 inPen;
	uint8 dlHilitePen;
	uint8 urHilitePen;
	uint8 inHilitePen;
};

struct textPallete {
	uint8   dlPen;
	uint8   urPen;
	uint8   inPen;
	uint8   dlHilitePen;
	uint8   urHilitePen;
	uint8   inHilitePen;

	// default constructor
	textPallete() {
		dlPen = urPen = inPen = dlHilitePen = urHilitePen = inHilitePen = 0;
	 }

	textPallete(uint8 dlP, uint8 urP, uint8 inP, uint8 dlHP, uint8 urHP, uint8 inHP) {
		set(dlP, urP, inP, dlHP, urHP, inHP);
	}

	textPallete(StaticTextPallete pal) {
		dlPen = pal.dlPen;
		urPen = pal.urPen;
		inPen = pal.inPen;
		dlHilitePen = pal.dlHilitePen;
		urHilitePen = pal.urHilitePen;
		inHilitePen = pal.inHilitePen;
	}

	void set(uint8 dlP, uint8 urP, uint8 inP, uint8 dlHP, uint8 urHP, uint8 inHP) {
		dlPen = dlP;
		urPen = urP;
		inPen = inP;
		dlHilitePen = dlHP;
		urHilitePen = urHP;
		inHilitePen = inHP;
	}
};

class GameObject;

/* ======================================================================= *
    Compressed image class
 * ======================================================================= */

class GfxCompImage : public gControl {
private:

	// number of image pointer allocated
	uint16      _numPtrAlloc;
	bool        _internalAlloc;

protected:
	void        **_compImages;       // double pointer(s) the compressed image(s)
	uint16      _currentImage;       // current image index
	uint16      _max, _min;           // min max of the index
	textPallete _textPal;            // contains info about coloring for multi-depth text rendering
	gFont       *_textFont;          // pointer to font for this button

protected:
	virtual void    *getCurrentCompImage();      // get the current image

	void init();

public:

	enum controlValue {
		enter = (1 << 0),
		leave = (1 << 1)
	};

	GfxCompImage(gPanelList &, const Rect16 &, void *, uint16, AppFunc *cmd = NULL);

	GfxCompImage(gPanelList &, const Rect16 &, void *, const char *,
	           textPallete &, uint16, AppFunc *cmd = NULL);

	GfxCompImage(gPanelList &, const Rect16 &, void **, int16, int16,
	           uint16, AppFunc *cmd = NULL);

	GfxCompImage(gPanelList &, const Rect16 &, void **, int16, int16,
	           const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	GfxCompImage(gPanelList &list,
	           const Rect16 &box,
	           uint32 contextID,
	           char a, char b, char c,
	           uint16 resNum,
	           uint16 numButtons,
	           uint16 ident,
	           AppFunc *cmd);

	GfxCompImage(gPanelList &, const StaticRect &, void **, int16, int16,
	           const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	~GfxCompImage();

	void    pointerMove(gPanelMessage &msg);
	void    enable(bool);
	void    invalidate(Rect16 *unused = nullptr);                    // invalidates the drawing
	int16   getCurrent()       {
		return _currentImage;
	}
	int16   getMin()           {
		return _min;
	}
	int16   getMax()           {
		return _max;
	}
	void    setCurrent(uint16 val) {
		_currentImage = clamp(_min, val, _max);
	}
	void    setExtent(const Rect16 &rect);
	void    select(uint16 val);
	void    select(uint16 val, const Rect16 &rect);
	void    setImages(void **images);
	void    setImage(void *image);

	void            draw();      // redraw the panel.
	virtual void    drawClipped(gPort &,
	                            const  Point16 &,
	                            const  Rect16 &);
};

class GfxSpriteImage : public GfxCompImage {
private:

	//  Color set to draw the object.
	ColorTable  _objColors;

protected:
	Sprite      *_sprPtr;

public:
	// this one takes a sprite pointer
	GfxSpriteImage(gPanelList &, const Rect16 &, GameObject *, char,
	             uint16, AppFunc *cmd = NULL);


	void drawClipped(gPort &,
	                 const   Point16 &,
	                 const   Rect16 &);
};

/* ======================================================================= *
    Compressed image button
 * ======================================================================= */

class GfxCompButton : public GfxCompImage {
protected:
	void        *_forImage;  // pointer to forground compress image data
	void        *_resImage;  // pointer to resessed compressed image data
	void        *_dimImage;  // pointer to dimmed commpressed image data
	Rect16      _extent;         // area that image covers
	bool        _dimmed;         // duh dim bit
	bool        _internalAlloc;  // set if memory allocated in class

public:

	GfxCompButton(gPanelList &, const Rect16 &, hResContext *, hResID res1, hResID res2,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResID contextID, hResID res1, hResID res2,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResContext *, char, char, char, int16, int16,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResID, char, char, char, int16, int16,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResContext *, uint32 resID, int8, int8,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResID, uint32, int8, int8,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, hResContext *, int16,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, void **, int16,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, void **, int16,
	            const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, void **, int16, void *, bool,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, void *,
	            uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const StaticRect &, void **, int16,
	            const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	GfxCompButton(gPanelList &, const Rect16 &, AppFunc *cmd = NULL);

	~GfxCompButton();


	bool            activate(gEventType why);        // activate the control
	void            deactivate();

	void    enable(bool);
	void    invalidate(Rect16 *unused = nullptr);                    // invalidates the drawing
	// area for this button
	void    draw();                          // redraw the panel.
	void    dim(bool);
	void    setForImage(void *image) {
		if (image) _forImage = image;
	}
	void    setResImage(void *image) {
		if (image) _resImage = image;
	}

private:
	void    loadImages(hResContext *con, hResID res1, hResID res2);
	void    loadImages(uint32 contextID, hResID res1, hResID res2);

protected:
	void            pointerMove(gPanelMessage &msg);
	bool            pointerHit(gPanelMessage &msg);
	void            pointerDrag(gPanelMessage &msg);
	void            pointerRelease(gPanelMessage &msg);
	virtual void    *getCurrentCompImage();
};

/************************************************************************
* GfxOwnerSelCompButton -- like a GfxCompButton but does not chage the      *
* selector bit                                                          *
************************************************************************/

class GfxOwnerSelCompButton : public GfxCompButton {
public:

	GfxOwnerSelCompButton(gPanelList &, const Rect16 &, void **, int16,
	                    uint16, AppFunc *cmd = NULL);

	//protected:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);
	void select(uint16 val);
};

/************************************************************************
* GfxMultCompButton -- like GfxCompButton but does any number of images     *
************************************************************************/

class GfxMultCompButton : public GfxCompButton {
private:
	bool    _response;   // tells whether to display an image when hit.

protected:
	void    **_images;
	Rect16  _extent;
	int16   _current;
	int16   _min;
	int16   _max;


public:
	GfxMultCompButton(gPanelList &, const Rect16 &, hResContext *, char, char, char, int16, int16, int16,
	                uint16, AppFunc *cmd = NULL);

	GfxMultCompButton(gPanelList &, const Rect16 &, void **, int16, int16,
	                uint16, AppFunc *cmd = NULL);

	GfxMultCompButton(gPanelList &, const Rect16 &, void **,
	                int16, int16, bool,
	                uint16, AppFunc *cmd = NULL);

	~GfxMultCompButton();

	int16   getCurrent()       {
		return _current;
	}
	int16   getMin()           {
		return _min;
	}
	int16   getMax()           {
		return _max;
	}
	void    setCurrent(int16 val)  {
		_current = clamp(_min, val, _max);
	}
	void    setImages(void **newImages) {
		if (_images && newImages)
			_images = newImages;
	}
	void setResponse(bool resp) {
		_response = resp;
	}

protected:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);
	virtual void    *getCurrentCompImage();
};


/* ===================================================================== *
   GfxSlider class
 * ===================================================================== */

class GfxSlider : public GfxMultCompButton {
protected:
	Rect16  _imageRect;
	int16   _slValMin;
	int16   _slValMax;
	int16   _slCurrent;
	int16   _imagePosX;

public:
	GfxSlider(gPanelList &, const Rect16 &, const Rect16 &, int16, int16,
	        void **, int16, int16,
	        uint16, AppFunc *cmd = NULL);

private:
	bool    activate(gEventType why);
	void    deactivate();
	bool    pointerHit(gPanelMessage &msg);
	void    pointerMove(gPanelMessage &msg);
	void    pointerRelease(gPanelMessage &);
	void    pointerDrag(gPanelMessage &msg);
	void    updateSliderIndexes(Point16 &pos);
	void    drawClipped(gPort &port,
	                    const Point16 &offset,
	                    const Rect16 &r);

public:
	void    setSliderCurrent(int16 val) {
		_slCurrent = val;
	}
	int16   getSliderCurrent() {
		return _slCurrent;
	}
	int16   getSliderLenVal();
	virtual void    *getCurrentCompImage();
	void    draw();
};


/* ===================================================================== *
   Misc functions
 * ===================================================================== */

void drawCompressedImageToMap(gPixelMap &map, void *image);

void writePlaqText(gPort            &port,
                   const Rect16    &r,
                   gFont           *font,
                   int16           textPos,
                   textPallete     &pal,
                   bool            hiLite,
                   const char      *msg, ...);


void writePlaqTextPos(gPort         &port,
                      const Point16    &pos,
                      gFont            *font,
                      int16            textPos,
                      textPallete      &pal,
                      bool             hiLite,
                      const char       *msg, ...);

} // end of namespace Saga2

#endif
