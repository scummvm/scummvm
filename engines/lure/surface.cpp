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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "lure/surface.h"
#include "lure/decode.h"
#include "lure/events.h"
#include "lure/screen.h"
#include "lure/lure.h"
#include "lure/room.h"
#include "lure/sound.h"
#include "lure/strings.h"
#include "common/endian.h"

namespace Lure {

// These variables hold resources commonly used by the Surfaces, and must be initialised and freed
// by the static Surface methods initialise and deinitailse

static MemoryBlock *int_font = NULL;
static MemoryBlock *int_dialog_frame = NULL;
static uint8 fontSize[NUM_CHARS_IN_FONT];

void Surface::initialise() {
	int_font = Disk::getReference().getEntry(FONT_RESOURCE_ID);
	int_dialog_frame = Disk::getReference().getEntry(DIALOG_RESOURCE_ID);

	// Calculate the size of each font character
	for (int ctr = 0; ctr < NUM_CHARS_IN_FONT; ++ctr) {
		byte *pChar = int_font->data() + (ctr * 8);
		fontSize[ctr] = 0;

		for (int yp = 0; yp < FONT_HEIGHT; ++yp) 
		{
			byte v = *pChar++;

			for (int xp = 0; xp < FONT_WIDTH; ++xp) {
				if ((v & 0x80) && (xp > fontSize[ctr])) 
					fontSize[ctr] = xp;
				v = (v << 1) & 0xff;
			}
		}

		// If character is empty, like for a space, give a default size
		if (fontSize[ctr] == 0) fontSize[ctr] = 2;
	}
}

void Surface::deinitialise() {
	delete int_font;
	delete int_dialog_frame;
}

/*--------------------------------------------------------------------------*/

Surface::Surface(MemoryBlock *src, uint16 wdth, uint16 hght): _data(src), 
		_width(wdth), _height(hght) {
	if ((uint32) (wdth * hght) != src->size())
		error("Surface dimensions do not match size of passed data");
}

Surface::Surface(uint16 wdth, uint16 hght): _data(Memory::allocate(wdth*hght)),
	_width(wdth), _height(hght) {
}

Surface::~Surface() {
	delete _data;
}

void Surface::loadScreen(uint16 resourceId) {
	MemoryBlock *rawData = Disk::getReference().getEntry(resourceId);
	PictureDecoder decoder;
	MemoryBlock *tmpScreen = decoder.decode(rawData, FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH);
	delete rawData;
	empty();
	copyFrom(tmpScreen, MENUBAR_Y_SIZE * FULL_SCREEN_WIDTH);
	
	delete tmpScreen;
}

void Surface::writeChar(uint16 x, uint16 y, uint8 ascii, bool transparent, uint8 colour) {
	byte *const addr = _data->data() + (y * _width) + x;

	if ((ascii < 32) || (ascii >= 32 + NUM_CHARS_IN_FONT))
		error("Invalid ascii character passed for display '%d'", ascii);
	
	uint8 v;
	byte *pFont = int_font->data() + ((ascii - 32) * 8);
	byte *pDest;
	uint8 charWidth = 0;

	for (int y1 = 0; y1 < 8; ++y1) {
		v = *pFont++;
		pDest = addr + (y1 * _width);

		for (int x1 = 0; x1 < 8; ++x1, ++pDest) {
			if (v & 0x80) {
				*pDest = colour;
				if (x1+1 > charWidth) charWidth = x1 + 1;
			}
			else if (!transparent) *pDest = 0;
			v = (v << 1) & 0xff;
		}
	}
}

void Surface::writeString(uint16 x, uint16 y, Common::String line, bool transparent, 
						  uint8 colour, bool varLength) {
	const char *sPtr = line.c_str();

	while (*sPtr) {
		writeChar(x, y, (uint8) *sPtr, transparent, colour);

		// Move to after the character in preparation for the next character
		if (!varLength) x += FONT_WIDTH;
		else x += fontSize[*sPtr - ' '] + 2;

		++sPtr;		// Move to next character
	}
}

void Surface::transparentCopyTo(Surface *dest) {
	if (dest->width() != _width) 
		error("Incompatible surface sizes for transparent copy");

	byte *pSrc = _data->data();
	byte *pDest = dest->data().data();
	uint16 numBytes = MIN(_height,dest->height()) * FULL_SCREEN_WIDTH;

	while (numBytes-- > 0) {
		if (*pSrc) *pDest = *pSrc;
		
		++pSrc;
		++pDest;
	}
}

void Surface::copyTo(Surface *dest)
{
	copyTo(dest, 0, 0);
}

void Surface::copyTo(Surface *dest, uint16 x, uint16 y)
{
	if ((x == 0) && (dest->width() == _width)) {
		// Use fast data transfer
		uint32 dataSize = dest->data().size() - (y * _width);
		if (dataSize > _data->size()) dataSize = _data->size();
		dest->data().copyFrom(_data, 0, y * _width, dataSize);
	} else {
		// Use slower transfer
		Rect rect;
		rect.left = 0; rect.top = 0;
		rect.right = _width-1; rect.bottom = _height-1;
		copyTo(dest, rect, x, y);
	}
}

void Surface::copyTo(Surface *dest, const Rect &srcBounds, 
					 uint16 destX, uint16 destY, int transparentColour) {
	int numBytes = srcBounds.right - srcBounds.left + 1;
	if (destX + numBytes > dest->width())
		numBytes = dest->width() - destX;
	if (numBytes <= 0) return;

	for (uint16 y=0; y<=(srcBounds.bottom-srcBounds.top); ++y) {
		const uint32 srcPos = (srcBounds.top + y) * _width + srcBounds.left;
		const uint32 destPos = (destY+y) * dest->width() + destX;

		if (transparentColour == -1) {
			// No trnnsparent colour, so copy all the bytes of the line
			dest->data().copyFrom(_data, srcPos, destPos, numBytes);
		} else {
			byte *pSrc = _data->data() + srcPos;
			byte *pDest = dest->data().data() + destPos;

			int bytesCtr = numBytes;
			while (bytesCtr-- > 0) {
				if (*pSrc != (uint8) transparentColour)
					*pDest = *pSrc;
				++pSrc;
				++pDest;
			}
		}
	}
}

void Surface::copyFrom(MemoryBlock *src, uint32 destOffset) {
	uint32 size = _data->size() - destOffset;
	if (src->size() > size) size = src->size();
	_data->copyFrom(src, 0, destOffset, size);
}

// fillRect
// Fills a rectangular area with a colour

void Surface::fillRect(const Rect &r, uint8 colour) {
	for (int yp = r.top; yp <= r.bottom; ++yp) {
		byte *const addr = _data->data() + (yp * _width) + r.left;
		memset(addr, colour, r.width());
	}
}

// createDialog
// Forms a dialog encompassing the entire surface

void copyLine(byte *pSrc, byte *pDest, uint16 leftSide, uint16 center, uint16 rightSide) {
	// Left area
	memcpy(pDest, pSrc, leftSide); 
	pSrc += leftSide; pDest += leftSide; 
	// Center area
	memset(pDest, *pSrc, center);
	++pSrc; pDest += center; 
	// Right side
	memcpy(pDest, pSrc, rightSide); 
	pSrc += rightSide; pDest += rightSide; 
}

void Surface::createDialog(bool blackFlag) {
	if ((_width < 20) || (_height < 20)) return;

	byte *pSrc = int_dialog_frame->data();
	byte *pDest = _data->data();
	uint16 xCenter = _width - DIALOG_EDGE_SIZE * 2;
	uint16 yCenter = _height - DIALOG_EDGE_SIZE * 2;
	int y;

	// Dialog top
	for (y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE - 2, xCenter + 2, DIALOG_EDGE_SIZE);
		pSrc += (DIALOG_EDGE_SIZE - 2) + 1 + DIALOG_EDGE_SIZE;
		pDest += _width;
	}

