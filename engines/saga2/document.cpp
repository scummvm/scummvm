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

#include "common/keyboard.h"

#include "saga2/saga2.h"
#include "saga2/document.h"
#include "saga2/script.h"
#include "saga2/intrface.h"
#include "saga2/grequest.h"
#include "saga2/mouseimg.h"
#include "saga2/version.h"
#include "saga2/fontlib.h"

namespace Saga2 {

/* ===================================================================== *
    About box
 * ===================================================================== */

#define PROGRAM_ABOUT   " v" VERSION_OPSYS " " VERSION_STAMP


// externs
extern BackWindow           *mainWindow;

// declarations
APPFUNC(cmdDocumentQuit);


/* ===================================================================== *
   scroll metrics
 * ===================================================================== */

// buttons
//Rect16    closeScrollBtnRect( 184, 206, 44, 42 );

// options dialog window decorations
static StaticWindow scrollDecorations[] = {
	{{202,  54, 232, 100}, nullptr, 0},
	{{212, 154, 212, 100}, nullptr, 1},
	{{202, 254, 236, 117}, nullptr, 2}
};

static uint8 scrollTextColors[] = { 65, 65, 65, 65, 65, 65, 65, 66, 66, 67, 67, 67, 67, 66, 66, 66 };

CDocumentAppearance scrollAppearance = {
	{202, 54, 236, 317},
	1,
	kPageOrientVertical,
	scrollTextColors,
	{ {50, 64, 131, 169}, {0, 0, 0, 0} },
	{184, 206,  44,  42},
	scrollDecorations,
	ARRAYSIZE(scrollDecorations),
	MKTAG('S', 'C', 'R', 'L'),
	MKTAG('S', 'R', 'L', 0)
};

/* ===================================================================== *
   Book metrics
 * ===================================================================== */

// options dialog window decorations
static StaticWindow bookDecorations[] = {
	{{123,  76, 394, 88}, nullptr, 0},
	{{123, 164, 394, 80}, nullptr, 1},
	{{123, 244, 394, 77}, nullptr, 2}
};

static uint8 bookTextColors[] = { 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65 };

CDocumentAppearance bookAppearance = {
	{123, 76, 394, 252},
	2,
	kPageOrientHorizontal,
	bookTextColors,
	{ {40, 26, 135, 205}, {218, 26, 135, 205} },
	{231, 217,  34,  27},
	bookDecorations,
	ARRAYSIZE(bookDecorations),
	MKTAG('B', 'O', 'O', 'K'),
	MKTAG('B', 'K', 'D', 0)
};

/* ===================================================================== *
   Parchment metrics
 * ===================================================================== */

// options dialog window decorations
static StaticWindow parchDecorations[] = {
	{{202,  54, 208, 256}, nullptr, 0}
};

CDocumentAppearance parchAppearance = {
	{202, 54, 208, 256},
	1,
	kPageOrientVertical,
	bookTextColors,
	{ {27, 18, 149, 212}, {0, 0, 0, 0} },
	{164, 229,  20,  20},
	parchDecorations,
	ARRAYSIZE(parchDecorations),
	MKTAG('P', 'A', 'R', 'C'),
	MKTAG('P', 'C', 'H', 0)
};

// deliminator defines
char deliminator    = '@';
char dPageBreak[3]  = { "pb" };
char dImage[3]      = { "im" };

/* ===================================================================== *
   Document base class
 * ===================================================================== */

CDocument::CDocument(CDocumentAppearance &dApp,
                     char            *buffer,        // buffer to edit
                     gFont           *font,          // font of the text
                     uint16          ident,          // control ID
                     AppFunc         *cmd)           // application command func
	: _app(dApp), ModalWindow(dApp.windowPos, ident, cmd) {
	// resource handle
	hResContext     *decRes;

	// init the resource context handle
	decRes = resFile->newContext(_app.groupID, "docimage context");

	// init con pointer to NULL
	_illustrationCon = nullptr;

	// set the maxium string length
	_maxSize = kMaxPages * kMaxLines * kMaxChars;

	// get the org text size
	_textSize = clamp(0, strlen(buffer), _maxSize);

	// set the original text pointer
	origText = new char[_textSize + 1];

	// and fill it
	Common::strlcpy(origText, buffer, _textSize + 1);

	// make a working buffer
	text = new char[_textSize + 1];

	// and fill it
	Common::strlcpy(text, origText, _textSize + 1);

	_textFont        = font;
	_textHeight      = (_textFont ? _textFont->height : 0);
	_lineWidth       = dApp.pageRect[0].width;
	_pageHeight      = dApp.pageRect[0].height;
	_currentPage     = 0;
	_totalLines      = 0;
	_totalPages      = 0;
	_pageBreakSet    = true;

	// null out the image pointer array
	for (int16 i = 0; i < kMaxPages; i++) {
		_images[i] = nullptr;
	}

	makePages();

	// attach the graphics for the book
	setDecorations(_app.decoList,
	               _app.numDecos,
	               decRes, _app.decoID);

	// remove the resource handle
	if (decRes) resFile->disposeContext(decRes);
	decRes = nullptr;


}

CDocument::~CDocument() {
	int16   i;

	for (i = 0; i < kMaxPages; i++) {
		if (_images[i]) {
			free(_images[i]);
		}
	}

	// get rid of the working text buffer
	if (text) {
		delete[] text;
		text = nullptr;
	}

	if (origText) {
		delete[] origText;
		origText = nullptr;
	}

	// get rid of the resource context
	if (_illustrationCon)
		resFile->disposeContext(_illustrationCon);
}

void CDocument::deactivate() {
	_selected = 0;
	gPanel::deactivate();
}

bool CDocument::activate(gEventType why) {
	if (why == kEventMouseDown) {           // momentarily depress
		_selected = 1;
		notify(why, 0);                      // notify App of successful hit
		return true;
	}
	return false;
}

bool CDocument::keyStroke(gPanelMessage &msg) {
	gEvent ev;
	switch (msg._key) {
	case Common::ASCII_ESCAPE:
		cmdDocumentEsc(ev);
		return true;
	case Common::KEYCODE_LEFT:
		cmdDocumentLt(ev);
		return true;
	case Common::KEYCODE_RIGHT:
		cmdDocumentRt(ev);
		return true;
	case Common::KEYCODE_UP:
		cmdDocumentUp(ev);
		return true;
	case Common::KEYCODE_DOWN:
		cmdDocumentDn(ev);
		return true;
	default:
		return false;
	}
}



gPanel *CDocument::keyTest(int16 key) {
	switch (key) {
	case Common::ASCII_ESCAPE:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_DOWN:
		return this;
	default:
		return nullptr;
	}
}

//  Cursor images for turning book pages
void CDocument::pointerMove(gPanelMessage &msg) {
	Point16 pos     = msg._pickPos;

	if (msg._inPanel && Rect16(0, 0, _extent.width, _extent.height).ptInside(pos)) {
		if (_app.orientation == kPageOrientVertical) {
			// find out which end of the book we're on
			if (pos.y < _extent.height / 2)   setMouseImage(kMousePgUpImage,   -7, -7);
			else                            setMouseImage(kMousePgDownImage, -7, -7);
		} else {
			// find out which side of the book we're on
			if (pos.x < _extent.width / 2)    setMouseImage(kMousePgLeftImage,  -7, -7);
			else                            setMouseImage(kMousePgRightImage, -7, -7);
		}
	} else if (msg._pointerLeave) {
		setMouseImage(kMouseArrowImage, 0, 0);
	}

	notify(kEventMouseMove, 0);
}

void CDocument::pointerDrag(gPanelMessage &) {
	if (_selected) {
		notify(kEventMouseDrag, 0);
	}
}

bool CDocument::pointerHit(gPanelMessage &msg) {
	Point16 pos     = msg._pickPos;

	if (msg._inPanel && Rect16(0, 0, _extent.width, _extent.height).ptInside(pos)) {
		gEvent ev;
		if (_app.orientation == kPageOrientVertical) {
			// find out which end of the book we're on
			if (pos.y < _extent.height / 2)   cmdDocumentUp(ev); //gotoPage( _currentPage - _app.numPages );
			else                            cmdDocumentDn(ev); //gotoPage( _currentPage + _app.numPages );
		} else {
			// find out which side of the book we're on
			if (pos.x < _extent.width / 2)    cmdDocumentLt(ev); //gotoPage( _currentPage - _app.numPages );
			else                            cmdDocumentRt(ev); //gotoPage( _currentPage + _app.numPages );
		}
	} else {
		// mouse hit outside book area, close book
		gWindow         *win;
		requestInfo     *ri;

		win = getWindow();      // get the window pointer
		ri = win ? (requestInfo *)win->_userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result  = _id;

			setMouseImage(kMouseArrowImage, 0, 0);
		}
	}

