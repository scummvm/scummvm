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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "m4/mads_anim.h"
#include "m4/m4.h"
#include "m4/compression.h"

namespace M4 {

#define TEXTVIEW_LINE_SPACING 2
#define TEXT_ANIMATION_DELAY 100
#define TV_NUM_FADE_STEPS 40
#define TV_FADE_DELAY_MILLI 50

TextviewView::TextviewView(M4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())),
		_bgSurface(vm->_screen->width(), MADS_SURFACE_HEIGHT),
		_textSurface(vm->_screen->width(), MADS_SURFACE_HEIGHT + vm->_font->getHeight() +
			TEXTVIEW_LINE_SPACING) {

	_screenType = VIEWID_TEXTVIEW;
	_screenFlags.layer = LAYER_BACKGROUND;
	_screenFlags.visible = true;
	_screenFlags.get = SCREVENT_ALL;
	_callback = NULL;
	_script = NULL;
	_spareScreen = NULL;
	_bgCurrent = NULL;
	_bgSpare = NULL;
	reset();

	// Set up system palette colors and the two colors for text display
	_vm->_palette->setMadsSystemPalette();
	RGB8 palData[3];
	palData[0].r = palData[0].g = palData[0].b = 0;
	palData[1].r = 0; palData[1].g = palData[1].b = 252;
	palData[2].r = 0; palData[2].g = palData[2].b = 180;
	_vm->_palette->setPalette(&palData[0], 4, 3);
	_vm->_palette->blockRange(4, 3);

	_vm->_font->setColors(5, 6, 4);

	clear();
	_bgSurface.clear();
	_textSurface.clear();

	int y = (height() - MADS_SURFACE_HEIGHT) / 2;
	setColor(2);
	hLine(0, width() - 1, y - 2);
	hLine(0, width() - 1, height() - y + 1);
}

TextviewView::~TextviewView() {
	if (_script)
		_vm->res()->toss(_resourceName);
	if (_spareScreen)
		delete _spareScreen;
	if (_bgCurrent)
		delete _bgCurrent;
	if (_bgSpare)
		delete _bgSpare;
}

void TextviewView::reset() {
	_bgSurface.clear();
	_textSurface.clear();
	_animating = false;
	_panX = 0;
	_panY = 0;
	_panSpeed = 0;
	_soundDriverLoaded = false;
	Common::set_to(&_spareScreens[0], &_spareScreens[10], 0);
	_scrollCount = 0;
	_lineY = -1;
	_scrollTimeout = 0;
	_panCountdown = 0;
	_processEvents = true;
}

void TextviewView::setScript(const char *resourceName, TextviewCallback callback) {
	_callback = callback;
	if (_script)
		_vm->res()->toss(_resourceName);
	if (_spareScreen) {
		delete _spareScreen;
		_spareScreen = NULL;
	}

	reset();

	strncpy(_resourceName, resourceName, 15);
	_resourceName[15] = '\0';
	if (!strchr(_resourceName, '.'))
		strcat(_resourceName, ".txr");

	_script = _vm->res()->get(_resourceName);

	processLines();
}

bool TextviewView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	if (!_processEvents)
		return false;

	// Wait for the Escape key or a mouse press
	if (((eventType == KEVENT_KEY) && (param == Common::KEYCODE_ESCAPE)) ||
		(eventType == MEVENT_LEFT_RELEASE) || (eventType == MEVENT_RIGHT_RELEASE)) {
		scriptDone();
		captureEvents = false;
		return true;
	}

	return false;
}

