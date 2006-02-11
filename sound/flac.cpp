/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2006 The ScummVM project
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
 * $URL$
 * $Id$
 *
 */

#include "sound/flac.h"

#ifdef USE_FLAC

#include "common/file.h"
#include "common/util.h"

#include "sound/audiostream.h"
#include "sound/audiocd.h"

#define FLAC__NO_DLL // that MS-magic gave me headaches - just link the library you like
#include <FLAC/seekable_stream_decoder.h>


using Common::File;


#pragma mark -
#pragma mark --- Flac stream ---
#pragma mark -

static const uint MAX_OUTPUT_CHANNELS = 2;


class FlacInputStream : public AudioStream {

public:
	FlacInputStream(File *sourceFile, const uint32 fileStart, const uint32 fileStop);
	FlacInputStream(File *sourceFile, const uint32 fileStart = 0);
	virtual ~FlacInputStream();

	int readBuffer(int16 *buffer, const int numSamples);

	bool isStereo() const { return _streaminfo.channels >= 2; }
	int getRate() const { return _streaminfo.sample_rate; }
	bool endOfStream() const { return _streaminfo.channels == 0 || (_lastSampleWritten && getBufferedSamples() == 0); }
	/** the mixer aint supporting it right now.. */
	//bool endOfData() const { return getBufferedSamples() == 0; }
	bool endOfData() const { return endOfStream(); }

	uint getChannels() const { return MIN(_streaminfo.channels, MAX_OUTPUT_CHANNELS); }
	uint getBufferedSamples() const { return _preBuffer.bufFill; };

	const FLAC__StreamMetadata_StreamInfo& getStreamInfo() const {return _streaminfo;}

	inline FLAC__SeekableStreamDecoderState getState() const;
	inline FLAC__StreamDecoderState getStreamDecoderState() const;


	bool isStreamDecoderReady() const { return getStreamDecoderState() == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC ; }
	bool init();
	bool finish();
	bool flush();
	inline bool processSingleBlock();
	inline bool processUntilEndOfMetadata();
	bool seekAbsolute(FLAC__uint64 sample);
	inline void setLastSample(FLAC__uint64 absoluteSample);

protected:
	inline ::FLAC__SeekableStreamDecoderReadStatus callbackRead(FLAC__byte buffer[], uint *bytes);
	inline ::FLAC__SeekableStreamDecoderSeekStatus callbackSeek(FLAC__uint64 absoluteByteOffset);
	inline ::FLAC__SeekableStreamDecoderTellStatus callbackTell(FLAC__uint64 *absoluteByteOffset);
	inline ::FLAC__SeekableStreamDecoderLengthStatus callbackLength(FLAC__uint64 *streamLength);
	inline bool callbackEOF();
	inline ::FLAC__StreamDecoderWriteStatus callbackWrite(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]);
	inline void callbackMetadata(const ::FLAC__StreamMetadata *metadata);
	inline void callbackError(::FLAC__StreamDecoderErrorStatus status);

	::FLAC__SeekableStreamDecoder *_decoder;

