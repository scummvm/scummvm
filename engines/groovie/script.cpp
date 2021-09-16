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
 */

#include <limits.h>

#include "audio/mididrv.h"

#include "groovie/script.h"
#include "groovie/cell.h"
#include "groovie/cursor.h"
#include "groovie/graphics.h"
#include "groovie/music.h"
#include "groovie/player.h"
#include "groovie/resource.h"
#include "groovie/saveload.h"
#include "groovie/tlcgame.h"
#include "groovie/t11hgame.h"

#include "gui/saveload.h"

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/translation.h"

#include "gui/message.h"

const uint NUM_OPCODES = 91;

namespace Groovie {

// Adapted from SCRIPT.GRV
const byte t7gMidiInitScript[] = {
	0x1A, 0x00, 0x01, 0xB1, 0x12, 0x00,		// strcmpnejmp (if (var 0100 != 01) jmp 0012)
	0x02, 0x46, 0x4C,						// playsong 4C46 (GM init)
	0x03,									// bf9on (fade-in)
	0x09, 0x60, 0x24,						// videofromref 2460 (GM init video)
	0x09, 0x60, 0x24,						// videofromref 2460 (GM init video)
	0x04,									// palfadeout
	0x29,									// stopmidi
	0x1A, 0x00, 0x01, 0xB2, 0x21, 0x00,		// :0012 - strcmpnejmp (if (var 0100 != 02) jmp 0021)
	0x02, 0x45, 0x4C,						// playsong 4C45 (MT-32 init)
	0x03,									// bf9on (fade-in)
	0x09, 0x61, 0x24,						// videofromref 2461 (MT-32 init video)
	0x04,									// palfadeout
	0x29,									// stopmidi
	0x31, 0x63, 0x00, 0x00, 0x00,			// :0021 - midivolume 0063, 0000
	0x3C,									// checkvalidsaves
	0x43, 0x00								// returnscript 00
};

Script::Script(GroovieEngine *vm, EngineVersion version) :
	_code(NULL), _savedCode(NULL), _stacktop(0), _debugger(NULL), _vm(vm),
	_videoFile(NULL), _videoRef(UINT_MAX), _staufsMove(NULL), _lastCursor(0xff),
	_version(version), _random("GroovieScripts"), _tlcGame(0), _t11hGame(0) {

	// Initialize the opcode set depending on the engine version
	if (version == kGroovieT7G) {
		_opcodes = _opcodesT7G;
	} else {
		_opcodes = _opcodesV2;
	}

	// Prepare the variables
	_bitflags = 0;
	for (int i = 0; i < 0x400; i++) {
		setVariable(i, 0);
	}

	// Initialize the music type variable
	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB | MDT_PREFER_GM);
	if (MidiDriver::getMusicType(dev) == MT_ADLIB) {
		// MIDI through AdLib
		setVariable(0x100, 0);
	} else if ((MidiDriver::getMusicType(dev) == MT_MT32) || ConfMan.getBool("native_mt32")) {
		// MT-32
		setVariable(0x100, 2);
	} else {
		// GM
		setVariable(0x100, 1);
	}

	_hotspotTopAction = 0;
	_hotspotBottomAction = 0;
	_hotspotRightAction = 0;
	_hotspotLeftAction = 0;
	_hotspotSlot = (uint16)-1;

	_oldInstruction = (uint16)-1;
	_videoSkipAddress = 0;
}

Script::~Script() {
	delete[] _code;
	delete[] _savedCode;
	delete _videoFile;
	delete _staufsMove;
	delete _tlcGame;
	delete _t11hGame;
}

void Script::setVariable(uint16 variablenum, byte value) {
	_variables[variablenum] = value;
	debugC(1, kDebugScriptvars, "script variable[0x%03X] = %d (0x%04X)", variablenum, value, value);
}

void Script::setBitFlag(int bitnum, bool value) {
	if (value) {
		_bitflags |= (1 << bitnum);
	} else {
		_bitflags &= ~(1 << bitnum);
	}
}

bool Script::getBitFlag(int bitnum) {
	return _bitflags & (1 << bitnum);
}

void Script::setDebugger(Debugger *debugger) {
	_debugger = debugger;
}

void Script::timerTick() {
	setVariable(0x103, _variables[0x103] + 1);
}

bool Script::loadScript(Common::String filename) {
	Common::SeekableReadStream *scriptfile = 0;

	if (_vm->_macResFork) {
		// Try to open the script file from the resource fork
		scriptfile = _vm->_macResFork->getResource(filename);
	} else {
		// Try to open the script file
		scriptfile = SearchMan.createReadStreamForMember(filename);
	}

	if (!scriptfile)
		return false;

	// Save the script filename
	_scriptFile = filename;

	// Load the code
	_codeSize = scriptfile->size();
	_code = new byte[_codeSize];
	if (!_code)
		return false;
	scriptfile->read(_code, _codeSize);
	delete scriptfile;

	// Patch the loaded code for known script bugs
	if (filename.equals("dr.grv")) {
		// WORKAROUND for the cake puzzle glitch (bug #4050): Lowering the
		// piece on the first column and second row updates the wrong script
		// variable
		assert(_codeSize == 5546);
		_code[0x03C2] = 0x38;
	} else if (filename.equals("maze.grv")) {
		// GRAPHICS ENHANCEMENT - Leave a skeleton in the maze.
		// Replaces one normal T intersection with the unused(?)
		// skeleton T intersection graphics.
		assert(_codeSize == 3652);

		// Terminating T branch
		_code[0x0769] = 0x46;
		_code[0x0774] = 0x3E;
		_code[0x077A] = 0x42;

		// T with branch on right
		_code[0x08E2] = 0x43;
		_code[0x08D7] = 0x44;
		_code[0x08E8] = 0x45;

		// T with branch on left
		_code[0x0795] = 0x41;
		_code[0x078A] = 0x40;
		_code[0x079B] = 0x3F;
	}

	// Initialize the script
	_currentInstruction = 0;

	return true;
}

void Script::directGameLoad(int slot) {
	// Reject invalid slots
	if (slot < 0 || slot > MAX_SAVES - 1) {
		return;
	}

	// Return to the main script if required
	if (_savedCode) {
		// Returning the correct spot, dealing with _savedVariables, etc
		// is not needed as game state is getting nuked anyway
		delete[] _code;
		_code = _savedCode;
		_codeSize = _savedCodeSize;
		_savedCode = nullptr;
	}

	uint16 targetInstruction;
	const byte *midiInitScript = 0;
	uint8 midiInitScriptSize = 0;

	// HACK: We set the slot to load in the appropriate variable, and set the
	// current instruction to the one that actually loads the saved game
	// specified in that variable. This differs depending on the game and its
	// version.
	if (_version == kGroovieT7G) {
		// 7th Guest
		setVariable(0x19, slot);
		targetInstruction = 0x287;
		// TODO Not sure if this works on or is necessary for Mac or iOS
		// versions. Disabling it to prevent breaking game loading.
		if (_vm->getPlatform() == Common::kPlatformDOS) {
			midiInitScript = t7gMidiInitScript;
			midiInitScriptSize = sizeof(t7gMidiInitScript);
		}
	} else {
		// 11th Hour
		setVariable(0xF, slot);
		// FIXME: This bypasses a lot of the game's initialization procedure
		targetInstruction = 0xE78E;
	}

	if (midiInitScript && !_vm->_musicPlayer->isMidiInit()) {
		// Run the MIDI init script as a subscript.

		// Backup the current script state
		_savedCode = _code;
		_savedCodeSize = _codeSize;
		_savedStacktop = _stacktop;
		_savedScriptFile = _scriptFile;
		// Set the game load instruction as the backup instruction. This
		// will run when the subscript returns.
		_savedInstruction = targetInstruction;

		// Set the MIDI init script as the current script.
		_codeSize = midiInitScriptSize;
		_code = new byte[_codeSize];
		memcpy(_code, midiInitScript, _codeSize);
		_stacktop = 0;
		_currentInstruction = 0;
	} else {
		// No MIDI initialization necessary. Just jump to the game load
		// instruction.
		_currentInstruction = targetInstruction;
		// Due to HACK above, the call to check valid save slots is not run.
		// As this is where we load save names, manually call it here.
		o_checkvalidsaves();
	}
}

void Script::step() {
	// Prepare the base debug string
	_debugString = _scriptFile + Common::String::format("@0x%04X: ", _currentInstruction);

	// Get the current opcode
	byte opcode = readScript8bits();
	_firstbit = ((opcode & 0x80) != 0);
	opcode = opcode & 0x7F;

	// Show the opcode debug string
	_debugString += Common::String::format("op 0x%02X: ", opcode);

	// Only output if we're not re-doing the previous instruction
	if (_currentInstruction != _oldInstruction) {
		debugCN(1, kDebugScript, "%s", _debugString.c_str());

		_oldInstruction = _currentInstruction;
	}

	// Detect invalid opcodes
	if (opcode >= NUM_OPCODES) {
		o_invalid();
		return;
	}

	// Execute the current opcode
	OpcodeFunc op = _opcodes[opcode];
	(this->*op)();
}

void Script::setMouseClick(uint8 button) {
	_eventMouseClicked = button;
}

void Script::setKbdChar(uint8 c) {
	_eventKbdChar = c;
}

Common::String &Script::getContext() {
	return _debugString;
}

uint8 Script::getCodeByte(uint16 address) {
	if (address >= _codeSize)
		error("Trying to read a script byte at address 0x%04X, while the "
			"script is just 0x%04X bytes long", address, _codeSize);
	return _code[address];
}

