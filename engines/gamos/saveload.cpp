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

#include "gamos/gamos.h"
#include "common/savefile.h"

namespace Gamos {

void GamosEngine::storeToGameScreen(int id) {
	GameScreen &gs = _gameScreens[id];
	gs._savedObjects.clear();

	gs._savedStates = _states;

	int objCount = 0;
	for (uint i = 0; i < _objects.size(); i++) {
		const Object &obj = _objects[i];
		if ((obj.flags & 3) == 3 || (obj.flags & 7) == Object::FLAG_VALID)
			objCount++;
	}

	int idx = 0;
	gs._savedObjects.resize(objCount);
	for (uint i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];

		if (obj.isActionObject()) {
			const int refObjIdx = idx;
			if (obj.tgtObjectId == -1) {
				gs._savedObjects[idx] = obj;
				gs._savedObjects[idx].index = idx;
				obj.flags = 0;
				idx++;
			} else {
				Object &drawObj = _objects[ obj.tgtObjectId ];
				gs._savedObjects[idx] = obj;
				gs._savedObjects[idx].index = idx;
				gs._savedObjects[idx].tgtObjectId = idx + 1;
				gs._savedObjects[idx].curObjectId = idx + 1;
				gs._savedObjects[idx + 1] = drawObj;
				gs._savedObjects[idx + 1].index = idx + 1;
				gs._savedObjects[idx + 1].actObjIndex = idx;
				obj.flags = 0;
				drawObj.flags = 0;
				idx += 2;
			}

			for (uint j = 0; j < _objects.size(); j++) {
				Object &lobj = _objects[ j ];
				if ((lobj.flags & 7) == Object::FLAG_VALID && lobj.actObjIndex == obj.index) {
					gs._savedObjects[idx] = lobj;
					gs._savedObjects[idx].index = idx;
					gs._savedObjects[idx].actObjIndex = refObjIdx;
					lobj.flags = 0;
					idx++;
				}
			}
		} else if ((obj.flags & 7) == Object::FLAG_VALID && obj.actObjIndex == -1) {
			gs._savedObjects[idx] = obj;
			gs._savedObjects[idx].index = idx;
			obj.flags = 0;
			idx++;
		}
	}

	_objects.clear();
}


bool GamosEngine::switchToGameScreen(int id, bool doNotStore) {
	if (_currentGameScreen != -1 && doNotStore == false)
		storeToGameScreen(_currentGameScreen);

	_currentGameScreen = id;
	GameScreen &gs = _gameScreens[id];

	addDirtyRect(Common::Rect(Common::Point(), _bkgSize));

	_states = gs._savedStates;

	for (const Object &obj : gs._savedObjects) {
		Object *nobj = getFreeObject();
		if (nobj->index != obj.index) {
			warning("Error!  nobj->index != obj.index");
			return false;
		}

		*nobj = obj;
	}

	gs._savedObjects.clear();
	gs._savedStates.clear();

	flushDirtyRects(false);

	if (doNotStore == false && !setPaletteCurrentGS())
		return false;

	return true;
}

Common::String GamosEngine::makeSaveName(const Common::String &main, int id, const Common::String &ext) const {
	Common::String tmp = main;
	tmp.toUppercase();
	uint32 idx = tmp.find(".EXE");
	if (idx != Common::String::npos)
		tmp.erase(idx);
	return Common::String::format("%s%d.%s", tmp.c_str(), id, ext.c_str());
}


bool GamosEngine::writeStateFile() {
	Common::String fname = makeSaveName(getRunFile(), _saveLoadID, _stateExt);
	Common::SaveFileManager *sm = _system->getSavefileManager();

	if (!_isResLoadingProcess) {
		if (sm->exists(fname)) {
			Common::InSaveFile *rsv = sm->openForLoading(fname);
			byte svdata[0x4c];
			rsv->read(svdata, 0x4c);
			delete rsv;

			Common::OutSaveFile *osv = sm->openForSaving(fname);
			osv->write(svdata, 0x4c);

			writeVMData(osv, _xorSeq[0]);
			writeVMData(osv, _xorSeq[1]);

			osv->finalize();
			delete osv;
		}
	} else {
		_d2_fld10 = 0;
		Common::OutSaveFile *osv = sm->openForSaving(fname);

		writeStateData(osv);
		writeVMData(osv, _xorSeq[0]);
		writeVMData(osv, _xorSeq[1]);

		osv->finalize();
		delete osv;
	}
	return true;
}

