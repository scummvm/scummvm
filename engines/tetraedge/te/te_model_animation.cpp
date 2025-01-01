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

#include "common/util.h"
#include "common/file.h"
#include "common/stream.h"
#include "common/substream.h"
#include "common/compression/deflate.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_model_animation.h"

namespace Tetraedge {

TeModelAnimation::TeModelAnimation() : _firstFrame(-1), _lastFrame(SHRT_MAX),
_curFrame(0), _curFrameValFresh(false), _repeatNum(0), _finishedSignalPending(false),
_curFrame2(0), _useNMOArrays(0), _speed(0.0f), _numNMOFrames(0) {
}

int TeModelAnimation::calcCurrentFrame(double millis) {
	if (!_curFrameValFresh) {
		int lastf = lastFrame();
		int firstf = _firstFrame < 0 ? 0 : _firstFrame;

		int curf = (millis / 1000.0) * _speed;

		curf = curf % ((lastf + 1) - firstf) + firstf;
		if (!_dontRepeat && curf < _curFrame) {
			_finishedSignalPending = true;
			if (_repeatCount > 0 && _repeatNum < _repeatCount)
				_repeatNum++;
		}
		if (_repeatCount > 0 && _repeatNum >= _repeatCount) {
			// finished!
			if (!_dontRepeat)
				curf = lastFrame();
			else
				curf = firstf;
		}
		_curFrame = curf;
		_curFrameValFresh = true;
	}
	return _curFrame;
}

void TeModelAnimation::cont() {
	_repeatNum = 0;
	TeAnimation::cont();
}

void TeModelAnimation::destroy() {
	_loadedPath = Common::Path();
	_curFrame2 = 0;
	_repeatNum = 0;
	_boneNames.clear();
	_nmoTransArrays.clear();
	_nmoRotArrays.clear();
	_nmoScaleArrays.clear();
	_fbxArrays.clear();
	_numNMOFrames = 0;
}

int TeModelAnimation::findBone(const Common::String &bname) {
	for (uint i = 0; i < _boneNames.size(); i++) {
		if (_boneNames[i] == bname)
			return i;
	}
	return -1;
}

int TeModelAnimation::firstFrame() const {
	if (_firstFrame < 0)
		return 0;
	return _firstFrame;
}

//TeMatrix4x4 TeModelAnimation::getMatrix(const Common::String &mname, uint frame, bool param_5);

TeQuaternion TeModelAnimation::getNMORotation(uint boneNo, float amount) const {
	if (boneNo < _nmoRotArrays.size()) {
		const Common::Array<NMORotation> &arr = _nmoRotArrays[boneNo];
		if (arr.size()) {
			uint i = 0;
			while (i < arr.size() && arr[i]._f < amount)
				i++;

			if (i == 0)
				return arr.front()._rot;

			if (i == arr.size() || arr.size() == 1)
				return arr.back()._rot;

			if (arr[i]._f - arr[i-1]._f == 0)
				return arr[i]._rot;

			float interp = (amount - arr[i - 1]._f) / (arr[i]._f - arr[i - 1]._f);
			return arr[i - 1]._rot.slerpQuat(arr[i]._rot, interp);
		}
	}
	return TeQuaternion(0, 0, 0, 1.0);
}

TeVector3f32 TeModelAnimation::getNMOTranslation(uint boneNo, float amount) const {
	if (boneNo < _nmoTransArrays.size()) {
		const Common::Array<NMOTranslation> &arr = _nmoTransArrays[boneNo];
		if (arr.size()) {
			uint i = 0;
			while (i < arr.size() && arr[i]._f < amount)
				i++;

			if (i == 0)
				return arr.front()._trans;

			if (i == arr.size() || arr.size() == 1)
				return arr.back()._trans;

			if (arr[i]._f - arr[i - 1]._f == 0)
				return arr[i - 1]._trans;

			float interp = (amount - arr[i - 1]._f) / (arr[i]._f - arr[i - 1]._f);
			return arr[i - 1]._trans * (1.0 - interp) + arr[i]._trans * interp;
		}
	}
	return TeVector3f32(0, 0, 0);
}

//TeTRS TeModelAnimation::getTRS(const Common::String &boneName, uint frame, bool forceUseFbx);

TeTRS TeModelAnimation::getTRS(uint boneNo, uint frame, bool forceUseFbx) const {
	TeTRS retval;

	if (!_useNMOArrays || forceUseFbx) {
		uint nframes = 0;
		if (!_useNMOArrays) {
			nframes = _fbxArrays[0].size();
		} else {
			nframes = _numNMOFrames;
		}
		if (nframes > frame) {
			return _fbxArrays[boneNo][frame];
		}
	} else {
		retval.setTranslation(getNMOTranslation(boneNo, frame));
		retval.setRotation(getNMORotation(boneNo, frame));
		retval.setScale(TeVector3f32(1.0, 1.0, 1.0));
	}

	return retval;
}

int TeModelAnimation::lastFrame() const {
	int result;
	if (!_useNMOArrays) {
		if (_fbxArrays.empty())
			result = 0;
		else
			result = _fbxArrays[0].size();
	} else {
		result = _numNMOFrames;
	}
	return MIN(_lastFrame, result - 1);
}

bool TeModelAnimation::load(const Common::Path &path) {
	TetraedgeFSNode foundFile = g_engine->getCore()->findFile(path);
	Common::ScopedPtr<Common::SeekableReadStream> modelFile(foundFile.createReadStream());
	if (!modelFile) {
		warning("[TeModel::load] Can't open file : %s.", path.toString().c_str());
		return false;
	}
	bool retval;
	if (Te3DObject2::loadAndCheckFourCC(*modelFile, "TEZ0")) {
		Common::SeekableReadStream *zlibStream = TeModel::tryLoadZlibStream(*modelFile);
		if (!zlibStream)
			return false;
		retval = load(*zlibStream);
		delete zlibStream;
	} else {
		modelFile->seek(0);
		retval = load(*modelFile);
	}
	_loadedPath = path;
	return retval;
}

bool TeModelAnimation::load(Common::SeekableReadStream &stream) {
	if (!Te3DObject2::loadAndCheckFourCC(stream, "TEAN")) {
		warning("[TeModelAnimation::load] Unknown format.");
		return false;
	}
	uint32 version = stream.readUint32LE();
	if (version != 3) {
		warning("[TeModelAnimation::load] Unsupported version %d (expect 3)", version);
		return false;
	}

	_useNMOArrays = stream.readUint32LE();
	uint32 nmoFrames = stream.readUint32LE();
	if (_useNMOArrays)
		_numNMOFrames = nmoFrames;
	uint32 numBones = stream.readUint32LE();
	if (numBones > 100000)
		error("TeModelAnimation::load: Improbable number of bones %d", numBones);

	if (_useNMOArrays == 0) {
		_fbxArrays.resize(numBones);
	} else {
		_nmoTransArrays.resize(numBones);
		_nmoRotArrays.resize(numBones);
		_nmoScaleArrays.resize(numBones);
	}

	_speed = stream.readFloatLE();
	for (uint i = 0; i < numBones; i++) {
		if (!Te3DObject2::loadAndCheckFourCC(stream, "BONE"))
			return false;
		const Common::String boneName = Te3DObject2::deserializeString(stream);
		TeModel::loadAlign(stream);
		setBoneName(i, boneName);
		if (!Te3DObject2::loadAndCheckFourCC(stream, "BTRA"))
			return false;
		uint32 numTrans = stream.readUint32LE();
		if (numTrans > 100000)
			error("TeModelAnimation::load: Improbable number of bone translations %d", numTrans);
		for (uint j = 0; j < numTrans; j++) {
			float f = stream.readFloatLE();
			TeVector3f32 trans;
			TeVector3f32::deserialize(stream, trans);
			setTranslation(i, f, trans);
		}
		if (!Te3DObject2::loadAndCheckFourCC(stream, "BROT"))
			return false;
		uint32 numRots = stream.readUint32LE();
		if (numRots > 100000)
			error("TeModelAnimation::load: Improbable number of bone rotations %d", numRots);
		for (uint j = 0; j < numRots; j++) {
			float f = stream.readFloatLE();
			TeQuaternion rot;
			TeQuaternion::deserialize(stream, rot);
			setRotation(i, f, rot);
		}
	}
	return true;
}

int TeModelAnimation::nbFrames() {
	if (!_useNMOArrays) {
		if (_fbxArrays.empty())
			return 0;
		return _fbxArrays[0].size();
	} else {
		return _numNMOFrames;
	}
}

void TeModelAnimation::reset() {
	TeAnimation::reset();
	_finishedSignalPending = false;
	_curFrame = 0;
	_curFrame2 = 0;
	_firstFrame = -1;
	_lastFrame = SHRT_MAX;
}

void TeModelAnimation::resizeFBXArrays(uint len) {
	_fbxArrays.resize(len);
}

void TeModelAnimation::resizeNMOArrays(uint len) {
	_nmoTransArrays.resize(len);
	_nmoRotArrays.resize(len);
	_nmoScaleArrays.resize(len);
}

//void TeModelAnimation::save(Common::SeekableWriteStream &stream) {

//void TeModelAnimation::saveBone(Common::SeekableWriteStream &stream, uint param_2);

void TeModelAnimation::setBoneName(uint boneNo, const Common::String &bname) {
	if (_boneNames.size() < boneNo + 1) {
		_boneNames.resize(boneNo + 1);
	}
	_boneNames[boneNo] = bname;
}

void TeModelAnimation::setRotation(uint num, float amount, const TeQuaternion &rot) {
	if (!_useNMOArrays) {
		uint frame = amount;
		if (_fbxArrays[num].size() <= frame)
			_fbxArrays[num].resize(frame + 1);
		_fbxArrays[num][frame].setRotation(rot);
	} else {
		NMORotation nmorot;
		nmorot._rot = rot;
		nmorot._f = amount;
		_nmoRotArrays[num].push_back(nmorot);
	}
}

void TeModelAnimation::setScale(uint num, float amount, const TeVector3f32 &scale) {
	// NOTE: This isn't actually implemented in the original.
	error("TeModelAnimation::setScale not implemented in original game.");
}

void TeModelAnimation::setTranslation(uint num, float amount, const TeVector3f32 &trans) {
	if (!_useNMOArrays) {
		uint frame = amount;
		if (_fbxArrays[num].size() <= frame)
			_fbxArrays[num].resize(frame + 1);
		_fbxArrays[num][frame].setTranslation(trans);
	} else {
		NMOTranslation nmotrans;
		nmotrans._trans = trans;
		nmotrans._f = amount;
		_nmoTransArrays[num].push_back(nmotrans);
	}
}

void TeModelAnimation::unbind() {
	_model.release();
}

void TeModelAnimation::update(double millis) {
	int frames;
	if (!_useNMOArrays) {
		if (_fbxArrays.empty())
			return;

		frames = _fbxArrays[0].size();
	} else {
		frames = _numNMOFrames;
	}

	if (frames) {
		_curFrameValFresh = false;
		_curFrame2 = calcCurrentFrame(millis);
		if (_finishedSignalPending) {
			_finishedSignalPending = false;
			_onFinishedSignal.call();
			if (g_engine->gameType() == TetraedgeEngine::kSyberia2) {
				if (_repeatNum >= _repeatCount && _repeatCount != -1)
					stop();
			}
		}
	}
}

} // end namespace Tetraedge
