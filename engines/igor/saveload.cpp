
#include "common/savefile.h"
#include "common/system.h"

#include "igor/igor.h"

namespace Igor {

class TypeSerializer {
public:

	TypeSerializer(Common::ReadStream *s) : _readStr(s), _writeStr(0), _saving(false) {}
	TypeSerializer(Common::WriteStream *s) : _readStr(0), _writeStr(s), _saving(true) {}

#define SAVEORLOAD_BASIC_TYPE(T, N, S) \
	void saveOrLoad ## N (T &t) { \
		if (_saving) { \
			_writeStr->write ## S (t); \
		} else { \
			t = _readStr->read ## S (); \
		} \
	}

	SAVEORLOAD_BASIC_TYPE(int8,   Byte,  Byte)
	SAVEORLOAD_BASIC_TYPE(uint8,  Byte,  Byte)
	SAVEORLOAD_BASIC_TYPE(int16,  Int16, Uint16LE)
	SAVEORLOAD_BASIC_TYPE(uint16, Int16, Uint16LE)

	void saveOrLoadBool(bool &b) {
		if (_saving) {
			_writeStr->writeByte(b ? 1 : 0);
		} else {
			b = _readStr->readByte() != 0;
		}
	}

	void saveOrLoadPad(int sz) {
		if (_saving) {
			while (sz--) {
				_writeStr->writeByte(0);
			}
		} else {
			while (sz--) {
				_readStr->readByte();
			}
		}
	}

	void saveOrLoadPascalString(char *str) {
		if (_saving) {
			int len = strlen(str);
			assert(len < 100);
			_writeStr->writeByte(len);
			_writeStr->write(str, len);
			for (; len < 100; ++len) {
				_writeStr->writeByte(' ');
			}
		} else {
			int len = _readStr->readByte();
			assert(len < 100);
			_readStr->read(str, len);
			str[len] = 0;
			for (; len < 100; ++len) {
				_readStr->readByte();
			}
		}
	}

	void saveOrLoadWalkData(WalkData &wd) {
		saveOrLoadPad(2);
		saveOrLoadInt16(wd.x);
		saveOrLoadInt16(wd.y);
		saveOrLoadByte(wd.posNum);
		saveOrLoadByte(wd.frameNum);
		saveOrLoadByte(wd.clipSkipX);
		saveOrLoadInt16(wd.clipWidth);
		saveOrLoadInt16(wd.scaleWidth);
		saveOrLoadByte(wd.xPosChanged);
		saveOrLoadInt16(wd.dxPos);
		saveOrLoadByte(wd.yPosChanged);
		saveOrLoadInt16(wd.dyPos);
		saveOrLoadByte(wd.scaleHeight);
	}

	void saveOrLoadAction(Action &a) {
		saveOrLoadByte(a.verb);
		saveOrLoadByte(a.object1Num);
		saveOrLoadByte(a.object1Type);
		saveOrLoadByte(a.verbType);
		saveOrLoadByte(a.object2Num);
		saveOrLoadByte(a.object2Type);
	}

	void saveOrLoadGameState(GameStateData &gs) {
		saveOrLoadByte(gs.enableLight);
		saveOrLoadByte(gs.colorLum);
		for (int i = 0; i < 5; ++i) {
			saveOrLoadInt16(gs.counter[i]);
		}
		saveOrLoadBool(gs.igorMoving);
		saveOrLoadBool(gs.dialogueTextRunning);
		saveOrLoadBool(gs.updateLight);
		saveOrLoadBool(gs.unkF);
		saveOrLoadByte(gs.unk10);
		saveOrLoadByte(gs.unk11);
		saveOrLoadBool(gs.dialogueStarted);
		saveOrLoadPad(1);
		for (int i = 0; i < 500; ++i) {
			saveOrLoadByte(gs.dialogueData[i]);
		}
		saveOrLoadByte(gs.dialogueChoiceStart);
		saveOrLoadByte(gs.dialogueChoiceCount);
		saveOrLoadPad(2);
		saveOrLoadByte(gs.nextMusicCounter);
		saveOrLoadBool(gs.jumpToNextMusic);
		saveOrLoadByte(gs.configSoundEnabled);
		saveOrLoadByte(gs.talkSpeed);
		saveOrLoadByte(gs.talkMode);
		saveOrLoadPad(3);
		saveOrLoadByte(gs.musicNum);
		saveOrLoadByte(gs.musicSequenceIndex);
	}

private:

