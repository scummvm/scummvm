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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_DOCUMENT_H
#define SAGA2_DOCUMENT_H

#include "saga2/modal.h"

namespace Saga2 {

// prototypes
void appendBookText(char *string = NULL);
void appendScrollText(char *string = NULL);

int16 openBook(uint16);
int16 openScroll(uint16);
int16 openParchment(uint16);


// constants
const uint32    kBookGroupID     = MKTAG('B', 'O', 'O', 'K');

const int kMaxVisiblePages = 2;

enum {
	kPageLeft = 0,
	kPageRight,
	kPageUp,
	kPageDown
};

enum pageOrientation {
	kPageOrientVertical = 0,
	kPageOrientHorizontal
};

struct CDocumentAppearance {
	StaticRect      windowPos;                  //  Position of window on screen
	int16           numPages;                   //  Number of visible pages
	int16           orientation;                //  Orientation of pages
	uint8           *textColors;                //  Text color array
	StaticRect      pageRect[kMaxVisiblePages];//  Array of visible page rects
	StaticRect      closeRect;                  //  Close-box rectangle
	StaticWindow    *decoList;                 //  List of decorator panels
	int16           numDecos;                   //  Number of decorator panels
	hResID          groupID;                    //  Resource ID for decoration group
	hResID          decoID;                     //  Resource ID for decorations
};

//  Base class for scrolls, books, and parchments

class CDocument : public ModalWindow {

	friend class CScroll;
	friend class CBook;

private:
	enum {
		kMaxPages            = 32,
		kMaxLines            = 32,
		kMaxChars            = 32,
		kTextPictureOffset   = 1
	};

	struct ImageHeader {
		Point16     size;
		int16       compress;
		int8        data[2];
	};

	CDocumentAppearance &_app;

	// image poiner array
	void            *_images[kMaxPages];

	uint16          _currentPage,
	                _lineWidth,
	                _pageHeight,
	                _totalLines,
	                _totalPages;

	gFont           *_textFont;
	uint16          _textHeight;
	uint16          _pages;
	uint16          _numLines[kMaxPages];
	uint16          _lineLen[kMaxPages][kMaxLines];
	uint16          _lineOffset[kMaxPages];
	Extent16        _imageSizes[kMaxPages];
	bool            _pageBreakSet;

	char            *_scan;                  // for parsing book text.

	// string sizes
	uint16  _maxSize;
	uint16  _textSize;

	// image context
	hResContext *_illustrationCon;

private:
	bool activate(gEventType why);       // activate the control
	void deactivate();

	void pointerMove(gPanelMessage &msg);
	bool pointerHit(gPanelMessage &msg);
	void pointerDrag(gPanelMessage &msg);
	void pointerRelease(gPanelMessage &msg);
	bool keyStroke(gPanelMessage &msg);

protected:
	void drawClipped(
	    gPort         &port,
	    const Point16 &offset,
	    const Rect16  &clipRect);

	void draw();                             // redraw the window

	void    renderText();
	void    makePages();
	bool    checkForPageBreak(char *string,
	                          uint16 index,
	                          int32 &offset);

	bool    checkForImage(char *string,
	                      uint16 index,
	                      uint16 pageIndex,
	                      int32 &offset);
	gPanel *keyTest(int16 key);
public:
	char            *origText;                  // the original text string
	char            *text;                      // the working text string

public:
	CDocument(CDocumentAppearance &app,
	          char            *buffer,        // buffer to edit
	          gFont           *font,          // font of the text
	          uint16          ident,          // control ID
	          AppFunc         *cmd = NULL);   // application command func
	~CDocument();

	void gotoPage(int8 page);

	APPFUNCV(cmdDocumentEsc);
	APPFUNCV(cmdDocumentLt);
	APPFUNCV(cmdDocumentRt);
	APPFUNCV(cmdDocumentUp);
	APPFUNCV(cmdDocumentDn);

//	void setText( char *text );
//	void appendText( char *string );
};

} // end of namespace Saga2

#endif