bool GamosEngine::loadStateFile() {
	Common::String fname = makeSaveName(getRunFile(), _saveLoadID, _stateExt);
	Common::SaveFileManager *sm = _system->getSavefileManager();

	if (!_isResLoadingProcess) {
		if (sm->exists(fname)) {
			Common::SeekableReadStream *rs = sm->openForLoading(fname);
			rs->seek(0x4c);
			readVMData(rs, _xorSeq[0]);
			readVMData(rs, _xorSeq[1]);
		}
	} else {
		if (!sm->exists(fname))
			writeStateFile();
		else {
			Common::SeekableReadStream *rs = sm->openForLoading(fname);

			loadStateData(rs);
			readVMData(rs, _xorSeq[0]);
			readVMData(rs, _xorSeq[1]);

			zeroVMData(_xorSeq[1]);

			_isResLoadingProcess = false;

			delete rs;
		}
	}
	return true;
}

void GamosEngine::writeStateData(Common::SeekableWriteStream *stream) {
	byte buf[4] = {0, 0, 0, 0};
	memcpy(buf, _stateExt.c_str(), _stateExt.size() > 4 ? 4 : _stateExt.size());

	stream->write(buf, 4); // 0
	stream->writeByte(_messageProc._inputFlags);  // 4
	stream->writeByte(0);  // 5
	stream->writeByte(0);  // 6
	stream->writeByte(0);  // 7
	stream->writeSint32LE(_svModuleId); // 8
	stream->writeSint32LE(_svGameScreen); // 0xc
	stream->writeUint32LE(_d2_fld10); // 0x10
	stream->writeByte(_enableSounds ? 1 : 0); // 0x14
	stream->writeByte(_enableMidi ? 1 : 0); // 0x15
	stream->writeByte(_enableInput ? 1 : 0); // 0x16
	stream->writeByte(_enableMovie ? 1 : 0); // 0x17
	stream->writeByte(_enableCDAudio ? 1 : 0); // 0x18
	stream->writeSByte(_cdAudioTrack); // 0x19
	stream->writeByte(0); // 0x1a
	stream->writeByte(0); // 0x1b
	stream->writeSint32LE(_scrollX); // 0x1c
	stream->writeSint32LE(_scrollY); // 0x20
	stream->writeSint16LE(_scrollTrackObj); // 0x24
	stream->writeSint16LE(_scrollSpeed); // 0x26
	stream->writeSint16LE(_scrollCutoff); // 0x28
	stream->writeSint16LE(_scrollSpeedReduce); // 0x2a
	stream->writeByte(_scrollBorderL); // 0x2c
	stream->writeByte(_scrollBorderR); // 0x2d
	stream->writeByte(_scrollBorderU); // 0x2e
	stream->writeByte(_scrollBorderB); // 0x2f
	stream->writeByte(_sndChannels); // 0x30
	stream->writeByte(_sndVolume); // 0x31
	stream->writeByte(_midiVolume); // 0x32
	stream->writeByte(_svFps); // 0x33
	stream->writeSint32LE(_svFrame); // 0x34
	stream->writeUint32LE(_midiTrack); // 0x38
	stream->writeSint32LE(_mouseCursorImgId); // 0x3c
	// 0x40
	for (int i = 0; i < 12; i++)
		stream->writeByte(_messageProc._keyCodes[i]);
}

void GamosEngine::loadStateData(Common::SeekableReadStream *dataStream) {
	_stateExt = dataStream->readString(0, 4); // FIX ME
	dataStream->seek(4);
	_messageProc._inputFlags = dataStream->readByte(); //4
	dataStream->seek(8);
	_svModuleId = dataStream->readSint32LE(); // 8
	_svGameScreen = dataStream->readSint32LE(); // c
	_d2_fld10 = dataStream->readUint32LE(); // x10
	_enableSounds = dataStream->readByte() != 0 ? true : false; // x14
	_enableMidi = dataStream->readByte() != 0 ? true : false; //x15
	_enableInput = dataStream->readByte() != 0 ? true : false; // x16
	_enableMovie = dataStream->readByte() != 0 ? true : false; // x17
	_enableCDAudio = dataStream->readByte() != 0 ? true : false; // x18
	_cdAudioTrack = dataStream->readSByte(); // x19
	dataStream->seek(0x1c);
	_scrollX = dataStream->readSint32LE(); // x1c
	_scrollY = dataStream->readSint32LE(); // x20
	_scrollTrackObj = dataStream->readSint16LE(); // x24
	_scrollSpeed = dataStream->readSint16LE(); // x26
	_scrollCutoff = dataStream->readSint16LE(); // x28
	_scrollSpeedReduce = dataStream->readSint16LE(); // x2a
	_scrollBorderL = dataStream->readByte(); // x2c
	_scrollBorderR = dataStream->readByte(); // x2d
	_scrollBorderU = dataStream->readByte(); // x2e
	_scrollBorderB = dataStream->readByte(); // x2f
	_sndChannels = dataStream->readByte(); // x30
	_sndVolume = dataStream->readByte(); // x34
	_midiVolume = dataStream->readByte(); // x1a
	_svFps = dataStream->readByte(); // x1b
	_svFrame = dataStream->readSint32LE(); // x1c
	_midiTrack = dataStream->readUint32LE(); //0x38
	_mouseCursorImgId = dataStream->readSint32LE(); //0x3c

	for (int i = 0; i < 12; i++)
		_messageProc._keyCodes[i] = dataStream->readByte();
}