	bool _saving;
	Common::ReadStream *_readStr;
	Common::WriteStream *_writeStr;
};

void IgorEngine::saveOrLoadGameState(TypeSerializer &typeSerializer) {
	for (int i = 0; i < 100; ++i) {
		typeSerializer.saveOrLoadWalkData(_walkData[i]);
	}
	typeSerializer.saveOrLoadPad(20);
	typeSerializer.saveOrLoadByte(_walkDataCurrentIndex);
	typeSerializer.saveOrLoadByte(_walkDataLastIndex);
	typeSerializer.saveOrLoadByte(_walkCurrentFrame);
	typeSerializer.saveOrLoadByte(_walkCurrentPos);
	typeSerializer.saveOrLoadPad(23);
	typeSerializer.saveOrLoadAction(_currentAction);
	typeSerializer.saveOrLoadPad(10);
	typeSerializer.saveOrLoadInt16(_currentPart);
	typeSerializer.saveOrLoadPad(8);
	typeSerializer.saveOrLoadByte(_actionCode);
	typeSerializer.saveOrLoadByte(_actionWalkPoint);
	typeSerializer.saveOrLoadPad(2);
	typeSerializer.saveOrLoadInt16(_inputVars[kInputCursorXPos]);
	typeSerializer.saveOrLoadInt16(_inputVars[kInputCursorYPos]);
	typeSerializer.saveOrLoadGameState(_gameState);
	for (int i = 0; i < 112; ++i) {
		typeSerializer.saveOrLoadByte(_objectsState[i]);
	}
	for (int i = 0; i < 74; ++i) {
		typeSerializer.saveOrLoadByte(_inventoryInfo[i]);
	}
}

Common::Error IgorEngine::loadGameState(int slot) {
	char name[64];
	generateGameStateFileName(slot, name, 63);
	Common::InSaveFile *isf = _saveFileMan->openForLoading(name);
	if (isf) {
		TypeSerializer ts(isf);
		ts.saveOrLoadPascalString(_saveStateDescriptions[slot]);
		saveOrLoadGameState(ts);
		delete isf;

		memcpy(_igorPalette, (_currentPart == 760) ? PAL_IGOR_2 : PAL_IGOR_1, 48);
		UPDATE_OBJECT_STATE(255);
		playMusic(_gameState.musicNum);
		_system->warpMouse(_inputVars[kInputCursorXPos], _inputVars[kInputCursorYPos]);
		if (_currentPart < 900) {
			showCursor();
		}
		debug(0, "Loaded state, current part %d", _currentPart);
	}

	return Common::kNoError;	// TODO: return success/failure
}

Common::Error IgorEngine::saveGameState(int slot) {
	char name[64];
	generateGameStateFileName(slot, name, 63);
	Common::OutSaveFile *osf = _saveFileMan->openForSaving(name);
	if (osf) {
		TypeSerializer ts(osf);
		ts.saveOrLoadPascalString(_saveStateDescriptions[slot]);
		saveOrLoadGameState(ts);
		delete osf;
	}

	return Common::kNoError;	// TODO: return success/failure
}

void IgorEngine::generateGameStateFileName(int num, char *dst, int len) const {
	snprintf(dst, len, "%s.%d", _targetName.c_str(), num);
	dst[len] = 0;
}

} // namespace Igor
