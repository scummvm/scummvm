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

#ifndef GROOVIE_SCRIPT_H
#define GROOVIE_SCRIPT_H

#include "groovie/groovie.h"
#ifdef ENABLE_GROOVIE2
#include "groovie/logic/beehive.h"
#include "groovie/logic/cake.h"
#include "groovie/logic/gallery.h"
#include "groovie/logic/mousetrap.h"
#include "groovie/logic/othello.h"
#include "groovie/logic/pente.h"
#include "groovie/logic/triangle.h"
#include "groovie/logic/winerack.h"
#endif

#include "common/random.h"
#include "common/rect.h"

namespace Common {
class SeekableReadStream;
}

namespace Graphics {
struct Surface;
}

namespace Groovie {

class CellGame;
class Debugger;
class GroovieEngine;
class TlcGame;

class Script {
	friend class Debugger;

public:
	Script(GroovieEngine *vm, EngineVersion version);
	~Script();

	void setDebugger(Debugger *debugger);
	void setVariable(uint16 varnum, byte value);

	void timerTick();

	bool loadScript(Common::String scriptfile);
	void directGameLoad(int slot);
	void directGameSave(int slot, const Common::String &desc);
	bool canDirectSave() const;
	void step();

	void setMouseClick(uint8 button);
	void setKbdChar(uint8 c);
	void setAction(uint8 a);

	void setBitFlag(int bitnum, bool value);
	bool getBitFlag(int bitnum);

	Common::String &getContext();

private:
	GroovieEngine *_vm;

	Common::RandomSource _random;

	bool _firstbit;
	uint8 _lastCursor;

	EngineVersion _version;

	// Script filename (for debugging purposes)
	Common::String _scriptFile;
	Common::String _savedScriptFile;

	// Save names
	Common::String _saveNames[MAX_SAVES];
	bool _wantAutosave;

	// Code
	byte *_code;
	uint16 _codeSize;
	uint16 _currentInstruction;
	byte *_savedCode;
	uint16 _savedCodeSize;
	uint16 _savedInstruction;

	// Variables
	byte _variables[0x400];
	byte _savedVariables[0x180];

	// Stack
	uint16 _stack[0x20];
	uint8 _stacktop;
	uint8 _savedStacktop;

	// Input
	bool _mouseClicked;
	uint8 _eventMouseClicked;
	uint8 _kbdChar;
	uint8 _eventKbdChar;
	uint8 _eventAction;
	uint16 _inputLoopAddress;
	uint8 _newCursorStyle;
	uint16 _hotspotTopAction;
	uint16 _hotspotTopCursor;
	uint16 _hotspotBottomAction;
	uint16 _hotspotBottomCursor;
	uint16 _hotspotRightAction;
	uint16 _hotspotLeftAction;
	uint16 _hotspotSlot;
	bool _fastForwarding;
	void resetFastForward();

	// Video
	Common::SeekableReadStream *_videoFile;
	uint32 _videoRef;
	uint16 _bitflags;
	uint16 _videoSkipAddress;

	// Debugging
	Debugger *_debugger;
	Common::String _debugString;
	uint16 _oldInstruction;

	// Special classes depending on played game
	CellGame *_cellGame;
	TlcGame *_tlcGame;

	// Helper functions
	uint8 getCodeByte(uint16 address);
	uint8 readScript8bits();
	uint16 readScript16bits();
	uint32 readScript32bits();
	uint16 readScript8or16bits();
	uint8 readScriptChar(bool allow7C, bool limitVal, bool limitVar);
	void readScriptString(Common::String &str);
	uint8 readScriptVar();
	uint32 getVideoRefString(Common::String &resName);

	void executeInputAction(uint16 address);
	bool hotspot(Common::Rect rect, uint16 addr, uint8 cursor);

	void loadgame(uint slot);
	bool preview_loadgame(uint slot);
	void savegame(uint slot, const Common::String &name);
	bool playvideofromref(uint32 fileref, bool loopUntilAudioDone = false);
	bool playBackgroundSound(uint32 fileref, uint32 loops);
	void printString(Graphics::Surface *surface, const char *str);

	// Opcodes
	typedef void (Script::*OpcodeFunc)();
	OpcodeFunc *_opcodes;
	static OpcodeFunc _opcodesT7G[];
	static OpcodeFunc _opcodesV2[];

	void o_invalid();

	void o_nop();
	void o_nop8();
	void o_nop16();
	void o_nop32();
	void o_nop8or16();

	void o_playsong();
	void o_bf9on();
	void o_palfadeout();
	void o_bf8on();
	void o_bf6on();
	void o_bf7on();
	void o_setbackgroundsong();
	void o_videofromref();
	void o_bf5on();
	void o_inputloopstart();
	void o_keyboardaction();
	void o_hotspot_rect();
	void o_hotspot_left();
	void o_hotspot_right();
	void o_hotspot_center();
	void o_hotspot_current();
	void o_inputloopend();
	void o_random();
	void o_jmp();
	void o_loadstring();
	void o_ret();
	void o_call();
	void o_sleep();
	void o_strcmpnejmp_var();
	void o_copybgtofg();
	void o_strcmpnejmp();
	void o_xor_obfuscate();
	void o_vdxtransition();
	void o_swap();
	void o_inc();
	void o_dec();
	void o_strcmpeqjmp();
	void o_mov();
	void o_add();
	void o_videofromstring1();
	void o_videofromstring2();
	void o_stopmidi();
	void o_endscript();
	void o_sethotspottop();
	void o_sethotspotbottom();
	void o_loadgame();
	void o_savegame();
	void o_hotspotbottom_4();
	void o_midivolume();
	void o_jne();
	void o_loadstringvar();
	void o_chargreatjmp();
	void o_bf7off();
	void o_charlessjmp();
	void o_copyrecttobg();
	void o_restorestkpnt();
	void o_obscureswap();
	void o_printstring();
	void o_hotspot_slot();
	void o_checkvalidsaves();
	void o_resetvars();
	void o_mod();
	void o_loadscript();
	void o_setvideoorigin();
	void o_sub();
	void o_returnscript();
	void o_sethotspotright();
	void o_sethotspotleft();
	void o_getcd();
	void o_playcd();
	void o_musicdelay();
	void o_hotspot_outrect();
	void o_stub56();
	void o_wipemaskfromstring58();
	void o_stub59();

	void o2_bf0on();
	void o2_copybgtofg();
	void o2_printstring();
	void o2_playsong();
	void o2_midicontrol();
	void o2_setbackgroundsong();
	void o2_videofromref();
	void o2_vdxtransition();
	void o2_setvideoskip();
	void o2_savescreen();
	void o2_restorescreen();
	void o_gamelogic();
	void o2_copyfgtobg();
	void o2_setscriptend();
	void o2_playsound();
	void o2_check_sounds_overlays();
	void o2_preview_loadgame();

#ifdef ENABLE_GROOVIE2
	BeehiveGame _beehive;
	CakeGame _cake;
	GalleryGame _gallery;
	MouseTrapGame _mouseTrap;
	OthelloGame _othello;
	PenteGame _pente;
	TriangleGame _triangle;
	WineRackGame _wineRack;
#endif
};

} // End of Groovie namespace

#endif // GROOVIE_SCRIPT_H