	// Dialog sides - note that the same source data gets used for all side lines
	for (y = 0; y < yCenter; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter, DIALOG_EDGE_SIZE);
		pDest += _width;
	}
	pSrc += DIALOG_EDGE_SIZE * 2 + 1;

	// Dialog bottom
	for (y = 0; y < 9; ++y) {
		copyLine(pSrc, pDest, DIALOG_EDGE_SIZE, xCenter + 1, DIALOG_EDGE_SIZE - 1);
		pSrc += DIALOG_EDGE_SIZE + 1 + (DIALOG_EDGE_SIZE - 1);
		pDest += _width;
	}

	// Final processing - if black flag set, clear dialog inside area
	if (blackFlag) {
		Rect r = Rect(DIALOG_EDGE_SIZE, DIALOG_EDGE_SIZE, 
			_width - DIALOG_EDGE_SIZE, _height-DIALOG_EDGE_SIZE);
		fillRect(r, 0);
	}
}

void Surface::copyToScreen(uint16 x, uint16 y) {
	OSystem &system = *g_system;
	system.copyRectToScreen(_data->data(), _width, x, y, _width, _height);
	system.updateScreen();
}

void Surface::centerOnScreen() {
	OSystem &system = *g_system;

	system.copyRectToScreen(_data->data(), _width, 
		(FULL_SCREEN_WIDTH - _width) / 2, (FULL_SCREEN_HEIGHT - _height) / 2,
		_width, _height);
	system.updateScreen();
}