	activate(kEventMouseDown);
	return true;
}

void CDocument::gotoPage(int8 page) {
	page = clamp(0, page, kMaxPages);

	while (page % _app.numPages) page++;

	if (page != _currentPage && page < _pages) {
		_currentPage = page;
		renderText();
	}
}

void CDocument::pointerRelease(gPanelMessage &) {
	if (_selected) notify(kEventMouseUp, 0);   // notify App of successful hit
	deactivate();
}

bool CDocument::checkForPageBreak(char *string, uint16 index, int32 &offset) {

	// get the current index into the string
	char *strIndex = string + index;

	// page break detected
	if (strIndex[1] == dPageBreak[0] &&
	        strIndex[2] == dPageBreak[1]) {
		// eat the page breaks chars
		// tie off the end
		strIndex[0] = 0;

		size_t txtlen = strlen(&strIndex[3]);

		// string them together
		memmove(&strIndex[0], &strIndex[3], txtlen);

		// take the offset to the end of this line
		offset = index;

		// and set the new page flag
		return true;
	}

	return false;
}

bool CDocument::checkForImage(char      *string,
                              uint16   index,
                              uint16   pageIndex,
                              int32    &offset) {
	// get the current index into the string
	char    *strIndex       = string + index;
	uint16  offPageIndex    = pageIndex;


	// if there was not just a page break
	if (!_pageBreakSet) {
		// then the images are going to end up on the next page
		offPageIndex++;
	}


	// image detected marker
	if (strIndex[1] == dImage[0] &&
	        strIndex[2] == dImage[1]) {
		int16   numEat = 0;         // number of characters to eat
		char    *argv = &strIndex[2 + 1];  // array to first element

		// delete context
		if (_illustrationCon) resFile->disposeContext(_illustrationCon);

		// resource handle
		_illustrationCon = resFile->newContext(MKTAG(argv[0], argv[1], argv[2], argv[3]),
		                                      "book internal resources");
		// set image for next page
		if (offPageIndex < kMaxPages) {
			// if the last entry is defined as a number
			if (argv[7] == ':') {
				// convert the text into a number
				char    numSt[2]  = { argv[8], 0 };
				uint8   num         = atoi(numSt);


				if (!_images[offPageIndex]) {
					// get the image
					_images[offPageIndex] = LoadResource(_illustrationCon,
					                                      MKTAG(argv[4], argv[5], argv[6], num),
					                                      "book internal image");
				}

				// number of chars to eat
				numEat = 9;
			} else {
				_images[offPageIndex] = LoadResource(_illustrationCon,
				                                      MKTAG(argv[4], argv[5], argv[6], argv[7]),
				                                      "book internal image");
				numEat = 8;
			}

			// get the size of the image
			_imageSizes[offPageIndex] = ((ImageHeader *)_images[offPageIndex])->size;

			// tie off the end
			strIndex[0] = 0;

			// and string them together
			// strIndex is inside text buffer
			Common::strcat_s(&strIndex[0], _textSize + 1 - (&strIndex[0] - text), &strIndex[2 + 1 + numEat]);

			// set new line length
			offset = index;

			// set the line offset
			_lineOffset[offPageIndex] =
				_imageSizes[offPageIndex].y / (_textHeight + 1) +
				kTextPictureOffset;
		} else {
			warning("CDocument: Document overflow");
		}

		// set the new page flag
		return true;
	}

	return false;
}