uint8 Script::readScript8bits() {
	uint8 data = getCodeByte(_currentInstruction);
	_currentInstruction++;
	return data;
}

uint8 Script::readScriptVar() {
	uint8 data = _variables[readScript8or16bits()];
	return data;
}

uint16 Script::readScript16bits() {
	uint8 lower = readScript8bits();
	uint8 upper = readScript8bits();
	return lower | (upper << 8);
}

uint32 Script::readScript32bits() {
	uint16 lower = readScript16bits();
	uint16 upper = readScript16bits();
	return lower | (upper << 16);
}

uint16 Script::readScript8or16bits() {
	if (_firstbit) {
		return readScript8bits();
	} else {
		return readScript16bits();
	}
}

uint8 Script::readScriptChar(bool allow7C, bool limitVal, bool limitVar) {
	uint8 result;
	uint8 data = readScript8bits();

	if (limitVal) {
		data &= 0x7F;
	}

	if (allow7C && (data == 0x7C)) {
		// Index a bidimensional array
		uint8 parta, partb;
		parta = readScriptChar(false, false, false);
		partb = readScriptChar(false, true, true);
		result = _variables[0x0A * parta + partb + 0x19];
	} else if (data == 0x23) {
		// Index an array
		data = readScript8bits();
		if (limitVar) {
			data &= 0x7F;
		}
		result = _variables[data - 0x61];
	} else {
		// Immediate value
		result = data - 0x30;
	}
	return result;
}

void Script::readScriptString(Common::String &str) {
	byte c;

	while ((c = readScript8bits())) {
		switch (c) {
		case 0x23:
			c = readScript8bits();
			c = _variables[c - 0x61] + 0x30;
			if (_version == kGroovieT7G) {
				if (c >= 0x41 && c <= 0x5A) {
					c += 0x20;
				}
			}
			break;
		case 0x7C:
			uint8 parta, partb;
			parta = readScriptChar(false, false, false);
			partb = readScriptChar(false, false, false);
			c = _variables[0x0A * parta + partb + 0x19] + 0x30;
			break;
		default:
			if (_version == kGroovieT7G) {
				if (c >= 0x41 && c <= 0x5A) {
					c += 0x20;
				}
			}
		}
		// Append the current character at the end of the string
		str += c;
	}
}

uint32 Script::getVideoRefString(Common::String &resName) {
	// Read String from Script (includes variable values)
	readScriptString(resName);

	// Add a trailing dot
	resName += 0x2E;

	debugCN(0, kDebugScript, "%s", resName.c_str());

	// Extract the script name.
	Common::String scriptname(_scriptFile.c_str(), _scriptFile.size() - 4);

	// Get the fileref of the resource
	return _vm->_resMan->getRef(resName, scriptname);
}

bool Script::hotspot(Common::Rect rect, uint16 address, uint8 cursor) {
	// Test if the current mouse position is contained in the specified rectangle
	Common::Point mousepos = _vm->_system->getEventManager()->getMousePos();
	bool contained = rect.contains(mousepos);

	// Show hotspots when debugging
	if (DebugMan.isDebugChannelEnabled(kDebugHotspots)) {
		if (_vm->_graphicsMan->isFullScreen())
			rect.translate(0, -80);
		Graphics::Surface *gamescreen = _vm->_system->lockScreen();
		gamescreen->frameRect(rect, 0xcc2338ff);
		_vm->_system->unlockScreen();
		_vm->_system->updateScreen();
	}

	// If there's an already planned action, do nothing
	if (_inputAction != -1) {
		return false;
	}

	if (contained) {
		// Change the mouse cursor
		if (_newCursorStyle == 5) {
			_newCursorStyle = cursor;
		}

		// If clicked with the mouse, jump to the specified address
		if (_mouseClicked) {
			_lastCursor = cursor;
			_inputAction = address;
		}
	}

	return contained;
}

void Script::loadgame(uint slot) {
	_vm->_musicPlayer->stop();

	Common::InSaveFile *file = SaveLoad::openForLoading(ConfMan.getActiveDomainName(), slot);

	// Loading the variables. It is endian safe because they're byte variables
	file->read(_variables, 0x400);

	delete file;

	// Hide the mouse cursor
	_vm->_grvCursorMan->show(false);
}

bool Script::preview_loadgame(uint slot) { // used by Clandestiny for the photos
	Common::InSaveFile *file = SaveLoad::openForLoading(ConfMan.getActiveDomainName(), slot);

	if (!file)
		return false;

	// Loading the variables. It is endian safe because they're byte variables
	uint32 size = 21;
	uint32 bytes_read = file->read(_variables, size);
	delete file;

	if (bytes_read < size)
		return false;

	return true;
}

bool Script::canDirectSave() const {
	// Disallow when running a subscript
	return _savedCode == nullptr;
}

void Script::directGameSave(int slot, const Common::String &desc) {
	if (slot < 0 || slot > MAX_SAVES - 1) {
		return;
	}
	const char *saveName = desc.c_str();
	for (int i = 0; i < 15; i++) {
		_variables[i] = saveName[i] - 0x30;
	}
	savegame(slot);
}

void Script::savegame(uint slot) {
	char save[15];
	char newchar;
	Common::OutSaveFile *file = SaveLoad::openForSaving(ConfMan.getActiveDomainName(), slot);

	if (!file) {
		debugC(9, kDebugScript, "Save file pointer is null");
		GUI::MessageDialog dialog(_("Failed to save game"), _("OK"));
		dialog.runModal();
		return;
	}

	// Saving the variables. It is endian safe because they're byte variables
	file->write(_variables, 0x400);
	delete file;

	// Cache the saved name
	for (int i = 0; i < 15; i++) {
		newchar = _variables[i] + 0x30;
		if ((newchar < 0x30 || newchar > 0x39) && (newchar < 0x41 || newchar > 0x7A) && newchar != 0x2E) {
			save[i] = '\0';
			break;
		} else if (newchar == 0x2E) { // '.', generated when space is pressed
			save[i] = ' ';
		} else {
			save[i] = newchar;
		}
	}
	_saveNames[slot] = save;
}

void Script::printString(Graphics::Surface *surface, const char *str) {
	char message[15];
	memset(message, 0, 15);

	// Preprocess the string
	for (int i = 0; i < 14; i++) {
		if (str[i] <= 0x00 || str[i] == 0x24)
			break;
		message[i] = str[i];
	}
	Common::rtrim(message);

	// Draw the string
	if (_version == kGroovieT7G) {
		_vm->_font->drawString(surface, message, 0, 16, 640, 0xE2, Graphics::kTextAlignCenter);
	} else {
		_vm->_font->drawString(surface, message, 190, 190, 640, _vm->_pixelFormat.RGBToColor(0xff, 0x0A, 0x0A), Graphics::kTextAlignLeft);
	}
}

// OPCODES

void Script::o_invalid() {
	error("Groovie::Script: Invalid opcode");
}

void Script::o_nop() {
	debugC(1, kDebugScript, "Groovie::Script: NOP");
}

void Script::o_nop8() {
	uint8 tmp = readScript8bits();
	debugC(1, kDebugScript, "Groovie::Script: NOP8: 0x%02X", tmp);
}

void Script::o_nop16() {
	uint16 tmp = readScript16bits();
	debugC(1, kDebugScript, "Groovie::Script: NOP16: 0x%04X", tmp);
}

void Script::o_nop32() {
	uint32 tmp = readScript32bits();
	debugC(1, kDebugScript, "Groovie::Script: NOP32: 0x%08X", tmp);
}

void Script::o_nop8or16() {
	uint16 tmp = readScript8or16bits();
	debugC(1, kDebugScript, "Groovie::Script: NOP8OR16: 0x%04X", tmp);
}

void Script::o_playsong() {			// 0x02
	uint16 fileref = readScript16bits();
	debugC(1, kDebugScript, "Groovie::Script: PlaySong(0x%04X): Play xmidi file", fileref);
	if (fileref == 0x4C17) {
		warning("Groovie::Script: this song is special somehow");
		// don't save the reference?
	}
	_vm->_musicPlayer->playSong(fileref);
}

void Script::o_bf9on() {			// 0x03
	debugC(1, kDebugScript, "Groovie::Script: BF9ON: bitflag 9 turned on");
	_bitflags |= 1 << 9;
}

void Script::o_palfadeout() {
	debugC(1, kDebugScript, "Groovie::Script: PALFADEOUT");
	debugC(2, kDebugVideo, "Groovie::Script: PALFADEOUT");
	_vm->_graphicsMan->fadeOut();
}

void Script::o_bf8on() {			// 0x05
	debugC(1, kDebugScript, "Groovie::Script: BF8ON: bitflag 8 turned on");
	_bitflags |= 1 << 8;
}

void Script::o_bf6on() {			// 0x06
	debugC(1, kDebugScript, "Groovie::Script: BF6ON: bitflag 6 turned on");
	_bitflags |= 1 << 6;
}

void Script::o_bf7on() {			// 0x07
	debugC(1, kDebugScript, "Groovie::Script: BF7ON: bitflag 7 turned on");
	_bitflags |= 1 << 7;
}

void Script::o2_bf0on() { // v2 0x0A
	debugC(1, kDebugScript, "Groovie::Script: BF0ON: bitflag 0 turned on");
	_bitflags |= 1;
}

