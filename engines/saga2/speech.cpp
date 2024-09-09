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

#include "saga2/saga2.h"
#include "saga2/fta.h"
#include "saga2/fontlib.h"
#include "saga2/speech.h"
#include "saga2/motion.h"
#include "saga2/panel.h"
#include "saga2/grabinfo.h"
#include "saga2/player.h"
#include "saga2/annoy.h"
#include "saga2/cmisc.h"
#include "saga2/tile.h"
#include "saga2/tilemode.h"

namespace Saga2 {

struct TextSpan {
	char            *text;                  // pointer to 1st char of span
	int16           charWidth,              // number of characters in span
	                pixelWidth;             // number of pixels in span
};

//-----------------------------------------------------------------------
//	externs

extern  StaticPoint16 fineScroll;
int                 kludgeHeight = 15;
extern  StaticTilePoint viewCenter;         // coordinates of view on map

//-----------------------------------------------------------------------
//	constants

const int           maxWidth = 420;
const int           defaultWidth = 380;
const int           actorHeight = 80;       // Assume 80

const int           lineLeading = 2;        // space between lines
const int           outlineWidth = 2;       // width of character outline
const int           bulletWidth = 13;       // width of bullet symbol

//-----------------------------------------------------------------------
//	prototypes

int16 buttonWrap(
    TextSpan        *lineList,              // indicates where line breaks are
    TextSpan        *buttonList,            // indicates where button breaks are
    int16           &buttonCount,           // returns number of buttons
    char            *text,                  // text to wrap
    int16           width,                  // width of text
    int16           supressText,
    gPort           &textPort);

//-----------------------------------------------------------------------
//	locals

//  Temporary: Alarm which determines when speech finishes
Alarm               speechFinished;


//  The list of active and non-active speech tasks for all actors
uint8 speechListBuffer[sizeof(SpeechTaskList)];
SpeechTaskList  &speechList = *((SpeechTaskList *)speechListBuffer);

static TextSpan     speechLineList[64],   // list of speech lines
       speechButtonList[64]; // list of speech buttons
int16               speechLineCount,        // count of speech lines
                    speechButtonCount;      // count of speech buttons

static StaticPoint16 initialSpeechPosition = {0, 0};  // initial coords of speech

//  Image data for the little "bullet"
static uint8 BulletData[] = {
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, // Row 0
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, // Row 1
	0x00, 0x18, 0x18, 0x4C, 0x4A, 0x4A, 0x18, 0x18, 0x00, // Row 2
	0x18, 0x18, 0x4E, 0x4C, 0x4A, 0x0A, 0x4A, 0x18, 0x18, // Row 3
	0x18, 0x18, 0x50, 0x4E, 0x4C, 0x4A, 0x4A, 0x18, 0x18, // Row 4
	0x18, 0x18, 0x4E, 0x50, 0x50, 0x4E, 0x4E, 0x18, 0x18, // Row 5
	0x00, 0x18, 0x18, 0x4E, 0x50, 0x50, 0x18, 0x18, 0x00, // Row 6
	0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, // Row 7
	0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x00, 0x00, 0x00, // Row 8
};

static StaticPixelMap BulletImage = {{9, 9}, BulletData};

//-----------------------------------------------------------------------
//	Speech button mode override.

extern gPanelList   *speakButtonControls;   // controls for embedded speech button

//-----------------------------------------------------------------------
//	Audio resource ID generator

static char convBuf[5];

inline uint32 extendID(int16 smallID) {
	Common::sprintf_s(convBuf, "%4.4d", smallID);
	return smallID ? MKTAG(convBuf[0] + 'A' - '0', convBuf[1], convBuf[2], convBuf[3]) : 0 ;
}

/* ===================================================================== *
   Speech member functions
 * ===================================================================== */

void Speech::read(Common::InSaveFile *in) {
	//  Restore the sample count and character count
	_sampleCount = in->readSint16LE();
	_charCount = in->readSint16LE();

	//  Restore the text boundaries
	_bounds.read(in);

	//  Restore the pen color and outline color
	_penColor = in->readUint16LE();
	_outlineColor = in->readUint16LE();

	//  Restore the object ID
	_objID = in->readUint16LE();

	//  Restore the thread ID
	_thread = in->readSint16LE();

	//  Restore the flags
	_speechFlags = in->readSint16LE();

	debugC(4, kDebugSaveload, "...... sampleCount = %d", _sampleCount);
	debugC(4, kDebugSaveload, "...... charCount = %d", _charCount);
	debugC(4, kDebugSaveload, "...... penColor = %d", _penColor);
	debugC(4, kDebugSaveload, "...... outlineColor = %d", _outlineColor);
	debugC(4, kDebugSaveload, "...... bounds = (%d, %d, %d, %d)",
	       _bounds.x, _bounds.y, _bounds.width, _bounds.height);
	debugC(4, kDebugSaveload, "...... objID = %d", _objID);
	debugC(4, kDebugSaveload, "...... thread = %d", _thread);
	debugC(4, kDebugSaveload, "...... speechFlags = %d", _speechFlags);

	//  Restore the sample ID's
	for (int i = 0; i < _sampleCount; i++) {
		_sampleID[i] = in->readUint32BE();
		debugC(4, kDebugSaveload, "...... sampleID[%d] = %d", i, _sampleID[i]);
	}

	//  Restore the text
	in->read(_speechBuffer, _charCount);
	_speechBuffer[_charCount] = '\0';
	debugC(4, kDebugSaveload, "...... _speechBuffer = %s", _speechBuffer);

	//  Requeue the speech if needed
	if (_speechFlags & kSpQueued) {
		//  Add to the active list
		speechList.remove(this);
		speechList._list.push_back(this);
	}
}

//-----------------------------------------------------------------------
//	Return the number of bytes needed to archive this SpeechTask

int32 Speech::archiveSize() {
	return      sizeof(_sampleCount)
	            +   sizeof(_charCount)
	            +   sizeof(_bounds)
	            +   sizeof(_penColor)
	            +   sizeof(_outlineColor)
	            +   sizeof(_objID)
	            +   sizeof(_thread)
	            +   sizeof(_speechFlags)
	            +   sizeof(uint32) * _sampleCount
	            +   sizeof(char) * _charCount;
}

void Speech::write(Common::MemoryWriteStreamDynamic *out) {
	//  Store the sample count and character count
	out->writeSint16LE(_sampleCount);
	out->writeSint16LE(_charCount);

	//  Store the text boundaries
	_bounds.write(out);

	//  Store the pen color and outline color
	out->writeUint16LE(_penColor);
	out->writeUint16LE(_outlineColor);

	//  Store the object's ID
	out->writeUint16LE(_objID);

	//  Store the thread ID
	out->writeSint16LE(_thread);

	//  Store the flags.  NOTE:  Make sure this speech is not stored
	//  as being active
	out->writeSint16LE(_speechFlags & ~kSpActive);

	debugC(4, kDebugSaveload, "...... _sampleCount = %d", _sampleCount);
	debugC(4, kDebugSaveload, "...... _charCount = %d", _charCount);
	debugC(4, kDebugSaveload, "...... _penColor = %d", _penColor);
	debugC(4, kDebugSaveload, "...... _outlineColor = %d", _outlineColor);
	debugC(4, kDebugSaveload, "...... _bounds = (%d, %d, %d, %d)",
	       _bounds.x, _bounds.y, _bounds.width, _bounds.height);
	debugC(4, kDebugSaveload, "...... _objID = %d", _objID);
	debugC(4, kDebugSaveload, "...... thread = %d", _thread);
	debugC(4, kDebugSaveload, "...... _speechFlags = %d", _speechFlags);

	for (int i = 0; i < _sampleCount; i++) {
		out->writeUint32BE(_sampleID[i]);
		debugC(4, kDebugSaveload, "...... _sampleID[%d] = %d", i, _sampleID[i]);
	}

	//  Store the text
	out->write(_speechBuffer, _charCount);
	debugC(4, kDebugSaveload, "...... _speechBuffer = %s", _speechBuffer);
}

//-----------------------------------------------------------------------
//	Append text and sample to existing speech record

bool Speech::append(char *text, int32 sampID) {
	int16           len = strlen(text);

	//  Check to see if there's enough room in the character buffer
	if (_charCount + len >= (long)sizeof(_speechBuffer)
	        ||  _sampleCount >= MAX_SAMPLES) return false;

	//  Copy text to end of text in buffer, including '\0'
	memcpy(&_speechBuffer[_charCount], text, len + 1);
	_charCount += len;

	//  Append sample ID to list of samples.
	//  REM: We should translate sample ID's from index to resource
	//  number here.
	if (sampID)
		_sampleID[_sampleCount++] = extendID(sampID);

	return true;
}

//-----------------------------------------------------------------------
//	Move speech to active list

bool Speech::activate() {

	//  Remove from existing list
	speechList.remove(this);

	//  Add to the active list
	speechList._list.push_back(this);

	_speechFlags |= kSpQueued;

	//  This routine can't fail
	return true;
}

//-----------------------------------------------------------------------
//	Move speech to active list

bool Speech::setupActive() {
	int16           x, y;
	int16           buttonNum = 0,
	                buttonChars;

	_speechFlags |= kSpActive;

	speechFinished.set((_charCount * 4 / 2) + kTicksPerSecond);

	//Turn Actor Towards Person They Are Talking To
//	MotionTask::turnObject( *obj, GameObject::objectAddress(32794)->getLocation());
//	Actor    *a = (Actor *)obj;
//	if(!a->setAction( kActionJumpUp, kAnimateRandom ))
//		throw gError( "Could Not Set Talk Animation");

	// Set up temp gport for blitting to bitmap
	_textPort.setStyle(kTextStyleThickOutline);    // extra Thick Outline
	_textPort.setOutlineColor(_outlineColor);      // outline black
	_textPort.setFont(&Amber13Font);              // speech font
	_textPort.setColor(_penColor);                 // color of letters
	_textPort.setMode(kDrawModeMatte);             // insure transparency

	setWidth();

	//  If speech position is off-screen, then skip
	if (!calcPosition(initialSpeechPosition)) return false;

	if (_sampleCount) {
		GameObject *go = GameObject::objectAddress(_objID);
		Location loc = go->notGetWorldLocation();
		_sampleID[_sampleCount] = 0;

		//sayVoice(_sampleID);

/// EO SEARCH ///
		if (sayVoiceAt(_sampleID, loc))
			_speechFlags |= kSpHasVoice;
		else
			_speechFlags &= ~kSpHasVoice;

	} else _speechFlags &= ~kSpHasVoice;

	speechLineCount = buttonWrap(speechLineList,
	                             speechButtonList,
	                             speechButtonCount,
	                             _speechBuffer,
	                             _bounds.width,
	                             !g_vm->_speechText && (_speechFlags & kSpHasVoice),
	                             _textPort);

	//  Compute height of bitmap based on number of lines of text.
	//  Include 4 for outline width
	_bounds.height =
	    (speechLineCount * (_textPort._font->height + lineLeading))
	    + outlineWidth * 2;

	//  Blit to temp bitmap
	_speechImage._size.x = _bounds.width;
	_speechImage._size.y = _bounds.height;
	_speechImage._data = new uint8[_speechImage.bytes()]();
	_textPort.setMap(&_speechImage);

	y = outlineWidth;                       // Plus 2 for Outlines
	buttonChars = speechButtonList[buttonNum].charWidth;

	for (int i = 0; i < speechLineCount; i++) {
		int16       lineChars = speechLineList[i].charWidth;
		char        *lineText = speechLineList[i].text;

		x   = (_bounds.width - speechLineList[i].pixelWidth) / 2
		      + outlineWidth;

		_textPort.moveTo(x, y);

		//  Draw each button on the line in turn.
		while (lineChars > 0) {
			int16   dChars;

			//  If this is the end of a button
			if (buttonChars <= 0) {
				//  Incr to next button
				buttonNum++;

				//  If no more buttons, then go nae ferther...
				//  Fer death awaits with nasty, pointy teeth...!
				if (buttonNum > speechButtonCount) break;

				buttonChars = speechButtonList[buttonNum].charWidth;
				_textPort.setColor(1 + 9);

				//  Blit the little bullet symbol
				lineChars--;
				lineText++;
				buttonChars--;

				_textPort.bltPixels(
				    BulletImage, 0, 0,
				    _textPort._penPos.x, _textPort._penPos.y + 1,
				    BulletImage.size.x, BulletImage.size.y);

				_textPort.move(bulletWidth, 0);
			}

			//  Compute how much of this button is on this line.
			dChars = MIN(lineChars, buttonChars);

			//  Draw however much of this button is on this line.
			_textPort.drawText(lineText, dChars);

			//  Move forward by dChars
			lineChars -= dChars;
			buttonChars -= dChars;
			lineText += dChars;
		}

		y += _textPort._font->height + lineLeading;
	}

	if (speechButtonCount > 0) {
		//  REM: Also set pointer to arrow shape.
		g_vm->_mouseInfo->setIntent(GrabInfo::kIntWalkTo);
		speakButtonControls->enable(true);

		speechList.SetLock(false);
	} else {
		//  If there is a lock flag on this speech, then LockUI()
		speechList.SetLock(_speechFlags & kSpLock);
	}

	if (!(_speechFlags & kSpNoAnimate) && isActor(_objID)) {
		Actor   *a = (Actor *)GameObject::objectAddress(_objID);

		if (!a->isDead() && !a->isMoving()) MotionTask::talk(*a);
	}

//	speechFinished.set( ticksPerSecond*2 );
	return (true);
}

//This Function Sets Up Width And Height For A Speech

void Speech::setWidth() {
	TextSpan        speechLineList_[32],   // list of speech lines
	                speechButtonList_[32]; // list of speech buttons
	int16           speechLineCount_,        // count of speech lines
	                speechButtonCount_;      // count of speech buttons

	//  How many word-wrapped lines does the speech take up if we word-wrap
	//  it to the default line width?

	speechLineCount_ = buttonWrap(speechLineList_,
	                             speechButtonList_,
	                             speechButtonCount_,
	                             _speechBuffer,
	                             defaultWidth,
	                             !g_vm->_speechText && (_speechFlags & kSpHasVoice),
	                             _textPort);

	//  If it's more than 3 lines, then use the max line width.

	if (speechLineCount_ > 3) {
		speechLineCount_ = buttonWrap(speechLineList_,
		                             speechButtonList_,
		                             speechButtonCount_,
		                             _speechBuffer,
		                             maxWidth,
		                             !g_vm->_speechText && (_speechFlags & kSpHasVoice),
		                             _textPort);
	}


	//  The actual width of the _bounds is the widest of the lines.

	_bounds.width = 0;
	for (int i = 0; i < speechLineCount_; i++) {
		_bounds.width = MAX(_bounds.width, speechLineList_[i].pixelWidth);
	}
	_bounds.width += outlineWidth * 2 + 4;       //  Some padding just in case.
}

//-----------------------------------------------------------------------
//	Calculate the position of the speech, emanating from the actor.

bool Speech::calcPosition(StaticPoint16 &p) {
	GameObject      *obj = GameObject::objectAddress(_objID);
	TilePoint       tp = obj->getWorldLocation();

	if (!isVisible(obj)) return false;

	TileToScreenCoords(tp, p);

	p.x = clamp(8,
	            p.x - _bounds.width / 2,
	            8 + maxWidth - _bounds.width);

	p.y = clamp(kTileRectY + 8,
	            p.y - (_bounds.height + actorHeight),
	            kTileRectHeight - 50 - _bounds.height);

	return true;
}

//-----------------------------------------------------------------------
//	Draw the text on the back buffer

bool Speech::displayText() {
	StaticPoint16 p;

	//  If there are button in the speech, then don't scroll the
	//  speech along with the display. Otherwise, calculate the
	//  position from the actor.
	if (speechButtonCount > 0)
		p = initialSpeechPosition;
	else if (!calcPosition(p))
		return false;

	//  Blit to the port
	g_vm->_backPort.setMode(kDrawModeMatte);
	g_vm->_backPort.bltPixels(_speechImage,
	                   0, 0,
	                   p.x + fineScroll.x,
	                   p.y + fineScroll.y,
	                   _bounds.width, _bounds.height);

	return true;
}

//-----------------------------------------------------------------------
//	Dispose of this speech object. If this is the one being displayed,
//	then dealloc the speech image

void Speech::dispose() {
	if (speechList.currentActive() == this) {
//		Actor   *a = (Actor *)sp->obj;
//		a->animationFlags |= kAnimateFinished;
//		a->setAction( kActionStand, kAnimateRandom );

		if (!longEnough())
			playVoice(0);
		//  Wake up the thread, and return the # of the button
		//  that was selected
		wakeUpThread(_thread, _selectedButton);

		//  De-allocate the speech data
		delete[] _speechImage._data;
		_speechImage._data = nullptr;

		//  Clear the number of active buttons
		speechLineCount = speechButtonCount = 0;
		speakButtonControls->enable(false);

		if (!(_speechFlags & kSpNoAnimate) && isActor(_objID)) {
			Actor   *a = (Actor *)GameObject::objectAddress(_objID);

			if (a->_moveTask)
				a->_moveTask->finishTalking();
		}
	} else wakeUpThread(_thread, 0);

	GameObject *obj = GameObject::objectAddress(_objID);

	debugC(1, kDebugTasks, "Speech: Disposing %p for %p (%s) (total = %d)'", (void *)this, (void *)obj, obj->objName(), speechList.speechCount());

	remove();
}

//-----------------------------------------------------------------------
//	Render the speech object at the head of the speech queue.

void updateSpeech() {
	Speech          *sp;

	//  if there is a speech object
	if ((sp = speechList.currentActive()) != nullptr) {
		//  If there is no bitmap, then set one up.
		if (!(sp->_speechFlags & Speech::kSpActive)) {
			sp->setupActive();

			//  If speech failed to set up, then skip it
			if (sp->_speechImage._data == nullptr) {
				sp->dispose();
				return;
			}
		}

		//  Draw the speech bitmap
		sp->displayText();

		//  If this speech has timed-out, then dispose of it.


		if (sp->longEnough() &&
		        (speechButtonCount == 0 || sp->_selectedButton != 0))
			sp->dispose();
	} else speechList.SetLock(false);
}

bool Speech::longEnough() {
	if (_speechFlags & kSpHasVoice)
		return (!stillDoingVoice(_sampleID));
	else
		return (_selectedButton != 0 || speechFinished.check());
}

//  Gets rid of the current speech

void Speech::abortSpeech() {
	//  Start by displaying first frame straight off, no delay
	speechFinished.set(0);
	if (_speechFlags & kSpHasVoice) {
		PlayVoice(nullptr);
	}
}

void abortSpeech() {
	if (speechList.currentActive()) speechList.currentActive()->abortSpeech();
}

//-----------------------------------------------------------------------
//	Delete all speeches relating to a particular actor

void deleteSpeech(ObjectID id) {         // voice sound sample ID
	Speech *sp;

	while ((sp = speechList.findSpeech(id)) != nullptr) sp->dispose();
}

//-----------------------------------------------------------------------
//	This routine does a word-wrap on the input text, and also checks for
//	the '@' symbol to see if there are any embedded buttons in the text.

int16 buttonWrap(
    TextSpan        *lineList,              // indicates where line breaks are
    TextSpan        *buttonList,            // indicates where button breaks are
    int16           &buttonCount,           // returns number of buttons
    char            *text,                  // text to wrap
    int16           width,                  // width of text
    int16           supressText,
    gPort           &textPort) {
	int16           i,                      // loop counter
	                line_start,             // start of current line
	                last_space,             // last space encountered
	                last_space_pixels = 0,  // pixel pos of last space
	                charPixels,             // pixel length of character
	                linePixels,             // pixels in current line
	                buttonPixels,           // pixels in current button
	                buttonChars,            // char count of current button
	                lineCount = 0;          // number of lines

	//  If we are not showing the text of the speech, skip over all text
	//  until we come to the first button definition.
	if (supressText) {
		while (*text && *text != '@') text++;
	}

	lineList->text = text;                  // set ptr to 1st line

	last_space      = -1;                   // no spaces to word-wrap yet
	line_start      = 0;                    // start index of 1st line
	linePixels      = 0;                    // no pixels counted yet

	width -= outlineWidth * 2;              // compensate for size of outline

	//  For each character in the string, check for word wrap

	for (i = 0; ; i++) {
		uint8           c = text[i];

//			REM: Translate from foreign character set if needed...
//		c = TranslationTable[c];

		// If deliberate end of line
		if (c == '\n' || c == '\r' || c == '\0') {
			lineList->charWidth = i - line_start;
			lineList->pixelWidth = linePixels;
			lineList++;
			lineCount++;

			line_start = i + 1;

			if (c == '\0') break;

			lineList->text = &text[line_start];

			last_space = -1;
			linePixels = 0;
			continue;
		} else if (c == '@') {          // button indicator...
			//  Set width of 'bullet' symbol.
			charPixels = bulletWidth;

		} else { //  Any other character
			//  if it's a space, save the word wrap position.
			if (c == ' ') {
				last_space = i;
				last_space_pixels = linePixels;
			}

			//  Add to pixel length
			charPixels
			    = textPort._font->charKern[c]
			      + textPort._font->charSpace[c];
		}

		linePixels += charPixels;

		//  If pixel runs off end of line
		if (linePixels > width && last_space > 0) {
			lineList->charWidth = last_space - line_start;
			lineList->pixelWidth = last_space_pixels;
			lineList++;
			lineCount++;

			line_start = last_space + 1;

			lineList->text = &text[line_start];

			last_space = -1;
			linePixels = 0;

			i = line_start - 1;
		}
	}

	buttonCount = 0;                        // assume zero buttons
	buttonPixels = 0;                       // no pixels counted yet
	buttonChars = 0;                        // no chars counted yet
	buttonList->text = text;                // set ptr to 1st button
	lineList -= lineCount;                  // reset line list

	//  For each line, look for button markers

	for (int l = 0; l < lineCount; l++, lineList++) {
		for (i = 0; i < lineList->charWidth; i++) {
			uint8           c = lineList->text[i];

			// REM: Translate from foreign character set if needed...
			// c = TranslationTable[c];

			if (c == '@') {             // button indicator...
				//  A new button
				buttonList->charWidth = buttonChars;
				buttonList->pixelWidth = buttonPixels;

				buttonPixels = 0;
				buttonChars = 0;
				buttonList++;
				buttonCount++;
				buttonList->text = text;    // set ptr to 1st button

				//  Set width of 'bullet' symbol.
				charPixels = bulletWidth;

			} else { //  Any other character
				//  Add to pixel length
				charPixels
				    = textPort._font->charKern[c]
				      + textPort._font->charSpace[c];
			}

			buttonPixels += charPixels;
			buttonChars++;
		}
	}

	//  Clean up the final button
	buttonList->charWidth = buttonChars;
	buttonList->pixelWidth = buttonPixels;

	return lineCount;
}

//-----------------------------------------------------------------------
//	Given the original word-wrap info, determines which button (if any)
//	was clicked.

int16 pickButton(
    Point16         &pt,
    TextSpan        *lineList,              // indicates where line breaks are
    int16           numLines,               // number of line breaks
    TextSpan        *buttonList,            // indicates where button breaks are
    int16           buttonCount,            // number of buttons
    int16           width,
    gPort           textPort) {                // width of rectangle
	int16           pickLine,
	                pickPixels = 0,
	                centerWidth;

	if (pt.y < 0                         // picked off top edge
	        ||  pt.x < 0                        // picked off left edge
	        ||  buttonCount < 1)                // no buttons defined
		return 0;

	pickLine = pt.y / (textPort._font->height + lineLeading);
	if (pickLine >= numLines) return 0;

	//  Strange algorithm:
	//
	//  When we first built these data structures, we took a continuous
	//  string of text and broke it up into several lines, and also
	//  broke it up into several buttons. Each of these has a count
	//  of how many pixels wide it was.
	//
	//  Now, consider if we were to lay each of those lines end
	//  to end as though we were reconstructing the original non-
	//  wrapped text string. And suppose the pick-point were carried
	//  along with the line, so that now our 2-d pixel point is
	//  now a 1-d pixel offset into the line.
	//
	//  We can do this by adding the length of each previous line
	//  to the pixel offset, and subtracting the margin space used
	//  for centering.

	for (int i = 0; i < pickLine; i++) {
		pickPixels += lineList[i].pixelWidth;
	}

	centerWidth = (width - lineList[pickLine].pixelWidth) / 2;

	//  Return 0 if mouse off left or right edge of text.
	if (pt.x < centerWidth || pt.x > width - centerWidth) return 0;

	pickPixels += pt.x - (width - lineList[pickLine].pixelWidth) / 2;

	//  Now, we lay all the buttons end to end in a similar fashion,
	//  and determine which button the pick point fell into, in a
	//  simple 1-d comparison.

	for (int j = 0; j <= buttonCount; j++) {
		pickPixels -= buttonList[j].pixelWidth;
		if (pickPixels < 0) return j;
	}

	return 0;
}

bool isVisible(GameObject *obj) {

	TilePoint tp = obj->getWorldLocation();
	Point16 p, vp;
	TileToScreenCoords(tp, p);

	//For Determining If Object Is Being Displayed
	//Could We Just Check Display List ???
	int16           distanceX, distanceY;
	int16           viewSizeY = kTileRectHeight;
	int16           viewSizeX = kTileRectWidth;

	//I Figure This Differently Than In Dispnode
	int16           loadDistX = viewSizeX / 2;
	int16           loadDistY = viewSizeY / 2;

	TileToScreenCoords(viewCenter, vp);

	distanceX = ABS(vp.x - p.x);
	distanceY = ABS(vp.y - p.y);

	if ((distanceY >= loadDistY) ||
	        (distanceX >= loadDistX))
		return (false);

	return (true);
}

/* ===================================================================== *
   SpeechTaskList member functions
 * ===================================================================== */

void SpeechTaskList::remove(Speech *p) {
	for (Common::List<Speech *>::iterator it = _list.begin();
			it != _list.end(); ++it) {
		if (p == *it) {
			_list.remove(p);
			break;
		}
	}

	for (Common::List<Speech *>::iterator it = _inactiveList.begin();
			it != _inactiveList.end(); ++it) {
		if (p == *it) {
			_inactiveList.remove(p);
			break;
		}
	}
}

//-----------------------------------------------------------------------
//	Initialize the SpeechTaskList

SpeechTaskList::SpeechTaskList() {
	_lockFlag = false;
}

SpeechTaskList::SpeechTaskList(Common::InSaveFile *in) {
	int16 count;

	_lockFlag = false;

	//  Get the speech count
	count = in->readSint16LE();
	debugC(3, kDebugSaveload, "... count = %d", count);

	//  Restore the speeches
	for (int i = 0; i < count; i++) {
		Speech *sp = new Speech;
		assert(sp != nullptr);
		debugC(3, kDebugSaveload, "Loading Speech %d", i++);

		_inactiveList.push_back(sp);
		sp->read(in);
	}
}

//-----------------------------------------------------------------------
//	Return the number of bytes needed to archive the speech tasks

int32 SpeechTaskList::archiveSize() {
	int32       size = 0;

	size += sizeof(int16);   //  Speech count

	for (Common::List<Speech *>::iterator it = _list.begin();
			it != _list.end(); ++it) {
		size += (*it)->archiveSize();
	}

	for (Common::List<Speech *>::iterator it = _inactiveList.begin();
			it != _inactiveList.end(); ++it) {
		size += (*it)->archiveSize();
	}

	return size;
}

void SpeechTaskList::write(Common::MemoryWriteStreamDynamic *out) {
	int i = 0;
	int16 count = 0;

	count += _list.size() + _inactiveList.size();

	//  Store speech count
	out->writeSint16LE(count);
	debugC(3, kDebugSaveload, "... count = %d", count);

	//  Store active speeches
	for (Common::List<Speech *>::iterator it = _list.begin();
			it != _list.end(); ++it) {
		debugC(3, kDebugSaveload, "Saving Speech %d (active)", i++);
		(*it)->write(out);
	}

	//  Store inactive speeches
	for (Common::List<Speech *>::iterator it = _inactiveList.begin();
			it != _inactiveList.end(); ++it) {
		debugC(3, kDebugSaveload, "Saving Speech %d (inactive)", i++);
		(*it)->write(out);
	}
}

//-----------------------------------------------------------------------
//	Cleanup the speech tasks

void SpeechTaskList::cleanup() {
	for (Common::List<Speech *>::iterator it = speechList._list.begin();
	     it != speechList._list.end(); ++it) {
		delete *it;
	}

	for (Common::List<Speech *>::iterator it = speechList._inactiveList.begin();
	     it != speechList._inactiveList.end(); ++it) {
		delete *it;
	}

	_list.clear();
	_inactiveList.clear();
}

//-----------------------------------------------------------------------
//	Search for a speech task associated with a particular GameObject.

Speech *SpeechTaskList::findSpeech(ObjectID id) {
	for (Common::List<Speech *>::iterator it = speechList._inactiveList.begin();
	     it != speechList._inactiveList.end(); ++it) {
		if ((*it)->_objID == id)
			return *it;
	}

	return nullptr;
}

//-----------------------------------------------------------------------
//	Get a new speech task, if there is one available, and initialize it.

Speech *SpeechTaskList::newTask(ObjectID id, uint16 flags) {
	Speech              *sp;
	GameObject          *obj = GameObject::objectAddress(id);

	//  Actors cannot speak if not in the world
	if (obj->world() != currentWorld) return nullptr;

	if (speechCount() >= MAX_SPEECH_PTRS) {
		warning("Too many speech tasks: > %d", MAX_SPEECH_PTRS);
		return nullptr;
	}

	sp = new Speech;
#if DEBUG
	if (sp == NULL) fatal("Ran out of Speech Tasks, Object = %s\n", obj->objName());
#endif
	if (sp == nullptr) return nullptr;

	debugC(1, kDebugTasks, "Speech: New Task: %p for %p (%s) (flags = %d) (total = %d)", (void *)sp, (void *)obj, obj->objName(), flags, speechCount());

	sp->_sampleCount = sp->_charCount = 0;
	sp->_objID       = id;
	sp->_speechFlags = flags & (Speech::kSpNoAnimate | Speech::kSpLock);
	sp->_outlineColor = 15 + 9;
	sp->_thread      = NoThread;
	sp->_selectedButton = 0;

	//  Set the pen color of the speech based on the actor
	if (isActor(id)) {
		Actor           *a = (Actor *)obj;

		//  If actor has color table loaded, then get the speech
		//  color for this particular color scheme; else use a
		//  default color.
		if (a == getCenterActor()) sp->_penColor = 3 + 9 /* 1 */;
		else if (a->_appearance
		         &&  a->_appearance->_schemeList) {
			sp->_penColor =
			    a->_appearance->_schemeList->_schemes[a->_colorScheme]->speechColor + 9;
		} else sp->_penColor = 4 + 9;
	} else {
		sp->_penColor = 4 + 9;
	}

	_inactiveList.push_back(sp);
	return sp;
}

void SpeechTaskList::SetLock(int newState) {
	if (newState && _lockFlag == false) {
		noStickyMap();
		LockUI(true);
		_lockFlag = true;
	} else if (_lockFlag && newState == false) {
		LockUI(false);
		_lockFlag = false;
	}
}

//-----------------------------------------------------------------------
//	When a speech task is finished, call this function to delete it.

void Speech::remove() {
	speechList.remove(this);
}

//-----------------------------------------------------------------------
//	AppFunc for handling clicks on speech

int16 pickSpeechButton(Point16 mouse, int16 size, gPort &textPort) {
	Point16 p = mouse - initialSpeechPosition;

	p.x -= kTileRectX;
	p.y -= kTileRectY;

	return pickButton(p,
	                  speechLineList, speechLineCount,
	                  speechButtonList, speechButtonCount,
	                  size,
	                  textPort);
}

APPFUNC(cmdClickSpeech) {
	Speech          *sp;

	switch (ev.eventType) {
	case kEventMouseMove:
	case kEventMouseDrag:

		g_vm->_mouseInfo->setDoable(Rect16(kTileRectX, kTileRectY, kTileRectWidth, kTileRectHeight).ptInside(ev.mouse));
		break;

	case kEventMouseDown:

		if ((sp = speechList.currentActive()) != nullptr) {
			sp->_selectedButton = pickSpeechButton(ev.mouse, sp->_speechImage._size.x, sp->_textPort);
		}
		break;

	default:
		break;
	}
}

/* ===================================================================== *
   SpeechTask management functions
 * ===================================================================== */

//-----------------------------------------------------------------------
//	Initialize the speech task list

void initSpeechTasks() {
	//  Simply call the SpeechTaskList default constructor
	new (&speechList) SpeechTaskList;
}

void saveSpeechTasks(Common::OutSaveFile *outS) {
	debugC(2, kDebugSaveload, "Saving Speech Tasks");

	outS->write("SPCH", 4);
	CHUNK_BEGIN;
	speechList.write(out);
	CHUNK_END;
}

void loadSpeechTasks(Common::InSaveFile *in, int32 chunkSize) {
	debugC(2, kDebugSaveload, "Loading Speech Tasks");

	//  If there is no saved data, simply call the default constructor
	if (chunkSize == 0) {
		new (&speechList) SpeechTaskList;
		return;
	}

	//  Reconstruct stackList from archived data
	new (&speechList) SpeechTaskList(in);
}

//-----------------------------------------------------------------------
//	Cleanup the speech task list

void cleanupSpeechTasks() {
	//  Call speechList's cleanup() function
	speechList.cleanup();
}

} // end of namespace Saga2
