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
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike: Mark and Thomas Thurman.
 */

#include "common/random.h"
#include "common/system.h"

#include "graphics/cursorman.h"
#include "audio/mixer.h"
#include "audio/mods/protracker.h"

#include "avalanche/avalanche.h"
#include "avalanche/graphics.h"
#include "avalanche/intro.h"

namespace Avalanche {

Intro::Intro(AvalancheEngine *vm) : _vm(vm), _thisLine(0), _nextBitline(16), _cutOut(false), _displayCounter(0) {
	// Allocate planes on the heap to avoid large stack allocations
	for (int i = 0; i < 4; i++) {
		_planes[i] = new uint8[40 * 200];
	}
	_musicHandle = new Audio::SoundHandle();
}

Intro::~Intro() {
	// Delete planes
	for (int i = 0; i < 4; i++) {
		delete[] _planes[i];
	}
	delete _musicHandle;
}

void Intro::resetPlanes() {
	for (int i = 0; i < 4; i++) {
		memset(_planes[i], 0, 40 * 200);
	}
}

void Intro::loadText() {
	_textStrings.clear();
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("It is the Year of Our Lord 1179.");
	_textStrings.push_back("");
	_textStrings.push_back("Our story starts on a tiny island");
	_textStrings.push_back("off the coast of the Lleyn Peninsula,");
	_textStrings.push_back("Gwynedd, Wales. The isle is called");
	_textStrings.push_back("Ynys Enlli by those who live there,");
	_textStrings.push_back("but in English its name is BARDSEY.");
	_textStrings.push_back("");
	_textStrings.push_back("The island has been a monarchy for");
	_textStrings.push_back("many years. Tonight, the King");
	_textStrings.push_back("of Bardsey, SIR GERALD WINTERWATER,");
	_textStrings.push_back("sits down to a quiet game of shove");
	_textStrings.push_back("ha'penny with his beautiful only");
	_textStrings.push_back("daughter, GEIDA.");
	_textStrings.push_back("");
	_textStrings.push_back("Suddenly...");
	_textStrings.push_back("there is a rattling at the door.");
	_textStrings.push_back("`Owpen! Owpen the dowwur!\"");
	_textStrings.push_back("cries a voice.");
	_textStrings.push_back("");
	_textStrings.push_back("One of the King's few guards");
	_textStrings.push_back("hurries up.");
	_textStrings.push_back("");
	_textStrings.push_back("`Your Majesty,\" he cries, `there's a");
	_textStrings.push_back("Brummie knight without who craves");
	_textStrings.push_back("entrance!\"");
	_textStrings.push_back("");
	_textStrings.push_back("`Well, he can stay without for all");
	_textStrings.push_back("I care.\" says the King, who is annoyed");
	_textStrings.push_back("at being interrupted at a crucial stage");
	_textStrings.push_back("in his game. `Pass the mead.\"");
	_textStrings.push_back("");
	_textStrings.push_back("`But, Sire, he wants your daughter's");
	_textStrings.push_back("hand in marriage!\" says the guard.");
	_textStrings.push_back("");
	_textStrings.push_back("`He can go without her, too.\" the King");
	_textStrings.push_back("replies.");
	_textStrings.push_back("");
	_textStrings.push_back("Just at that moment, the door bursts");
	_textStrings.push_back("open, and a tall baron stands in the");
	_textStrings.push_back("doorway. `Sir Gerald!\" he declaims,");
	_textStrings.push_back("`I claim your daughter's hand!\"");
	_textStrings.push_back("");
	_textStrings.push_back("`Go away.\" says the King, waving his");
	_textStrings.push_back("ha'penny at the Baron.");
	_textStrings.push_back("");
	_textStrings.push_back("`Then, Sire, I shall hold your");
	_textStrings.push_back("daughter to ransom!\" He motions to one");
	_textStrings.push_back("of his guards, who strides forward to");
	_textStrings.push_back("grab her.");
	_textStrings.push_back("");
	_textStrings.push_back("She knocks him to the ground. But even");
	_textStrings.push_back("Geida cannot repel twelve strong");
	_textStrings.push_back("knights. Soon she is bound up");
	_textStrings.push_back("with rope.");
	_textStrings.push_back("");
	_textStrings.push_back("`Fifty pounds if you want her back.\"");
	_textStrings.push_back("smirks the Baron.");
	_textStrings.push_back("");
	_textStrings.push_back("`FIFTY POUNDS?!\" says the King faintly.");
	_textStrings.push_back("` 50? You could buy Essex for that!\"");
	_textStrings.push_back("");
	_textStrings.push_back("`Exactly.\" says the Baron. He claps his");
	_textStrings.push_back("hands, and the guards begin to drag");
	_textStrings.push_back("Geida away.");
	_textStrings.push_back("");
	_textStrings.push_back("`Wait!\" calls Sir Gerald. `I had a");
	_textStrings.push_back("letter from the Duke of Cardiff this");
	_textStrings.push_back("morning saying that he was going to");
	_textStrings.push_back("give away Essex and Cornwall in a");
	_textStrings.push_back("prize draw. Why don't you enter that?\"");
	_textStrings.push_back("");
	_textStrings.push_back("`Sir Gerald,\" says the Baron, `don't");
	_textStrings.push_back("be a fool. If you haven't got  50,");
	_textStrings.push_back("get it. Soon.\"");
	_textStrings.push_back("");
	_textStrings.push_back("He starts to go out of the door.");
	_textStrings.push_back("`Never trifle with Baron du Lustie!\"");
	_textStrings.push_back("");
	_textStrings.push_back("`You won't get away with this!\" calls");
	_textStrings.push_back("the King. `I have friends in high");
	_textStrings.push_back("places! I'll get the best knight in");
	_textStrings.push_back("England to win her back!\"");
	_textStrings.push_back("");
	_textStrings.push_back("`Who?\"");
	_textStrings.push_back("enquires the Baron sarcastically.");
	_textStrings.push_back("`Lord Avalot d'Argent?\"");
	_textStrings.push_back("");
	_textStrings.push_back("`I'm serious. Sir Roamalot of Romford.\"");
	_textStrings.push_back("");
	_textStrings.push_back("`You haven't got a hope, Sir Gerald.");
	_textStrings.push_back("Pay up.\" With that, du Lustie leaves");
	_textStrings.push_back("the throneroom. Soon he is on his way");
	_textStrings.push_back("back to Warwickshire, to lock Geida");
	_textStrings.push_back("up in his deepest dungeon...");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("To Be Continued...");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
	_textStrings.push_back("");
}

void Intro::plotStar(uint8 plane, int x, int y) {
	int offset = (y * 40) + (x / 8);
	int bit = 7 - (x % 8); // choose correct bit inside byte (MSB first, left→right pixels)
	_planes[plane][offset] |= (1 << bit);
}

void Intro::plotStars(uint8 plane, int y) {
	int times = 0;
	int r = _vm->_rnd->getRandomNumber(6);
	if (r == 1 || r == 2 || r == 3)
		times = r;
	else
		return;

	for (int i = 0; i < times; i++)
		plotStar(plane, _vm->_rnd->getRandomNumber(319), y);
}

void Intro::movePlanes() {
	// Stars move DOWN (Planes 0-2)
	for (int p = 0; p < 3; p++) {
		bool moveThisFrame = false;
		if ((p == 0 && (_displayCounter % 10 == 0)) || (p == 1 && (_displayCounter % 2 == 0)) || (p == 2))
			moveThisFrame = true;

		if (moveThisFrame) {
			// Shift plane index 0-2 (stars) DOWN by one row
			// Using memmove to shift 199 rows (7960 bytes) forward by 40 bytes
			memmove(_planes[p] + 40, _planes[p], 199 * 40);
			// Clear the top row
			memset(_planes[p], 0, 40);
			// Add new stars at the top
			plotStars(p, 0);
		}
	}

	// Text moves UP (Plane 3)
	memmove(_planes[3], _planes[3] + 40, 199 * 40);
	// Clear the bottom-most row
	memset(_planes[3] + 199 * 40, 0, 40);
}

void Intro::combineAndDraw() {
	for (int y = 0; y < 200; y++) {
		uint8 *dstRow = (uint8 *)_vm->_graphics->getSurface().getBasePtr(0, y); // Get the pointer to the starting pixel of each row
		for (int xByte = 0; xByte < 40; xByte++) {
			// Get bytes for particular row + position in 4 planes
			uint8 b0 = _planes[0][y * 40 + xByte];
			uint8 b1 = _planes[1][y * 40 + xByte];
			uint8 b2 = _planes[2][y * 40 + xByte];
			uint8 b3 = _planes[3][y * 40 + xByte];

			// Process 8 pixels inside the byte (bit by bit)
			for (int bit = 7; bit >= 0; bit--) {
				uint8 color = kColorBlack; // default bg color
				
				if (b3 & (1 << bit))
					color = kColorWhite;
				else if (b2 & (1 << bit))
					color = kColorLightgray;
				else if (b1 & (1 << bit))
					color = kColorLightblue;
				else if (b0 & (1 << bit))
					color = kColorBlue;

				uint screenX = (xByte * 8 + (7 - bit)) * 2;
				dstRow[screenX] = color;
				dstRow[screenX + 1] = color;
			}
		}
	}
}

void Intro::run() {
	CursorMan.showMouse(false);
	_vm->_graphics->menuRestoreScreen();

    // Load and play music (glover.mod)
    Common::File musicFile;
    if (musicFile.open("glover.mod")) {
        Audio::AudioStream *stream = Audio::makeProtrackerStream(&musicFile);
        if (stream) {
            _vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, _musicHandle, stream);
        }
    }