void Script::o_setbackgroundsong() {			// 0x08
	uint16 fileref = readScript16bits();
	debugC(1, kDebugScript, "Groovie::Script: SetBackgroundSong(0x%04X)", fileref);
	_vm->_musicPlayer->setBackgroundSong(fileref);
}

void Script::o_videofromref() {			// 0x09
	uint16 fileref = readScript16bits();

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(1, kDebugScript, "Groovie::Script: VIDEOFROMREF(0x%04X) (Not fully imp): Play video file from ref", fileref);
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d via 0x09 (VideoFromRef)", _currentInstruction-3, fileref);
	}
	switch (fileref) {
	case 0x1C03:	// Trilobyte logo
	case 0x1C04:	// Virgin logo
	case 0x1C05:	// Credits
		if (fileref != _videoRef) {
			debugC(1, kDebugScript, "Groovie::Script: Use external file if available");
		}
		break;

	case 0x400D:	// floating objects in music room
	case 0x5060:	// a sound from gamwav?
	case 0x5098:	// a sound from gamwav?
	case 0x2402:	// House becomes book in intro?
	case 0x1426:	// Turn to face front in hall: played after intro
	case 0x206D:	// Cards on table puzzle (bedroom)
	case 0x2001:	// Coins on table puzzle (bedroom)
		if (fileref != _videoRef) {
			debugCN(1, kDebugScript, "Groovie::Script:  (This video is special somehow!)");
			warning("Groovie::Script: (This video (0x%04X) is special somehow!)", fileref);
		}
		break;

	case 0x2420:	// load from the main menu
		if (_version == kGroovieT7G && !ConfMan.getBool("originalsaveload")) {
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
			int slot = dialog->runModalWithCurrentTarget();
			delete dialog;

			if (slot >= 0) {
				_currentInstruction = 0x287;
				_bitflags = 0;
				setVariable(0x19, slot);
			} else {
				_currentInstruction = 0x016;	// back to main menu (load game / new game)
			}

			return;
		}
		break;

	case 0x2422: // save from the in-game menu
		if (_version == kGroovieT7G && !ConfMan.getBool("originalsaveload")) {
			GUI::MessageDialog saveOrLoad(_("Would you like to save or restore a game?"), _("Save"), _("Restore"));

			int choice = saveOrLoad.runModal();
			if (choice == GUI::kMessageOK) {
				// Save
				GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
				int slot = dialog->runModalWithCurrentTarget();
				Common::String saveName = dialog->getResultString();
				delete dialog;

				if (slot >= 0) {
					directGameSave(slot, saveName);
				}

				_currentInstruction = 0x17C8; // back to game menu
			} else {
				// Restore
				GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
				int slot = dialog->runModalWithCurrentTarget();
				delete dialog;

				if (slot >= 0) {
					_currentInstruction = 0x287;
					_bitflags = 0;
					setVariable(0x19, slot);
				} else {
					_currentInstruction = 0x17C8; // back to game menu
				}
			}

			return;
		}
		break;

	default:
		break;
	}
	if (fileref != _videoRef) {
		debugCN(1, kDebugScript, "\n");
	}

	// Determine if the MT-32 or GM initialization video is being played
	const bool enhancedMusicTracksExist = _version == kGroovieT7G && Common::File::exists("gu16.ogg");
	const bool gmInitVideo = _version == kGroovieT7G && fileref == 0x2460 && !enhancedMusicTracksExist;
	const bool mt32InitVideo = _version == kGroovieT7G && fileref == 0x2461 && !enhancedMusicTracksExist;
	// Play the video
	// If a MIDI init video is being played, loop it until the "audio"
	// (init commands) has finished playing
	if (!playvideofromref(fileref, gmInitVideo || mt32InitVideo)) {
		// Move _currentInstruction back
		_currentInstruction -= 3;
	} else if (gmInitVideo || mt32InitVideo) {
		// MIDI initialization has completed. Set this on the music player,
		// so that MIDI init will not be done again on game load.
		_vm->_musicPlayer->setMidiInit(true);
		if (gmInitVideo)
			// The script plays the GM init video twice to give the "audio"
			// enough time to play. It has just looped until the audio finished,
			// so the second play is no longer necessary.
			// Skip the next instruction.
			_currentInstruction += 3;
	}
}

bool Script::playvideofromref(uint32 fileref, bool loopUntilAudioDone) {
	// It isn't the current video, open it
	if (fileref != _videoRef) {

		// Debug bitflags
		debugCN(1, kDebugScript, "Groovie::Script: Play video 0x%04X (bitflags:", fileref);
		for (int i = 15; i >= 0; i--) {
			debugCN(1, kDebugScript, "%d", _bitflags & (1 << i)? 1 : 0);
			if (i % 4 == 0) {
				debugCN(1, kDebugScript, " ");
			}
		}
		debugC(1, kDebugScript, " <- 0)");

		// Close the previous video file
		if (_videoFile) {
			_videoRef = UINT_MAX;
			delete _videoFile;
		}

		// Try to open the new file
		_videoFile = _vm->_resMan->open(fileref);

		if (_videoFile) {
			_videoRef = fileref;
			// If teeth or mask cursor, and in main script, mark video prefer low-speed.
			// Filename check as sometimes teeth used for puzzle movements (bishops)
			if (_version == kGroovieT7G && (_lastCursor == 7 || _lastCursor == 4) && _scriptFile == "script.grv")
				_bitflags |= (1 << 15);
			_vm->_videoPlayer->load(_videoFile, _bitflags);
		} else {
			error("Groovie::Script: Couldn't open file");
			return true;
		}

		// Reset the clicked mouse events
		_eventMouseClicked = 0;
	}

	// Check if the user wants to skip the video
	if ((_eventMouseClicked == 2) && (_videoSkipAddress != 0)) {
		// Jump to the given address
		_currentInstruction = _videoSkipAddress;

		// Reset the skip address
		_videoSkipAddress = 0;

		_bitflags = 0;

		// End the playback
		return true;
	} else if (_eventMouseClicked == 2) {
		_vm->_videoPlayer->fastForward();
		_eventMouseClicked = 0;
	}

	// Video available, play one frame
	if (_videoFile) {
		bool endVideo = _vm->_videoPlayer->playFrame();
		_vm->_musicPlayer->frameTick();

		if (endVideo && loopUntilAudioDone && _vm->_musicPlayer->isPlaying()) {
			// The video has ended, but the audio hasn't. Loop the video.
			_videoFile->seek(0);
			// Clear bit flag 9 (fade-in)
			_vm->_videoPlayer->load(_videoFile, _bitflags & ~(1 << 9));
			return false;
		}

		if (endVideo || (loopUntilAudioDone && !_vm->_musicPlayer->isPlaying())) {
			// The video has ended, or it was being looped and the audio has ended.

			// Close the file
			delete _videoFile;
			_videoFile = NULL;
			_videoRef = UINT_MAX;

			// Clear the input events while playing the video
			_eventMouseClicked = 0;
			_eventKbdChar = 0;

			// Newline
			debugCN(1, kDebugScript, "\n");

			_bitflags = 0;

			// Let the caller know if the video has ended
			return true;
		}

		// The video has not ended yet.
		return false;
	}

	// If the file is closed, finish the playback
	_bitflags = 0;
	return true;
}

bool Script::playBackgroundSound(uint32 fileref, uint32 loops) {
	if (fileref == UINT_MAX) {
		return false;
	}

	// Try to open the new file
	Common::SeekableReadStream *_soundFile = _vm->_resMan->open(fileref);

	if (_soundFile) {
		_vm->_soundQueue.queue(_soundFile, loops);
	} else {
		warning("Groovie::Script: Couldn't open file");
		return false;
	}

	return true;
}

void Script::o_bf5on() {			// 0x0A
	debugC(1, kDebugScript, "Groovie::Script: BF5ON: bitflag 5 turned on");
	_bitflags |= 1 << 5;
}

void Script::o_inputloopstart() {	//0x0B
	debugC(5, kDebugScript, "Groovie::Script: Input loop start");

	// For TLC the regions for many questions are in an extra database. Reset internal region counters
	if (_version == kGroovieTLC && _tlcGame != NULL) {
#ifdef ENABLE_GROOVIE2
		_tlcGame->getRegionRewind();
#endif
	}

	// Reset the input action and the mouse cursor
	_inputAction = -1;
	_newCursorStyle = 5;

	// Save the input loop address
	_inputLoopAddress = _currentInstruction - 1;

	// Save the current mouse state for the whole loop
	_mouseClicked = (_eventMouseClicked == 1);
	_eventMouseClicked = 0;

	// Save the current pressed character for the whole loop
	_kbdChar = _eventKbdChar;
	_eventKbdChar = 0;
}

void Script::o_keyboardaction() {
	uint8 val = readScript8bits();
	uint16 address = readScript16bits();

	// If there's an already planned action, do nothing
	if (_inputAction != -1) {
		debugC(5, kDebugScript, "Groovie::Script: Test key == 0x%02X @0x%04X - skipped", val, address);
		return;
	}

	// Check the typed key
	if (_kbdChar == val) {
		debugC(5, kDebugScript, "Groovie::Script: Test key == 0x%02X @0x%04X - match", val, address);

		// Exit the input loop
		_inputLoopAddress = 0;

		// Save the action address
		_inputAction = address;
	} else {
		debugC(5, kDebugScript, "Groovie::Script: Test key == 0x%02X @0x%04X", val, address);
	}
}