void TextviewView::updateState() {
	if (!_animating)
		return;

	// Only update state if wait period has expired
	uint32 currTime = g_system->getMillis();

	// If a screen transition is in progress and it's time for another column, handle it
	if (_spareScreen) {
		byte *srcP = _spareScreen->getBasePtr(_translationX, 0);
		byte *destP = _bgSurface.getBasePtr(_translationX, 0);

		for (int y = 0; y < _bgSurface.height(); ++y, srcP += _spareScreen->width(),
				destP += _bgSurface.width()) {
			*destP = *srcP;
		}

		if (++_translationX >= _bgSurface.width()) {
			// Surface transition is complete
			delete _spareScreen;
			_spareScreen = NULL;

			_vm->_palette->deleteRange(_bgCurrent);
			delete _bgCurrent;
			_bgCurrent = _bgSpare;
			_bgSpare = NULL;
		}
	}

	// Make sure it's time for an update
	if (currTime < _scrollTimeout)
		return;
	_scrollTimeout = g_system->getMillis() + TEXT_ANIMATION_DELAY;

	// If any panning values are set, pan the background surface
	if ((_panX != 0) || (_panY != 0)) {
		if (_panCountdown > 0) {
			--_panCountdown;
			return;
		}

		// Handle horizontal panning
		if (_panX != 0) {
			byte *lineTemp = new byte[_panX];
			for (int y = 0; y < _bgSurface.height(); ++y) {
				byte *pixelsP = _bgSurface.getBasePtr(0, y);

				// Copy the first X pixels into temp buffer, move the rest of the line
				// to the start of the line, and then move temp buffer pixels to end of line
				Common::copy(pixelsP, pixelsP + _panX, lineTemp);
				Common::copy(pixelsP + _panX, pixelsP + _bgSurface.width(), pixelsP);
				Common::copy(lineTemp, lineTemp + _panX, pixelsP + _bgSurface.width() - _panX);
			}

			delete[] lineTemp;
		}

		// Handle vertical panning
		if (_panY != 0) {
			// Store the bottom Y lines into a temp buffer, move the rest of the lines down,
			// and then copy the stored lines back to the top of the screen
			byte *linesTemp = new byte[_panY * _bgSurface.width()];
			byte *pixelsP = _bgSurface.getBasePtr(0, _bgSurface.height() - _panY);
			Common::copy(pixelsP, pixelsP + _bgSurface.width() * _panY, linesTemp);

			for (int y = _bgSurface.height() - 1; y >= _panY; --y) {
				byte *destP = _bgSurface.getBasePtr(0, y);
				byte *srcP = _bgSurface.getBasePtr(0, y - _panY);
				Common::copy(srcP, srcP + _bgSurface.width(), destP);
			}

			Common::copy(linesTemp, linesTemp + _panY * _bgSurface.width(), (byte *)_bgSurface.pixels);
			delete[] linesTemp;
		}
	}

	// Scroll the text surface up by one row
	byte *pixelsP = (byte *)_textSurface.pixels;
	Common::copy(pixelsP + width(),  pixelsP + _textSurface.width() * _textSurface.height(), pixelsP);
	pixelsP = _textSurface.getBasePtr(0, _textSurface.height() - 1);
	Common::set_to(pixelsP, pixelsP + _textSurface.width(), _vm->_palette->BLACK);

	if (_scrollCount > 0) {
		// Handling final scrolling of text off of screen
		if (--_scrollCount == 0) {
			scriptDone();
			return;
		}
	} else {
		// Handling a text row
		if (++_lineY == (_vm->_font->getHeight() + TEXTVIEW_LINE_SPACING))
			processLines();
	}

	// Refresh the view
	int yp = (height() - _bgSurface.height()) / 2;
	_bgSurface.copyTo(this, 0, yp);
	_textSurface.copyTo(this, Common::Rect(0, 0, _textSurface.width(), _bgSurface.height()),
		0, yp, _vm->_palette->BLACK);
}

void TextviewView::scriptDone() {
	TextviewCallback fn = _callback;
	M4Engine *vm = _vm;

	// Remove this view from manager and destroy it
	_vm->_viewManager->deleteView(this);

	if (fn)
		fn(vm);
}