void CDocument::makePages() {
	// copy the original text back to the working buffer
	Common::strlcpy(text, origText, _textSize + 1);


	char    *str            = text;
	int32   offset          = 0;
	uint16  lineIndex       = 0;
	uint16  pageIndex       = 0;
	uint16  linesPerPage    = _pageHeight / (_textHeight + 1);
	uint16  dummy;
	uint16  i;
	bool    newPage         = false;


	while (offset >= 0 && pageIndex < kMaxPages) {
		while (offset >= 0 &&
		        lineIndex < linesPerPage &&
		        !newPage) {
			offset = GTextWrap(_textFont, str, dummy, _lineWidth, 0);

			// check for page breaks and images
			for (i = 0; i <= offset; i++) {
				// we hit a diliminator
				if (str[i] == deliminator) {
					// page break check
					if (checkForPageBreak(str, i, offset)) {
						// if a break did not just occur
						if (!_pageBreakSet) {
							newPage         = true;
							_pageBreakSet    = true;
						} else {
							// eat the newPage and
							// reset the flag for a just set break
							_pageBreakSet = false;
						}
					}

					// image check
					if (checkForImage(str, i, pageIndex, offset)) {
						// if a break did not just occur
						if (!_pageBreakSet) {
							newPage         = true;
							_pageBreakSet    = true;
						} else {
							// eat the newPage and
							// reset the flag for a just set break
							_pageBreakSet = false;
						}

						lineIndex   = _lineOffset[pageIndex];
					}
				}

				// we got token that was not a page break so reset the flag
				_pageBreakSet = false;
			}

			// set the length of this line
			if (offset >= 0) {
				// number of characters on this line
				_lineLen[pageIndex][lineIndex] = offset;
			} else {
				// remaining number of characters in string
				_lineLen[pageIndex][lineIndex] = strlen(str);
			}


			// increment the str pointer and line index
			str += offset;
			lineIndex++;
		}

		_numLines[pageIndex] = lineIndex;
		pageIndex++;
		newPage     = false;

		lineIndex = 0;
	}

	_pages = pageIndex;
}