void Script::o_hotspot_rect() {
	uint16 left = readScript16bits();
	uint16 top = readScript16bits();
	uint16 right = readScript16bits();
	uint16 bottom = readScript16bits();
	uint16 address = readScript16bits();
	uint8 cursor = readScript8bits();

	// TLC: The regions for many questions are in an extra database
	if (_version == kGroovieTLC && left == 0 && top == 0 && right == 0 && bottom == 0 && _tlcGame != NULL) {
#ifdef ENABLE_GROOVIE2
		if (_tlcGame->getRegionNext(left, top, right, bottom) < 0) {
			debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-RECT(%d,%d,%d,%d) @0x%04X cursor=%d SKIPPED", left, top, right, bottom, address, cursor);
			return;
		}
#endif
	}

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-RECT(%d,%d,%d,%d) @0x%04X cursor=%d", left, top, right, bottom, address, cursor);

	// Mark the specified rectangle
	Common::Rect rect(left, top, right, bottom);
	hotspot(rect, address, cursor);
}

void Script::o_hotspot_left() {
	uint16 address = readScript16bits();

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-LEFT @0x%04X", address);

	// Mark the leftmost 100 pixels of the game area
	Common::Rect rect(0, 80, 100, 400);
	hotspot(rect, address, 1);
}

void Script::o_hotspot_right() {
	uint16 address = readScript16bits();

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-RIGHT @0x%04X", address);

	// Mark the rightmost 100 pixels of the game area
	Common::Rect rect(540, 80, 640, 400);
	hotspot(rect, address, 2);
}

void Script::o_hotspot_center() {
	uint16 address = readScript16bits();

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-CENTER @0x%04X", address);

	// Mark the centermost 240 pixels of the game area
	Common::Rect rect(200, 80, 440, 400);
	hotspot(rect, address, 0);
}

void Script::o_hotspot_current() {
	uint16 address = readScript16bits();

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-CURRENT @0x%04X", address);

	// The original interpreter doesn't check the position, so accept the
	// whole screen
	Common::Rect rect(0, 0, 640, 480);
	hotspot(rect, address, 0);
}

void Script::o_inputloopend() {
	debugC(5, kDebugScript, "Groovie::Script: Input loop end");

	// Handle the predefined hotspots
	if (_hotspotTopAction) {
		Common::Rect rect(0, 0, 640, 80);
		hotspot(rect, _hotspotTopAction, _hotspotTopCursor);
	}
	if (_hotspotBottomAction) {
		Common::Rect rect(0, 400, 640, 480);
		hotspot(rect, _hotspotBottomAction, _hotspotBottomCursor);
	}
	if (_hotspotRightAction) {
		Common::Rect rect(560, 0, 640, 480);
		hotspot(rect, _hotspotRightAction, 2);
	}
	if (_hotspotLeftAction) {
		Common::Rect rect(0, 0, 80, 480);
		hotspot(rect, _hotspotLeftAction, 1);
	}

	// Actually execute the planned action
	if (_inputAction != -1) {
		// Jump to the planned address
		_currentInstruction = _inputAction;

		// Exit the input loop
		_inputLoopAddress = 0;
		_vm->_grvCursorMan->show(false);

		// Force immediate hiding of the mouse cursor (required when the next
		// video just contains audio)
		_vm->_graphicsMan->change();
	}

	// Nothing to do
	if (_inputLoopAddress) {
		if (_newCursorStyle != _vm->_grvCursorMan->getStyle()) {
			_vm->_grvCursorMan->setStyle(_newCursorStyle);
		}
		_vm->_grvCursorMan->show(true);

		// Go back to the begining of the loop
		_currentInstruction = _inputLoopAddress;

		// There's nothing to do until we get some input
		_vm->waitForInput();
	}
}

void Script::o_random() {
	uint16 varnum = readScript8or16bits();
	uint8 maxnum = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: RANDOM: var[0x%04X] = rand(%d)", varnum, maxnum);

	// TODO: Check if this is really different between the Engines
	if (_version == kGroovieT7G) {
		setVariable(varnum, _random.getRandomNumber(maxnum));
	} else {
		setVariable(varnum, _random.getRandomNumber(maxnum - 1));
	}
}

void Script::o_jmp() {
	uint16 address = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: JMP @0x%04X", address);

	// Set the current address
	_currentInstruction = address;
}

void Script::o_loadstring() {
	uint16 varnum = readScript8or16bits();

	debugCN(1, kDebugScript, "Groovie::Script: LOADSTRING var[0x%04X..] =", varnum);
	do {
		setVariable(varnum++, readScriptChar(true, true, true));
		debugCN(1, kDebugScript, " 0x%02X", _variables[varnum - 1]);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));
	debugCN(1, kDebugScript, "\n");
}

void Script::o_ret() {
	uint8 val = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: RET %d", val);

	// Set the return value
	setVariable(0x102, val);

	// Get the return address
	if (_stacktop > 0) {
		_stacktop--;
		_currentInstruction = _stack[_stacktop];
	} else {
		error("Groovie::Script: Return: Stack is empty");
	}
}

void Script::o_call() {
	uint16 address = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: CALL @0x%04X", address);

	// Save return address in the call stack
	_stack[_stacktop] = _currentInstruction;
	_stacktop++;

	// Change the current instruction
	_currentInstruction = address;
}

void Script::o_sleep() {
	uint16 time = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: SLEEP 0x%04X (%d ms)", time, time*3);

	_vm->_system->delayMillis(time * 3);
}

void Script::o_strcmpnejmp() {			// 0x1A
	uint16 varnum = readScript8or16bits();
	uint8 result = 1;

	debugCN(1, kDebugScript, "Groovie::Script: STRCMP-NEJMP: var[0x%04X..],", varnum);

	do {
		uint8 val = readScriptChar(true, true, true);

		if (_variables[varnum] != val) {
			result = 0;
		}
		varnum++;
		debugCN(1, kDebugScript, " 0x%02X", val);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	uint16 address = readScript16bits();
	if (!result) {
		debugC(1, kDebugScript, " jumping to @0x%04X", address);
		_currentInstruction = address;
	} else {
		debugC(1, kDebugScript, " not jumping");
	}
}

void Script::o_xor_obfuscate() {
	uint16 varnum = readScript8or16bits();

	debugCN(1, kDebugScript, "Groovie::Script: XOR OBFUSCATE: var[0x%04X..] = ", varnum);
	do {
		uint8 val = readScript8bits();
		_firstbit = ((val & 0x80) != 0);
		val &= 0x4F;

		setVariable(varnum, _variables[varnum] ^ val);
		debugCN(1, kDebugScript, "%c", _variables[varnum]);

		varnum++;
	} while (!_firstbit);
	debugCN(1, kDebugScript, "\n");
}

void Script::o_vdxtransition() {		// 0x1C
	uint16 fileref = readScript16bits();

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(1, kDebugScript, "Groovie::Script: VDX transition fileref = 0x%04X", fileref);
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d via 0x1C (VdxTransition)", _currentInstruction-3, fileref);
	}

	// Set bit 1
	_bitflags |= 1 << 1;

	// Clear bit 7
	_bitflags &= ~(1 << 7);

	// Set bit 2 if _firstbit
	if (_firstbit) {
		_bitflags |= 1 << 2;
	}

	// Play the video
	if (!playvideofromref(fileref)) {
		// Move _currentInstruction back
		_currentInstruction -= 3;
	}
}

void Script::o_swap() {
	uint16 varnum1 = readScript8or16bits();
	uint16 varnum2 = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: SWAP var[0x%04X] <-> var[0x%04X]", varnum1, varnum2);

	uint8 tmp = _variables[varnum1];
	setVariable(varnum1, _variables[varnum2]);
	setVariable(varnum2, tmp);
}

void Script::o_inc() {
	uint16 varnum = readScript8or16bits();

	debugC(1, kDebugScript, "Groovie::Script: INC var[0x%04X]", varnum);

	setVariable(varnum, _variables[varnum] + 1);
}

void Script::o_dec() {
	uint16 varnum = readScript8or16bits();

	debugC(1, kDebugScript, "Groovie::Script: DEC var[0x%04X]", varnum);

	setVariable(varnum, _variables[varnum] - 1);
}