uint16 Surface::textWidth(const char *s, int numChars) {
	uint16 result = 0;
	if (numChars == 0) numChars = strlen(s);

	while (numChars-- > 0) result += fontSize[*s++ - ' '] + 2;
	return result;
}

void Surface::wordWrap(char *text, uint16 width, char **&lines, uint8 &numLines) {
	numLines = 1;
	uint16 lineWidth = 0;
	char *s;
	bool newLine;

	s = text;

	// Scan through the text and insert NULLs to break the line into allowable widths

	while (*s != '\0') {
		char *wordStart = s;
		while (*wordStart == ' ') ++wordStart;
		char *wordEnd = strchr(wordStart, ' ');
		char *wordEnd2 = strchr(wordStart, '\n');
		if ((!wordEnd) || ((wordEnd2) && (wordEnd2 < wordEnd))) {
			wordEnd = wordEnd2;
			newLine = (wordEnd2 != NULL);
		} else {
			newLine = false;
		}

		if (wordEnd) {
			if (!newLine) --wordEnd;
		} else {
			wordEnd = strchr(s, '\0') - 1;
		}

		uint16 wordSize = textWidth(s, (int) (wordEnd - s + 1));

		if (lineWidth + wordSize > width) {
			// Break word onto next line
			*(wordStart - 1) = '\0';
			++numLines;
			lineWidth = 0;
			wordEnd = wordStart;
		} else if (newLine) {
			// Break on newline
			++numLines;
			*wordEnd = '\0';
			lineWidth = 0;
		} else {
			// Add word's length to total for line
			lineWidth += wordSize;
		}

		s = wordEnd+1;
	}

	// Set up a list for the start of each line 
	lines = (char **) Memory::alloc(sizeof(char *) * numLines);
	lines[0] = text;
	for (int ctr = 1; ctr < numLines; ++ctr) 
		lines[ctr] = strchr(lines[ctr-1], 0) + 1;
}

Surface *Surface::newDialog(uint16 width, uint8 numLines, const char **lines, bool varLength, uint8 colour) {
	Surface *s = new Surface(width, (DIALOG_EDGE_SIZE + 3) * 2 + 
		numLines * (FONT_HEIGHT - 1));
	s->createDialog();

	for (uint8 ctr = 0; ctr < numLines; ++ctr)
		s->writeString(DIALOG_EDGE_SIZE + 3, DIALOG_EDGE_SIZE + 2 + 
			(ctr * (FONT_HEIGHT - 1)), lines[ctr], true, colour, varLength);
	return s;
}

Surface *Surface::newDialog(uint16 width, const char *line, uint8 colour) {
	char **lines;
	char *lineCopy = strdup(line);
	uint8 numLines;
	wordWrap(lineCopy, width - (DIALOG_EDGE_SIZE + 3) * 2, lines, numLines);

	// Create the dialog 
	Surface *result = newDialog(width, numLines, const_cast<const char **>(lines), true, colour);

	// Deallocate used resources
	free(lines);
	free(lineCopy);

	return result;
}

Surface *Surface::getScreen(uint16 resourceId) {
	MemoryBlock *block = Disk::getReference().getEntry(resourceId);
	PictureDecoder d;
	MemoryBlock *decodedData = d.decode(block);
	delete block;
	return new Surface(decodedData, FULL_SCREEN_WIDTH, decodedData->size() / FULL_SCREEN_WIDTH);
}