// This function will draw the text onto the book.
void CDocument::renderText() {
	gPort           tPort;
	gPort           &port = _window._windowPort;
	uint16          pageIndex;
	uint16          lineIndex;
	uint16          linesPerPage = _pageHeight / (_textHeight + 1);
	char            *str = text;

	assert(_textFont);

	Rect16  bltRect(0, 0, _extent.width, _extent.height);

	if (NewTempPort(tPort, bltRect.width, bltRect.height)) {
		// clear out the text buffer
		int16           i, k;
		uint8           *buffer = (uint8 *)tPort._map->_data;

		for (i = 0; i < tPort._map->_size.x; i++) {
			for (k = 0; k < tPort._map->_size.y; k++) {
				*buffer++ = 0;
			}
		}

		// draw a new copy of the background to the temp port
		drawClipped(tPort,
		            Point16(_extent.x, _extent.y),
		            Rect16(0, 0, _extent.width, _extent.height));

		tPort.setFont(_textFont);         // setup the string pointer
		for (pageIndex = 0; pageIndex < _currentPage; pageIndex++) {
			if (_images[pageIndex]) {
				lineIndex = _lineOffset[pageIndex];

				assert(lineIndex < linesPerPage);
			} else {
				lineIndex = 0;
			}

			for (; lineIndex < _numLines[pageIndex]; lineIndex++) {
				int16   temp = _lineLen[pageIndex][lineIndex];

				assert(pageIndex < kMaxPages);
				assert(temp < 35);

				str += _lineLen[pageIndex][lineIndex];
			}
		}

		// draw the text onto the pages of the book
		for (pageIndex = _currentPage;
		        pageIndex - _currentPage < _app.numPages && pageIndex < _pages;
		        pageIndex++) {
			StaticRect *pageRect = &_app.pageRect[pageIndex % _app.numPages];

			// if there is an image on this page
			if (_images[pageIndex]) {
				Point16 pos;

				pos.x = pageRect->x + (pageRect->width - _imageSizes[pageIndex].x) / 2;
				pos.y = pageRect->y;

				drawCompressedImage(tPort, pos, _images[pageIndex]);

				lineIndex = _lineOffset[pageIndex];
			} else {
				lineIndex = 0;
			}

			for (; lineIndex < _numLines[pageIndex]; lineIndex++) {
				assert(pageIndex <= kMaxPages);

				tPort.moveTo(pageRect->x, pageRect->y + (_textHeight * lineIndex) + 1);
				tPort.setColor(_app.textColors[lineIndex]);
				tPort.drawText(str, _lineLen[pageIndex][lineIndex]);

				// grab the next text offset
				int16 temp = _lineLen[pageIndex][lineIndex];

				assert(temp < 35);

				str += _lineLen[pageIndex][lineIndex];
			}
		}

		port.setMode(kDrawModeMatte);

		g_vm->_pointer->hide();

		port.bltPixels(*tPort._map, 0, 0,
		               bltRect.x, bltRect.y,
		               bltRect.width, bltRect.height);

		g_vm->_pointer->show();

		DisposeTempPort(tPort);              // dispose of temporary pixelmap
	}
}