private:
	static ::FLAC__SeekableStreamDecoderReadStatus callWrapRead(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], uint *bytes, void *clientData);
	static ::FLAC__SeekableStreamDecoderSeekStatus callWrapSeek(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absoluteByteOffset, void *clientData);
	static ::FLAC__SeekableStreamDecoderTellStatus callWrapTell(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absoluteByteOffset, void *clientData);
	static ::FLAC__SeekableStreamDecoderLengthStatus callWrapLength(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *streamLength, void *clientData);
	static FLAC__bool callWrapEOF(const ::FLAC__SeekableStreamDecoder *decoder, void *clientData);
	static ::FLAC__StreamDecoderWriteStatus callWrapWrite(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData);
	static void callWrapMetadata(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *clientData);
	static void callWrapError(const ::FLAC__SeekableStreamDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *clientData);
	// Private and undefined so you can't use them:
	FlacInputStream(const FlacInputStream &);
	void operator=(const FlacInputStream &);

	bool isValid() const { return _decoder != NULL; }

	bool allocateBuffer(uint minSamples);
	inline void flushBuffer();
	inline void deleteBuffer();

	/** Header of the Stream */
	FLAC__StreamMetadata_StreamInfo _streaminfo;

	struct {
		/** Handle to the File */
		File *fileHandle;
		/** Index of next Byte to read */
		uint32 filePos;
		/** start of stream - not necessary start of file */
		uint32 fileStartPos;
		/** last index of Stream + 1(!) - not necessary end of file */
		uint32 fileEndPos;
	} _fileInfo;

	/** index of the first Sample to be played */
	FLAC__uint64 _firstSample;
	/** index + 1(!) of the last Sample to be played - 0 is end of Stream*/
	FLAC__uint64 _lastSample;

	/** true if the last Sample was decoded from the FLAC-API - there might still be data in the buffer */
	bool _lastSampleWritten;

	typedef int16 bufType;
	enum { BUFTYPE_BITS = 16 };

	struct {
		bufType *bufData;
		bufType *bufReadPos;
		uint bufSize;
		uint bufFill;
	} _preBuffer;

	bufType *_outBuffer;
	uint _requestedSamples;

	void setBestConvertBufferMethod();
	typedef void (*PFCONVERTBUFFERS)(bufType*,const FLAC__int32*[], uint, const uint, const uint8);
	PFCONVERTBUFFERS _methodConvertBuffers;
	static void convertBuffersGeneric(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersStereoNS(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersStereo8Bit(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersMonoNS(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
	static void convertBuffersMono8Bit(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits);
};

FlacInputStream::FlacInputStream(File *sourceFile, const uint32 fileStart)
			:	_decoder(::FLAC__seekable_stream_decoder_new()), _firstSample(0), _lastSample(0),
				_outBuffer(NULL), _requestedSamples(0), _lastSampleWritten(true),
				_methodConvertBuffers(&FlacInputStream::convertBuffersGeneric)
{
	assert(sourceFile != NULL && sourceFile->isOpen());

	memset(&_streaminfo, 0, sizeof(_streaminfo));

	_preBuffer.bufData = NULL;
	_preBuffer.bufFill = 0;
	_preBuffer.bufSize = 0;

	_fileInfo.fileHandle = sourceFile;
	_fileInfo.fileStartPos = fileStart;
	_fileInfo.filePos = fileStart;
	_fileInfo.fileEndPos = sourceFile->size();

	_fileInfo.fileHandle->incRef();
}

FlacInputStream::FlacInputStream(File *sourceFile, const uint32 fileStart, const uint32 fileStop)
			:	_decoder(::FLAC__seekable_stream_decoder_new()), _firstSample(0), _lastSample(0),
				_outBuffer(NULL), _requestedSamples(0), _lastSampleWritten(true),
				_methodConvertBuffers(&FlacInputStream::convertBuffersGeneric)
{
	assert(sourceFile != NULL && sourceFile->isOpen());
	assert(fileStop <= 0 || (fileStart < fileStop && fileStop <= sourceFile->size()));

	memset(&_streaminfo, 0, sizeof(_streaminfo));

	_preBuffer.bufData = NULL;
	_preBuffer.bufFill = 0;
	_preBuffer.bufSize = 0;

	_fileInfo.fileHandle = sourceFile;
	_fileInfo.fileStartPos = fileStart;
	_fileInfo.filePos = fileStart;
	_fileInfo.fileEndPos = fileStop;

	_fileInfo.fileHandle->incRef();
}

FlacInputStream::~FlacInputStream() {
	if (_decoder != NULL) {
		(void) ::FLAC__seekable_stream_decoder_finish(_decoder);
		::FLAC__seekable_stream_decoder_delete(_decoder);
	}
	if (_preBuffer.bufData != NULL)
		delete[] _preBuffer.bufData;

	_fileInfo.fileHandle->decRef();
}

inline FLAC__SeekableStreamDecoderState FlacInputStream::getState() const {
	assert(isValid());
	return ::FLAC__seekable_stream_decoder_get_state(_decoder);
}

inline FLAC__StreamDecoderState FlacInputStream::getStreamDecoderState() const {
	assert(isValid());
	return ::FLAC__seekable_stream_decoder_get_stream_decoder_state(_decoder);
}

bool FlacInputStream::init() {
	assert(isValid());

	memset(&_streaminfo, 0, sizeof (_streaminfo));
	deleteBuffer();
	_fileInfo.filePos = _fileInfo.fileStartPos;
	_lastSampleWritten = false;
	_methodConvertBuffers = &FlacInputStream::convertBuffersGeneric;

	::FLAC__seekable_stream_decoder_set_read_callback(_decoder, &FlacInputStream::callWrapRead);
	::FLAC__seekable_stream_decoder_set_seek_callback(_decoder, &FlacInputStream::callWrapSeek);
	::FLAC__seekable_stream_decoder_set_tell_callback(_decoder, &FlacInputStream::callWrapTell);
	::FLAC__seekable_stream_decoder_set_length_callback(_decoder, &FlacInputStream::callWrapLength);
	::FLAC__seekable_stream_decoder_set_eof_callback(_decoder, &FlacInputStream::callWrapEOF);
	::FLAC__seekable_stream_decoder_set_write_callback(_decoder, &FlacInputStream::callWrapWrite);
	::FLAC__seekable_stream_decoder_set_metadata_callback(_decoder, &FlacInputStream::callWrapMetadata);
	::FLAC__seekable_stream_decoder_set_error_callback(_decoder, &FlacInputStream::callWrapError);
	::FLAC__seekable_stream_decoder_set_client_data(_decoder, (void*)this);

	if (::FLAC__seekable_stream_decoder_init(_decoder) == FLAC__SEEKABLE_STREAM_DECODER_OK) {
		if (processUntilEndOfMetadata() && _streaminfo.channels > 0) {
			if (_firstSample == 0 || 0 != ::FLAC__seekable_stream_decoder_seek_absolute(_decoder, _firstSample)) {
				// FLAC__StreamDecoderState state = getStreamDecoderState();
				return true; // no error occured
			}
		}
	}

	warning("FlacInputStream: could not create an Audiostream from File %s", _fileInfo.fileHandle->name());
	return false;
}

bool FlacInputStream::finish() {
	assert(isValid());
	deleteBuffer();
	return 0 != ::FLAC__seekable_stream_decoder_finish(_decoder);
}

bool FlacInputStream::flush() {
	assert(isValid());
	flushBuffer();
	return 0 != ::FLAC__seekable_stream_decoder_flush(_decoder);
}

inline bool FlacInputStream::processSingleBlock() {
	assert(isValid());
	return 0 != ::FLAC__seekable_stream_decoder_process_single(_decoder);
}

inline bool FlacInputStream::processUntilEndOfMetadata() {
	assert(isValid());
	return 0 != ::FLAC__seekable_stream_decoder_process_until_end_of_metadata(_decoder);
}

bool FlacInputStream::seekAbsolute(FLAC__uint64 sample) {
	assert(isValid());
	const bool result = (0 != ::FLAC__seekable_stream_decoder_seek_absolute(_decoder, sample));
	if (result) {
		flushBuffer();
		_lastSampleWritten = (_lastSample != 0 && sample >= _lastSample); // only set if we are SURE
	}
	return result;
}

int FlacInputStream::readBuffer(int16 *buffer, const int numSamples) {
	const uint kNumChannels = getChannels();

	if (kNumChannels == 0) {
		warning("FlacInputStream: Stream not sucessfully initialised, cant playback");
		return -1; // streaminfo wasnt read!
	}

	assert(numSamples % kNumChannels == 0); // must be multiple of channels!
	assert(buffer != NULL);
	assert(_outBuffer == NULL);
	assert(_requestedSamples == 0);

	_outBuffer = buffer;
	_requestedSamples = numSamples;

	if (_preBuffer.bufFill > 0) {
		assert(_preBuffer.bufData != NULL && _preBuffer.bufReadPos != NULL && _preBuffer.bufSize > 0);
		assert(_preBuffer.bufReadPos >= _preBuffer.bufData);
		assert(_preBuffer.bufFill % kNumChannels == 0);

		const uint copySamples = MIN((uint)numSamples, _preBuffer.bufFill);
		memcpy(buffer, _preBuffer.bufReadPos, copySamples*sizeof(buffer[0]));

		_outBuffer = buffer + copySamples;
		_requestedSamples = numSamples - copySamples;
		_preBuffer.bufReadPos += copySamples;
		_preBuffer.bufFill -= copySamples;
	}

	bool decoderOk = true;

	if (!_lastSampleWritten) {
		FLAC__StreamDecoderState state = getStreamDecoderState();

		for (; _requestedSamples > 0 && state == FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC; state = getStreamDecoderState()) {
			assert(_preBuffer.bufFill == 0);
			assert(_requestedSamples % kNumChannels == 0);
			processSingleBlock();
		}

		if (state != FLAC__STREAM_DECODER_SEARCH_FOR_FRAME_SYNC) {
			switch (state) {
			case FLAC__STREAM_DECODER_END_OF_STREAM :
				_lastSampleWritten = true;
				decoderOk = true; // no REAL error
				break;

			default:
				decoderOk = false;
				warning("FlacInputStream: An error occured while decoding. DecoderState is: %s",
					FLAC__StreamDecoderStateString[getStreamDecoderState()]);
			}
		}
	}

	const int samples = (int)(_outBuffer - buffer);
	assert(samples % kNumChannels == 0);

	_outBuffer = NULL; // basically unnessecary, only for the purpose of the asserts
	_requestedSamples = 0; // basically unnessecary, only for the purpose of the asserts

	return decoderOk ? samples : -1;
}

inline ::FLAC__SeekableStreamDecoderReadStatus FlacInputStream::callbackRead(FLAC__byte buffer[], uint *bytes) {
	assert(_fileInfo.fileHandle != NULL);

	if (*bytes == 0)
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR; /* abort to avoid a deadlock */

	const uint32 length = MIN(_fileInfo.fileEndPos - _fileInfo.filePos, static_cast<uint32>(*bytes));

	_fileInfo.fileHandle->seek(_fileInfo.filePos);
	const uint32 bytesRead = _fileInfo.fileHandle->read(buffer, length);

	if (bytesRead == 0 && _fileInfo.fileHandle->ioFailed())
		return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_ERROR;

	_fileInfo.filePos += bytesRead;
	*bytes = static_cast<uint>(bytesRead);
	return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
}

inline void FlacInputStream::setLastSample(FLAC__uint64 absoluteSample) {
	if (_lastSampleWritten && absoluteSample > _lastSample)
		_lastSampleWritten = false;
	_lastSample = absoluteSample;
}

inline void FlacInputStream::flushBuffer() {
	_lastSampleWritten = _lastSampleWritten && _preBuffer.bufFill == 0;
	_preBuffer.bufFill = 0;
}

inline void FlacInputStream::deleteBuffer() {
	flushBuffer();
	_preBuffer.bufSize = 0;
	if (_preBuffer.bufData != NULL) {
		delete[] _preBuffer.bufData;
		_preBuffer.bufData = NULL;
	}
}

bool FlacInputStream::allocateBuffer(uint minSamples) {
	uint allocateSize = minSamples / getChannels();
	/** insert funky algorythm for optimum buffersize here */
	allocateSize = MIN(_streaminfo.max_blocksize, MAX(_streaminfo.min_blocksize, allocateSize));
	allocateSize += 8 - (allocateSize % 8); // make sure its an nice even amount
	allocateSize *= getChannels();

	deleteBuffer();

	_preBuffer.bufData = new bufType[allocateSize];
	if (_preBuffer.bufData != NULL) {
		_preBuffer.bufSize = allocateSize;
		return true;
	}
	return false;
}

void FlacInputStream::setBestConvertBufferMethod()
{
	PFCONVERTBUFFERS tempMethod = &FlacInputStream::convertBuffersGeneric;

	const uint kNumChannels = getChannels();
	const uint8 kNumBits = (uint8)_streaminfo.bits_per_sample;

	assert(kNumChannels >= 1);
	assert(kNumBits >= 4 && kNumBits <=32);

	if (kNumChannels == 1) {
		if (kNumBits == 8)
			tempMethod = &FlacInputStream::convertBuffersMono8Bit;
		if (kNumBits == BUFTYPE_BITS)
			tempMethod = &FlacInputStream::convertBuffersMonoNS;
	} else if (kNumChannels == 2) {
		if (kNumBits == 8)
			tempMethod = &FlacInputStream::convertBuffersStereo8Bit;
		if (kNumBits == BUFTYPE_BITS)
			tempMethod = &FlacInputStream::convertBuffersStereoNS;
	} /* else ... */

	_methodConvertBuffers = tempMethod;
}

// 1 channel, no scaling
void FlacInputStream::convertBuffersMonoNS(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits)
{
	assert(numChannels == 1);
	assert(numBits == BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];

	while (numSamples >= 4) {
		bufDestination[0] = static_cast<bufType>(inChannel1[0]);
		bufDestination[1] = static_cast<bufType>(inChannel1[1]);
		bufDestination[2] = static_cast<bufType>(inChannel1[2]);
		bufDestination[3] = static_cast<bufType>(inChannel1[3]);
		bufDestination += 4;
		inChannel1 += 4;
		numSamples -= 4;
	}

	for (; numSamples > 0; --numSamples) {
		*bufDestination++ = static_cast<bufType>(*inChannel1++);
	}

	inChannels[0] = inChannel1;
	assert(numSamples == 0); // dint copy too many samples
}

// 1 channel, scaling from 8Bit
void FlacInputStream::convertBuffersMono8Bit(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits)
{
	assert(numChannels == 1);
	assert(numBits == 8);
	assert(8 < BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];

	while (numSamples >= 4) {
		bufDestination[0] = static_cast<bufType>(inChannel1[0]) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<bufType>(inChannel1[1]) << (BUFTYPE_BITS - 8);
		bufDestination[2] = static_cast<bufType>(inChannel1[2]) << (BUFTYPE_BITS - 8);
		bufDestination[3] = static_cast<bufType>(inChannel1[3]) << (BUFTYPE_BITS - 8);
		bufDestination += 4;
		inChannel1 += 4;
		numSamples -= 4;
	}

	for (; numSamples > 0; --numSamples) {
		*bufDestination++ = static_cast<bufType>(*inChannel1++) << (BUFTYPE_BITS - 8);
	}

	inChannels[0] = inChannel1;
	assert(numSamples == 0); // dint copy too many samples
}

// 2 channels, no scaling
void FlacInputStream::convertBuffersStereoNS(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits)
{
	assert(numChannels == 2);
	assert(numBits == BUFTYPE_BITS);
	assert(numSamples % 2 == 0); // must be integral multiply of channels


	FLAC__int32 const* inChannel1 = inChannels[0];	// Left Channel
	FLAC__int32 const* inChannel2 = inChannels[1];	// Right Channel

	while (numSamples >= 2*2) {
		bufDestination[0] = static_cast<bufType>(inChannel1[0]);
		bufDestination[1] = static_cast<bufType>(inChannel2[0]);
		bufDestination[2] = static_cast<bufType>(inChannel1[1]);
		bufDestination[3] = static_cast<bufType>(inChannel2[1]);
		bufDestination += 2 * 2;
		inChannel1 += 2;
		inChannel2 += 2;
		numSamples -= 2 * 2;
	}

	while (numSamples > 0) {
		bufDestination[0] = static_cast<bufType>(*inChannel1++);
		bufDestination[1] = static_cast<bufType>(*inChannel2++);
		bufDestination += 2;
		numSamples -= 2;
	}

	inChannels[0] = inChannel1;
	inChannels[1] = inChannel2;
	assert(numSamples == 0); // dint copy too many samples
}

// 2 channels, scaling from 8Bit
void FlacInputStream::convertBuffersStereo8Bit(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits)
{
	assert(numChannels == 2);
	assert(numBits == 8);
	assert(numSamples % 2 == 0); // must be integral multiply of channels
	assert(8 < BUFTYPE_BITS);

	FLAC__int32 const* inChannel1 = inChannels[0];	// Left Channel
	FLAC__int32 const* inChannel2 = inChannels[1];	// Right Channel

	while (numSamples >= 2*2) {
		bufDestination[0] = static_cast<bufType>(inChannel1[0]) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<bufType>(inChannel2[0]) << (BUFTYPE_BITS - 8);
		bufDestination[2] = static_cast<bufType>(inChannel1[1]) << (BUFTYPE_BITS - 8);
		bufDestination[3] = static_cast<bufType>(inChannel2[1]) << (BUFTYPE_BITS - 8);
		bufDestination += 2 * 2;
		inChannel1 += 2;
		inChannel2 += 2;
		numSamples -= 2 * 2;
	}

	while (numSamples > 0) {
		bufDestination[0] = static_cast<bufType>(*inChannel1++) << (BUFTYPE_BITS - 8);
		bufDestination[1] = static_cast<bufType>(*inChannel2++) << (BUFTYPE_BITS - 8);
		bufDestination += 2;
		numSamples -= 2;
	}

	inChannels[0] = inChannel1;
	inChannels[1] = inChannel2;
	assert(numSamples == 0); // dint copy too many samples
}

// all Purpose-conversion - slowest of em all
void FlacInputStream::convertBuffersGeneric(bufType* bufDestination, const FLAC__int32 *inChannels[], uint numSamples, const uint numChannels, const uint8 numBits)
{
	assert(numSamples % numChannels == 0); // must be integral multiply of channels

	if (numBits < BUFTYPE_BITS) {
		const uint8 kPower = (uint8)(BUFTYPE_BITS - numBits);

		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<bufType>(*(inChannels[i]++)) << kPower;
		}
	} else if (numBits > BUFTYPE_BITS) {
		const uint8 kPower = (uint8)(numBits - BUFTYPE_BITS);

		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<bufType>(*(inChannels[i]++) >> kPower) ;
		}
	} else {
		for (; numSamples > 0; numSamples -= numChannels) {
			for (uint i = 0; i < numChannels; ++i)
				*bufDestination++ = static_cast<bufType>(*(inChannels[i]++));
		}
	}

	assert(numSamples == 0); // dint copy too many samples
}

inline ::FLAC__StreamDecoderWriteStatus FlacInputStream::callbackWrite(const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[]) {
    assert(frame->header.channels == _streaminfo.channels);
	assert(frame->header.sample_rate == _streaminfo.sample_rate);
	assert(frame->header.bits_per_sample == _streaminfo.bits_per_sample);
	assert(frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER || _streaminfo.min_blocksize == _streaminfo.max_blocksize);

	assert(_preBuffer.bufFill == 0); // we dont append data

	uint nSamples = frame->header.blocksize;
	const uint kNumChannels = getChannels();
	const uint8 kNumBits = (uint8)_streaminfo.bits_per_sample;

	assert(_requestedSamples % kNumChannels == 0); // must be integral multiply of channels

	const FLAC__uint64 firstSampleNumber = (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) ?
		frame->header.number.sample_number : (static_cast<FLAC__uint64>(frame->header.number.frame_number)) * _streaminfo.max_blocksize;

	if (_lastSample != 0 && firstSampleNumber + nSamples >= _lastSample) {
		nSamples = (uint)(firstSampleNumber >= _lastSample ? 0 : _lastSample - firstSampleNumber);
		_requestedSamples = MIN(_requestedSamples, nSamples * kNumChannels);
		_lastSampleWritten = true;
	}

	nSamples *= kNumChannels;

	const FLAC__int32 *inChannels[MAX_OUTPUT_CHANNELS] = { buffer[0] }; // one channel is a given...
	for (uint i = 1; i < kNumChannels; ++i)
		inChannels[i] = buffer[i];


	// writing DIRECTLY to the Buffer ScummVM provided
	if (_requestedSamples > 0) {
		assert(_requestedSamples % kNumChannels == 0); // must be integral multiply of channels
		assert(_outBuffer != NULL);

		const uint copySamples = MIN(_requestedSamples,nSamples);
		(*_methodConvertBuffers)(_outBuffer, inChannels, copySamples, kNumChannels, kNumBits);

		_requestedSamples -= copySamples;
		nSamples -= copySamples;
		_outBuffer += copySamples;
	}

	// checking if Buffer fits
	if (_preBuffer.bufSize < nSamples) {
		if (!allocateBuffer(nSamples))
			return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	} // optional check if buffer is wasting too much memory ?

	(*_methodConvertBuffers)(_preBuffer.bufData, inChannels, nSamples, kNumChannels, kNumBits);

	_preBuffer.bufFill = nSamples;
	_preBuffer.bufReadPos = _preBuffer.bufData;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

inline ::FLAC__SeekableStreamDecoderSeekStatus FlacInputStream::callbackSeek(FLAC__uint64 absoluteByteOffset) {
    FLAC__uint64 newPos = absoluteByteOffset + _fileInfo.fileStartPos;
	const bool result = (newPos < _fileInfo.fileEndPos);

	if (result)
		_fileInfo.filePos = static_cast<uint32>(newPos);

	return result ? FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK : FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;

}

inline ::FLAC__SeekableStreamDecoderTellStatus FlacInputStream::callbackTell(FLAC__uint64 *absoluteByteOffset) {
	/*if ()
		return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_ERROR;*/
	*absoluteByteOffset = static_cast<FLAC__uint64>(_fileInfo.filePos-_fileInfo.fileStartPos);
	return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
}

inline ::FLAC__SeekableStreamDecoderLengthStatus FlacInputStream::callbackLength(FLAC__uint64 *streamLength) {
	if (_fileInfo.fileStartPos > _fileInfo.fileEndPos)
		return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;

	*streamLength = static_cast<FLAC__uint64>(_fileInfo.fileEndPos - _fileInfo.fileStartPos);
	return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
}

inline bool FlacInputStream::callbackEOF() {
	return _fileInfo.filePos >= _fileInfo.fileEndPos;
}


inline void FlacInputStream::callbackMetadata(const ::FLAC__StreamMetadata *metadata) {
	assert(isValid());
	assert(metadata->type == FLAC__METADATA_TYPE_STREAMINFO); // others arent really interesting

	_streaminfo = metadata->data.stream_info;
	setBestConvertBufferMethod(); // should be set after getting stream-information. FLAC always parses the info first
}
inline void FlacInputStream::callbackError(::FLAC__StreamDecoderErrorStatus status) {
	// some of these are non-critical-Errors
	debug(1, "FlacInputStream: An error occured while decoding. DecoderState is: %s",
			FLAC__StreamDecoderErrorStatusString[status]);
}

/* Static Callback Wrappers */
::FLAC__SeekableStreamDecoderReadStatus FlacInputStream::callWrapRead(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], uint *bytes, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackRead(buffer, bytes);
}

::FLAC__SeekableStreamDecoderSeekStatus FlacInputStream::callWrapSeek(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absoluteByteOffset, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackSeek(absoluteByteOffset);
}

::FLAC__SeekableStreamDecoderTellStatus FlacInputStream::callWrapTell(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absoluteByteOffset, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackTell(absoluteByteOffset);
}

::FLAC__SeekableStreamDecoderLengthStatus FlacInputStream::callWrapLength(const ::FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *streamLength, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackLength(streamLength);
}

FLAC__bool FlacInputStream::callWrapEOF(const ::FLAC__SeekableStreamDecoder *decoder, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackEOF();
}

::FLAC__StreamDecoderWriteStatus FlacInputStream::callWrapWrite(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__Frame *frame, const FLAC__int32 * const buffer[], void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	return instance->callbackWrite(frame, buffer);
}

void FlacInputStream::callWrapMetadata(const ::FLAC__SeekableStreamDecoder *decoder, const ::FLAC__StreamMetadata *metadata, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	instance->callbackMetadata(metadata);
}

void FlacInputStream::callWrapError(const ::FLAC__SeekableStreamDecoder *decoder, ::FLAC__StreamDecoderErrorStatus status, void *clientData) {
	assert(0 != clientData);
	FlacInputStream *instance = reinterpret_cast<FlacInputStream *>(clientData);
	assert(0 != instance);
	instance->callbackError(status);
}


#pragma mark -
#pragma mark --- Flac Audio CD emulation ---
#pragma mark -


class FlacTrackInfo : public DigitalTrackInfo {
private:
	File *_file;
	FlacInputStream *_firstStream; // avoid having to open the Stream twice the first time

public:
	FlacTrackInfo(File *file);
	~FlacTrackInfo();
	bool error() { return _file == NULL; }
	void play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration);
};