void Script::o_strcmpnejmp_var() {			// 0x21
	uint16 data = readScriptVar();

	if (data > 9) {
		data -= 7;
	}
	data = _variables[data + 0x19];
	bool stringsmatch = 1;
	do {
		if (_variables[data++] != readScriptChar(true, true, true)) {
			stringsmatch = 0;
		}
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	uint16 offset = readScript16bits();
	if (!stringsmatch) {
		_currentInstruction = offset;
	}
}

void Script::o_copybgtofg() { // 0x22
	debugC(1, kDebugScript, "Groovie::Script: COPY_BG_TO_FG");
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: COPY_BG_TO_FG", _currentInstruction - 1);
	size_t len = _vm->_graphicsMan->_foreground.pitch * _vm->_graphicsMan->_foreground.h;
	memcpy(_vm->_graphicsMan->_foreground.getPixels(), _vm->_graphicsMan->_background.getPixels(), len);
}

void Script::o2_copybgtofg() { // 0x22
	debugC(1, kDebugScript, "Groovie::Script: COPY_SCREEN_TO_BG");
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: COPY_SCREEN_TO_BG", _currentInstruction - 1);

	Graphics::Surface *screen = _vm->_system->lockScreen();
	if (_vm->_graphicsMan->isFullScreen()) {
		_vm->_graphicsMan->_foreground.copyFrom(screen->getSubArea(Common::Rect(0, 0, 640, 480)));
	} else {
		_vm->_graphicsMan->_foreground.copyFrom(screen->getSubArea(Common::Rect(0, 80, 640, 400)));
	}
	_vm->_system->unlockScreen();
}

void Script::o_strcmpeqjmp() {			// 0x23
	uint16 varnum = readScript8or16bits();
	uint8 result = 1;

	debugCN(1, kDebugScript, "Groovie::Script: STRCMP-EQJMP: var[0x%04X..],", varnum);
	do {
		uint8 val = readScriptChar(true, true, true);

		if (_variables[varnum] != val) {
			result = 0;
		}
		varnum++;
		debugCN(1, kDebugScript, " 0x%02X", val);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	uint16 address = readScript16bits();
	if (result) {
		debugC(1, kDebugScript, " jumping to @0x%04X", address);
		_currentInstruction = address;
	} else {
		debugC(1, kDebugScript, " not jumping");
	}
}

void Script::o_mov() {
	uint16 varnum1 = readScript8or16bits();
	uint16 varnum2 = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: MOV var[0x%04X] = var[0x%04X]", varnum1, varnum2);

	setVariable(varnum1, _variables[varnum2]);
}

void Script::o_add() {
	uint16 varnum1 = readScript8or16bits();
	uint16 varnum2 = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: ADD var[0x%04X] += var[0x%04X]", varnum1, varnum2);

	setVariable(varnum1, _variables[varnum1] + _variables[varnum2]);
}

void Script::o_videofromstring1() {
	Common::String vidName;
	uint16 instStart = _currentInstruction;
	uint32 fileref = getVideoRefString(vidName);

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(0, kDebugScript, "Groovie::Script: VIDEOFROMSTRING1 %d ('%s')", fileref, vidName.c_str());
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d ('%s') via 0x26 (VideoFromString1)", instStart-1, fileref, vidName.c_str());
	}

	if (_version != kGroovieT7G) {
		// Clear bit 1
		_bitflags &= ~(1 << 1);
	}

	// Play the video
	if (!playvideofromref(fileref)) {
		// Move _currentInstruction back
		_currentInstruction = instStart - 1;
	}
}

void Script::o_videofromstring2() {
	Common::String vidName;
	uint16 instStart = _currentInstruction;
	uint32 fileref = getVideoRefString(vidName);

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(0, kDebugScript, "Groovie::Script: VIDEOFROMSTRING2 %d ('%s')", fileref, vidName.c_str());
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d ('%s') via 0x27 (VideoFromString2)", instStart-1, fileref, vidName.c_str());
	}

	// Set bit 1
	_bitflags |= 1 << 1;

	// Set bit 2 if _firstbit
	if (_firstbit) {
		_bitflags |= 1 << 2;
	}

	// Play the video
	if (!playvideofromref(fileref)) {
		// Move _currentInstruction back
		_currentInstruction = instStart - 1;
	}
}

void Script::o_stopmidi() {
	debugC(1, kDebugScript, "Groovie::Script: STOPMIDI (TODO)");
}

void Script::o_endscript() {
	debugC(1, kDebugScript, "Groovie::Script: END OF SCRIPT");
	_vm->quitGame();
}

void Script::o_sethotspottop() {
	uint16 address = readScript16bits();
	uint8 cursor = readScript8bits();

	debugC(5, kDebugScript, "Groovie::Script: SETHOTSPOTTOP @0x%04X cursor=%d", address, cursor);

	_hotspotTopAction = address;
	_hotspotTopCursor = cursor;
}

void Script::o_sethotspotbottom() {
	uint16 address = readScript16bits();
	uint8 cursor = readScript8bits();

	debugC(5, kDebugScript, "Groovie::Script: SETHOTSPOTBOTTOM @0x%04X cursor=%d", address, cursor);

	_hotspotBottomAction = address;
	_hotspotBottomCursor = cursor;
}

void Script::o_loadgame() {
	uint16 varnum = readScript8or16bits();
	uint8 slot = _variables[varnum];

	debugC(1, kDebugScript, "Groovie::Script: LOADGAME var[0x%04X] -> slot=%d (TODO)", varnum, slot);

	loadgame(slot);
	if (_version == kGroovieT7G) {
		_vm->_system->fillScreen(0);
	}
}

void Script::o_savegame() {
	uint16 varnum = readScript8or16bits();
	uint8 slot = _variables[varnum];

	debugC(1, kDebugScript, "Groovie::Script: SAVEGAME var[0x%04X] -> slot=%d (TODO)", varnum, slot);

	savegame(slot);
}

void Script::o_hotspotbottom_4() {	//0x30
	uint16 address = readScript16bits();

	debugC(5, kDebugScript, "Groovie::Script: HOTSPOT-BOTTOM @0x%04X", address);

	// Mark the 80 pixels under the game area
	Common::Rect rect(0, 400, 640, 480);
	hotspot(rect, address, 4);
}

void Script::o_midivolume() {
	uint16 arg1 = readScript16bits();
	uint16 arg2 = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: MIDI volume: %d %d", arg1, arg2);
	_vm->_musicPlayer->setGameVolume(arg1, arg2);
}

void Script::o_jne() {
	int16 varnum1 = readScript8or16bits();
	uint16 varnum2 = readScript16bits();
	uint16 address = readScript16bits();

	debugCN(1, kDebugScript, "Groovie::Script: JNE: var[var[0x%04X] - 0x31] != var[0x%04X] @0x%04X", varnum1, varnum2, address);

	if (_variables[_variables[varnum1] - 0x31] != _variables[varnum2]) {
		_currentInstruction = address;
		debugC(1, kDebugScript, " jumping to @0x%04X", address);
	} else {
		debugC(1, kDebugScript, " not jumping");
	}
}

void Script::o_loadstringvar() {
	uint16 varnum = readScript8or16bits();

	varnum = _variables[varnum] - 0x31;
	debugCN(1, kDebugScript, "Groovie::Script: LOADSTRINGVAR var[0x%04X..] =", varnum);
	do {
		setVariable(varnum++, readScriptChar(true, true, true));
		debugCN(1, kDebugScript, " 0x%02X", _variables[varnum - 1]);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));
	debugCN(1, kDebugScript, "\n");
}