void CDocument::drawClipped(
    gPort         &port,
    const Point16 &offset,
    const Rect16  &clipRect) {
	g_vm->_pointer->hide();
	ModalWindow::drawClipped(port, offset, clipRect);
	g_vm->_pointer->show();
}

void CDocument::draw() {         // redraw the window
	// draw the book image
	drawClipped(g_vm->_mainPort, Point16(0, 0), _extent);

	// draw the text onto the book
	renderText();
}

/////////
// Notes

/*
        // page breaks like:
    @pb

        // images declared like:
    @imDIALBTN:8
    @imCONTHED:0

*/

/* ===================================================================== *
   Text buffer
 * ===================================================================== */

const int       _textSize = 4096;
char            bookText[_textSize] = { "" };

void appendBookText(char *string) {
	if (string) {
		Common::strlcat(bookText, string, _textSize - 1);
		bookText[_textSize - 1] = 0;
	}
}

/* ===================================================================== *
   scroll window
 * ===================================================================== */

void buildText(uint16 textScript) {
	if (textScript > 0) {
		// clear out the scroll text
		Common::strlcpy(bookText, "", sizeof(bookText));

		if (textScript == resImports->reserved[0]) {
			Common::strlcpy(bookText, PROGRAM_ABOUT, sizeof(bookText));
		}

		// generate the text for the book
		scriptCallFrame scf;

		scf.invokedObject   = Nothing;
		scf.enactor         = Nothing;
		scf.directObject    = Nothing;
		scf.indirectObject  = Nothing;
		scf.value           = 0;

		//  Run the script
		runScript(textScript, scf);
	} else {
		Common::sprintf_s(bookText, "Invalid textScript: %d", textScript);
	}
}