FlacTrackInfo::FlacTrackInfo(File *file) : _file(NULL), _firstStream(NULL)
{
	FlacInputStream *tempStream = new FlacInputStream(file);
	/* first time the file will be tested, but not used */
	if (tempStream->init()) {
		_firstStream = tempStream;
		_file = file;
	} else
		delete tempStream;
}

void FlacTrackInfo::play(Audio::Mixer *mixer, Audio::SoundHandle *handle, int startFrame, int duration) {
	if (error()) {
		debug(1, "FlacTrackInfo::play:  invalid state, method should not been called");
	}

	FlacInputStream *flac;

	if (_firstStream != NULL) {
		flac = _firstStream;
		_firstStream = NULL;
	} else {
		flac = new FlacInputStream(_file);
		flac->init();
	}

	if (flac->isStreamDecoderReady()) {
		const FLAC__StreamMetadata_StreamInfo &info = flac->getStreamInfo();
		if (duration)
			flac->setLastSample(static_cast<FLAC__uint64>(startFrame + duration) * (info.sample_rate / 75));
		else
			flac->setLastSample(0);

		if (flac->seekAbsolute(static_cast<FLAC__uint64>(startFrame) * (info.sample_rate / 75))) {
			mixer->playInputStream(Audio::Mixer::kMusicSoundType, handle, flac);
			return;
		}
		// startSample is beyond the existing Samples
		debug(1, "FlacTrackInfo: Audiostream %s could not seek to frame %d (ca %d secs)", _file->name(), startFrame, startFrame/75);
		flac->finish();
	}
	delete flac;
}

FlacTrackInfo::~FlacTrackInfo()
{
	delete _firstStream;
	delete _file;
}

DigitalTrackInfo* getFlacTrack(int track)
{
	assert(track >=1);
	char track_name[32];
	File *file = new File();

	sprintf(track_name, "track%d.flac", track);
	file->open(track_name);

	if (file->isOpen()) {
		FlacTrackInfo *trackInfo = new FlacTrackInfo(file);
		if (!trackInfo->error())
			return trackInfo;
		delete trackInfo;
	}

	sprintf(track_name, "track%d.fla", track);
	file->open(track_name);

	if (file->isOpen()) {
		FlacTrackInfo *trackInfo = new FlacTrackInfo(file);
		if (!trackInfo->error())
			return trackInfo;
		delete trackInfo;
	}


	delete file;
	return NULL;
}

AudioStream *makeFlacStream(File *file, uint32 length)
{
	assert(file != NULL);
	uint32 start = file->pos();

	FlacInputStream *flac = new FlacInputStream(file, start, start + length);
	if (flac->init())
		return flac;

	delete flac;
	return NULL;
}


#endif // #ifdef USE_FLAC