void Script::o_chargreatjmp() {
	uint16 varnum = readScript8or16bits();
	uint8 result = 0;

	debugCN(1, kDebugScript, "Groovie::Script: CHARGREAT-JMP: var[0x%04X..],", varnum);
	do {
		uint8 val = readScriptChar(true, true, true);

		if (val < _variables[varnum]) {
			result = 1;
		}
		varnum++;
		debugCN(1, kDebugScript, " 0x%02X", val);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	uint16 address = readScript16bits();
	if (result) {
		debugC(1, kDebugScript, " jumping to @0x%04X", address);
		_currentInstruction = address;
	} else {
		debugC(1, kDebugScript, " not jumping");
	}
}

void Script::o_bf7off() {
	debugC(1, kDebugScript, "Groovie::Script: BF7OFF: bitflag 7 turned off");
	_bitflags &= ~(1 << 7);
}

void Script::o_charlessjmp() {
	uint16 varnum = readScript8or16bits();
	uint8 result = 0;

	debugCN(1, kDebugScript, "Groovie::Script: CHARLESS-JMP: var[0x%04X..],", varnum);
	do {
		uint8 val = readScriptChar(true, true, true);

		if (val > _variables[varnum]) {
			result = 1;
		}
		varnum++;
		debugCN(1, kDebugScript, " 0x%02X", val);
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	uint16 address = readScript16bits();
	if (result) {
		debugC(1, kDebugScript, " jumping to @0x%04X", address);
		_currentInstruction = address;
	} else {
		debugC(1, kDebugScript, " not jumping");
	}
}

void Script::o_copyrecttobg() {	// 0x37
	uint16 left = readScript16bits();
	uint16 top = readScript16bits();
	uint16 right = readScript16bits();
	uint16 bottom = readScript16bits();
	uint16 baseTop = (!_vm->_graphicsMan->isFullScreen()) ? 80 : 0;

	// Sanity checks to prevent bad pointer access crashes
	if (left > right) {
		warning("Groovie::Script: COPYRECT left:%d > right:%d", left, right);
		// swap over left and right parameters
		uint16 j;
		j = right;
		right = left;
		left = j;
	}
	if (top > bottom) {
		warning("Groovie::Script: COPYRECT top:%d > bottom:%d", top, bottom);
		// swap over top and bottom parameters
		uint16 j;
		j = bottom;
		bottom = top;
		top = j;
	}
	if (top < baseTop) {
		warning("Groovie::Script: COPYRECT top < baseTop... clamping");
		top = baseTop;
	}
	if (top >= 480) {
		warning("Groovie::Script: COPYRECT top >= 480... clamping");
		top = 480 - 1;
	}
	if (bottom >= 480) {
		warning("Groovie::Script: COPYRECT bottom >= 480... clamping");
		bottom = 480 - 1;
	}
	if (left >= 640) {
		warning("Groovie::Script: COPYRECT left >= 640... clamping");
		left = 640 - 1;
	}
	if (right >= 640) {
		warning("Groovie::Script: COPYRECT right >= 640... clamping");
		right = 640 - 1;
	}

	uint16 width = right - left, height = bottom - top;
	uint32 offset = 0;
	uint32 pitch = _vm->_graphicsMan->_foreground.pitch;

	debugC(1, kDebugScript, "Groovie::Script: COPYRECT((%d,%d)->(%d,%d))", left, top, right, bottom);
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: COPYRECT((%d,%d)->(%d,%d))",_currentInstruction-9, left, top, right, bottom);

	byte *fg = (byte *)_vm->_graphicsMan->_foreground.getBasePtr(left, top - baseTop);
	byte *bg = (byte *)_vm->_graphicsMan->_background.getBasePtr(left, top - baseTop);
	for (uint16 i = 0; i < height; i++) {
		memcpy(bg + offset, fg + offset, width * _vm->_graphicsMan->_foreground.format.bytesPerPixel);
		offset += pitch;
	}

	_vm->_system->copyRectToScreen(bg, pitch, left, top, width, height);
	_vm->_graphicsMan->change();
}

void Script::o_restorestkpnt() {
	debugC(1, kDebugScript, "Groovie::Script: Restore stack pointer from saved (TODO)");
}

void Script::o_obscureswap() {
	uint16 var1, var2, tmp;

	debugC(1, kDebugScript, "Groovie::Script: OBSCSWAP");

	// Read the first variable
	var1 = readScriptChar(false, true, true) * 10;
	var1 += readScriptChar(false, true, true) + 0x19;

	// Read the second variable
	var2 = readScriptChar(false, true, true) * 10;
	var2 += readScriptChar(false, true, true) + 0x19;

	// Swap the values
	tmp = _variables[var1];
	setVariable(var1, _variables[var2]);
	setVariable(var2, tmp);
}

void Script::o_printstring() {
	char stringstorage[15];
	uint8 counter = 0;

	debugC(1, kDebugScript, "Groovie::Script: PRINTSTRING");
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: PRINTSTRING", _currentInstruction - 1);

	memset(stringstorage, 0, 15);
	do {
		char newchar = readScriptChar(true, true, true) + 0x30;
		if (newchar < 0x30 || newchar > 0x39) {		// If character is invalid, chuck a space in
			if (newchar < 0x41 || newchar > 0x7A) {
				newchar = 0x20;
			}
		}

		stringstorage[counter] = newchar;
		counter++;
	} while (!(getCodeByte(_currentInstruction - 1) & 0x80));

	stringstorage[counter] = 0;

	Common::Rect topbar(640, 80);
	Graphics::Surface *gamescreen = _vm->_system->lockScreen();

	// Clear the top bar
	gamescreen->fillRect(topbar, 0);

	// Draw the string
	printString(gamescreen, stringstorage);

	_vm->_system->unlockScreen();
}

void Script::o_hotspot_slot() {
	uint16 slot = readScript8bits();
	uint16 left = readScript16bits();
	uint16 top = readScript16bits();
	uint16 right = readScript16bits();
	uint16 bottom = readScript16bits();
	uint16 address = readScript16bits();
	uint16 cursor = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: HOTSPOT-SLOT %d (%d,%d,%d,%d) @0x%04X cursor=%d (TODO)", slot, left, top, right, bottom, address, cursor);

	// Set rectangle according to the used engine. To remove the previously written text an the screen.
	Common::Rect removeText;
	if (_version == kGroovieT7G) {
		removeText.left = 0;
		removeText.top = 0;
		removeText.right = 640;
		removeText.bottom = 80;
	} else {
		// Only tested for 11th hour. TLC does not use this command.
		removeText.left = 120;
		removeText.top = 185;
		removeText.right = 400;
		removeText.bottom = 215;
	}

	Common::Rect rect(left, top, right, bottom);
	if (hotspot(rect, address, cursor)) {
		if (_hotspotSlot == slot) {
			return;
		}

		Graphics::Surface *gamescreen = _vm->_system->lockScreen();

		// Clear the top bar
		gamescreen->fillRect(removeText, 0);	// 0 works for both color formats (Groovie V1 and V2)

		printString(gamescreen, _saveNames[slot].c_str());

		_vm->_system->unlockScreen();

		// Save the currently highlighted slot
		_hotspotSlot = slot;
		_vm->_graphicsMan->change();

	} else {
		if (_hotspotSlot == slot) {
			Graphics::Surface *gamescreen;
			gamescreen = _vm->_system->lockScreen();

			gamescreen->fillRect(removeText, 0);	// 0 works for both color formats (Groovie V1 and V2)

			_vm->_system->unlockScreen();

			// Removing the slot highlight
			_hotspotSlot = (uint16)-1;

			_vm->_graphicsMan->change();
		}
	}
}

// Checks valid save games. Even for TLC (uses only 4 user save games) the function
// checks for 10 save games.
void Script::o_checkvalidsaves() {
	debugC(1, kDebugScript, "Groovie::Script: CHECKVALIDSAVES");

	// Reset the array of valid saves and the savegame names cache
	for (int i = 0; i < MAX_SAVES; i++) {
		setVariable(i, 0);
		_saveNames[i] = "E M P T Y";
	}

	// Get the list of savefiles
	SaveStateList list = SaveLoad::listValidSaves(ConfMan.getActiveDomainName());

	// Mark the existing savefiles as valid
	uint count = 0;
	SaveStateList::iterator it = list.begin();
	while (it != list.end()) {
		int8 slot = it->getSaveSlot();
		if (SaveLoad::isSlotValid(slot)) {
			debugC(2, kDebugScript, "Groovie::Script:  Found valid savegame: %s", it->getDescription().encode().c_str());

			// Mark this slot as used
			setVariable(slot, 1);

			// Cache this slot's description
			_saveNames[slot] = it->getDescription();
			count++;
		}
		it++;
	}

	// Save the number of valid saves
	setVariable(0x104, count);
	debugC(1, kDebugScript, "Groovie::Script:   Found %d valid savegames", count);
}

void Script::o_resetvars() {
	debugC(1, kDebugScript, "RESETVARS");
	for (int i = 0; i < 0x100; i++) {
		setVariable(i, 0);
	}
}

void Script::o_mod() {
	uint16 varnum = readScript8or16bits();
	uint8 val = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: MOD var[0x%04X] %%= %d", varnum, val);

	setVariable(varnum, _variables[varnum] % val);
}

void Script::o_loadscript() {
	Common::String filename;
	char c;

	while ((c = readScript8bits())) {
		filename += c;
	}
	debugC(1, kDebugScript, "Groovie::Script: LOADSCRIPT %s", filename.c_str());

	// Just 1 level of sub-scripts are allowed
	if (_savedCode) {
		error("Groovie::Script: Tried to load a level 2 sub-script");
	}

	// Save the current code
	_savedCode = _code;
	_savedCodeSize = _codeSize;
	_savedInstruction = _currentInstruction;

	// Save the filename of the current script
	_savedScriptFile = _scriptFile;

	// Load the sub-script
	if (!loadScript(filename)) {
		error("Groovie::Script: Couldn't load sub-script %s", filename.c_str());
	}

	// Save the current stack top
	_savedStacktop = _stacktop;

	// Save the variables
	memcpy(_savedVariables, _variables + 0x107, 0x180);
}

void Script::o_setvideoorigin() {
	// Read the two offset arguments
	int16 origX = readScript16bits();
	int16 origY = readScript16bits();

	// Set bitflag 7
	_bitflags |= 1 << 7;

	debugC(1, kDebugScript, "Groovie::Script: SetVideoOrigin(0x%04X,0x%04X) (%d, %d)", origX, origY, origX, origY);
	_vm->_videoPlayer->setOrigin(origX, origY);
}

void Script::o_sub() {
	uint16 varnum1 = readScript8or16bits();
	uint16 varnum2 = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: SUB var[0x%04X] -= var[0x%04X]", varnum1, varnum2);

	setVariable(varnum1, _variables[varnum1] - _variables[varnum2]);
}

void Script::o_cellmove() {
	uint16 depth = readScript8bits();
	byte *scriptBoard = &_variables[0x19];
	byte startX, startY, endX, endY;

	debugC(1, kDebugScript, "Groovie::Script: CELL MOVE var[0x%02X]", depth);

	if (!_staufsMove)
		_staufsMove = new CellGame;

	_staufsMove->playStauf(2, depth, scriptBoard);

	startX = _staufsMove->getStartX();
	startY = _staufsMove->getStartY();
	endX = _staufsMove->getEndX();
	endY = _staufsMove->getEndY();

	// Set the movement origin
	setVariable(0, startY); // y
	setVariable(1, startX); // x
	// Set the movement destination
	setVariable(2, endY);
	setVariable(3, endX);
}

void Script::o_returnscript() {
	uint8 val = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: RETURNSCRIPT @0x%02X", val);

	// Are we returning from a sub-script?
	if (!_savedCode) {
		error("Groovie::Script: Tried to return from the main script");
	}

	// Set the return value
	setVariable(0x102, val);

	// Restore the code
	delete[] _code;
	_code = _savedCode;
	_codeSize = _savedCodeSize;
	_savedCode = NULL;
	_currentInstruction = _savedInstruction;

	// Restore the stack
	_stacktop = _savedStacktop;

	// Restore the variables
	memcpy(_variables + 0x107, _savedVariables, 0x180);

	// Restore the filename of the script
	_scriptFile = _savedScriptFile;

	_vm->_videoPlayer->resetFlags();
	_vm->_videoPlayer->setOrigin(0, 0);
}

void Script::o_sethotspotright() {
	uint16 address = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: SETHOTSPOTRIGHT @0x%04X", address);

	_hotspotRightAction = address;
}

void Script::o_sethotspotleft() {
	uint16 address = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: SETHOTSPOTLEFT @0x%04X", address);

	_hotspotLeftAction = address;
}

void Script::o_getcd() {
	debugC(1, kDebugScript, "Groovie::Script: GETCD");

	// By default set it to no CD available
	int8 cd = -1;

	// Try to open one file from each CD
	Common::File cdfile;
	if (cdfile.open("b.gjd")) {
		cdfile.close();
		cd = 1;
	}
	if (cdfile.open("at.gjd")) {
		cdfile.close();
		if (cd == 1) {
			// Both CDs are available
			cd = 0;
		} else {
			cd = 2;
		}
	}

	setVariable(0x106, cd);
}

void Script::o_playcd() {
	uint8 val = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: PLAYCD %d", val);

	if (val == 2) {
		// TODO: Play the alternative logo
	}

	_vm->_musicPlayer->playCD(val);
}

void Script::o_musicdelay() {
	uint16 delay = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: MUSICDELAY %d", delay);

	_vm->_musicPlayer->setBackgroundDelay(delay);
}

void Script::o_hotspot_outrect() {
	uint16 left = readScript16bits();
	uint16 top = readScript16bits();
	uint16 right = readScript16bits();
	uint16 bottom = readScript16bits();
	uint16 address = readScript16bits();

	debugC(1, kDebugScript, "Groovie::Script: HOTSPOT-OUTRECT(%d,%d,%d,%d) @0x%04X (TODO)", left, top, right, bottom, address);

	// Test if the current mouse position is outside the specified rectangle
	Common::Rect rect(left, top, right, bottom);
	Common::Point mousepos = _vm->_system->getEventManager()->getMousePos();
	bool contained = rect.contains(mousepos);

	if (!contained) {
		_currentInstruction = address;
	}
}

void Script::o_stub56() {
	uint32 val1 = readScript32bits();
	uint8 val2 = readScript8bits();
	uint8 val3 = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: STUB56: 0x%08X 0x%02X 0x%02X", val1, val2, val3);
}

void Script::o_stub59() {
	uint16 val1 = readScript8or16bits();
	uint8 val2 = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: STUB59: 0x%04X 0x%02X", val1, val2);
}

void Script::o2_printstring() {
	uint16 posx = readScript16bits();
	uint16 posy = readScript16bits();
	uint8  colr = readScript8bits();
	uint8  colg = readScript8bits();
	uint8  colb = readScript8bits();
	uint32 col = _vm->_pixelFormat.RGBToColor(colr, colg, colb);
	Common::String text;

	// Read string from Script
	readScriptString(text);
	debugC(1, kDebugScript, "Groovie::Script: PRINTSTRING (%d, %d): %s", posx, posy, text.c_str());

	Graphics::Surface *gamescreen = _vm->_system->lockScreen();
	_vm->_font->drawString(gamescreen, text.c_str(), posx, posy, 640, col, Graphics::kTextAlignLeft);
	_vm->_system->unlockScreen();
	_vm->_graphicsMan->change();	// Force Update screen after step
}

void Script::o2_playsong() {
	uint32 fileref = readScript32bits();
	debugC(1, kDebugScript, "Groovie::Script: PlaySong(0x%08X): Play xmidi file", fileref);
	_vm->_musicPlayer->playSong(fileref);
}

void Script::o2_midicontrol() {
	uint16 arg1 = readScript16bits();
	uint16 arg2 = readScript16bits();

	switch (arg1) {
	case 0:
		// Stop Playback
		debugC(1, kDebugScript, "Groovie::Script: MIDI %d:Stop: %d", arg1, arg2);
		_vm->_musicPlayer->stop();
		_vm->_soundQueue.stopAll();
		break;

	case 1:
		// Play song from index
		debugC(1, kDebugScript, "Groovie::Script: MIDI %d: Play song %d", arg1, arg2);
		_vm->_musicPlayer->playSong(arg2);
		_vm->_musicPlayer->setUserVolume(100);
		break;

	case 3:
		// TODO: Set Volume? Or is it some kind of fade in / out
		debugC(1, kDebugScript, "Groovie::Script: MIDI %d: Set volume/time:  %d", arg1, arg2);
		//_vm->_musicPlayer->setUserVolume(arg2);
		break;
	}

	//_vm->_musicPlayer->setGameVolume(arg1, arg2);
}
void Script::o2_setbackgroundsong() {
	uint32 fileref = readScript32bits();
	debugC(1, kDebugScript, "Groovie::Script: SetBackgroundSong(0x%08X)", fileref);
	_vm->_musicPlayer->setBackgroundSong(fileref);
}

void Script::o2_videofromref() {
	uint32 fileref = readScript32bits();

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(1, kDebugScript, "Groovie::Script: VIDEOFROMREF(0x%08X) (Not fully imp): Play video file from ref", fileref);
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d via 0x09", _currentInstruction-5, fileref);
	}

	// Clear bit 1
	_bitflags &= ~(1 << 1);

	// Play the video
	if (!playvideofromref(fileref)) {
		// Move _currentInstruction back
		_currentInstruction -= 5;
	}
}

void Script::o2_vdxtransition() {
	uint32 fileref = readScript32bits();

	// Show the debug information just when starting the playback
	if (fileref != _videoRef) {
		debugC(1, kDebugScript, "Groovie::Script: VDX transition fileref = 0x%08X", fileref);
		debugC(2, kDebugVideo, "\nGroovie::Script: @0x%04X: Playing video %d with transition via 0x1C", _currentInstruction-5, fileref);
	}

	// Set bit 1
	_bitflags |= 1 << 1;

	// Set bit 2 if _firstbit
	if (_firstbit) {
		_bitflags |= 1 << 2;
	}

	// Play the video
	if (!playvideofromref(fileref)) {
		// Move _currentInstruction back
		_currentInstruction -= 5;
	}
}

void Script::o2_savescreen() {
	uint16 val = readScript16bits();

	// TODO: Parameter
	if (val)
		warning("Groovie::Script: o2_copyscreentobg: Param is %d", val);

	_vm->_graphicsMan->saveScreen();

	debugC(1, kDebugScript, "Groovie::Script: CopyScreenToBG3: 0x%04X", val);
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: CopyScreenToBG3: 0x%04X", _currentInstruction-3, val);
}

void Script::o2_restorescreen() {
	uint16 val = readScript16bits();

	// TODO: Parameter
	if (val)
		warning("Groovie::Script: o2_copybgtoscreen: Param is %d", val);

	_vm->_graphicsMan->restoreScreen();

	debugC(1, kDebugScript, "Groovie::Script: CopyBG3ToScreen: 0x%04X", val);
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: CopyBG3ToScreen: 0x%04X", _currentInstruction-3, val);
}

void Script::o2_setvideoskip() {
	_videoSkipAddress = readScript16bits();
	debugC(1, kDebugScript, "Groovie::Script: SetVideoSkip (0x%04X)", _videoSkipAddress);
}

// This function depends on the actual game played. So it is different for 
// T7G, 11H, TLC, ...
void Script::o2_gamespecial() {
	uint8 arg = readScript8bits();

	switch (_version) {
#ifdef ENABLE_GROOVIE2
	case kGroovieTLC:
		if (_tlcGame == NULL) {
			_tlcGame = new TlcGame();
			_tlcGame->setVariables(_variables);
		}
		switch (arg) {
		case 0:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Regions", arg);
			_tlcGame->opRegions();
			break;

		case 1:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Exit Polls", arg);
			_tlcGame->opExitPoll();
			break;

		case 2:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC TATFlags", arg);
			_tlcGame->opFlags();
			break;

		case 3:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC TATs (TODO)", arg);
			_tlcGame->opTat();
			break;

		default:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): TLC Invalid -> NOP", arg);
		}
		break;

	case kGroovieT11H:
		if (_t11hGame == NULL) {
			_t11hGame = new T11hGame();
			_t11hGame->setVariables(_variables);
		}
		switch (arg) {
		case 1:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Connect four in the dining room. (tb.grv) TODO", arg);
			break;

		case 2:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Beehive Puzzle in the top room (hs.grv) TODO", arg);
			_t11hGame->opBeehive();
			break;

		case 3:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Make last move on modern art picture in the gallery (bs.grv) TODO", arg);
			_t11hGame->opGallery();
			break;

		case 4:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Triangle in the Chapel (tx.grv)", arg);
			break;

		case 5:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Mouse Trap in the lab (al.grv)", arg);
			_t11hGame->opMouseTrap();
			break;

		case 6:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): T11H Pente (pt.grv)", arg);
			_t11hGame->opPente();
			break;


		default:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): T11H Invalid -> NOP", arg);
		}
		break;
	case kGroovieUHP:
		if (_t11hGame == NULL) {
			_t11hGame = new T11hGame();
			_t11hGame->setVariables(_variables);
		}
		switch (arg) {
		case 2:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): UHP Beehive Puzzle", arg);
			_t11hGame->opBeehive();
			break;
		case 5:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): UHP Mouse Trap", arg);
			_t11hGame->opMouseTrap();
			break;
		case 8:
			debugC(1, kDebugScript, "Groovie::Script Op42 (0x%02X): UHP Othello", arg);
			// TODO: Same as the Clandestiny Othello/Reversi puzzle (opOthello)
			break;
		default:
			debugC(1, kDebugScript, "Groovie::Script: Op42 (0x%02X): UHP Invalid -> NOP", arg);
		}
		break;
