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

#ifndef SAGA2_BUTTON_H
#define SAGA2_BUTTON_H

#include "saga2/panel.h"
#include "saga2/cmisc.h"
#include "saga2/idtypes.h"

//#include "saga2/sprite.h"

namespace Saga2 {

struct Sprite;
class hResContext;

/* ======================================================================= *
    Misc. Structs and classes
 * ======================================================================= */


struct textPallete {
	uint8   dlPen;
	uint8   urPen;
	uint8   inPen;
	uint8   dlHilitePen;
	uint8   urHilitePen;
	uint8   inHilitePen;

	// default constructor
	textPallete(void) { ; }      // do nothing

	textPallete(uint8 dlP, uint8 urP, uint8 inP, uint8 dlHP, uint8 urHP, uint8 inHP) {
		set(dlP, urP, inP, dlHP, urHP, inHP);
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

class gCompImage : public gControl {
private:

	enum {
		textSize    = 128
	};

	// number of image pointer allocated
	uint16      numPtrAlloc;
	bool        internalAlloc;

protected:
	void        **compImages;       // double pointer(s) the compressed image(s)
	uint16      currentImage;       // current image index
	uint16      max, min;           // min max of the index
	textPallete textPal;            // contains info about coloring for multi-depth text rendering
	gFont       *textFont;          // pointer to font for this button
//	char     imageText[textSize];  // text to render on button

protected:
	virtual void    *getCurrentCompImage(void);      // get the current image

	void init(void);

public:

	enum controlValue {
		enter = (1 << 0),
		leave = (1 << 1)
	};

	gCompImage(gPanelList &, const Rect16 &, void *, uint16, AppFunc *cmd = NULL);

	gCompImage(gPanelList &, const Rect16 &, void *, const char *,
	           textPallete &, uint16, AppFunc *cmd = NULL);

	gCompImage(gPanelList &, const Rect16 &, void **, int16, int16,
	           uint16, AppFunc *cmd = NULL);

	gCompImage(gPanelList &, const Rect16 &, void **, int16, int16,
	           const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	gCompImage(gPanelList &list,
	           const Rect16 &box,
	           uint32 contextID,
	           char a, char b, char c,
	           uint16 resNum,
	           uint16 numButtons,
	           uint16 ident,
	           AppFunc *cmd);

	gCompImage(gPanelList &, const StaticRect &, void **, int16, int16,
	           const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	~gCompImage(void);

	void    pointerMove(gPanelMessage &msg);
	void    enable(bool);
	void    invalidate(Rect16 *unused = nullptr);                    // invalidates the drawing
	int16   getCurrent(void)       {
		return currentImage;
	}
	int16   getMin(void)           {
		return min;
	}
	int16   getMax(void)           {
		return max;
	}
	void    setCurrent(uint16 val) {
		currentImage = clamp(min, val, max);
	}
	void    setExtent(const Rect16 &rect);
	void    select(uint16 val);
	void    select(uint16 val, const Rect16 &rect);
	void    setImages(void **images);
	void    setImage(void *image);

	void            draw(void);      // redraw the panel.
	virtual void    drawClipped(gPort &,
	                            const  Point16 &,
	                            const  Rect16 &);
};

class gSpriteImage : public gCompImage {
private:

	//  Color set to draw the object.
	ColorTable  objColors;

protected:
	Sprite      *sprPtr;

public:
	// this one takes a sprite pointer
	gSpriteImage(gPanelList &, const Rect16 &, GameObject *, char,
	             uint16, AppFunc *cmd = NULL);


	void drawClipped(gPort &,
	                 const   Point16 &,
	                 const   Rect16 &);
};

/* ======================================================================= *
    Compressed image button
 * ======================================================================= */

class gCompButton : public gCompImage {
protected:
	void        *forImage;  // pointer to forground compress image data
	void        *resImage;  // pointer to resessed compressed image data
	void        *dimImage;  // pointer to dimmed commpressed image data
	Rect16      extent;         // area that image covers
	bool        dimmed;         // duh dim bit
	bool        internalAlloc;  // set if memory allocated in class

public:

	gCompButton(gPanelList &, const Rect16 &, hResContext *, hResID res1, hResID res2,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResID contextID, hResID res1, hResID res2,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResContext *, char, char, char, int16, int16,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResID, char, char, char, int16, int16,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResContext *, uint32 resID, int8, int8,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResID, uint32, int8, int8,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, hResContext *, int16,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, void **, int16,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, void **, int16,
	            const char *, textPallete &, uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, void **, int16, void *, bool,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const Rect16 &, void *,
	            uint16, AppFunc *cmd = NULL);

	gCompButton(gPanelList &, const StaticRect &, void **, int16,
	            const char *, textPallete &, uint16, AppFunc *cmd = NULL);


	~gCompButton(void);


	bool            activate(gEventType why);        // activate the control
	void            deactivate(void);

	void    enable(bool);
	void    invalidate(Rect16 *unused = nullptr);                    // invalidates the drawing
	// area for this button
	void    draw(void);                          // redraw the panel.
	void    dim(bool);
	void    setForImage(void *image) {
		if (image) forImage = image;
	}
	void    setResImage(void *image) {
		if (image) resImage = image;
	}

private:
	void    loadImages(hResContext *con, hResID res1, hResID res2);
	void    loadImages(uint32 contextID, hResID res1, hResID res2);

protected:
	void            pointerMove(gPanelMessage &msg);
	bool            pointerHit(gPanelMessage &msg);
	void            pointerDrag(gPanelMessage &msg);
	void            pointerRelease(gPanelMessage &msg);
	virtual void    *getCurrentCompImage(void);
};


/************************************************************************
* gToggleCompButton -- like a gCompButton but toggle on and off.        *
************************************************************************/

class gToggleCompButton : public gCompButton {
public:
	gToggleCompButton(gPanelList &, const Rect16 &, hResContext *, char, char, char, int16, int16,
	                  uint16, AppFunc *cmd = NULL);

	gToggleCompButton(gPanelList &, const Rect16 &, hResContext *, int16,
	                  uint16, AppFunc *cmd = NULL);

	gToggleCompButton(gPanelList &, const Rect16 &, void **, int16,
	                  uint16, AppFunc *cmd = NULL);

	gToggleCompButton(gPanelList &, const Rect16 &, void **, int16,
	                  char *, textPallete &, uint16, AppFunc *cmd = NULL);

protected:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);

public:
	void select(uint16 val);

};

/************************************************************************
* gOwnerSelCompButton -- like a gCompButton but does not chage the      *
* selector bit                                                          *
************************************************************************/

class gOwnerSelCompButton : public gCompButton {
public:

	gOwnerSelCompButton(gPanelList &, const Rect16 &, void **, int16,
	                    uint16, AppFunc *cmd = NULL);

	//protected:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);
	void select(uint16 val);
};

/************************************************************************
* gMultCompButton -- like gCompButton but does any number of images     *
************************************************************************/

class gMultCompButton : public gCompButton {
private:
	bool    response;   // tells whether to display an image when hit.

protected:
	void    **images;
	Rect16  extent;
	int16   current;
	int16   min;
	int16   max;


public:
	gMultCompButton(gPanelList &, const Rect16 &, hResContext *, char, char, char, int16, int16, int16,
	                uint16, AppFunc *cmd = NULL);

	gMultCompButton(gPanelList &, const Rect16 &, void **, int16, int16,
	                uint16, AppFunc *cmd = NULL);

	gMultCompButton(gPanelList &, const Rect16 &, void **,
	                int16, int16, bool,
	                uint16, AppFunc *cmd = NULL);

	~gMultCompButton(void);

	int16   getCurrent(void)       {
		return current;
	}
	int16   getMin(void)           {
		return min;
	}
	int16   getMax(void)           {
		return max;
	}
	void    setCurrent(int16 val)  {
		current = clamp(min, val, max);
	}
	void    setImages(void **newImages) {
		if (images && newImages) images = newImages;
	}
	void setResponse(bool resp) {
		response = resp;
	}

protected:
	bool activate(gEventType why);       // activate the control
	bool pointerHit(gPanelMessage &msg);
	virtual void    *getCurrentCompImage(void);
};


/* ===================================================================== *
   gSlider class
 * ===================================================================== */

class gSlider : public gMultCompButton {
protected:
	Rect16  imageRect;
	int16   slValMin;
	int16   slValMax;
	int16   slCurrent;
	int16   imagePosX;

public:
	gSlider(gPanelList &, const Rect16 &, const Rect16 &, int16, int16,
	        void **, int16, int16,
	        uint16, AppFunc *cmd = NULL);

private:
	bool    activate(gEventType why);
	void    deactivate(void);
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
		slCurrent = val;
	}
	int16   getSliderCurrent(void) {
		return slCurrent;
	}
	int16   getSliderLenVal(void);
	virtual void    *getCurrentCompImage(void);
	void    draw(void);
};


/* ===================================================================== *
   Misc functions
 * ===================================================================== */
// duplicate declaration in floating.h

void drawCompressedImage(gPort &port, const Point16 pos, void *image);
void drawCompressedImageGhosted(gPort &port, const Point16 pos, void *image);
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