void TextviewView::processLines() {
	strncpy(_currentLine, _script->readLine().c_str(), 79);
	if (_script->eos() || _script->err())
		error("Attempted to read past end of response file");

	while (!_script->eos() && !_script->err()) {
		// Commented out line, so go loop for another
		if (_currentLine[0] == '#') {
			strncpy(_currentLine, _script->readLine().c_str(), 79);
			continue;
		}

		// Process the line
		char *cStart = strchr(_currentLine, '[');
		if (cStart) {
			while (cStart) {
				// Loop for possible multiple commands on one line
				char *cEnd = strchr(_currentLine, ']');
				if (!cEnd)
					error("Unterminated command '%s' in response file", _currentLine);

				*cEnd = '\0';
				processCommand();

				// Copy rest of line (if any) to start of buffer
				strcpy(_currentLine, cEnd + 1);

				cStart = strchr(_currentLine, '[');
			}

			if (_currentLine[0]) {
				processText();
				break;
			}

		} else {
			processText();
			break;
		}

		strncpy(_currentLine, _script->readLine().c_str(), 79);
	}
}

void TextviewView::processCommand() {
	char commandStr[80];
	char *paramP;
	strcpy(commandStr, _currentLine + 1);
	str_upper(commandStr);

	if (!strncmp(commandStr, "BACKGROUND", 10)) {
		// Set the background
		paramP = commandStr + 10;
		int screenId = getParameter(&paramP);
		_bgSurface.loadBackground(screenId, &_bgCurrent);
		_vm->_palette->addRange(_bgCurrent);
		_bgSurface.translate(_bgCurrent);

	} else if (!strncmp(commandStr, "GO", 2)) {
		_animating = true;

		// Grab what the final palete will be
		RGB8 destPalette[256];
		_vm->_palette->grabPalette(destPalette, 0, 256);

		// Copy the loaded background, if any, to the view surface
		int yp = (height() - _bgSurface.height()) / 2;
		_bgSurface.copyTo(this, 0, yp);

		// Handle fade-in
		_processEvents = false;		// stop processing events during fade-in
		_vm->_palette->fadeIn(TV_NUM_FADE_STEPS, TV_FADE_DELAY_MILLI, destPalette, 256);
		_processEvents = true;

	} else if (!strncmp(commandStr, "PAN", 3)) {
		// Set panning values
		paramP = commandStr + 3;
		int panX = getParameter(&paramP);
		int panY = getParameter(&paramP);
		int panSpeed = getParameter(&paramP);

		if ((panX != 0) || (panY != 0)) {
			_panX = panX;
			_panY = panY;
			_panSpeed = panSpeed;
		}

	} else if (!strncmp(commandStr, "DRIVER", 6)) {
		// Set the driver to use
		// TODO: Handling of the sound drivers

	} else if (!strncmp(commandStr, "SOUND", 5)) {
		// Set sound number
		paramP = commandStr + 5;
		//int soundId = getParameter(&paramP);

		//TODO: Proper handling of the sound drivers/sounds
		//if (!_soundDriverLoaded)
		//	error("Attempted to set sound without loading any driver");

	} else if (!strncmp(commandStr, "COLOR", 5) && ((commandStr[5] == '0') || (commandStr[5] == '1'))) {
		// Set the text colors
		int index = commandStr[5] - '0';
		paramP = commandStr + 6;

		RGB8 palEntry;
		palEntry.r = getParameter(&paramP) << 2;
		palEntry.g = getParameter(&paramP) << 2;
		palEntry.b = getParameter(&paramP) << 2;
		_vm->_palette->setPalette(&palEntry, 5 + index, 1);

	} else if (!strncmp(commandStr, "SPARE", 5)) {
		// Sets a secondary background number that can be later switched in with a PAGE command
		paramP = commandStr + 6;
		int spareIndex = commandStr[5] - '0';
		if ((spareIndex >= 0) && (spareIndex <= 9)) {
			int screenId = getParameter(&paramP);

			_spareScreens[spareIndex] = screenId;
		}

	} else if (!strncmp(commandStr, "PAGE", 4)) {
		// Signals to change to a previous specified secondary background
		paramP = commandStr + 4;
		int spareIndex = getParameter(&paramP);

		// Only allow background switches if one isn't currently in progress
		if (!_spareScreen && (_spareScreens[spareIndex] != 0)) {
			_spareScreen = new M4Surface(width(), MADS_SURFACE_HEIGHT);
			_spareScreen->loadBackground(_spareScreens[spareIndex], &_bgSpare);
			_vm->_palette->addRange(_bgSpare);
			_spareScreen->translate(_bgSpare);

			_translationX = 0;
		}

	} else {
		error("Unknown response command: '%s'", commandStr);
	}
}