void GamosEngine::writeVMData(Common::SeekableWriteStream *stream, const Common::Array<XorArg> &seq) {
	for (const XorArg &xarg : seq) {
		Common::Array<byte> tmp = _vm.readMemBlocks(xarg.pos, xarg.len);

		//xor data in tmp
		//...

		// and write it
		stream->write(tmp.data(), xarg.len);
	}
}

void GamosEngine::readVMData(Common::SeekableReadStream *stream, const Common::Array<XorArg> &seq) {
	Common::Array<byte> buf;
	for (const XorArg &xarg : seq) {
		if (buf.size() < xarg.len)
			buf.resize(xarg.len);
		stream->read(buf.data(), xarg.len);
		//xor data in buf
		//...

		// and write it
		_vm.writeMemory(xarg.pos, buf.data(), xarg.len);
	}
}

void GamosEngine::zeroVMData(const Common::Array<XorArg> &seq) {
	for (const XorArg &xarg : seq)
		_vm.zeroMemory(xarg.pos, xarg.len);
}




bool GamosEngine::writeSaveFile(int id) {
	Common::String fname = makeSaveName(getRunFile(), id, "sav");
	Common::SaveFileManager *sm = _system->getSavefileManager();

	Common::OutSaveFile *osv = sm->openForSaving(fname);
	if (!osv)
		return false;

	storeToGameScreen(_currentGameScreen);
	_svFps = _vm.memory().getU8(_addrFPS);
	_svFrame = _vm.memory().getU32(_addrCurrentFrame);
	_d2_fld10 = _countReadedBkg;
	_svModuleId = _currentModuleID;
	_svGameScreen = _currentGameScreen;

	writeStateData(osv);

	writeVMData(osv, _xorSeq[0]);
	writeVMData(osv, _xorSeq[1]);
	writeVMData(osv, _xorSeq[2]);

	for (uint i = 0; i < _gameScreens.size(); i++) {
		GameScreen &scr = _gameScreens[i];

		if (scr.loaded) {
			osv->writeUint32LE(i);

			for (uint j = 0; j < scr._savedStates.size(); j++) {
				const ObjState &ost = scr._savedStates[j];
				osv->writeByte(ost.actid);
				osv->writeByte(ost.flags);
				osv->writeByte(ost.t);
			}

			osv->writeUint32LE(scr._savedObjects.size());

			for (const Object &obj : scr._savedObjects) {
				writeObjectData(osv, &obj);
			}
		}
	}

	osv->finalize();
	delete osv;

	switchToGameScreen(_currentGameScreen, true);
	return true;
}