bool Surface::getString(Common::String &line, int maxSize, bool isNumeric, bool varLength, int16 x, int16 y) {
	OSystem &system = *g_system;
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();
	uint8 bgColour = *(screen.screen().data().data() + (y * FULL_SCREEN_WIDTH) + x);
	String newLine(line);
	bool abortFlag = false;
	bool refreshFlag = false;

	mouse.cursorOff();

	// Insert a cursor character at the end of the string
	newLine.insertChar('_', newLine.size());

	while (!abortFlag) {
		// Display the string
		screen.screen().writeString(x, y, newLine, true, DIALOG_TEXT_COLOUR, varLength);
		screen.update();
		int stringSize = textWidth(newLine.c_str());

		// Loop until the input string changes
		refreshFlag = false;
		while (!refreshFlag && !abortFlag) {
			abortFlag = events.quitFlag;
			if (abortFlag) break;

			if (events.pollEvent()) {
				if (events.type() == Common::EVENT_KEYDOWN) {
					char ch = events.event().kbd.ascii;
					uint16 keycode = events.event().kbd.keycode;

					if ((keycode == Common::KEYCODE_RETURN) || (keycode == Common::KEYCODE_KP_ENTER)) {
						// Return character
						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						screen.update();
						newLine.deleteLastChar();
						line = newLine;
						mouse.cursorOn();
						return true;
					}
					else if (keycode == Common::KEYCODE_ESCAPE) {
						// Escape character
						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						screen.update();
						abortFlag = true;
					} else if (keycode == Common::KEYCODE_BACKSPACE) {
						// Delete the last character
						if (newLine.size() == 1) continue;

						screen.screen().fillRect(
							Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
						newLine.deleteChar(newLine.size() - 2);
						refreshFlag = true;

					} else if ((ch >= ' ') && (stringSize + 8 < maxSize)) {
						if (((ch >= '0') && (ch <= '9')) || !isNumeric) {
							screen.screen().fillRect(
								Rect(x, y, x + maxSize - 1, y + FONT_HEIGHT), bgColour);
							newLine.insertChar(ch, newLine.size() - 1);
							refreshFlag = true;
						}
					}
				}
			}

			system.updateScreen();
			system.delayMillis(10);
		}
	}

	mouse.cursorOn();
	return false;
}


/*--------------------------------------------------------------------------*/

void Dialog::show(const char *text) {
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	Room &room = Room::getReference();
	mouse.cursorOff();

	room.update();
	Surface *s = Surface::newDialog(INFO_DIALOG_WIDTH, text);
	s->copyToScreen(INFO_DIALOG_X, INFO_DIALOG_Y);

	// Wait for a keypress or mouse button
	Events::getReference().waitForPress();

	screen.update();
	mouse.cursorOn();
}

void Dialog::show(uint16 stringId, const char *hotspotName, const char *characterName) {
	char buffer[MAX_DESC_SIZE];
	StringData &sl = StringData::getReference();

	sl.getString(stringId, buffer, hotspotName, characterName);
	show(buffer);
}

void Dialog::show(uint16 stringId) {
	show(stringId, NULL, NULL);
}

/*--------------------------------------------------------------------------*/

TalkDialog::TalkDialog(uint16 characterId, uint16 destCharacterId, uint16 activeItemId, uint16 descId) {
	debugC(ERROR_DETAILED, kLureDebugAnimations, "TalkDialog(chars=%xh/%xh, item=%d, str=%d", 
		characterId, destCharacterId, activeItemId, descId);
	StringData &strings = StringData::getReference();
	Resources &res = Resources::getReference();
	char srcCharName[MAX_DESC_SIZE];
	char destCharName[MAX_DESC_SIZE];
	char itemName[MAX_DESC_SIZE];
	int characterArticle, hotspotArticle = 3;

	HotspotData *talkingChar = res.getHotspot(characterId);
	HotspotData *destCharacter = (destCharacterId == 0) ? NULL : 
		res.getHotspot(destCharacterId);
	HotspotData *itemHotspot = (activeItemId == 0) ? NULL :
		res.getHotspot(activeItemId);
	assert(talkingChar);

	strings.getString(talkingChar->nameId & 0x1fff, srcCharName);
	characterArticle = talkingChar->nameId >> 14;

	strcpy(destCharName, "");
	if (destCharacter != NULL)
		strings.getString(destCharacter->nameId, destCharName);
	strcpy(itemName, "");
	if (itemHotspot != NULL) {
		strings.getString(itemHotspot->nameId & 0x1fff, itemName);
		hotspotArticle = itemHotspot->nameId >> 14;
	}

	strings.getString(descId, _desc, itemName, destCharName, hotspotArticle, characterArticle);

	// Apply word wrapping to figure out the needed size of the dialog
	Surface::wordWrap(_desc, TALK_DIALOG_WIDTH - (TALK_DIALOG_EDGE_SIZE + 3) * 2,
		_lines, _numLines);

	_surface = new Surface(TALK_DIALOG_WIDTH, 
		(_numLines + 1) * FONT_HEIGHT + TALK_DIALOG_EDGE_SIZE * 4);

	// Draw the dialog
	byte *pSrc = res.getTalkDialogData().data();
	byte *pDest = _surface->data().data();
	int xPos, yPos;

	// Handle the dialog top
	for (yPos = 0; yPos < TALK_DIALOG_EDGE_SIZE; ++yPos) {
		*pDest++ = *pSrc++;
		*pDest++ = *pSrc++;

		for (xPos = 0; xPos < TALK_DIALOG_WIDTH - TALK_DIALOG_EDGE_SIZE - 2; ++xPos) 
			*pDest++ = *pSrc;
		++pSrc;

		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrc++;
	}

	// Handle the middle section
	for (yPos = 0; yPos < _surface->height() - TALK_DIALOG_EDGE_SIZE * 2; ++yPos) {
		byte *pSrcTemp = pSrc;

		// Left edge
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrcTemp++;
		
		// Middle section
		for (xPos = 0; xPos < _surface->width() - TALK_DIALOG_EDGE_SIZE * 2; ++xPos)
			*pDest++ = *pSrcTemp;
		++pSrcTemp;

		// Right edge
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrcTemp++;
	}

	//  Bottom section
	pSrc += TALK_DIALOG_EDGE_SIZE * 2 + 1;
	for (yPos = 0; yPos < TALK_DIALOG_EDGE_SIZE; ++yPos) {
		for (xPos = 0; xPos < TALK_DIALOG_EDGE_SIZE; ++xPos)
			*pDest++ = *pSrc++;

		for (xPos = 0; xPos < TALK_DIALOG_WIDTH - TALK_DIALOG_EDGE_SIZE - 2; ++xPos) 
			*pDest++ = *pSrc;
		++pSrc;

		*pDest++ = *pSrc++;
		*pDest++ = *pSrc++;
	}

	// Write out the character name
	uint16 charWidth = Surface::textWidth(srcCharName);
	_surface->writeString((TALK_DIALOG_WIDTH-charWidth)/2, TALK_DIALOG_EDGE_SIZE + 2,
		srcCharName, true, DIALOG_WHITE_COLOUR);

	// TEMPORARY CODE - write out description. More properly, the text is meant to
	// be displayed slowly, word by word
	for (int lineCtr = 0; lineCtr < _numLines; ++lineCtr) 
		_surface->writeString(TALK_DIALOG_EDGE_SIZE + 2, 
			TALK_DIALOG_EDGE_SIZE + 4 + (lineCtr + 1) * FONT_HEIGHT,
			_lines[lineCtr], true);
}

TalkDialog::~TalkDialog() {
	delete _lines;
	delete _surface;
}

/*--------------------------------------------------------------------------*/

#define SR_SEPARATOR_Y 21
#define SR_SEPARATOR_X 5
#define SR_SEPARATOR_HEIGHT 5
#define SR_SAVEGAME_NAMES_Y (SR_SEPARATOR_Y + SR_SEPARATOR_HEIGHT + 1)

void SaveRestoreDialog::toggleHightlight(int xs, int xe, int ys, int ye) {
	Screen &screen = Screen::getReference();
	byte *addr = screen.screen().data().data() + FULL_SCREEN_WIDTH * ys + xs;

	for (int y = 0; y < ye - ys + 1; ++y, addr += FULL_SCREEN_WIDTH) {
		for (int x = 0; x < xe - xs + 1; ++x) {
			if (addr[x] == DIALOG_TEXT_COLOUR) addr[x] = DIALOG_WHITE_COLOUR;
			else if (addr[x] == DIALOG_WHITE_COLOUR) addr[x] = DIALOG_TEXT_COLOUR;
		}
	}

	screen.update();
}

bool SaveRestoreDialog::show(bool saveDialog) {
	OSystem &system = *g_system;
	Screen &screen = Screen::getReference();
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Room &room = Room::getReference();
	Resources &res = Resources::getReference();
	LureEngine &engine = LureEngine::getReference();
	int selectedLine = -1;
	int index;

	// Figure out a list of present savegames
	String **saveNames = (String **)Memory::alloc(sizeof(String *) * MAX_SAVEGAME_SLOTS);
	int numSaves = 0;
	while ((numSaves < MAX_SAVEGAME_SLOTS) && 
		((saveNames[numSaves] = engine.detectSave(numSaves + 1)) != NULL))
		++numSaves;

	// For the save dialog, if all the slots have not been used up, create a 
	// blank entry for a new savegame
	if (saveDialog && (numSaves < MAX_SAVEGAME_SLOTS))
		saveNames[numSaves++] = new String();

	// For the restore dialog, if there are no savegames, return immediately
	if (!saveDialog && (numSaves == 0)) {
		Memory::dealloc(saveNames);
		return false;
	}

	room.update();
	Surface *s = new Surface(INFO_DIALOG_WIDTH, SR_SAVEGAME_NAMES_Y + 
		numSaves * FONT_HEIGHT + FONT_HEIGHT + 2);

	// Create the outer dialog and dividing line
	s->createDialog();
	byte *pDest = s->data().data() + (s->width() * SR_SEPARATOR_Y) + SR_SEPARATOR_X;
	uint8 rowColours[5] = {*(pDest-2), *(pDest-1), *(pDest-1), *(pDest-2), *(pDest+1)};
	for (int y = 0; y < SR_SEPARATOR_HEIGHT; ++y, pDest += s->width())
		memset(pDest, rowColours[y], s->width() - 12);

	// Create title line
	Common::String title(res.stringList().getString(
		saveDialog ? S_SAVE_GAME : S_RESTORE_GAME));
	s->writeString((s->width() - s->textWidth(title.c_str())) / 2, FONT_HEIGHT+2, title, true);

	// Write out any existing save names
	for (index = 0; index < numSaves; ++index)
		s->writeString(DIALOG_EDGE_SIZE, SR_SAVEGAME_NAMES_Y + (index * 8), saveNames[index]->c_str(), true);

	// Display the dialog
	s->copyTo(&screen.screen(), SAVE_DIALOG_X, SAVE_DIALOG_Y);
	screen.update();
	mouse.pushCursorNum(CURSOR_ARROW);

	bool abortFlag = false;
	bool doneFlag = false;
	while (!abortFlag && !doneFlag) {
		// Provide highlighting of lines to select a save slot
		while (!(mouse.lButton() && (selectedLine != -1)) && !mouse.rButton()) {
			abortFlag = events.quitFlag;
			if (abortFlag) break;

			if (events.pollEvent()) {
				if ((events.type() == Common::EVENT_KEYDOWN) &&
					(events.event().kbd.keycode == Common::KEYCODE_ESCAPE)) {
					abortFlag = true;
					break;
				}
				if (events.type() == Common::EVENT_MOUSEMOVE) {
					// Mouse movement
					int lineNum;
					if ((mouse.x() < (SAVE_DIALOG_X + DIALOG_EDGE_SIZE)) ||
						(mouse.x() >= (SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE)) ||
						(mouse.y() < SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y) ||
						(mouse.y() >= SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + numSaves * FONT_HEIGHT))
						// Outside displayed lines
						lineNum = -1;
					else
						lineNum = (mouse.y() - (SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y)) / FONT_HEIGHT;

					if (lineNum != selectedLine) {
						if (selectedLine != -1)
							// Deselect previously selected line
							toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
								SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);

						// Highlight new line
						selectedLine = lineNum;
						if (selectedLine != -1)
							toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
								SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
								SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);
					}
				}
			}

			system.updateScreen();
			system.delayMillis(10);
		}

		// Deselect selected row
		if (selectedLine != -1) 
			toggleHightlight(SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
				SAVE_DIALOG_X + s->width() - DIALOG_EDGE_SIZE,
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + (selectedLine + 1) * FONT_HEIGHT - 1);

		if (mouse.lButton() || mouse.rButton()) {
			abortFlag = mouse.rButton();
			mouse.waitForRelease();
		}
		if (abortFlag) break;

		// If in save mode, allow the entry of a new savename
		if (saveDialog) {
			if (!screen.screen().getString(*saveNames[selectedLine], 
				INFO_DIALOG_WIDTH - (DIALOG_EDGE_SIZE * 2), 
				false, true, SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
				SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT)) {
				// Aborted out of name selection, so restore old name and 
				// go back to slot selection
				screen.screen().writeString(
					SAVE_DIALOG_X + DIALOG_EDGE_SIZE, 
					SAVE_DIALOG_Y + SR_SAVEGAME_NAMES_Y + selectedLine * FONT_HEIGHT,
                    saveNames[selectedLine]->c_str(), true, DIALOG_TEXT_COLOUR, true);
				selectedLine = -1;
				continue;
			}
		}
		doneFlag = true;
	}

	if (doneFlag) {
		// Handle save or restore
		if (saveDialog)
			doneFlag = engine.saveGame(selectedLine + 1, *saveNames[selectedLine]);
		else
			doneFlag = engine.loadGame(selectedLine + 1);
	}

	mouse.popCursor();

	// Free savegame caption list
	for (index = 0; index < numSaves; ++index) delete saveNames[index];
	Memory::dealloc(saveNames);

	return doneFlag;
}

