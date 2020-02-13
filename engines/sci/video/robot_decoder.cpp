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

#include "sci/video/robot_decoder.h"
#include "common/archive.h"          // for SearchMan
#include "common/debug.h"            // for debugC
#include "common/endian.h"           // for MKTAG
#include "common/memstream.h"        // for MemoryReadStream
#include "common/platform.h"         // for Platform::kPlatformMacintosh
#include "common/rational.h"         // for operator*, Rational
#include "common/str.h"              // for String
#include "common/stream.h"           // for SeekableReadStream
#include "common/substream.h"        // for SeekableSubReadStreamEndian
#include "common/textconsole.h"      // for error, warning
#include "common/types.h"            // for Flag::NO, Flag::YES
#include "sci/engine/seg_manager.h"  // for SegManager
#include "sci/graphics/celobj32.h"   // for Ratio, ::kLowResX, ::kLowResY
#include "sci/graphics/text32.h"     // for BitmapResource
#include "sci/sound/audio32.h"       // for Audio32
#include "sci/sci.h"                 // for kDebugLevels::kDebugLevelVideo
#include "sci/util.h"                // for READ_SCI11ENDIAN_UINT16, READ_SC...

namespace Sci {

#pragma mark RobotAudioStream

extern void deDPCM16Mono(int16 *out, const byte *in, const uint32 numBytes, int16 &sample);

RobotAudioStream::RobotAudioStream(const int32 bufferSize) :
	_loopBuffer((byte *)malloc(bufferSize)),
	_loopBufferSize(bufferSize),
	_decompressionBuffer(nullptr),
	_decompressionBufferSize(0),
	_decompressionBufferPosition(-1),
	_waiting(true),
	_finished(false),
	_firstPacketPosition(-1) {}

RobotAudioStream::~RobotAudioStream() {
	free(_loopBuffer);
	free(_decompressionBuffer);
}

static void interpolateChannel(int16 *buffer, int32 numSamples, const int8 bufferIndex) {
	if (numSamples <= 0) {
		return;
	}

	int16 *inBuffer, *outBuffer;
	int16 sample, previousSample;

	if (bufferIndex) {
		outBuffer = buffer + 1;
		inBuffer = buffer + 2;
		previousSample = sample = *buffer;
		--numSamples;
	} else {
		outBuffer = buffer;
		inBuffer = buffer + 1;
		previousSample = sample = *inBuffer;
	}

	while (numSamples--) {
		sample = (*inBuffer + previousSample) >> 1;
		previousSample = *inBuffer;
		*outBuffer = sample;
		inBuffer += RobotAudioStream::kEOSExpansion;
		outBuffer += RobotAudioStream::kEOSExpansion;
	}

	if (bufferIndex) {
		*outBuffer = sample;
	}
}

static void copyEveryOtherSample(int16 *out, const int16 *in, int numSamples) {
	while (numSamples--) {
		*out = *in++;
		out += 2;
	}
}

bool RobotAudioStream::addPacket(const RobotAudioPacket &packet) {
	Common::StackLock lock(_mutex);

	if (_finished) {
		warning("Packet %d sent to finished robot audio stream", packet.position);
		return false;
	}

	// `packet.position` is the decompressed (doubled) position of the packet,
	// so values of `position` will always be divisible either by 2 (even) or by
	// 4 (odd).
	const int8 bufferIndex = packet.position % 4 ? 1 : 0;

	// Packet 0 is the first primer, packet 2 is the second primer, packet 4+
	// are regular audio data
	if (packet.position <= 2 && _firstPacketPosition == -1) {
		_readHead = 0;
		_readHeadAbs = 0;
		_maxWriteAbs = _loopBufferSize;
		_writeHeadAbs = 2;
		_jointMin[0] = 0;
		_jointMin[1] = 2;
		_waiting = true;
		_finished = false;
		_firstPacketPosition = packet.position;
		fillRobotBuffer(packet, bufferIndex);
		return true;
	}

	const int32 packetEndByte = packet.position + (packet.dataSize * (sizeof(int16) + kEOSExpansion));

	// Already read all the way past this packet (or already wrote valid samples
	// to this channel all the way past this packet), so discard it
	if (packetEndByte <= MAX(_readHeadAbs, _jointMin[bufferIndex])) {
		debugC(kDebugLevelVideo, "Rejecting packet %d, read past %d / %d", packet.position, _readHeadAbs, _jointMin[bufferIndex]);
		return true;
	}

	// The loop buffer is full, so tell the caller to send the packet again
	// later
	if (_maxWriteAbs <= _jointMin[bufferIndex]) {
		debugC(kDebugLevelVideo, "Rejecting packet %d, full buffer", packet.position);
		return false;
	}

	fillRobotBuffer(packet, bufferIndex);

	// This packet is the second primer, so allow playback to begin
	if (_firstPacketPosition != -1 && _firstPacketPosition != packet.position) {
		debugC(kDebugLevelVideo, "Done waiting. Robot audio begins");
		_waiting = false;
		_firstPacketPosition = -1;
	}

	// Only part of the packet could be read into the loop buffer before it was
	// full, so tell the caller to send the packet again later
	if (packetEndByte > _maxWriteAbs) {
		debugC(kDebugLevelVideo, "Partial read of packet %d (%d / %d)", packet.position, packetEndByte - _maxWriteAbs, packetEndByte - packet.position);
		return false;
	}

	// The entire packet was successfully read into the loop buffer
	return true;
}

void RobotAudioStream::fillRobotBuffer(const RobotAudioPacket &packet, const int8 bufferIndex) {
	int32 sourceByte = 0;

	const int32 decompressedSize = packet.dataSize * sizeof(int16);
	if (_decompressionBufferPosition != packet.position) {
		if (decompressedSize != _decompressionBufferSize) {
			_decompressionBuffer = (byte *)realloc(_decompressionBuffer, decompressedSize);
			_decompressionBufferSize = decompressedSize;
		}

		int16 carry = 0;
		deDPCM16Mono((int16 *)_decompressionBuffer, packet.data, packet.dataSize, carry);
		_decompressionBufferPosition = packet.position;
	}

	int32 numBytes = decompressedSize;
	int32 packetPosition = packet.position;
	int32 endByte = packet.position + decompressedSize * kEOSExpansion;
	int32 startByte = MAX(_readHeadAbs + bufferIndex * 2, _jointMin[bufferIndex]);
	int32 maxWriteByte = _maxWriteAbs + bufferIndex * 2;
	if (packetPosition < startByte) {
		sourceByte = (startByte - packetPosition) / kEOSExpansion;
		numBytes -= sourceByte;
		packetPosition = startByte;
	}
	if (packetPosition > maxWriteByte) {
		numBytes += (packetPosition - maxWriteByte) / kEOSExpansion;
		packetPosition = maxWriteByte;
	}
	if (endByte > maxWriteByte) {
		numBytes -= (endByte - maxWriteByte) / kEOSExpansion;
		endByte = maxWriteByte;
	}

	const int32 maxJointMin = MAX(_jointMin[0], _jointMin[1]);
	if (endByte > maxJointMin) {
		_writeHeadAbs += endByte - maxJointMin;
	}

	if (packetPosition > _jointMin[bufferIndex]) {
		int32 packetEndByte = packetPosition % _loopBufferSize;
		int32 targetBytePosition;
		int32 numBytesToEnd;
		if ((packetPosition & ~3) > (_jointMin[1 - bufferIndex] & ~3)) {
			targetBytePosition = _jointMin[1 - bufferIndex] % _loopBufferSize;
			if (targetBytePosition >= packetEndByte) {
				numBytesToEnd = _loopBufferSize - targetBytePosition;
				memset(_loopBuffer + targetBytePosition, 0, numBytesToEnd);
				targetBytePosition = (1 - bufferIndex) ? 2 : 0;
			}
			numBytesToEnd = packetEndByte - targetBytePosition;
			if (numBytesToEnd > 0) {
				memset(_loopBuffer + targetBytePosition, 0, numBytesToEnd);
			}
		}
		targetBytePosition = _jointMin[bufferIndex] % _loopBufferSize;
		if (targetBytePosition >= packetEndByte) {
			numBytesToEnd = _loopBufferSize - targetBytePosition;
			interpolateChannel((int16 *)(_loopBuffer + targetBytePosition), numBytesToEnd / (sizeof(int16) + kEOSExpansion), 0);
			targetBytePosition = bufferIndex ? 2 : 0;
		}
		numBytesToEnd = packetEndByte - targetBytePosition;
		if (numBytesToEnd > 0) {
			interpolateChannel((int16 *)(_loopBuffer + targetBytePosition), numBytesToEnd / (sizeof(int16) + kEOSExpansion), 0);
		}
	}

	if (numBytes > 0) {
		int32 targetBytePosition = packetPosition % _loopBufferSize;
		int32 packetEndByte = endByte % _loopBufferSize;
		int32 numBytesToEnd = 0;
		if (targetBytePosition >= packetEndByte) {
			numBytesToEnd = (_loopBufferSize - (targetBytePosition & ~3)) / kEOSExpansion;
			copyEveryOtherSample((int16 *)(_loopBuffer + targetBytePosition), (int16 *)(_decompressionBuffer + sourceByte), numBytesToEnd / kEOSExpansion);
			targetBytePosition = bufferIndex ? 2 : 0;
		}
		copyEveryOtherSample((int16 *)(_loopBuffer + targetBytePosition), (int16 *)(_decompressionBuffer + sourceByte + numBytesToEnd), (packetEndByte - targetBytePosition) / (sizeof(int16) + kEOSExpansion));
	}
	_jointMin[bufferIndex] = endByte;
}

void RobotAudioStream::interpolateMissingSamples(int32 numSamples) {
	int32 numBytes = numSamples * (sizeof(int16) + kEOSExpansion);
	int32 targetPosition = _readHead;
	const int32 nextReadHeadPosition = _readHeadAbs + numBytes;

	if (nextReadHeadPosition > _jointMin[1]) {
		if (nextReadHeadPosition > _jointMin[0]) {
			if (targetPosition + numBytes >= _loopBufferSize) {
				const int32 numBytesToEdge = (_loopBufferSize - targetPosition);
				memset(_loopBuffer + targetPosition, 0, numBytesToEdge);
				numBytes -= numBytesToEdge;
				targetPosition = 0;
			}
			memset(_loopBuffer + targetPosition, 0, numBytes);
			_jointMin[0] = nextReadHeadPosition;
			_jointMin[1] = nextReadHeadPosition + sizeof(int16);
		} else {
			if (targetPosition + numBytes >= _loopBufferSize) {
				const int32 numSamplesToEdge = (_loopBufferSize - targetPosition) / (sizeof(int16) + kEOSExpansion);
				interpolateChannel((int16 *)(_loopBuffer + targetPosition), numSamplesToEdge, 1);
				numSamples -= numSamplesToEdge;
				targetPosition = 0;
			}
			interpolateChannel((int16 *)(_loopBuffer + targetPosition), numSamples, 1);
			_jointMin[1] = nextReadHeadPosition + sizeof(int16);
		}
	} else if (nextReadHeadPosition > _jointMin[0]) {
		if (targetPosition + numBytes >= _loopBufferSize) {
			const int32 numSamplesToEdge = (_loopBufferSize - targetPosition) / (sizeof(int16) + kEOSExpansion);
			interpolateChannel((int16 *)(_loopBuffer + targetPosition), numSamplesToEdge, 0);
			numSamples -= numSamplesToEdge;
			targetPosition = 2;
		}
		interpolateChannel((int16 *)(_loopBuffer + targetPosition), numSamples, 0);
		_jointMin[0] = nextReadHeadPosition;
	}
}

void RobotAudioStream::finish() {
	Common::StackLock lock(_mutex);
	_finished = true;
}

RobotAudioStream::StreamState RobotAudioStream::getStatus() const {
	Common::StackLock lock(_mutex);
	StreamState status;
	status.bytesPlaying = _readHeadAbs;
	status.rate = getRate();
	status.bits = 8 * sizeof(int16);
	return status;
}

int RobotAudioStream::readBuffer(Audio::st_sample_t *outBuffer, int numSamples) {
	Common::StackLock lock(_mutex);

	if (_waiting) {
		return 0;
	}

	assert(!((_writeHeadAbs - _readHeadAbs) & 1));
	const int maxNumSamples = (_writeHeadAbs - _readHeadAbs) / sizeof(Audio::st_sample_t);
	numSamples = MIN(numSamples, maxNumSamples);

	if (!numSamples) {
		return 0;
	}

	interpolateMissingSamples(numSamples);

	Audio::st_sample_t *inBuffer = (Audio::st_sample_t *)(_loopBuffer + _readHead);

	assert(!((_loopBufferSize - _readHead) & 1));
	const int numSamplesToEnd = (_loopBufferSize - _readHead) / sizeof(Audio::st_sample_t);

	int numSamplesToRead = MIN(numSamples, numSamplesToEnd);
	Common::copy(inBuffer, inBuffer + numSamplesToRead, outBuffer);

	if (numSamplesToRead < numSamples) {
		inBuffer = (Audio::st_sample_t *)_loopBuffer;
		outBuffer += numSamplesToRead;
		numSamplesToRead = numSamples - numSamplesToRead;
		Common::copy(inBuffer, inBuffer + numSamplesToRead, outBuffer);
	}

	const int32 numBytes = numSamples * sizeof(Audio::st_sample_t);

	_readHead += numBytes;
	if (_readHead > _loopBufferSize) {
		_readHead -= _loopBufferSize;
	}
	_readHeadAbs += numBytes;
	_maxWriteAbs += numBytes;
	assert(!(_readHead & 1));
	assert(!(_readHeadAbs & 1));

	return numSamples;
}

#pragma mark -
#pragma mark RobotDecoder

RobotDecoder::RobotDecoder(SegManager *segMan) :
	_delayTime(this),
	_segMan(segMan),
	_status(kRobotStatusUninitialized),
	_audioBuffer(nullptr),
	_rawPalette((uint8 *)malloc(kRawPaletteSize)) {}

RobotDecoder::~RobotDecoder() {
	close();
	free(_rawPalette);
	free(_audioBuffer);
}

#pragma mark -
#pragma mark RobotDecoder - Initialization

void RobotDecoder::initStream(const GuiResourceId robotId) {
	const Common::String fileName = Common::String::format("%d.rbt", robotId);
	Common::SeekableReadStream *stream = SearchMan.createReadStreamForMember(fileName);
	_fileOffset = 0;

	if (stream == nullptr) {
		error("Unable to open robot file %s", fileName.c_str());
	}

	_robotId = robotId;

	const uint16 id = stream->readUint16LE();
	if (id != 0x16) {
		error("Invalid robot file %s", fileName.c_str());
	}

	// Determine the robot file's endianness by examining the version field.
	//  Some games such as Lighthouse were distributed as dual PC/Mac CDs
	//  that shared the same little endian robot files, so endianness doesn't
	//  always correspond to platform.
	stream->seek(6, SEEK_SET);
	const uint16 version = stream->readUint16BE();
	const bool bigEndian = (0 < version && version <= 0x00ff);

	_stream = new Common::SeekableSubReadStreamEndian(stream, 0, stream->size(), bigEndian, DisposeAfterUse::YES);
	_stream->seek(2, SEEK_SET);
	if (_stream->readUint32BE() != MKTAG('S', 'O', 'L', 0)) {
		error("Resource %s is not Robot type!", fileName.c_str());
	}
}

void RobotDecoder::initPlayback() {
	_startFrameNo = 0;
	_startTime = -1;
	_startingFrameNo = -1;
	_cueForceShowFrame = -1;
	_previousFrameNo = -1;
	_currentFrameNo = 0;
	_status = kRobotStatusPaused;
}

void RobotDecoder::initAudio() {
	_audioRecordInterval = RobotAudioStream::kRobotSampleRate / _frameRate;

	_expectedAudioBlockSize = _audioBlockSize - kAudioBlockHeaderSize;
	_audioBuffer = (byte *)realloc(_audioBuffer, kRobotZeroCompressSize + _expectedAudioBlockSize);

	if (_primerReservedSize != 0) {
		const int32 primerHeaderPosition = _stream->pos();
		_totalPrimerSize = _stream->readSint32();
		const int16 compressionType = _stream->readSint16();
		_evenPrimerSize = _stream->readSint32();
		_oddPrimerSize = _stream->readSint32();
		_primerPosition = _stream->pos();

		if (compressionType) {
			error("Unknown audio header compression type %d", compressionType);
		}

		if (_evenPrimerSize + _oddPrimerSize != _primerReservedSize) {
			_stream->seek(primerHeaderPosition + _primerReservedSize, SEEK_SET);
		}
	} else if (_primerZeroCompressFlag) {
		_evenPrimerSize = 19922;
		_oddPrimerSize = 21024;
	}

	_firstAudioRecordPosition = _evenPrimerSize * 2;

	const int usedEachFrame = (RobotAudioStream::kRobotSampleRate / 2) / _frameRate;
	_maxSkippablePackets = MAX(0, _audioBlockSize / usedEachFrame - 1);
}

void RobotDecoder::initVideo(const int16 x, const int16 y, const int16 scale, const reg_t plane, const bool hasPalette, const uint16 paletteSize) {
	_position = Common::Point(x, y);

	_scaleInfo.x = scale;
	_scaleInfo.y = scale;
	_scaleInfo.signal = scale == 128 ? kScaleSignalNone : kScaleSignalManual;

	_plane = g_sci->_gfxFrameout->getPlanes().findByObject(plane);
	if (_plane == nullptr) {
		error("Invalid plane %04x:%04x passed to RobotDecoder::open", PRINT_REG(plane));
	}

	_planeId = plane;
	_minFrameRate = _frameRate - kMaxFrameRateDrift;
	_maxFrameRate = _frameRate + kMaxFrameRateDrift;

	if (_xResolution == 0 || _yResolution == 0) {
		// In SSCI, default values were taken from RESOURCE.CFG hires property
		// if it exists, but no games seem to take advantage of this
		_xResolution = g_sci->_gfxFrameout->getScreenWidth();
		_yResolution = g_sci->_gfxFrameout->getScreenHeight();
	}

	if (hasPalette) {
		_stream->read(_rawPalette, paletteSize);
	} else {
		_stream->seek(paletteSize, SEEK_CUR);
	}

	_screenItemList.reserve(kScreenItemListSize);
	_maxCelArea.reserve(kFixedCelListSize);

	// Fixed cel buffers are for version 5 and newer
	_fixedCels.reserve(MIN(_maxCelsPerFrame, (int16)kFixedCelListSize));
	_celDecompressionBuffer.reserve(_maxCelArea[0] + SciBitmap::getBitmapHeaderSize() + kRawPaletteSize);
	_celDecompressionArea = _maxCelArea[0];
}

void RobotDecoder::initRecordAndCuePositions() {
	PositionList recordSizes;
	_videoSizes.reserve(_numFramesTotal);
	_recordPositions.reserve(_numFramesTotal);
	recordSizes.reserve(_numFramesTotal);

	switch(_version) {
	case 5: // 16-bit sizes and positions
		for (int i = 0; i < _numFramesTotal; ++i) {
			_videoSizes.push_back(_stream->readUint16());
		}
		for (int i = 0; i < _numFramesTotal; ++i) {
			recordSizes.push_back(_stream->readUint16());
		}
		break;
	case 6: // 32-bit sizes and positions
		for (int i = 0; i < _numFramesTotal; ++i) {
			_videoSizes.push_back(_stream->readSint32());
		}
		for (int i = 0; i < _numFramesTotal; ++i) {
			recordSizes.push_back(_stream->readSint32());
		}
		break;
	default:
		error("Unknown Robot version %d", _version);
	}

	for (int i = 0; i < kCueListSize; ++i) {
		_cueTimes[i] = _stream->readSint32();
	}

	for (int i = 0; i < kCueListSize; ++i) {
		_cueValues[i] = _stream->readUint16();
	}

	Common::copy(_cueTimes, _cueTimes + kCueListSize, _masterCueTimes);

	int bytesRemaining = (_stream->pos() - _fileOffset) % kRobotFrameSize;
	if (bytesRemaining != 0) {
		_stream->seek(kRobotFrameSize - bytesRemaining, SEEK_CUR);
	}

	int position = _stream->pos();
	_recordPositions.push_back(position);
	for (int i = 0; i < _numFramesTotal - 1; ++i) {
		position += recordSizes[i];
		_recordPositions.push_back(position);
	}
}

#pragma mark -
#pragma mark RobotDecoder - Playback

void RobotDecoder::open(const GuiResourceId robotId, const reg_t plane, const int16 priority, const int16 x, const int16 y, const int16 scale) {
	if (_status != kRobotStatusUninitialized) {
		close();
	}

	initStream(robotId);

	_version = _stream->readUint16();

	// TODO: Version 4 for PQ:SWAT demo?
	if (_version < 5 || _version > 6) {
		error("Unsupported version %d of Robot resource", _version);
	}

	debugC(kDebugLevelVideo, "Opening version %d robot %d", _version, robotId);

	initPlayback();

	_syncFrame = true;
	_audioBlockSize = _stream->readUint16();
	_primerZeroCompressFlag = _stream->readSint16();
	_stream->seek(2, SEEK_CUR); // unused
	_numFramesTotal = _stream->readUint16();
	const uint16 paletteSize = _stream->readUint16();
	_primerReservedSize = _stream->readUint16();
	_xResolution = _stream->readSint16();
	_yResolution = _stream->readSint16();
	const bool hasPalette = (bool)_stream->readByte();
	_hasAudio = (bool)_stream->readByte();
	_stream->seek(2, SEEK_CUR); // unused
	_frameRate = _normalFrameRate = _stream->readSint16();
	_isHiRes = (bool)_stream->readSint16();
	_maxSkippablePackets = _stream->readSint16();
	_maxCelsPerFrame = _stream->readSint16();

	// used for memory preallocation of fixed cels
	_maxCelArea.push_back(_stream->readSint32());
	_maxCelArea.push_back(_stream->readSint32());
	_maxCelArea.push_back(_stream->readSint32());
	_maxCelArea.push_back(_stream->readSint32());
	_stream->seek(8, SEEK_CUR); // reserved

	if (_hasAudio) {
		initAudio();
	} else {
		_stream->seek(_primerReservedSize, SEEK_CUR);
	}

	_priority = priority;
	initVideo(x, y, scale, plane, hasPalette, paletteSize);
	initRecordAndCuePositions();
}

void RobotDecoder::close() {
	if (_status == kRobotStatusUninitialized) {
		return;
	}

	debugC(kDebugLevelVideo, "Closing robot");

	for (CelHandleList::size_type i = 0; i < _celHandles.size(); ++i) {
		if (_celHandles[i].status == CelHandleInfo::kFrameLifetime) {
			_segMan->freeBitmap(_celHandles[i].bitmapId);
		}
	}
	_celHandles.clear();

	for (FixedCelsList::size_type i = 0; i < _fixedCels.size(); ++i) {
		_segMan->freeBitmap(_fixedCels[i]);
	}
	_fixedCels.clear();

	if (g_sci->_gfxFrameout->getPlanes().findByObject(_planeId) != nullptr) {
		for (RobotScreenItemList::size_type i = 0; i < _screenItemList.size(); ++i) {
			if (_screenItemList[i] != nullptr) {
				g_sci->_gfxFrameout->deleteScreenItem(*_screenItemList[i]);
			}
		}
	}
	_screenItemList.clear();

	if (_hasAudio) {
		_audioList.reset();
	}

	_robotId = -1;
	_planeId = NULL_REG;
	_plane = nullptr;
	_status = kRobotStatusUninitialized;
	_videoSizes.clear();
	_recordPositions.clear();
	_celDecompressionBuffer.clear();
	_doVersion5Scratch.clear();
	delete _stream;
	_stream = nullptr;
}

void RobotDecoder::pause() {
	if (_status != kRobotStatusPlaying) {
		return;
	}

	if (_hasAudio) {
		_audioList.stopAudioNow();
	}

	_status = kRobotStatusPaused;
	_frameRate = _normalFrameRate;
}

void RobotDecoder::resume() {
	if (_status != kRobotStatusPaused) {
		return;
	}

	_startingFrameNo = _currentFrameNo;
	_status = kRobotStatusPlaying;
	if (_hasAudio) {
		primeAudio(_currentFrameNo * 60 / _frameRate);
		_syncFrame = true;
	}

	setRobotTime(_currentFrameNo);
	for (int i = 0; i < kCueListSize; ++i) {
		if (_masterCueTimes[i] != -1 && _masterCueTimes[i] < _currentFrameNo) {
			_cueTimes[i] = -1;
		} else {
			_cueTimes[i] = _masterCueTimes[i];
		}
	}
}

void RobotDecoder::showFrame(const uint16 frameNo, const uint16 newX, const uint16 newY, const uint16 newPriority) {
	debugC(kDebugLevelVideo, "Show frame %d (%d %d %d)", frameNo, newX, newY, newPriority);

	if (newX != kUnspecified) {
		_position.x = newX;
	}

	if (newY != kUnspecified) {
		_position.y = newY;
	}

	if (newPriority != kUnspecified) {
		_priority = newPriority;
	}

	_currentFrameNo = frameNo;
	pause();

	if (frameNo != _previousFrameNo) {
		seekToFrame(frameNo);
		doVersion5(false);
	} else {
		for (RobotScreenItemList::size_type i = 0; i < _screenItemList.size(); ++i) {
			if (_isHiRes) {
				SciBitmap &bitmap = *_segMan->lookupBitmap(_celHandles[i].bitmapId);

				const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
				const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
				const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
				const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();

				if (scriptWidth == kLowResX && scriptHeight == kLowResY) {
					const Ratio lowResToScreenX(screenWidth, kLowResX);
					const Ratio lowResToScreenY(screenHeight, kLowResY);
					const Ratio screenToLowResX(kLowResX, screenWidth);
					const Ratio screenToLowResY(kLowResY, screenHeight);

					const int16 scaledX = _originalScreenItemX[i] + (_position.x * lowResToScreenX).toInt();
					const int16 scaledY1 = _originalScreenItemY[i] + (_position.y * lowResToScreenY).toInt();
					const int16 scaledY2 = scaledY1 + bitmap.getHeight() - 1;

					const int16 lowResX = (scaledX * screenToLowResX).toInt();
					const int16 lowResY = (scaledY2 * screenToLowResY).toInt();

					bitmap.setOrigin(Common::Point(
						(scaledX - (lowResX * lowResToScreenX).toInt()) * -1,
						(lowResY * lowResToScreenY).toInt() - scaledY1
					));

					_screenItemX[i] = lowResX;
					_screenItemY[i] = lowResY;
				} else {
					const int16 scaledX = _originalScreenItemX[i] + _position.x;
					const int16 scaledY = _originalScreenItemY[i] + _position.y + bitmap.getHeight() - 1;
					bitmap.setOrigin(Common::Point(0, bitmap.getHeight() - 1));
					_screenItemX[i] = scaledX;
					_screenItemY[i] = scaledY;
				}
			} else {
				_screenItemX[i] = _originalScreenItemX[i] + _position.x;
				_screenItemY[i] = _originalScreenItemY[i] + _position.y;
			}

			if (_screenItemList[i] == nullptr) {
				CelInfo32 celInfo;
				celInfo.type = kCelTypeMem;
				celInfo.bitmap = _celHandles[i].bitmapId;
				ScreenItem *screenItem = new ScreenItem(_planeId, celInfo);
				_screenItemList[i] = screenItem;
				screenItem->_position = Common::Point(_screenItemX[i], _screenItemY[i]);
				if (_priority == -1) {
					screenItem->_fixedPriority = false;
				} else {
					screenItem->_priority = _priority;
					screenItem->_fixedPriority = true;
				}
				g_sci->_gfxFrameout->addScreenItem(*screenItem);
			} else {
				ScreenItem *screenItem = _screenItemList[i];
				screenItem->_celInfo.bitmap = _celHandles[i].bitmapId;
				screenItem->_position = Common::Point(_screenItemX[i], _screenItemY[i]);
				if (_priority == -1) {
					screenItem->_fixedPriority = false;
				} else {
					screenItem->_priority = _priority;
					screenItem->_fixedPriority = true;
				}
				g_sci->_gfxFrameout->updateScreenItem(*screenItem);
			}
		}
	}

	_previousFrameNo = frameNo;
}

int16 RobotDecoder::getCue() const {
	if (_status == kRobotStatusUninitialized ||
		_status == kRobotStatusPaused ||
		_syncFrame) {
		return 0;
	}

	if (_status == kRobotStatusEnd) {
		return -1;
	}

	const uint16 estimatedNextFrameNo = MIN(calculateNextFrameNo(_delayTime.predictedTicks()), _numFramesTotal);

	for (int i = 0; i < kCueListSize; ++i) {
		if (_cueTimes[i] != -1 && _cueTimes[i] <= estimatedNextFrameNo) {
			if (_cueTimes[i] >= _previousFrameNo) {
				_cueForceShowFrame = _cueTimes[i] + 1;
			}

			_cueTimes[i] = -1;
			return _cueValues[i];
		}
	}

	return 0;
}

int16 RobotDecoder::getFrameNo() const {
	if (_status == kRobotStatusUninitialized) {
		return 0;
	}

	return _currentFrameNo;
}

RobotDecoder::RobotStatus RobotDecoder::getStatus() const {
	return _status;
}

bool RobotDecoder::seekToFrame(const int frameNo) {
	return _stream->seek(_recordPositions[frameNo], SEEK_SET);
}

void RobotDecoder::setRobotTime(const int frameNo) {
	_startTime = getTickCount();
	_startFrameNo = frameNo;
}

#pragma mark -
#pragma mark RobotDecoder - Timing

RobotDecoder::DelayTime::DelayTime(RobotDecoder *decoder) :
	_decoder(decoder) {
	for (int i = 0; i < kDelayListSize; ++i) {
		_timestamps[i] = i;
		_delays[i] = 0;
	}

	_oldestTimestamp = 0;
	_newestTimestamp = kDelayListSize - 1;
	_startTime = 0;
}

void RobotDecoder::DelayTime::startTiming() {
	_startTime = _decoder->getTickCount();
}

void RobotDecoder::DelayTime::endTiming() {
	const int timeDelta = _decoder->getTickCount() - _startTime;
	for (uint i = 0; i < kDelayListSize; ++i) {
		if (_timestamps[i] == _oldestTimestamp) {
			_timestamps[i] = ++_newestTimestamp;
			_delays[i] = timeDelta;
			break;
		}
	}
	++_newestTimestamp;
	_startTime = 0;
	sortList();
}

bool RobotDecoder::DelayTime::timingInProgress() const {
	return _startTime != 0;
}

int RobotDecoder::DelayTime::predictedTicks() const {
	return _delays[kDelayListSize / 2];
}

void RobotDecoder::DelayTime::sortList() {
	for (uint i = 0; i < kDelayListSize - 1; ++i) {
		int smallestDelay = _delays[i];
		uint smallestIndex = i;

		for (uint j = i + 1; j < kDelayListSize - 1; ++j) {
			if (_delays[j] < smallestDelay) {
				smallestDelay = _delays[j];
				smallestIndex = j;
			}
		}

		if (smallestIndex != i) {
			SWAP(_delays[i], _delays[smallestIndex]);
			SWAP(_timestamps[i], _timestamps[smallestIndex]);
		}
	}
}

uint16 RobotDecoder::calculateNextFrameNo(const uint32 extraTicks) const {
	return ticksToFrames(getTickCount() + extraTicks - _startTime) + _startFrameNo;
}

uint32 RobotDecoder::ticksToFrames(const uint32 ticks) const {
	return (ticks * _frameRate) / 60;
}

uint32 RobotDecoder::getTickCount() const {
	return g_sci->getTickCount();
}

#pragma mark -
#pragma mark RobotDecoder - Audio

RobotDecoder::AudioList::AudioList() :
	_blocks(),
	_blocksSize(0),
	_oldestBlockIndex(0),
	_newestBlockIndex(0),
	_startOffset(0),
	_status(kRobotAudioReady) {}

void RobotDecoder::AudioList::startAudioNow() {
	submitDriverMax();
	g_sci->_audio32->resume(kRobotChannel);
	_status = kRobotAudioPlaying;
}

void RobotDecoder::AudioList::stopAudio() {
	g_sci->_audio32->finishRobotAudio();
	freeAudioBlocks();
	_status = kRobotAudioStopping;
}

void RobotDecoder::AudioList::stopAudioNow() {
	if (_status == kRobotAudioPlaying || _status == kRobotAudioStopping || _status == kRobotAudioPaused) {
		g_sci->_audio32->stopRobotAudio();
		_status = kRobotAudioStopped;
	}

	freeAudioBlocks();
}

void RobotDecoder::AudioList::submitDriverMax() {
	while (_blocksSize != 0) {
		if (!_blocks[_oldestBlockIndex]->submit(_startOffset)) {
			return;
		}

		delete _blocks[_oldestBlockIndex];
		_blocks[_oldestBlockIndex] = nullptr;
		++_oldestBlockIndex;
		if (_oldestBlockIndex == kAudioListSize) {
			_oldestBlockIndex = 0;
		}

		--_blocksSize;
	}
}

void RobotDecoder::AudioList::addBlock(const int position, const int size, const byte *data) {
	assert(data != nullptr);
	assert(size >= 0);
	assert(position >= -1);

	if (_blocksSize == kAudioListSize) {
		delete _blocks[_oldestBlockIndex];
		_blocks[_oldestBlockIndex] = nullptr;
		++_oldestBlockIndex;
		if (_oldestBlockIndex == kAudioListSize) {
			_oldestBlockIndex = 0;
		}
		--_blocksSize;
	}

	if (_blocksSize == 0) {
		_oldestBlockIndex = _newestBlockIndex = 0;
	} else {
		++_newestBlockIndex;
		if (_newestBlockIndex == kAudioListSize) {
			_newestBlockIndex = 0;
		}
	}

	_blocks[_newestBlockIndex] = new AudioBlock(position, size, data);
	++_blocksSize;
}

void RobotDecoder::AudioList::reset() {
	stopAudioNow();
	_startOffset = 0;
	_status = kRobotAudioReady;
}

void RobotDecoder::AudioList::prepareForPrimer() {
	g_sci->_audio32->pause(kRobotChannel);
	_status = kRobotAudioPaused;
}

void RobotDecoder::AudioList::setAudioOffset(const int offset) {
	_startOffset = offset;
}

RobotDecoder::AudioList::AudioBlock::AudioBlock(const int position, const int size, const byte* const data) :
	_position(position),
	_size(size) {
	_data = (byte *)malloc(size);
	memcpy(_data, data, size);
}

RobotDecoder::AudioList::AudioBlock::~AudioBlock() {
	free(_data);
}

bool RobotDecoder::AudioList::AudioBlock::submit(const int startOffset) {
	assert(_data != nullptr);
	RobotAudioStream::RobotAudioPacket packet(_data, _size, (_position - startOffset) * 2);
	return g_sci->_audio32->playRobotAudio(packet);
}

void RobotDecoder::AudioList::freeAudioBlocks() {
	while (_blocksSize != 0) {
		delete _blocks[_oldestBlockIndex];
		_blocks[_oldestBlockIndex] = nullptr;
		++_oldestBlockIndex;
		if (_oldestBlockIndex == kAudioListSize) {
			_oldestBlockIndex = 0;
		}

		--_blocksSize;
	}
}

bool RobotDecoder::primeAudio(const uint32 startTick) {
	bool success = true;
	_audioList.reset();

	if (startTick == 0) {
		_audioList.prepareForPrimer();
		byte *evenPrimerBuff = new byte[_evenPrimerSize];
		byte *oddPrimerBuff = new byte[_oddPrimerSize];

		success = readPrimerData(evenPrimerBuff, oddPrimerBuff);
		if (success) {
			if (_evenPrimerSize != 0) {
				_audioList.addBlock(0, _evenPrimerSize, evenPrimerBuff);
			}
			if (_oddPrimerSize != 0) {
				_audioList.addBlock(1, _oddPrimerSize, oddPrimerBuff);
			}
		}

		delete[] evenPrimerBuff;
		delete[] oddPrimerBuff;
	} else {
		assert(_evenPrimerSize * 2 >= _audioRecordInterval || _oddPrimerSize * 2 >= _audioRecordInterval);

		int audioStartFrame = 0;
		int videoStartFrame = startTick * _frameRate / 60;
		assert(videoStartFrame < _numFramesTotal);

		int audioStartPosition = (startTick * RobotAudioStream::kRobotSampleRate) / 60;
		if (audioStartPosition & 1) {
			audioStartPosition--;
		}
		_audioList.setAudioOffset(audioStartPosition);
		_audioList.prepareForPrimer();

		if (audioStartPosition < _evenPrimerSize * 2 ||
			audioStartPosition + 1 < _oddPrimerSize * 2) {

			byte *evenPrimerBuffer = new byte[_evenPrimerSize];
			byte *oddPrimerBuffer = new byte[_oddPrimerSize];
			success = readPrimerData(evenPrimerBuffer, oddPrimerBuffer);
			if (success) {
				int halfAudioStartPosition = audioStartPosition / 2;
				if (audioStartPosition < _evenPrimerSize * 2) {
					_audioList.addBlock(audioStartPosition, _evenPrimerSize - halfAudioStartPosition, &evenPrimerBuffer[halfAudioStartPosition]);
				}

				if (audioStartPosition + 1 < _oddPrimerSize * 2) {
					_audioList.addBlock(audioStartPosition + 1, _oddPrimerSize - halfAudioStartPosition, &oddPrimerBuffer[halfAudioStartPosition]);
				}
			}

			delete[] evenPrimerBuffer;
			delete[] oddPrimerBuffer;
		}

		if (audioStartPosition >= _firstAudioRecordPosition) {
			int audioRecordSize = _expectedAudioBlockSize;
			assert(audioRecordSize > 0);
			assert(_audioRecordInterval > 0);
			assert(_firstAudioRecordPosition >= 0);

			audioStartFrame = (audioStartPosition - _firstAudioRecordPosition) / _audioRecordInterval;
			assert(audioStartFrame < videoStartFrame);

			if (audioStartFrame > 0) {
				int lastAudioFrame = audioStartFrame - 1;
				int oddRemainder = lastAudioFrame & 1;
				int audioRecordStart = (lastAudioFrame * _audioRecordInterval) + oddRemainder + _firstAudioRecordPosition;
				int audioRecordEnd = (audioRecordStart + ((audioRecordSize - 1) * 2)) + oddRemainder + _firstAudioRecordPosition;

				if (audioStartPosition >= audioRecordStart && audioStartPosition <= audioRecordEnd) {
					--audioStartFrame;
				}
			}

			assert(!(audioStartPosition & 1));
			if (audioStartFrame & 1) {
				++audioStartPosition;
			}

			if (!readPartialAudioRecordAndSubmit(audioStartFrame, audioStartPosition)) {
				return false;
			}

			++audioStartFrame;
			assert(audioStartFrame < videoStartFrame);

			int oddRemainder = audioStartFrame & 1;
			int audioRecordStart = (audioStartFrame * _audioRecordInterval) + oddRemainder + _firstAudioRecordPosition;
			int audioRecordEnd = (audioRecordStart + ((audioRecordSize - 1) * 2)) + oddRemainder + _firstAudioRecordPosition;

			if (audioStartPosition >= audioRecordStart && audioStartPosition <= audioRecordEnd) {
				if (!readPartialAudioRecordAndSubmit(audioStartFrame, audioStartPosition + 1)) {
					return false;
				}

				++audioStartFrame;
			}
		}

		int audioPosition, audioSize;
		for (int i = audioStartFrame; i < videoStartFrame; i++) {
			if (!readAudioDataFromRecord(i, _audioBuffer, audioPosition, audioSize)) {
				break;
			}

			_audioList.addBlock(audioPosition, audioSize, _audioBuffer);
		}
	}

	return success;
}

bool RobotDecoder::readPrimerData(byte *outEvenBuffer, byte *outOddBuffer) {
	if (_primerReservedSize != 0) {
		if (_totalPrimerSize != 0) {
			_stream->seek(_primerPosition, SEEK_SET);
			if (_evenPrimerSize > 0) {
				_stream->read(outEvenBuffer, _evenPrimerSize);
			}

			if (_oddPrimerSize > 0) {
				_stream->read(outOddBuffer, _oddPrimerSize);
			}
		}
	} else if (_primerZeroCompressFlag) {
		memset(outEvenBuffer, 0, _evenPrimerSize);
		memset(outOddBuffer, 0, _oddPrimerSize);
	} else {
		error("ReadPrimerData - Flags corrupt");
	}

	return !_stream->err();
}

bool RobotDecoder::readAudioDataFromRecord(const int frameNo, byte *outBuffer, int &outAudioPosition, int &outAudioSize) {
	_stream->seek(_recordPositions[frameNo] + _videoSizes[frameNo], SEEK_SET);
	_audioList.submitDriverMax();

	// Compressed absolute position of the audio block in the audio stream
	const int position = _stream->readSint32();

	// Size of the block of audio, excluding the audio block header
	int size = _stream->readSint32();

	assert(size <= _expectedAudioBlockSize);

	if (position == 0) {
		return false;
	}

	if (size != _expectedAudioBlockSize) {
		memset(outBuffer, 0, kRobotZeroCompressSize);
		_stream->read(outBuffer + kRobotZeroCompressSize, size);
		size += kRobotZeroCompressSize;
	} else {
		_stream->read(outBuffer, size);
	}

	outAudioPosition = position;
	outAudioSize = size;
	return !_stream->err();
}

bool RobotDecoder::readPartialAudioRecordAndSubmit(const int startFrame, const int startPosition) {
	int audioPosition, audioSize;
	bool success = readAudioDataFromRecord(startFrame, _audioBuffer, audioPosition, audioSize);
	if (success) {
		const int relativeStartOffset = (startPosition - audioPosition) / 2;
		_audioList.addBlock(startPosition, audioSize - relativeStartOffset, _audioBuffer + relativeStartOffset);
	}

	return success;
}

#pragma mark -
#pragma mark RobotDecoder - Rendering

uint16 RobotDecoder::getFrameSize(Common::Rect &outRect) const {
	assert(_plane != nullptr);

	if (_screenItemList.size() == 0) {
		outRect.clip(0, 0);
		return _numFramesTotal;
	}

	outRect = _screenItemList[0]->getNowSeenRect(*_plane);
	for (RobotScreenItemList::size_type i = 1; i < _screenItemList.size(); ++i) {
		ScreenItem &screenItem = *_screenItemList[i];
		outRect.extend(screenItem.getNowSeenRect(*_plane));
	}

	return _numFramesTotal;
}

void RobotDecoder::doRobot() {
	if (_status != kRobotStatusPlaying) {
		return;
	}

	if (!_syncFrame) {
		if (_cueForceShowFrame != -1) {
			_currentFrameNo = _cueForceShowFrame;
			_cueForceShowFrame = -1;
		} else {
			const int nextFrameNo = calculateNextFrameNo(_delayTime.predictedTicks());
			if (nextFrameNo < _currentFrameNo) {
				return;
			}
			_currentFrameNo = nextFrameNo;
		}
	}

	if (_currentFrameNo >= _numFramesTotal) {
		const int finalFrameNo = _numFramesTotal - 1;
		if (_previousFrameNo == finalFrameNo) {
			_status = kRobotStatusEnd;
			if (_hasAudio) {
				_audioList.stopAudio();
				_frameRate = _normalFrameRate;
				_hasAudio = false;
			}
			return;
		} else {
			_currentFrameNo = finalFrameNo;
		}
	}

	if (_currentFrameNo == _previousFrameNo) {
		_audioList.submitDriverMax();
		return;
	}

	if (_hasAudio) {
		for (int candidateFrameNo = _previousFrameNo + _maxSkippablePackets + 1; candidateFrameNo < _currentFrameNo; candidateFrameNo += _maxSkippablePackets + 1) {

			_audioList.submitDriverMax();

			int audioPosition, audioSize;
			if (readAudioDataFromRecord(candidateFrameNo, _audioBuffer, audioPosition, audioSize)) {
				_audioList.addBlock(audioPosition, audioSize, _audioBuffer);
			}
		}
		_audioList.submitDriverMax();
	}

	_delayTime.startTiming();
	seekToFrame(_currentFrameNo);
	doVersion5();
	if (_hasAudio) {
		_audioList.submitDriverMax();
	}
}

void RobotDecoder::frameAlmostVisible() {
	if (_status == kRobotStatusPlaying && !_syncFrame) {
		if (_previousFrameNo != _currentFrameNo) {
			while (calculateNextFrameNo() < _currentFrameNo) {
				_audioList.submitDriverMax();
			}
		}
	}
}

void RobotDecoder::frameNowVisible() {
	if (_status != kRobotStatusPlaying) {
		return;
	}

	if (_syncFrame) {
		_syncFrame = false;
		if (_hasAudio) {
			_audioList.startAudioNow();
			_checkAudioSyncTime = _startTime + kAudioSyncCheckInterval;
		}

		setRobotTime(_currentFrameNo);
	}

	if (_delayTime.timingInProgress()) {
		_delayTime.endTiming();
	}

	if (_hasAudio) {
		_audioList.submitDriverMax();
	}

	if (_previousFrameNo != _currentFrameNo) {
		_previousFrameNo = _currentFrameNo;
	}

	if (!_syncFrame && _hasAudio && getTickCount() >= _checkAudioSyncTime) {
		RobotAudioStream::StreamState status;
		const bool success = g_sci->_audio32->queryRobotAudio(status);
		if (!success) {
			return;
		}

		const int bytesPerFrame = status.rate / _normalFrameRate * (status.bits == 16 ? 2 : 1);
		// check again in 1/3rd second
		_checkAudioSyncTime = getTickCount() + 60 / 3;

		const int currentVideoFrameNo = calculateNextFrameNo() - _startingFrameNo;
		const int currentAudioFrameNo = status.bytesPlaying / bytesPerFrame;
		debugC(kDebugLevelVideo, "Video frame %d %s audio frame %d", currentVideoFrameNo, currentVideoFrameNo == currentAudioFrameNo ? "=" : currentVideoFrameNo < currentAudioFrameNo ? "<" : ">", currentAudioFrameNo);
		if (currentVideoFrameNo < _numFramesTotal &&
			currentAudioFrameNo < _numFramesTotal) {

			bool shouldResetRobotTime = false;

			if (currentAudioFrameNo < currentVideoFrameNo - 1 && _frameRate != _minFrameRate) {
				debugC(kDebugLevelVideo, "[v] Reducing frame rate");
				_frameRate = _minFrameRate;
				shouldResetRobotTime = true;
			} else if (currentAudioFrameNo > currentVideoFrameNo + 1 && _frameRate != _maxFrameRate) {
				debugC(kDebugLevelVideo, "[^] Increasing frame rate");
				_frameRate = _maxFrameRate;
				shouldResetRobotTime = true;
			} else if (_frameRate != _normalFrameRate) {
				debugC(kDebugLevelVideo, "[=] Setting to normal frame rate");
				_frameRate = _normalFrameRate;
				shouldResetRobotTime = true;
			}

			if (shouldResetRobotTime) {
				if (currentAudioFrameNo < _currentFrameNo) {
					setRobotTime(_currentFrameNo);
				} else {
					setRobotTime(currentAudioFrameNo);
				}
			}
		}
	}
}

void RobotDecoder::expandCel(byte* target, const byte* source, const int16 celWidth, const int16 celHeight) const {
	assert(source != nullptr && target != nullptr);

	const int sourceHeight = (celHeight * _verticalScaleFactor) / 100;
	assert(sourceHeight > 0);

	const int16 numerator = celHeight;
	const int16 denominator = sourceHeight;
	int remainder = 0;
	for (int16 y = sourceHeight - 1; y >= 0; --y) {
		remainder += numerator;
		int16 linesToDraw = remainder / denominator;
		remainder %= denominator;

		while (linesToDraw--) {
			memcpy(target, source, celWidth);
			target += celWidth;
		}

		source += celWidth;
	}
}

int16 RobotDecoder::getPriority() const {
	return _priority;
}

void RobotDecoder::setPriority(const int16 newPriority) {
	_priority = newPriority;
}

void RobotDecoder::doVersion5(const bool shouldSubmitAudio) {
	const RobotScreenItemList::size_type oldScreenItemCount = _screenItemList.size();
	const int videoSize = _videoSizes[_currentFrameNo];
	_doVersion5Scratch.resize(videoSize);

	byte *videoFrameData = _doVersion5Scratch.begin();

	if (!_stream->read(videoFrameData, videoSize)) {
		error("RobotDecoder::doVersion5: Read error");
	}

	const RobotScreenItemList::size_type screenItemCount = READ_SCI11ENDIAN_UINT16(videoFrameData);

	if (screenItemCount > kScreenItemListSize) {
		return;
	}

	if (_hasAudio &&
		(getSciVersion() < SCI_VERSION_3 || shouldSubmitAudio)) {
		int audioPosition, audioSize;
		if (readAudioDataFromRecord(_currentFrameNo, _audioBuffer, audioPosition, audioSize)) {
			_audioList.addBlock(audioPosition, audioSize, _audioBuffer);
		}
	}

	if (screenItemCount > oldScreenItemCount) {
		_screenItemList.resize(screenItemCount);
		_screenItemX.resize(screenItemCount);
		_screenItemY.resize(screenItemCount);
		_originalScreenItemX.resize(screenItemCount);
		_originalScreenItemY.resize(screenItemCount);
	}

	createCels5(videoFrameData + 2, screenItemCount, true);
	for (RobotScreenItemList::size_type i = 0; i < screenItemCount; ++i) {
		Common::Point position(_screenItemX[i], _screenItemY[i]);

		if (_scaleInfo.signal == kScaleSignalManual) {
			position.x = (position.x * _scaleInfo.x) / 128;
			position.y = (position.y * _scaleInfo.y) / 128;
		}

		if (_screenItemList[i] == nullptr) {
			CelInfo32 celInfo;
			celInfo.bitmap = _celHandles[i].bitmapId;
			ScreenItem *screenItem = new ScreenItem(_planeId, celInfo, position, _scaleInfo);
			_screenItemList[i] = screenItem;

			if (_priority == -1) {
				screenItem->_fixedPriority = false;
			} else {
				screenItem->_fixedPriority = true;
				screenItem->_priority = _priority;
			}
			g_sci->_gfxFrameout->addScreenItem(*screenItem);
		} else {
			ScreenItem *screenItem = _screenItemList[i];
			screenItem->_celInfo.bitmap = _celHandles[i].bitmapId;
			screenItem->_position = position;

			if (_priority == -1) {
				screenItem->_fixedPriority = false;
			} else {
				screenItem->_fixedPriority = true;
				screenItem->_priority = _priority;
			}
			g_sci->_gfxFrameout->updateScreenItem(*screenItem);
		}
	}

	for (RobotScreenItemList::size_type i = screenItemCount; i < oldScreenItemCount; ++i) {
		if (_screenItemList[i] != nullptr) {
			g_sci->_gfxFrameout->deleteScreenItem(*_screenItemList[i]);
			_screenItemList[i] = nullptr;
		}
	}

	if (screenItemCount < oldScreenItemCount) {
		_screenItemList.resize(screenItemCount);
		_screenItemX.resize(screenItemCount);
		_screenItemY.resize(screenItemCount);
		_originalScreenItemX.resize(screenItemCount);
		_originalScreenItemY.resize(screenItemCount);
	}
}

void RobotDecoder::createCels5(const byte *rawVideoData, const int16 numCels, const bool usePalette) {
	preallocateCelMemory(rawVideoData, numCels);
	for (int16 i = 0; i < numCels; ++i) {
		rawVideoData += createCel5(rawVideoData, i, usePalette);
	}
}

uint32 RobotDecoder::createCel5(const byte *rawVideoData, const int16 screenItemIndex, const bool usePalette) {
	_verticalScaleFactor = rawVideoData[1];
	const int16 celWidth = (int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 2);
	const int16 celHeight = (int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 4);
	const Common::Point celPosition((int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 10),
									(int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 12));
	const uint16 dataSize = READ_SCI11ENDIAN_UINT16(rawVideoData + 14);
	const int16 numDataChunks = (int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 16);

	rawVideoData += kCelHeaderSize;

	const int16 scriptWidth = g_sci->_gfxFrameout->getScriptWidth();
	const int16 scriptHeight = g_sci->_gfxFrameout->getScriptHeight();
	const int16 screenWidth = g_sci->_gfxFrameout->getScreenWidth();
	const int16 screenHeight = g_sci->_gfxFrameout->getScreenHeight();

	Common::Point origin;
	if (scriptWidth == kLowResX && scriptHeight == kLowResY) {
		const Ratio lowResToScreenX(screenWidth, kLowResX);
		const Ratio lowResToScreenY(screenHeight, kLowResY);
		const Ratio screenToLowResX(kLowResX, screenWidth);
		const Ratio screenToLowResY(kLowResY, screenHeight);

		const int16 scaledX = celPosition.x + (_position.x * lowResToScreenX).toInt();
		const int16 scaledY1 = celPosition.y + (_position.y * lowResToScreenY).toInt();
		const int16 scaledY2 = scaledY1 + celHeight - 1;

		const int16 lowResX = (scaledX * screenToLowResX).toInt();
		const int16 lowResY = (scaledY2 * screenToLowResY).toInt();

		origin.x = (scaledX - (lowResX * lowResToScreenX).toInt()) * -1;
		origin.y = (lowResY * lowResToScreenY).toInt() - scaledY1;
		_screenItemX[screenItemIndex] = lowResX;
		_screenItemY[screenItemIndex] = lowResY;

		debugC(kDebugLevelVideo, "Low resolution position c: %d %d l: %d/%d %d/%d d: %d %d s: %d/%d %d/%d x: %d y: %d", celPosition.x, celPosition.y, lowResX, scriptWidth, lowResY, scriptHeight, origin.x, origin.y, scaledX, screenWidth, scaledY2, screenHeight, scaledX - origin.x, scaledY2 - origin.y);
	} else {
		const int16 highResX = celPosition.x + _position.x;
		const int16 highResY = celPosition.y + _position.y + celHeight - 1;

		origin.x = 0;
		origin.y = celHeight - 1;
		_screenItemX[screenItemIndex] = highResX;
		_screenItemY[screenItemIndex] = highResY;

		debugC(kDebugLevelVideo, "High resolution position c: %d %d s: %d %d d: %d %d", celPosition.x, celPosition.y, highResX, highResY, origin.x, origin.y);
	}

	_originalScreenItemX[screenItemIndex] = celPosition.x;
	_originalScreenItemY[screenItemIndex] = celPosition.y;

	assert(_celHandles[screenItemIndex].area >= celWidth * celHeight);

	SciBitmap &bitmap = *_segMan->lookupBitmap(_celHandles[screenItemIndex].bitmapId);
	assert(bitmap.getWidth() == celWidth && bitmap.getHeight() == celHeight);
	assert(bitmap.getXResolution() == _xResolution && bitmap.getYResolution() == _yResolution);
	assert(bitmap.getHunkPaletteOffset() == (uint32)bitmap.getWidth() * bitmap.getHeight() + SciBitmap::getBitmapHeaderSize());
	bitmap.setOrigin(origin);

	byte *targetBuffer;
	if (_verticalScaleFactor == 100) {
		// direct copy to bitmap
		targetBuffer = bitmap.getPixels();
	} else {
		// go through squashed cel decompressor
		_celDecompressionBuffer.resize(_celDecompressionArea >= celWidth * (celHeight * _verticalScaleFactor / 100));
		targetBuffer = _celDecompressionBuffer.begin();
	}

	for (int i = 0; i < numDataChunks; ++i) {
		uint compressedSize = READ_SCI11ENDIAN_UINT32(rawVideoData);
		uint decompressedSize = READ_SCI11ENDIAN_UINT32(rawVideoData + 4);
		uint16 compressionType = READ_SCI11ENDIAN_UINT16(rawVideoData + 8);
		rawVideoData += 10;

		switch (compressionType) {
		case kCompressionLZS: {
			Common::MemoryReadStream videoDataStream(rawVideoData, compressedSize, DisposeAfterUse::NO);
			_decompressor.unpack(&videoDataStream, targetBuffer, compressedSize, decompressedSize);
			break;
		}
		case kCompressionNone:
			Common::copy(rawVideoData, rawVideoData + decompressedSize, targetBuffer);
			break;
		default:
			error("Unknown compression type %d!", compressionType);
		}

		rawVideoData += compressedSize;
		targetBuffer += decompressedSize;
	}

	if (_verticalScaleFactor != 100) {
		expandCel(bitmap.getPixels(), _celDecompressionBuffer.begin(), celWidth, celHeight);
	}

	if (usePalette) {
		Common::copy(_rawPalette, _rawPalette + kRawPaletteSize, bitmap.getHunkPalette());
	}

	return kCelHeaderSize + dataSize;
}

void RobotDecoder::preallocateCelMemory(const byte *rawVideoData, const int16 numCels) {
	for (CelHandleList::size_type i = 0; i < _celHandles.size(); ++i) {
		CelHandleInfo &celHandle = _celHandles[i];

		if (celHandle.status == CelHandleInfo::kFrameLifetime) {
			_segMan->freeBitmap(celHandle.bitmapId);
			celHandle.bitmapId = NULL_REG;
			celHandle.status = CelHandleInfo::kNoCel;
			celHandle.area = 0;
		}
	}
	_celHandles.resize(numCels);

	const int numFixedCels = MIN(numCels, (int16)kFixedCelListSize);
	for (int i = 0; i < numFixedCels; ++i) {
		CelHandleInfo &celHandle = _celHandles[i];

		// In SSCI, there was a check to see if the cel handle was not allocated
		// here, for some reason, which would mean that nothing was ever
		// allocated from fixed cels, because the _celHandles array just got
		// deleted and recreated...
		if (celHandle.bitmapId == NULL_REG) {
			break;
		}

		celHandle.bitmapId = _fixedCels[i];
		celHandle.status = CelHandleInfo::kRobotLifetime;
		celHandle.area = _maxCelArea[i];
	}

	uint maxFrameArea = 0;
	for (int i = 0; i < numCels; ++i) {
		const int16 celWidth = (int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 2);
		const int16 celHeight = (int16)READ_SCI11ENDIAN_UINT16(rawVideoData + 4);
		const uint16 dataSize = READ_SCI11ENDIAN_UINT16(rawVideoData + 14);
		const uint area = celWidth * celHeight;

		if (area > maxFrameArea) {
			maxFrameArea = area;
		}

		const bool remap = (getSciVersion() == SCI_VERSION_3);

		CelHandleInfo &celHandle = _celHandles[i];
		if (celHandle.status == CelHandleInfo::kRobotLifetime) {
			if (_maxCelArea[i] < area) {
				_segMan->freeBitmap(celHandle.bitmapId);
				_segMan->allocateBitmap(&celHandle.bitmapId, celWidth, celHeight, 255, 0, 0, _xResolution, _yResolution, kRawPaletteSize, remap, false);
				celHandle.area = area;
				celHandle.status = CelHandleInfo::kFrameLifetime;
			}
		} else if (celHandle.status == CelHandleInfo::kNoCel) {
			_segMan->allocateBitmap(&celHandle.bitmapId, celWidth, celHeight, 255, 0, 0, _xResolution, _yResolution, kRawPaletteSize, remap, false);
			celHandle.area = area;
			celHandle.status = CelHandleInfo::kFrameLifetime;
		} else {
			error("Cel Handle has bad status");
		}

		rawVideoData += kCelHeaderSize + dataSize;
	}

	if (maxFrameArea > _celDecompressionBuffer.size()) {
		_celDecompressionBuffer.resize(maxFrameArea);
	}
}

} // End of namespace Sci
