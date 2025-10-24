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
	for (int i = 0; i < _objects.size(); i++) {
		const Object &obj = _objects[i];
		if ((obj.flags & 3) == 3 || (obj.flags & 7) == 1)
			objCount++;
	}

	int idx = 0;
	gs._savedObjects.resize(objCount);
	for (int i = 0; i < _objects.size(); i++) {
		Object &obj = _objects[i];

		if ((obj.flags & 3) == 3) {
			const int refObjIdx = idx;
			if (obj.x == -1) {
				gs._savedObjects[idx] = obj;
				gs._savedObjects[idx].index = idx;
				obj.flags = 0;
				idx++;
			} else {
				Object &drawObj = _objects[ obj.x ];
				gs._savedObjects[idx] = obj;
				gs._savedObjects[idx].index = idx;
				gs._savedObjects[idx].x = idx + 1;
				gs._savedObjects[idx].y = idx + 1;
				gs._savedObjects[idx + 1] = drawObj;
				gs._savedObjects[idx + 1].index = idx + 1;
				gs._savedObjects[idx + 1].pos = idx & 0xff;
				gs._savedObjects[idx + 1].blk = (idx >> 8) & 0xff;
				obj.flags = 0;
				drawObj.flags = 0;
				idx += 2;
			}

			for (int j = 0; j < _objects.size(); j++) {
				Object &lobj = _objects[ j ];
				if ((lobj.flags & 7) == 1 && ((lobj.blk << 8) | lobj.pos) == obj.index) {
					gs._savedObjects[idx] = lobj;
					gs._savedObjects[idx].index = idx;
					gs._savedObjects[idx].pos = refObjIdx & 0xff;
					gs._savedObjects[idx].blk = (refObjIdx >> 8) & 0xff;
					lobj.flags = 0;
					idx++;
				}
			}
		} else if ((obj.flags & 7) == 1 && obj.pos == 0xff && obj.blk == 0xff) {
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

	addDirtyRect(Common::Rect(Common::Point(), _bkgUpdateSizes));

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
	Common::String fname = makeSaveName(getGameId(), _saveLoadID, _stateExt);
	Common::SaveFileManager *sm = _system->getSavefileManager();

	if (!_runReadDataMod) {
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
	Common::String fname = makeSaveName(getGameId(), _saveLoadID, _stateExt);
	Common::SaveFileManager *sm = _system->getSavefileManager();

	if (!_runReadDataMod) {
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

			_runReadDataMod = false;

			delete rs;
		}
	}
	return true;
}

void GamosEngine::writeStateData(Common::SeekableWriteStream *stream) {
	byte buf[4] = {0, 0, 0, 0};
	memcpy(buf, _stateExt.c_str(), _stateExt.size() > 4 ? 4 : _stateExt.size());

	stream->write(buf, 4); // 0
	stream->writeByte(_messageProc._gd2flags);  // 4
	stream->writeByte(0);  // 5
	stream->writeByte(0);  // 6
	stream->writeByte(0);  // 7
	stream->writeSint32LE(_svModuleId); // 8
	stream->writeSint32LE(_svGameScreen); // 0xc
	stream->writeUint32LE(_d2_fld10); // 0x10
	stream->writeByte(_d2_fld14); // 0x14
	stream->writeByte(_enableMidi ? 1 : 0); // 0x15
	stream->writeByte(_d2_fld16); // 0x16
	stream->writeByte(_d2_fld17); // 0x17
	stream->writeByte(_d2_fld18); // 0x18
	stream->writeByte(_d2_fld19); // 0x19
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
	_messageProc._gd2flags = dataStream->readByte(); //4
	dataStream->seek(8);
	_svModuleId = dataStream->readSint32LE(); // 8
	_svGameScreen = dataStream->readSint32LE(); // c
	_d2_fld10 = dataStream->readUint32LE(); // x10
	_d2_fld14 = dataStream->readByte(); // x14
	_enableMidi = dataStream->readByte() != 0 ? true : false; //x15
	_d2_fld16 = dataStream->readByte(); // x16
	_d2_fld17 = dataStream->readByte(); // x17
	_d2_fld18 = dataStream->readByte(); // x18
	_d2_fld19 = dataStream->readByte(); // x19
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
		Common::Array<byte> tmp = VM::readMemBlocks(xarg.pos, xarg.len);

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
		VM::writeMemory(xarg.pos, buf.data(), xarg.len);
	}
}

void GamosEngine::zeroVMData(const Common::Array<XorArg> &seq) {
	for (const XorArg &xarg : seq)
		VM::zeroMemory(xarg.pos, xarg.len);
}

}