bool GamosEngine::loadSaveFile(int id) {
	Common::String fname = makeSaveName(getRunFile(), id, "sav");
	Common::SaveFileManager *sm = _system->getSavefileManager();

	Common::SeekableReadStream *rs = sm->openForLoading(fname);
	if (!rs)
		return false;

	const bool sv1 = _enableCDAudio;
	const bool sv2 = _enableMovie;
	const bool sv3 = _enableInput;
	const bool svmdi = _enableMidi;
	const bool sv4 = _enableSounds;

	loadStateData(rs);

	_sndVolume = _sndVolumeTarget;
	_midiVolume = 0;
	_enableSounds = sv4;
	_enableMidi = svmdi;
	_enableInput = sv3;
	_enableMovie = sv2;
	_enableCDAudio = sv1;

	_musicPlayer.setVolume(0);

	const int32 cursorImgId = _mouseCursorImgId;
	const int32 svMidiTrack = _midiTrack;
	const int8 cdtrack = _cdAudioTrack;

	_isResLoadingProcess = true;
	_isSaveLoadingProcess = true;

	loadModule(_svModuleId);

	readVMData(rs, _xorSeq[0]);
	readVMData(rs, _xorSeq[1]);
	readVMData(rs, _xorSeq[2]);

	for (int i = 0; i < _countReadedBkg; i++) {
		uint32 val = rs->readUint32LE();
		GameScreen &scr = _gameScreens[val];

		scr._savedStates.resize( _states.sizes() );

		for (uint j = 0; j < scr._savedStates.size(); j++) {
			ObjState &st = scr._savedStates[j];
			st.actid = rs->readByte();
			st.flags = rs->readByte();
			st.t = rs->readByte();
		}

		val = rs->readUint32LE();

		scr._savedObjects.resize(val);

		for (Object &obj : scr._savedObjects) {
			loadObjectData(rs, &obj);
			if (((obj.flags & Object::FLAG_HASACTION) == 0) && obj.sprId >= 0 && obj.frame >= 0 && obj.seqId >= 0) {
				obj.pImg = &_sprites[obj.sprId].sequences[obj.seqId]->operator[](obj.frame);
			}
		}
	}

	delete rs;

	switchToGameScreen(_svGameScreen, false);

	_vm.memory().setU8(_addrFPS, _svFps);
	_vm.memory().setU32(_addrCurrentFrame, _svFrame);

	_isResLoadingProcess = false;
	_isSaveLoadingProcess = false;

	if (cdtrack != -1) {
		//vmfunc_58(cdtrack);
	}

	if (svMidiTrack != -1)
		playMidiTrack(svMidiTrack);

	_midiVolume = 0;

	if (cursorImgId != -1)
		setCursor(cursorImgId, false);

	setNeedReload();
	return true;
}

void GamosEngine::writeObjectData(Common::SeekableWriteStream *stream, const Object *obj) {
	stream->writeUint16LE(obj->index);
	stream->writeByte(obj->flags);
	stream->writeByte(obj->priority);
	stream->writeSint16LE(obj->cell.x);
	stream->writeSint16LE(obj->cell.y);

	if (obj->flags & Object::FLAG_HASACTION) {
		stream->writeByte(obj->actID);
		stream->writeByte(obj->t);
		stream->writeByte(obj->state.actid);
		stream->writeByte(obj->state.flags);
		stream->writeByte(obj->state.t);
		stream->writeByte(obj->inputFlag);
		stream->writeSint16LE(obj->tgtObjectId);
		stream->writeSint16LE(obj->curObjectId);
		stream->writeUint32LE(obj->storage.size());
		stream->write(obj->storage.data(), obj->storage.size());
	} else {
		stream->writeSint32LE(obj->sprId);
		stream->writeSint32LE(obj->seqId);
		stream->writeSint16LE(obj->frame);
		stream->writeSint16LE(obj->frameMax);
		stream->writeSint16LE(obj->position.x);
		stream->writeSint16LE(obj->position.y);
		stream->writeSint16LE(obj->actObjIndex);
	}
}

void GamosEngine::loadObjectData(Common::SeekableReadStream *stream, Object *obj) {
	obj->index = stream->readUint16LE();
	obj->flags = stream->readByte();
	obj->priority = stream->readByte();
	obj->cell.x = stream->readSint16LE();
	obj->cell.y = stream->readSint16LE();

	if (obj->flags & Object::FLAG_HASACTION) {
		obj->actID = stream->readByte();
		obj->t = stream->readByte();
		obj->state.actid = stream->readByte();
		obj->state.flags = stream->readByte();
		obj->state.t = stream->readByte();
		obj->inputFlag = stream->readByte();
		obj->tgtObjectId = stream->readSint16LE();
		obj->curObjectId = stream->readSint16LE();

		uint32 storSize = stream->readUint32LE();
		if (storSize) {
			obj->storage.resize(storSize);
			stream->read(obj->storage.data(), storSize);
		} else
			obj->storage.clear();
	} else {
		obj->sprId = stream->readSint32LE();
		obj->seqId = stream->readSint32LE();
		obj->frame = stream->readSint16LE();
		obj->frameMax = stream->readSint16LE();
		obj->position.x = stream->readSint16LE();
		obj->position.y = stream->readSint16LE();
		obj->actObjIndex = stream->readSint16LE();
	}
}


bool GamosEngine::deleteSaveFile(int id) {
	Common::String fname = makeSaveName(getRunFile(), id, "sav");
	Common::SaveFileManager *sm = _system->getSavefileManager();

	if ( !sm->exists(fname) )
		return true;

	if ( sm->removeSavefile(fname) )
		return true;
	return false;
}

}