    loadText();
    resetPlanes();
    _thisLine = 0;
    _nextBitline = 16;
    _displayCounter = 0;
    _cutOut = false;

    // Load original font
    uint8 font[256][16];
    Common::File fontFile;
    if (fontFile.open("avalot.fnt")) {
        for (int i = 0; i < 256; i++)
			fontFile.read(font[i], 16);
        fontFile.close();
    } else {
		warning("Failed to load font avalot.fnt");
        memset(font, 0, sizeof(font)); // Clear the text plane if font file not found
    }

    // Pre-populate stars across the screen before we can start shifting
    for (int p = 0; p < 3; p++) {
        for (int y = 0; y < 200; y++)
			plotStars(p, y);
    }
	// Clear the text plane before we start shifting
	memset(_planes[3], 0, 200 * 40);

    // Buffer for the character line
    uint8 nextLineBuffer[40][16];

	while (!_vm->shouldQuit() && !_cutOut) {
		uint32 beginLoop = g_system->getMillis();

        _displayCounter++;
        if (_displayCounter > 40)
			_displayCounter = 1;

        movePlanes();

        if (_nextBitline == 16) {
            if (_thisLine >= (int)_textStrings.size()) {
				// Story over
                _cutOut = true;
            } else {
                memset(nextLineBuffer, 0, sizeof(nextLineBuffer));
                Common::String currentLine = _textStrings[_thisLine++];
                int start = (20 - (currentLine.size() / 2)) - 1;

                for (uint charIdx = 0; charIdx < currentLine.size(); charIdx++) {
                    uint8 ascii = (uint8)currentLine[charIdx];
                    for (int fontByte = 0; fontByte < 16; fontByte++) {
                        uint8 fontBits = font[ascii][fontByte];
						if (start + charIdx + 1 < 40)
							nextLineBuffer[start + charIdx + 1][fontByte] = fontBits;
                    }
                }
                _nextBitline = 0;
            }
        }

        // Draw current bitline of the text into the bottom-most row of Plane 3
        if (!_cutOut) {
            for (int charIdx = 0; charIdx < 40; charIdx++) {
                _planes[3][199 * 40 + charIdx] = nextLineBuffer[charIdx][_nextBitline];
            }
            _nextBitline++;
        }

		combineAndDraw();
		_vm->_graphics->refreshScreen();

		Common::Event event;
		while (_vm->getEvent(event)) {
			if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
                _vm->_mixer->stopHandle(*_musicHandle);
				CursorMan.showMouse(true);
				return;
			}
		}

		uint32 delay = g_system->getMillis() - beginLoop;
		if (delay < 55)
			g_system->delayMillis(55 - delay);
	}

    _vm->_mixer->stopHandle(*_musicHandle);
	CursorMan.showMouse(true);
}

} // End of namespace Avalanche