int TextviewView::getParameter(char **paramP) {
	if ((**paramP != '=') && (**paramP != ','))
		return 0;

	int result = 0;
	++*paramP;
	while ((**paramP >= '0') && (**paramP <= '9')) {
		result = result * 10 + (**paramP - '0');
		++*paramP;
	}

	return result;
}

void TextviewView::processText() {
	int lineWidth, xStart;

	if (!strcmp(_currentLine, "***")) {
		// Special signifier for end of script
		_scrollCount = _vm->_font->getHeight() * 13;
		_lineY = -1;
		return;
	}

	_lineY = 0;

	// Lines are always centered, except if line contains a '@', in which case the
	// '@' marks the position that must be horizontally centered
	char *centerP = strchr(_currentLine, '@');
	if (centerP) {
		*centerP = '\0';
		xStart = (width() / 2) - _vm->_font->getWidth(_currentLine);

		// Delete the @ character and shift back the remainder of the string
		char *p = centerP + 1;
		if (*p == ' ') ++p;
		strcpy(centerP, p);

	} else {
		lineWidth = _vm->_font->getWidth(_currentLine);
		xStart = (width() - lineWidth) / 2;
	}

	// Copy the text line onto the bottom of the textSurface surface, which will allow it
	// to gradually scroll onto the screen
	int yp = _textSurface.height() - _vm->_font->getHeight() - TEXTVIEW_LINE_SPACING;
	_textSurface.fillRect(Common::Rect(0, yp, _textSurface.width(), _textSurface.height()),
		_vm->_palette->BLACK);
	_vm->_font->writeString(&_textSurface, _currentLine, xStart, yp);
}


//--------------------------------------------------------------------------

AnimviewView::AnimviewView(M4Engine *vm):
		View(vm, Common::Rect(0, 0, vm->_screen->width(), vm->_screen->height())),
		_bgSurface(vm->_screen->width(), MADS_SURFACE_HEIGHT) {

	_screenType = VIEWID_ANIMVIEW;
	_screenFlags.layer = LAYER_BACKGROUND;
	_screenFlags.visible = true;
	_screenFlags.get = SCREVENT_ALL;
	_callback = NULL;
	_script = NULL;
	_palData = NULL;
	_previousUpdate = 0;
	_transition = kTransitionNone;
	reset();

	// Set up system palette colors
	_vm->_palette->setMadsSystemPalette();

	clear();
	_bgSurface.clear();

	int y = (height() - MADS_SURFACE_HEIGHT) / 2;
	setColor(2);
	hLine(0, width() - 1, y - 2);
	hLine(0, width() - 1, height() - y + 1);
}

AnimviewView::~AnimviewView() {
	if (_script)
		_vm->res()->toss(_resourceName);
}

void AnimviewView::reset() {
	_bgSurface.clear();
	_soundDriverLoaded = false;
}

void AnimviewView::setScript(const char *resourceName, AnimviewCallback callback) {
	_callback = callback;
	if (_script)
		_vm->res()->toss(_resourceName);

	reset();

	strncpy(_resourceName, resourceName, 15);
	_resourceName[15] = '\0';
	if (!strchr(_resourceName, '.'))
		strcat(_resourceName, ".res");

	_script = _vm->res()->get(_resourceName);
}

bool AnimviewView::onEvent(M4EventType eventType, int param, int x, int y, bool &captureEvents) {
	// Wait for the Escape key or a mouse press
	if (((eventType == KEVENT_KEY) && (param == Common::KEYCODE_ESCAPE)) ||
		(eventType == MEVENT_LEFT_RELEASE) || (eventType == MEVENT_RIGHT_RELEASE)) {
		scriptDone();
		captureEvents = false;
		return true;
	}

	return false;
}