int16 openScroll(uint16 textScript) {
	buildText(textScript);

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;


	// point to book
	CDocument       *win = nullptr;

	// close button
	GfxCompButton     *closeScroll;
	void            **closeBtnImage;
	uint16          buttonResID     = 0;
	hResContext     *decRes;

	// init the resource context handle
	decRes = resFile->newContext(MKTAG('S', 'C', 'R', 'L'), "book resources");

	// get the graphics associated with the buttons
	closeBtnImage = loadButtonRes(decRes, buttonResID, kNumBtnImages);

	// create the window
	win = new CDocument(scrollAppearance, bookText, &Script10Font, 0, nullptr);

	// make the quit button
	closeScroll = new GfxCompButton(*win, scrollAppearance.closeRect, closeBtnImage, kNumBtnImages, 0, cmdDocumentQuit);

	closeScroll->_accelKey = 0x1B;

	// attach the structure to the book, open the book
	win->_userData = &rInfo;
	win->open();

	// do stuff
	EventLoop(rInfo.running, true);

	// remove the window all attatched controls
	delete  win;

	// unload all image arrays
	unloadImageRes(closeBtnImage, kNumBtnImages);

	// remove the resource handle
	if (decRes)
		resFile->disposeContext(decRes);

	// return the result code
	return rInfo.result;
}

/* ===================================================================== *
   Book window
 * ===================================================================== */

int16 openBook(uint16 textScript) {

	buildText(textScript);

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;


	// point to book
	CDocument       *win = nullptr;

	GfxCompButton *closeBook;
	hResContext *decRes;

	decRes = resFile->newContext(MKTAG('S', 'C', 'R', 'L'), "book resources");

	// create the window
	win = new CDocument(bookAppearance, bookText, &Script10Font, 0, nullptr);

	// make the quit button
	closeBook = new GfxCompButton(*win, bookAppearance.closeRect, cmdDocumentQuit);
	closeBook->_accelKey = 0x1B;

	// attach the structure to the book, open the book
	win->_userData = &rInfo;
	win->open();

	// do stuff
	EventLoop(rInfo.running, true);

	// remove the window all attatched controls
	delete  win;

	if (decRes)
		resFile->disposeContext(decRes);

	// return the result code
	return rInfo.result;
}

/* ===================================================================== *
   Parchment window
 * ===================================================================== */

int16 openParchment(uint16 textScript) {
	buildText(textScript);

	// requester info struct
	requestInfo     rInfo;

	rInfo.result    = -1;
	rInfo.running   = true;


	// point to book
	CDocument       *win = nullptr;

	GfxCompButton *closeParchment;
	hResContext *decRes;

	decRes = resFile->newContext(MKTAG('S', 'C', 'R', 'L'), "book resources");

	// create the window
	win = new CDocument(parchAppearance, bookText, &Script10Font, 0, nullptr);
	// make the quit button
	closeParchment = new GfxCompButton(*win, parchAppearance.closeRect, cmdDocumentQuit);
	closeParchment->_accelKey = 0x1B;

	// attach the structure to the book, open the book
	win->_userData = &rInfo;
	win->open();

	// do stuff
	EventLoop(rInfo.running, true);

	// remove the window all attatched controls
	delete  win;

	if (decRes)
		resFile->disposeContext(decRes);

	// return the result code
	return rInfo.result;
}

APPFUNC(cmdDocumentQuit) {
	gWindow         *win;
	requestInfo     *ri;

	if (ev.panel && ev.eventType == kEventNewValue && ev.value) {
		win = ev.panel->getWindow();        // get the window pointer
		ri = win ? (requestInfo *)win->_userData : nullptr;

		if (ri) {
			ri->running = 0;
			ri->result = ev.panel->_id;
		}
	}
}

APPFUNCV(CDocument::cmdDocumentEsc) {
	requestInfo     *ri = (requestInfo *)_userData;
	if (ri) {
		ri->running = 0;
		ri->result = 0;
	}
}

APPFUNCV(CDocument::cmdDocumentLt) {
	gotoPage(_currentPage - _app.numPages);    //draw();
}

APPFUNCV(CDocument::cmdDocumentRt) {
	gotoPage(_currentPage + _app.numPages);   //draw();
}

APPFUNCV(CDocument::cmdDocumentUp) {
	gotoPage(_currentPage - _app.numPages);   //draw();
}

APPFUNCV(CDocument::cmdDocumentDn) {
	gotoPage(_currentPage + _app.numPages);   //draw();
}

} // end of namespace Saga2