#endif
	default:
		debugC(1, kDebugScript, "Groovie::Script: GameSpecial (0x%02X)", arg);
		warning("Groovie::Script: OpCode 0x42 for (GameSpecial) current game not implemented yet.");
	}
}

void Script::o2_copyfgtobg() {
	uint8 arg = readScript8bits();
	debugC(1, kDebugScript, "Groovie::Script: o2_copyfgtobg (0x%02X)", arg);
	debugC(2, kDebugVideo, "Groovie::Script: @0x%04X: o2_copyfgtobg (0x%02X)", _currentInstruction-2, arg);
	// return;

	_vm->_graphicsMan->_background.copyFrom(_vm->_graphicsMan->_foreground);
	_vm->_graphicsMan->updateScreen(&_vm->_graphicsMan->_background);
}

void Script::o2_setscriptend() {
	uint16 arg = readScript16bits();
	debugC(1, kDebugScript, "Groovie::Script: SetScriptEnd (0x%04X)", arg);
}

void Script::o2_playsound() {
	uint32 fileref = readScript32bits();
	uint8 loops = readScript8bits();// 0 means loop forever, 1 means play once
	uint8 val3 = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: o2_playsound: 0x%08X 0x%02X 0x%02X", fileref, loops, val3);

	if (fileref == 0 && loops == 0) {
		_vm->_soundQueue.stopAll();
		return;
	}

	playBackgroundSound(fileref, loops);
}

