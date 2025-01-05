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

#ifndef AGI_PREAGI_PREAGI_H
#define AGI_PREAGI_PREAGI_H

#include "agi/agi.h"

namespace Audio {
class SoundHandle;
class PCSpeaker;
}

namespace Agi {

// default attributes
#define IDA_DEFAULT     0x0F
#define IDA_DEFAULT_REV 0xF0

#define IDI_SND_OSCILLATOR_FREQUENCY    1193180
#define IDI_SND_TIMER_RESOLUTION        0.0182

#define kColorDefault 0x1337

#define IDI_MAX_ROW_PIC 20

enum SelectionTypes {
	kSelYesNo,
	kSelNumber,
	kSelSpace,
	kSelAnyKey,
	kSelBackspace
};

// Options for controlling behavior during waits and sound playback
enum WaitOptions {
	kWaitBlock          = 0x00, // no event processing, cannot be interrupted
	kWaitProcessEvents  = 0x01, // process events, stops on quit
	kWaitAllowInterrupt = 0x03  // process events, stops on input or quit
};

class PreAgiEngine : public AgiBase {
	int _gameId;

protected:
	void initialize() override;

	int getKeypress() override { return 0; }
	bool isKeypress() override { return false; }
	void clearKeyQueue() override {}

	PreAgiEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	~PreAgiEngine() override;
	int getGameId() const { return _gameId; }

	void clearImageStack() override {}
	void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                          int16 p4, int16 p5, int16 p6, int16 p7) override {}
	void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                          int16 p4, int16 p5, int16 p6, int16 p7) override {}
	void releaseImageStack() override {}
	int saveGame(const Common::String &fileName, const Common::String &saveName) { return -1; }
	int loadGame(const Common::String &fileName, bool checkId = true) { return -1; }

	// Game
	Common::String getTargetName() const { return _targetName; }

	// Screen
	void clearScreen(int attr, bool overrideDefault = true);
	void clearGfxScreen(int attr);
	void setDefaultTextColor(int attr) { _defaultColor = attr; }
	byte getWhite() const;

	// Keyboard
	int getSelection(SelectionTypes type);

	// Random number between 1 and max. Example: rnd(2) returns 1 or 2.
	int rnd(int max);

	// Text
	void drawStr(int row, int col, int attr, const char *buffer);
	void clearTextArea();
	void clearRow(int row);
	static void XOR80(char *buffer);
	void printStr(const char *szMsg);
	void printStrXOR(char *szMsg);

	// Saved Games
	Common::SaveFileManager *getSaveFileMan() { return _saveFileMan; }

	bool playSpeakerNote(int16 frequency, int32 length, WaitOptions options);
	bool wait(uint32 delay, WaitOptions options = kWaitProcessEvents);

private:
	int _defaultColor;

	Audio::PCSpeaker *_speakerStream;
	Audio::SoundHandle *_speakerHandle;
};

} // End of namespace Agi


#endif