void AnimviewView::updateState() {
	if (!_script)
		return;

	// Only update state if wait period has expired
	if (_previousUpdate > 0) {
		if (g_system->getMillis() - _previousUpdate < 100)
			return;

		_previousUpdate = g_system->getMillis();
	}

	// Check if we're ready for the next command
	bool animRunning = false;
	if (!animRunning) {
		if (_script->eos() ||  _script->err()) {
			scriptDone();
			return;
		}
		
		readNextCommand();

		// FIXME: Replace flag with proper animation end check
		animRunning = true;
	}
}

void AnimviewView::readNextCommand() {
	while (!_script->eos() && !_script->err()) {
		strncpy(_currentLine, _script->readLine().c_str(), 79);

		// Process any switches on the line
		char *cStart = strchr(_currentLine, '-');
		while (cStart) {
			// Loop for possible multiple commands on one line
			char *cEnd = strchr(_currentLine, ' ');
			if (!cEnd)
				error("Unterminated command '%s' in response file", _currentLine);

			*cEnd = '\0';
			processCommand();

			// Copy rest of line (if any) to start of buffer
			// Don't use strcpy() here, because if the
			// rest of the line is the longer of the two
			// strings, the memory areas will overlap.
			memmove(_currentLine, cEnd + 1, strlen(cEnd + 1) + 1);

			cStart = strchr(_currentLine, '-');
		}
	
		// If there's something left, presume it's a resource name to process
		if (_currentLine[0])
			break;
	}

	if (strchr(_currentLine, '.') == NULL)
		strcat(_currentLine, ".aa");

	AAFile aaFile(_currentLine, _vm);

	// Initial validation
	if (aaFile.flags & AA_HAS_FONT) {
		assert(_vm->_resourceManager->resourceExists(aaFile.fontResource.c_str()));
	}

	for (int seriesCtr = 0; seriesCtr < aaFile.seriesCount; ++seriesCtr)
		assert(_vm->_resourceManager->resourceExists(aaFile.filenames[seriesCtr].c_str()));

	// Start sound
	if (aaFile.flags & AA_HAS_SOUND)
	{
		char buffer[100];
		strcpy(buffer, aaFile.soundName.c_str());
		buffer[0] = 'A';	// A for Adlib resource

		Common::SeekableReadStream *stream = _vm->_resourceManager->get(buffer);
		
		_vm->_resourceManager->toss(buffer);
	}
	

	char artFile[80];
	sprintf(artFile, "rm%d.art", aaFile.roomNumber);

	// Not all scenes have a background. If there is one, refresh it
	if (_vm->_resourceManager->resourceExists(artFile)) {
		if (_palData) {
			_vm->_palette->deleteRange(_palData);
			delete _palData;
		}
		_bgSurface.loadBackground(aaFile.roomNumber, &_palData);
		_vm->_palette->addRange(_palData);
		_bgSurface.translate(_palData);
	}

	// Grab what the final palete will be
	RGB8 destPalette[256];
	_vm->_palette->grabPalette(destPalette, 0, 256);

	// Handle scene transition
	switch (_transition) {
		case kTransitionNone:
			// nothing to do
			break;
		case kTransitionFadeIn:
		case kTransitionFadeIn2:
			_vm->_palette->fadeIn(TV_NUM_FADE_STEPS, TV_FADE_DELAY_MILLI, destPalette, 256);
			break;
		case kTransitionBoxInBottomLeft:
		case kTransitionBoxInBottomRight:
		case kTransitionBoxInTopLeft:
		case kTransitionBoxInTopRight:
			// unused
			warning("Unsupported box in scene effect");
			break;
		case kTransitionPanLeftToRight:
			// TODO
			break;
		case kTransitionPanRightToLeft:
			// TODO
			break;
		case kTransitionCircleIn:
			// TODO
			break;
		default:
			// nothing to do
			break;
	}

	// Refresh the view
	int yp = (height() - _bgSurface.height()) / 2;
	_bgSurface.copyTo(this, 0, yp);

	_vm->_resourceManager->toss(_currentLine);
}


