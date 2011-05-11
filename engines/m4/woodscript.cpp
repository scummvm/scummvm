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
 */

#include "m4/woodscript.h"

#include "common/memstream.h"
#include "graphics/palette.h"

namespace M4 {

// FIXME: Put in Engine/WoodScript class
RGB8 _mainPalette[256];

//Woodscript Assembler/Compiler

int32 Bytecode::_dataFormats[] = {0, 5, 8, 12, 16};

Bytecode::Bytecode(WoodScript *ws, byte *code, int32 codeSize, Sequence *seq) {
	_ws = ws;
	_code = new Common::MemoryReadStream(code, codeSize);
	_sequence = seq;
}

Bytecode::~Bytecode() {
	delete _code;
}

int Bytecode::loadInstruction(Instruction &instruction) {

	//debugCN(kDebugScript, "Bytecode::loadInstruction() ip = %08X\n", _code->pos());

	int32 format, data;
	uint32 code, code2;

	code = _code->readUint32LE();

	instruction.instr = (code >> 25) & 0xFF;
	instruction.argp[0] = NULL;
	instruction.argp[1] = NULL;
	instruction.argp[2] = NULL;
	instruction.argc = 0;

	// Maybe make this a for-loop?

	format = (code >> 22) & 7;
	if (format) {
		/* Load argument 1 */
		data = code & 0xFFFF;
		decodeArgument(format, data, instruction.argp[0], instruction.argv[0]);
		instruction.argc++;
		/* Load argument 2 */
		format = (code >> 19) & 7;
		if (format) {
			code2 = _code->readUint32LE();
			data = (code2 >> 16) & 0xFFFF;
			decodeArgument(format, data, instruction.argp[1], instruction.argv[1]);
			instruction.argc++;
			/* Load argument 3 */
			format = (code >> 16) & 7;
			if (format) {
				data = code2 & 0xFFFF;
				decodeArgument(format, data, instruction.argp[2], instruction.argv[2]);
				instruction.argc++;
			}
		}
	}

	return 0; //FIXME check if instruction size is needed by caller

}

void Bytecode::jumpAbsolute(int32 ofs) {
	_code->seek(ofs * 4);
	//debugCN(kDebugScript, "Bytecode::jumpAbsolute() ofs = %08X\n", _code->pos());
}

void Bytecode::jumpRelative(int32 ofs) {
	_code->seek(ofs * 4, SEEK_CUR);
}

void Bytecode::setSequence(Sequence *seq) {
	_sequence = seq;
}

void Bytecode::setCode(byte *code, int32 codeSize) {
	delete _code;
	_code = new Common::MemoryReadStream(code, codeSize);
}

Sequence *Bytecode::sequence() const {
	assert(_sequence);
	return _sequence;
}

bool Bytecode::decodeArgument(int32 format, int32 data, long *&arg, long &value) {

	int32 index;

	if (format == 1) {
		if (data & 0x8000)
			index = _sequence->indexReg();
		else
			index = data & 0x0FFF;
		switch (data & 0x7000) {
		case 0x0000:
			arg = sequence()->getParentVarPtr(index);
			value = *arg;
			break;
		case 0x1000:
			arg = sequence()->getVarPtr(index);
			value = *arg;
			break;
		case 0x2000:
			arg = sequence()->getDataPtr(index);
			value = *arg;
			break;
		}
	} else if (format == 2) {
		if (data & 0x8000)
			index = _sequence->indexReg();
		else
			index = data & 0x0FFF;
		arg = _ws->getGlobalPtr(index);
		value = *arg;
	} else {
		if (data & 0x8000) {
			value = -(data & 0x7FFF) << (_dataFormats[format - 3]);
		} else {
			value = (data & 0x7FFF) << (_dataFormats[format - 3]);
		}
		arg = &value;
	}

	return true;
}

WoodScript::WoodScript(MadsM4Engine *vm) {
	_vm = vm;
	_machineId = 0;
	_assets = new AssetManager(vm);
	_globals = new long[256]; //FIXME Find out how many globals there should be
	memset(_globals, 0, sizeof(long));

	_backgroundSurface = NULL;

	//Common::Rect viewBounds = Common::Rect(0, 0, 640, 480);
	//_surfaceView = new View(viewBounds);
}

WoodScript::~WoodScript() {
	delete _assets;
	delete[] _globals;
}

Sequence *WoodScript::createSequence(Machine *machine, int32 sequenceHash) {
	Sequence *sequence = new Sequence(this, machine, sequenceHash);
	_sequences.push_back(sequence);
	_layers.push_back(sequence);
	return sequence;
}

void WoodScript::runSequencePrograms() {
	// A lot TODO
	for (Common::Array<Sequence*>::iterator it = _sequences.begin(); it != _sequences.end(); ++it) {
		Sequence *sequence = *it;
		if (sequence->isActive()) {
			sequence->runProgram();
			if (sequence->isTerminated() && sequence->hasEndOfSequenceRequestPending()) {
				_endOfSequenceRequestList.push_back(sequence);
			}
		}
	}
}

void WoodScript::runEndOfSequenceRequests() {
}

void WoodScript::runTimerSequenceRequests() {
}

Machine *WoodScript::createMachine(int32 machineHash, Sequence *parentSeq,
	int32 dataHash, int32 dataRowIndex, int callbackHandler, const char *machineName) {

	//debugCN(kDebugScript, "WoodScript::createMachine(%d)\n", machineHash);

	Machine *machine = new Machine(this, machineHash, parentSeq, dataHash, dataRowIndex, callbackHandler, machineName, _machineId);
	_machineId++;

	_machines.push_back(machine);

	// goto first state for initialization
	machine->enterState();

	return machine;
}

int32 WoodScript::loadSeries(const char* seriesName, int32 hash, RGB8* palette) {
	return _assets->addSpriteAsset(seriesName, hash, palette);
}

void WoodScript::unloadSeries(int32 hash) {
	_assets->clearAssets(kAssetTypeCELS, hash, hash);
}

void WoodScript::setSeriesFramerate(Machine *machine, int32 frameRate) {
}

Machine *WoodScript::playSeries(const char *seriesName, long layer, uint32 flags, int32 triggerNum,
	int32 frameRate, int32 loopCount, int32 s, int32 x, int32 y,
	int32 firstFrame, int32 lastFrame) {

	//debugCN(kDebugScript, "WoodScript::playSeries(%s)\n", seriesName);

	RGB8 *palette = NULL;
	if (flags & SERIES_LOAD_PALETTE)
		palette = &_mainPalette[0];

	int32 spriteHash = _assets->addSpriteAsset(seriesName, -1, palette);

	_globals[kGlobTemp1] = (long)spriteHash << 24;
	_globals[kGlobTemp2] = layer << 16;
	_globals[kGlobTemp3] = _vm->_kernel->createTrigger(triggerNum);
	_globals[kGlobTemp4] = frameRate << 16;
	_globals[kGlobTemp5] = loopCount << 16;
	_globals[kGlobTemp6] = (s << 16) / 100;
	_globals[kGlobTemp7] = x << 16;
	_globals[kGlobTemp8] = y << 16;
	_globals[kGlobTemp9] = firstFrame << 16;
	_globals[kGlobTemp10] = lastFrame << 16;
	_globals[kGlobTemp11] = (flags & SERIES_PINGPONG) ? 0x10000 : 0;
	_globals[kGlobTemp12] = (flags & SERIES_BACKWARD) ? 0x10000 : 0;
	_globals[kGlobTemp13] = (flags & SERIES_RANDOM)	? 0x10000 : 0;
	_globals[kGlobTemp14] = (flags & SERIES_STICK) ? 0x10000 : 0;
	_globals[kGlobTemp15] = (flags & SERIES_LOOP_TRIGGER) ? 0x10000 : 0;
	_globals[kGlobTemp16] = (flags & SERIES_HORZ_FLIP) ? 0x10000 : 0;

	return createMachine(0, NULL, -1, -1, kCallbackTriggerDispatch, seriesName);

}

Machine *WoodScript::showSeries(const char *seriesName, long layer, uint32 flags, int32 triggerNum,
	int32 duration, int32 index, int32 s, int32 x, int32 y) {

	RGB8 *palette = NULL;
	if (flags & SERIES_LOAD_PALETTE)
		palette = &_mainPalette[0];

	int32 spriteHash = _assets->addSpriteAsset(seriesName, -1, palette);

	_globals[kGlobTemp1] = spriteHash << 24;
	_globals[kGlobTemp2] = layer << 16;
	_globals[kGlobTemp3] = _vm->_kernel->createTrigger(triggerNum);
	_globals[kGlobTemp4] = duration << 16;
	_globals[kGlobTemp5] = index << 16;
	_globals[kGlobTemp6] = (s << 16) / 100;
	_globals[kGlobTemp7] = x << 16;
	_globals[kGlobTemp8] = y << 16;
	_globals[kGlobTemp14] = (flags & SERIES_STICK) ? 0x10000 : 0;
	_globals[kGlobTemp16] = (flags & SERIES_HORZ_FLIP) ? 0x10000 : 0;

	return createMachine(1, NULL, -1, -1, kCallbackTriggerDispatch, seriesName);

}

Machine *WoodScript::streamSeries(const char *seriesName, int32 frameRate, long layer, int32 triggerNum) {
	//debugCN(kDebugScript, "WoodScript::streamSeries(%s)\n", seriesName);
	_globals[kGlobTemp1] = frameRate << 16;
	/* FIXME: Single frames from a stream series will be decompressed on-the-fly, contrary to
			  "normal" sprite series, to save some memory, and since no random access to single
			  frames is needed, this is ok.
	*/
	_globals[kGlobTemp4] = 0; // The actual stream is opened in the Sequence
	_globals[kGlobTemp5] = 0;//TODO: kernel_trigger_create(triggerNum);	// trigger
	_globals[kGlobTemp6] = layer << 16;												// layer
	return createMachine(6, NULL, -1, -1, kCallbackTriggerDispatch, seriesName);
}

void WoodScript::update() {
	// TODO: Don't show hidden sequences etc.

	// TODO: For now, prevent any engine action if a menu is being displayed - eventually this should be
	// changed to a proper check of the engine paused variable, which the menus should set while active
	if (_vm->_viewManager->getView(VIEWID_MENU) != NULL)
		return;

	//TODO: Include _pauseTime
	uint32 clockTime = g_system->getMillis() / 60; // FIXME: g_system
	_globals[kGlobTimeDelta] = clockTime - _globals[kGlobTime];
	_globals[kGlobTime] += _globals[kGlobTimeDelta];

	runSequencePrograms();

	if (_backgroundSurface) {
		// FIXME: For now, copy the whole surface. Later, copy only the rectangles that need updating.
		_backgroundSurface->copyTo(_surfaceView);
	} else {
		// "This should never happen."
		_surfaceView->fillRect(Common::Rect(0, 0, 640, 480), 0);
	}

	{
		// FIXME: This should be done when a new palette is set
		byte palette[768];
		g_system->getPaletteManager()->grabPalette(palette, 0, 256);
		for (int i = 0; i < 256; i++) {
			_mainPalette[i].r = palette[i * 3 + 0];
			_mainPalette[i].g = palette[i * 3 + 1];
			_mainPalette[i].b = palette[i * 3 + 2];
		}
	}

	for (Common::Array<Sequence*>::iterator it = _layers.begin(); it != _layers.end(); ++it) {
		Sequence *sequence = *it;

		// TODO: Use correct clipRect etc.
		Common::Rect clipRect = Common::Rect(0, 0, 640, 480);
		Common::Rect updateRect;

		sequence->draw(_surfaceView, clipRect, updateRect);

	}

	// Handle end-of-sequence requests
	if (_endOfSequenceRequestList.size() > 0) {
		for (Common::Array<Sequence*>::iterator it = _endOfSequenceRequestList.begin(); it != _endOfSequenceRequestList.end(); ++it) {
			Sequence *sequence = *it;

			EndOfSequenceRequestItem endOfSequenceRequestItem = sequence->getEndOfSequenceRequestItem();
			sequence->getMachine()->execBlock(endOfSequenceRequestItem.codeOffset, endOfSequenceRequestItem.count);
		}
		_endOfSequenceRequestList.clear();
	}

}

void WoodScript::clear() {

	for (Common::Array<Sequence*>::iterator it = _sequences.begin(); it != _sequences.end(); ++it)
		delete *it;
	_sequences.clear();

	for (Common::Array<Machine*>::iterator it = _machines.begin(); it != _machines.end(); ++it)
		delete *it;
	_machines.clear();

	_layers.clear();
	_endOfSequenceRequestList.clear();

}

void WoodScript::setDepthTable(int16 *depthTable) {
	_depthTable = depthTable;
}

long *WoodScript::getGlobalPtr(int index) {
	return &_globals[index];
}

long WoodScript::getGlobal(int index) {
	return _globals[index];
}

void WoodScript::setGlobal(int index, long value) {
	_globals[index] = value;
}

void WoodScript::setBackgroundSurface(M4Surface *backgroundSurface) {
	_backgroundSurface = backgroundSurface;
}

void WoodScript::setSurfaceView(View *view) {
	_surfaceView = view;
}

RGB8 *WoodScript::getMainPalette() const {
	return _mainPalette;
}

}