/*--------------------------------------------------------------------------*/

struct RestartRecordPos {
	int16 x, y;
};

struct RestartRecord {
	Common::Language Language;
	int16 width, height;
	RestartRecordPos BtnRestart;
	RestartRecordPos BtnRestore;
};

RestartRecord buttonBounds[] = {
	{ EN_ANY, 48, 14, { 118, 152 }, { 168, 152 } },
	{ DE_DEU, 48, 14, { 106, 152 }, { 168, 152 } },
	{ UNK_LANG, 48, 14, { 112, 152 }, { 168, 152 } }
};


bool RestartRestoreDialog::show()
{
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	LureEngine &engine = LureEngine::getReference();

	Sound.killSounds();
	Sound.musicInterface_Play(60, true, 0);
	mouse.setCursorNum(CURSOR_ARROW);

	// See if there are any savegames that can be restored
	String *firstSave = engine.detectSave(1);
	bool restartFlag = (firstSave == NULL);

	if (!restartFlag) {
		Memory::dealloc(firstSave);

		// Get the correct button bounds record to use
		RestartRecord *btnRecord = &buttonBounds[0];
		while ((btnRecord->Language != engine.getLanguage()) && 
			   (btnRecord->Language != UNK_LANG))
			++btnRecord;

		// Fade in the restart/restore screen
		Palette p(RESTART_RESOURCE_ID + 1);
		Surface *s = Surface::getScreen(RESTART_RESOURCE_ID);

		res.activeHotspots().clear();
		Hotspot *btnHotspot = new Hotspot();
		// Restart button
		btnHotspot->setSize(btnRecord->width, btnRecord->height);
		btnHotspot->setPosition(btnRecord->BtnRestart.x, btnRecord->BtnRestart.y);
		btnHotspot->setAnimation(0x184B);
		btnHotspot->copyTo(s);
		// Restore button
		btnHotspot->setFrameNumber(1);
		btnHotspot->setPosition(btnRecord->BtnRestore.x, btnRecord->BtnRestore.y);
		btnHotspot->copyTo(s);

		// Copy the surface to the screen
		screen.setPaletteEmpty();
		s->copyToScreen(0, 0);
		delete s;

		screen.paletteFadeIn(&p);

		events.waitForPress();
		screen.paletteFadeOut();

		//restartFlag = !SaveRestoreDialog::show(false);
	}

	Sound.killSounds();

	return restartFlag;
}

} // end of namespace Lure
