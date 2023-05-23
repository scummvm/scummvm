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

#include "common/stream.h"

#include "vcruise/sampleloop.h"

namespace VCruise {

SampleLoop::SampleLoop() : identifier(0), type(0), start(0), end(0), fraction(0), playCount(0) {
}

bool SampleLoop::read(Common::ReadStream &stream, uint &availableBytes) {
	if (availableBytes < 24)
		return false;

	byte bytes[24];

	uint32 bytesRead = stream.read(bytes, 24);
	availableBytes -= bytesRead;
	if (bytesRead != 24)
		return false;

	this->identifier = READ_LE_UINT32(bytes + 0);
	this->type = READ_LE_UINT32(bytes + 4);
	this->start = READ_LE_UINT32(bytes + 8);
	this->end = READ_LE_UINT32(bytes + 12);
	this->fraction = READ_LE_UINT32(bytes + 16);
	this->playCount = READ_LE_UINT32(bytes + 20);

	return true;
}

SampleChunk::SampleChunk() : manufacturer(0), product(0), samplePeriod(0), midiUnityNote(0), midiPitchFraction(0), smpteFormat(0), smpteOffset(0) {
}

bool SampleChunk::read(Common::ReadStream &stream, uint &availableBytes) {
	if (availableBytes < 36)
		return false;

	byte bytes[36];

	uint32 bytesRead = stream.read(bytes, 36);
	availableBytes -= bytesRead;
	if (bytesRead != 36)
		return false;
	
	this->manufacturer = READ_LE_UINT32(bytes + 0);
	this->product = READ_LE_UINT32(bytes + 4);
	this->samplePeriod = READ_LE_UINT32(bytes + 8);
	this->midiUnityNote = READ_LE_UINT32(bytes + 12);
	this->midiPitchFraction = READ_LE_UINT32(bytes + 16);
	this->smpteFormat = READ_LE_UINT32(bytes + 20);
	this->smpteOffset = READ_LE_UINT32(bytes + 24);

	uint32 numSampleLoops = READ_LE_UINT32(bytes + 28);
	uint32 sizeOfSamplerData = READ_LE_UINT32(bytes + 32);

	loops.resize(numSampleLoops);
	samplerData.resize(sizeOfSamplerData);

	for (uint32 i = 0; i < numSampleLoops; i++)
		if (!loops[i].read(stream, availableBytes))
			return false;

	if (sizeOfSamplerData > 0) {
		if (availableBytes < sizeOfSamplerData)
			return false;

		bytesRead = stream.read(&samplerData[0], sizeOfSamplerData);
		availableBytes -= bytesRead;
		if (bytesRead != sizeOfSamplerData)
			return false;
	}

	return true;
}

Common::SharedPtr<SoundLoopInfo> SoundLoopInfo::readFromWaveFile(Common::SeekableReadStream &stream) {
	if (!stream.seek(0))
		return nullptr;

	int64 waveSize64 = stream.size();

	if (waveSize64 > static_cast<int64>(0xffffffffu))
		return nullptr;

	uint availableBytes = waveSize64;

	if (availableBytes < 8)
		return nullptr;

	byte riffHeader[8];
	if (stream.read(riffHeader, 8) != 8)
		return nullptr;

	availableBytes -= 8;

	if (READ_LE_UINT32(riffHeader + 0) != 0x46464952)
		return nullptr;

	uint riffDataSize = READ_LE_UINT32(riffHeader + 4);

	if (riffDataSize > availableBytes)
		return nullptr;

	availableBytes = riffDataSize;

	if (availableBytes < 4)
		return nullptr;

	byte waveHeader[4];
	if (stream.read(waveHeader, 4) != 4)
		return nullptr;

	availableBytes -= 4;

	if (READ_LE_UINT32(waveHeader + 0) != 0x45564157)
		return nullptr;

	while (availableBytes > 0) {
		if (availableBytes < 8)
			return nullptr;

		byte chunkHeader[8];
		if (stream.read(chunkHeader, 8) != 8)
			return nullptr;

		availableBytes -= 8;

		uint32 chunkType = READ_LE_UINT32(chunkHeader + 0);
		uint32 chunkSize = READ_LE_UINT32(chunkHeader + 4);

		if (chunkSize > availableBytes)
			return nullptr;

		if (chunkType == 0x6c706d73) {
			Common::SharedPtr<SoundLoopInfo> sndLoop(new SoundLoopInfo());

			uint chunkAvailableBytes = chunkSize;
			if (!sndLoop->_sampleChunk.read(stream, chunkAvailableBytes))
				return nullptr;

			if (sndLoop->_sampleChunk.loops.size() == 0)
				return nullptr;

			return sndLoop;
		}

		if (!stream.seek(chunkSize, SEEK_CUR))
			return nullptr;

		availableBytes -= chunkSize;
	}

	return nullptr;
}

SampleLoopAudioStream::LoopRange::LoopRange() : _startSampleInclusive(0), _endSampleExclusive(0), _playCount(0) {
}

SampleLoopAudioStream::SampleLoopAudioStream(Audio::SeekableAudioStream *baseStream, const SoundLoopInfo *loopInfo)
	: _baseStream(baseStream), _terminated(false), _ignoreLoops(false), _currentSampleOffset(0), _currentLoop(-1), _currentLoopIteration(0), _streamFrames(0), _streamSamples(0) {

	_streamFrames = baseStream->getLength().convertToFramerate(baseStream->getRate()).totalNumberOfFrames();
	_streamSamples = _streamFrames;

	if (_baseStream->isStereo())
		_streamSamples *= 2;

	if (loopInfo) {
		_loopRanges.resize(loopInfo->_sampleChunk.loops.size());
		for (uint i = 0; i < _loopRanges.size(); i++) {
			const SampleLoop &inLoop = loopInfo->_sampleChunk.loops[i];
			LoopRange &outLoop = _loopRanges[i];

			outLoop._startSampleInclusive = inLoop.start;
			outLoop._endSampleExclusive = inLoop.end;
			outLoop._playCount = inLoop.playCount;
		}
	} else {
		_loopRanges.resize(1);
		_loopRanges[0]._startSampleInclusive = 0;
		_loopRanges[0]._endSampleExclusive = _streamFrames;
	}

	for (LoopRange &range : _loopRanges) {
		range._restartTimestamp = Audio::Timestamp(0, range._startSampleInclusive, baseStream->getRate());

		if (range._startSampleInclusive > static_cast<uint>(_streamFrames))
			range._startSampleInclusive = _streamFrames;
		if (range._endSampleExclusive > static_cast<uint>(_streamFrames))
			range._endSampleExclusive = _streamFrames;
		if (range._endSampleExclusive < range._startSampleInclusive)
			range._endSampleExclusive = range._startSampleInclusive;

		if (_baseStream->isStereo()) {
			range._startSampleInclusive *= 2;
			range._endSampleExclusive *= 2;
		}
	}

	for (uint i = 0; i < _loopRanges.size(); ) {
		const LoopRange &thisRange = _loopRanges[i];

		bool isValid = true;
		if (thisRange._endSampleExclusive == thisRange._startSampleInclusive)
			isValid = false;

		if (i > 0) {
			const LoopRange &prevRange = _loopRanges[i - 1];
			if (thisRange._startSampleInclusive < prevRange._endSampleExclusive)
				isValid = false;
		}

		if (isValid)
			i++;
		else
			_loopRanges.remove_at(i);
	}

}

SampleLoopAudioStream::~SampleLoopAudioStream() {
}

void SampleLoopAudioStream::stopLooping() {
	_mutex.lock();
	_ignoreLoops = true;
	_mutex.unlock();
}

int SampleLoopAudioStream::readBuffer(int16 *buffer, int numSamples) {
	bool ignoreLoops = false;

	_mutex.lock();
	ignoreLoops = _ignoreLoops;
	_mutex.unlock();

	int totalSamplesRead = 0;

	for (;;) {
		if (_terminated || numSamples == 0)
			return totalSamplesRead;

		int consecutiveSamplesAvailable = 0;
		bool terminateIfReadCompletes = false;

		if (_ignoreLoops) {
			consecutiveSamplesAvailable = _streamSamples - _currentSampleOffset;
			terminateIfReadCompletes = true;
		} else if (_currentLoop < 0) {
			// Not currently in a loop
			int samplesUntilLoop = -1;
			uint scanLoopIndex = 0;
			for (const LoopRange &loopRange : _loopRanges) {
				if (static_cast<int>(loopRange._startSampleInclusive) >= _currentSampleOffset) {
					samplesUntilLoop = loopRange._startSampleInclusive - _currentSampleOffset;
					break;
				} else
					scanLoopIndex++;
			}

			if (samplesUntilLoop < 0) {
				// Past the end of the last loop
				consecutiveSamplesAvailable = _streamSamples - _currentSampleOffset;
				terminateIfReadCompletes = true;
			} else if (samplesUntilLoop == 0) {
				// At the start of a loop
				_currentLoop = scanLoopIndex;
				_currentLoopIteration = 0;
				continue;
			} else {
				// Before a loop
				consecutiveSamplesAvailable = samplesUntilLoop;
			}
		} else {
			// In a loop
			const LoopRange &loopRange = _loopRanges[_currentLoop];

			int samplesAvailable = loopRange._endSampleExclusive - static_cast<int>(_currentSampleOffset);
			if (samplesAvailable == 0) {
				// At the end of the loop
				if (loopRange._playCount > 0) {
					if (_currentLoopIteration == loopRange._playCount) {
						// Exit loop
						_currentLoop = -1;
						continue;
					} else
						_currentLoopIteration++;
				}

				if (!_baseStream->seek(loopRange._restartTimestamp)) {
					_terminated = true;
					return totalSamplesRead;
				}

				_currentSampleOffset = loopRange._startSampleInclusive;
				continue;
			} else {
				// Inside of a loop
				consecutiveSamplesAvailable = samplesAvailable;
			}
		}

		if (consecutiveSamplesAvailable == 0)
			_terminated = true;
		else {
			int samplesDesired = numSamples;
			if (samplesDesired > consecutiveSamplesAvailable)
				samplesDesired = consecutiveSamplesAvailable;

			int samplesRead = _baseStream->readBuffer(buffer, samplesDesired);

			if (samplesRead > 0)
				totalSamplesRead += samplesRead;

			if (samplesRead != samplesDesired)
				_terminated = true;
			else {
				_currentSampleOffset += samplesRead;
				buffer += samplesRead;
				numSamples -= samplesRead;

				if (samplesRead == consecutiveSamplesAvailable && terminateIfReadCompletes)
					_terminated = true;
			}
		}
	}
}

bool SampleLoopAudioStream::isStereo() const {
	return _baseStream->isStereo();
}

int SampleLoopAudioStream::getRate() const {
	return _baseStream->getRate();
}

bool SampleLoopAudioStream::endOfData() const {
	return _terminated;
}

} // namespace VCruise