void AnimviewView::scriptDone() {
	AnimviewCallback fn = _callback;
	M4Engine *vm = _vm;

	// Remove this view from manager and destroy it
	_vm->_viewManager->deleteView(this);

	if (fn)
		fn(vm);
}

/*
Switches are: (taken from the help of the original executable)
  -b       Toggle background load status off/on.
  -c:char  Specify sound card id letter.
  -g       Stay in graphics mode on exit.
  -h[:ex]  Disable EMS/XMS high memory support.
  -i       Switch sound interrupts mode off/on.
  -j       Wait for music to finish at end.
  -k       Keystroke jumps to end instead of abort.
  -m       Operate in non-MADS mode.
  -o:xxx   Specify opening special effect.
  -p       Switch MADS path mode to CONCAT.
  -r[:abn] Resynch timer (always, beginning, never).
  -s:file  Specify sound file.
  -u[:...] Use DMA speech [optional: addr,type,irq,drq].
  -w       Toggle white bars off/on.
  -x       Exit immediately after last frame.
  -y       Do not clear screen initially
  -z       Display statistics after run.

  Opening special effects are:
  0: no effect
  1: fade in
  2: fade in (looks to be the same as 1)
  3: box in from bottom left (unused)
  4: box in from bottom right (unused)
  5: box in from top left (unused)
  6: box in from top right (unused)
  7: pan in from left to right
  8: pan in from right to left
  9: circle in (new scene appears in a circle that expands)

  Animview is ran like this from the original games:
  animview.exe @resfilename -c:P,220,20 -u:220,20,07,01 -p -a:mainmenu -p

  Note that the first -p is necessary to watch the animation, otherwise
  the program just exits

  To watch an animation within the *.res file, just run animview like this:
  animview.exe -x -r:b -o:2 animfilename -p
*/
void AnimviewView::processCommand() {
	char commandStr[80];
	strcpy(commandStr, _currentLine + 1);
	str_upper(commandStr);
	char *param = commandStr;

	if (!strncmp(commandStr, "X", 1)) {
		//printf("X ");
	} else if (!strncmp(commandStr, "W", 1)) {
		//printf("W ");
	} else if (!strncmp(commandStr, "R", 1)) {
		param = param + 2;
		//printf("R:%s ", param);
	} else if (!strncmp(commandStr, "O", 1)) {
		param = param + 2;
		//printf("O:%i ", atoi(param));
		_transition = atoi(param);
	} else {
		error("Unknown response command: '%s'", commandStr);
	}
}

AAFile::AAFile(const char *resourceName, M4Engine* vm): MadsPack(resourceName, vm) {
	Common::MemoryReadStream stream1(*getItemStream(1));
	Common::MemoryReadStream stream2(*getItemStream(2));

	Common::MemoryReadStream stream(*getItemStream(0));
printf("ss %d %d %d\n", stream.size(), stream1.size(), stream2.size());
	seriesCount = stream.readUint16LE();
	frameCount = stream.readUint16LE();
	frameEntryCount = stream.readUint16LE();
	stream.skip(3);
	flags = stream.readByte();
	stream.skip(4);
	roomNumber = stream.readUint16LE();
	stream.skip(10);
	frameTicks = stream.readUint16LE();

	stream.skip(21);
	for (int i = 0; i < 10; ++i) {
		char filename[13];
		stream.read(filename, 13);
		filenames.push_back(Common::String(filename, 13));
	}

	stream.skip(81);
	char name[100];
	stream.read(name, 13);
	lbmFilename = Common::String(name, 13);

	stream.skip(365);
	stream.read(name, 13);
	spritesFilename = Common::String(name, 13);

	stream.skip(48);
	stream.read(name, 13);
	soundName = Common::String(name, 13);

	stream.skip(26);
	stream.read(name, 14);
	fontResource = Common::String(name, 14);
}

}