void Script::o2_check_sounds_overlays() {
	uint16 val1 = readScript8or16bits();
	uint8 val2 = readScript8bits();

	debugC(1, kDebugScript, "Groovie::Script: STUB59: 0x%04X 0x%02X", val1, val2);

	// bitflag 0 is set by background sounds (clock chimes, wind, heart, drip in the kitchen)
	// bitflag 2 is set by overlay videos
	// this instruction is notably used at the end of the game when you have until midnight to choose a door
	_variables[val1] = getBitFlag(0) || getBitFlag(2);
}

void Script::o2_preview_loadgame() {
	uint8 save_slot = readScript8bits();

	if (preview_loadgame(save_slot))
		return;

	for (int i = 0; i < 15; i++) {
		_variables[i] = 0xf0;
	}

	for (int i = 15; i < 22; i++) {
		_variables[i] = 0x4a;
	}
}

Script::OpcodeFunc Script::_opcodesT7G[NUM_OPCODES] = {
	&Script::o_nop, // 0x00
	&Script::o_nop,
	&Script::o_playsong,
	&Script::o_bf9on,
	&Script::o_palfadeout, // 0x04
	&Script::o_bf8on,
	&Script::o_bf6on,
	&Script::o_bf7on,
	&Script::o_setbackgroundsong, // 0x08
	&Script::o_videofromref,
	&Script::o_bf5on,
	&Script::o_inputloopstart,
	&Script::o_keyboardaction, // 0x0C
	&Script::o_hotspot_rect,
	&Script::o_hotspot_left,
	&Script::o_hotspot_right,
	&Script::o_hotspot_center, // 0x10
	&Script::o_hotspot_center,
	&Script::o_hotspot_current,
	&Script::o_inputloopend,
	&Script::o_random, // 0x14
	&Script::o_jmp,
	&Script::o_loadstring,
	&Script::o_ret,
	&Script::o_call, // 0x18
	&Script::o_sleep,
	&Script::o_strcmpnejmp,
	&Script::o_xor_obfuscate,
	&Script::o_vdxtransition, // 0x1C
	&Script::o_swap,
	&Script::o_nop8,
	&Script::o_inc,
	&Script::o_dec, // 0x20
	&Script::o_strcmpnejmp_var,
	&Script::o_copybgtofg,
	&Script::o_strcmpeqjmp,
	&Script::o_mov, // 0x24
	&Script::o_add,
	&Script::o_videofromstring1, // Reads a string and then does stuff: used by book in library
	&Script::o_videofromstring2, // play vdx file from string, after setting 1 (and 2 if firstbit)
	&Script::o_nop16, // 0x28
	&Script::o_stopmidi,
	&Script::o_endscript,
	&Script::o_nop,
	&Script::o_sethotspottop, // 0x2C
	&Script::o_sethotspotbottom,
	&Script::o_loadgame,
	&Script::o_savegame,
	&Script::o_hotspotbottom_4, // 0x30
	&Script::o_midivolume,
	&Script::o_jne,
	&Script::o_loadstringvar,
	&Script::o_chargreatjmp, // 0x34
	&Script::o_bf7off,
	&Script::o_charlessjmp,
	&Script::o_copyrecttobg,
	&Script::o_restorestkpnt, // 0x38
	&Script::o_obscureswap,
	&Script::o_printstring,
	&Script::o_hotspot_slot,
	&Script::o_checkvalidsaves, // 0x3C
	&Script::o_resetvars,
	&Script::o_mod,
	&Script::o_loadscript,
	&Script::o_setvideoorigin, // 0x40
	&Script::o_sub,
	&Script::o_cellmove,
	&Script::o_returnscript,
	&Script::o_sethotspotright, // 0x44
	&Script::o_sethotspotleft,
	&Script::o_nop,
	&Script::o_nop,
	&Script::o_nop8, // 0x48
	&Script::o_nop,
	&Script::o_nop16,
	&Script::o_nop8,
	&Script::o_getcd, // 0x4C
	&Script::o_playcd,
	&Script::o_musicdelay,
	&Script::o_nop16,
	&Script::o_nop16, // 0x50
	&Script::o_nop16,
	//&Script::o_nop8,
	&Script::o_invalid,		// Do loads with game area, maybe draw dirty areas?
	&Script::o_hotspot_outrect,
	&Script::o_nop, // 0x54
	&Script::o_nop16,
	&Script::o_stub56,
	//&Script::o_nop32,
	&Script::o_invalid,		// completely unimplemented, plays vdx in some way
	//&Script::o_nop, // 0x58
	&Script::o_invalid, // 0x58	// like above, but plays from string not ref
	&Script::o_stub59,
	&Script::o_invalid
};

Script::OpcodeFunc Script::_opcodesV2[NUM_OPCODES] = {
	&Script::o_invalid, // 0x00
	&Script::o_nop,
	&Script::o2_playsong,
	&Script::o_nop,
	&Script::o_nop, // 0x04
	&Script::o_nop,
	&Script::o_nop,
	&Script::o_nop,
	&Script::o2_setbackgroundsong, // 0x08
	&Script::o2_videofromref,
	&Script::o2_bf0on,
	&Script::o_inputloopstart,
	&Script::o_keyboardaction, // 0x0C
	&Script::o_hotspot_rect,
	&Script::o_hotspot_left,
	&Script::o_hotspot_right,
	&Script::o_hotspot_center, // 0x10
	&Script::o_hotspot_center,
	&Script::o_hotspot_current,
	&Script::o_inputloopend,
	&Script::o_random, // 0x14
	&Script::o_jmp,
	&Script::o_loadstring,
	&Script::o_ret,
	&Script::o_call, // 0x18
	&Script::o_sleep,
	&Script::o_strcmpnejmp,
	&Script::o_xor_obfuscate,
	&Script::o2_vdxtransition, // 0x1C
	&Script::o_swap,
	&Script::o_invalid,
	&Script::o_inc,
	&Script::o_dec, // 0x20
	&Script::o_strcmpnejmp_var,
	&Script::o2_copybgtofg,
	&Script::o_strcmpeqjmp,
	&Script::o_mov, // 0x24
	&Script::o_add,
	&Script::o_videofromstring1,
	&Script::o_videofromstring2,
	&Script::o_invalid, // 0x28
	&Script::o_nop,
	&Script::o_endscript,
	&Script::o_invalid,
	&Script::o_sethotspottop, // 0x2C
	&Script::o_sethotspotbottom,
	&Script::o_loadgame,
	&Script::o_savegame,
	&Script::o_hotspotbottom_4, // 0x30
	&Script::o2_midicontrol,
	&Script::o_jne,
	&Script::o_loadstringvar,
	&Script::o_chargreatjmp, // 0x34
	&Script::o_bf7off,
	&Script::o_charlessjmp,
	&Script::o_copyrecttobg,
	&Script::o_restorestkpnt, // 0x38
	&Script::o_obscureswap,
	&Script::o2_printstring,
	&Script::o_hotspot_slot,
	&Script::o_checkvalidsaves, // 0x3C
	&Script::o_resetvars,
	&Script::o_mod,
	&Script::o_loadscript,
	&Script::o_setvideoorigin, // 0x40
	&Script::o_sub,
	&Script::o2_gamespecial,
	&Script::o_returnscript,
	&Script::o_sethotspotright, // 0x44
	&Script::o_sethotspotleft,
	&Script::o_invalid,
	&Script::o_invalid,
	&Script::o_invalid, // 0x48
	&Script::o_invalid,
	&Script::o_nop16,
	&Script::o_invalid,
	&Script::o_invalid, // 0x4C
	&Script::o_invalid,
	&Script::o_invalid,
	&Script::o2_savescreen,
	&Script::o2_restorescreen, // 0x50
	&Script::o2_setvideoskip,
	&Script::o2_copyfgtobg,
	&Script::o_hotspot_outrect,
	&Script::o_invalid, // 0x54
	&Script::o2_setscriptend,
	&Script::o2_playsound,
	&Script::o_invalid,
	&Script::o_invalid, // 0x58
	&Script::o2_check_sounds_overlays,
	&Script::o2_preview_loadgame
};

} // End of Groovie namespace
