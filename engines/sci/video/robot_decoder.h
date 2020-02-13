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

#ifndef SCI_SOUND_DECODERS_ROBOT_H
#define SCI_SOUND_DECODERS_ROBOT_H

#include "audio/audiostream.h"           // for AudioStream
#include "audio/rate.h"                  // for st_sample_t
#include "common/array.h"                // for Array
#include "common/mutex.h"                // for StackLock, Mutex
#include "common/rect.h"                 // for Point, Rect (ptr only)
#include "common/scummsys.h"             // for int16, int32, byte, uint16
#include "sci/engine/vm_types.h"         // for NULL_REG, reg_t
#include "sci/graphics/helpers.h"        // for GuiResourceId
#include "sci/graphics/screen_item32.h"  // for ScaleInfo, ScreenItem (ptr o...

namespace Common { class SeekableSubReadStreamEndian; }
namespace Sci {
class Plane;
class SegManager;

// There were 3 different Robot video versions, used in the following games:
// - v4: PQ:SWAT demo
// - v5: KQ7 DOS, Phantasmagoria, PQ:SWAT, Lighthouse
// - v6: RAMA
//
// Notes on Robot v5/v6 format:
//
// Robot is a packetized streaming AV format that encodes multiple bitmaps +
// positioning data, plus synchronised audio, for rendering in the SCI graphics
// system.
//
// Unlike traditional AV formats, Robot videos almost always require playback
// within the game engine because certain information (like the resolution of
// the Robot coordinates and the background for the video) is dependent on data
// that does not exist within the Robot file itself. In version 6, robots could
// also participate in palette remapping by drawing remap pixels, and the
// information for processing these pixels is also not stored within the Robot
// file.
//
// The Robot container consists of a file header, an optional primer audio
// section, an optional colour palette, a frame seek index, a set of cuepoints,
// and variable-sized packets of compressed video+audio data.
//
// Integers in Robot files are coded using native endianness (LSB for x86
// versions, MSB for 68k/PPC versions).
//
// Robot video coding is a relatively simple variable-length compression with no
// interframe compression. Each cel in a frame is constructed from multiple
// contiguous data blocks, each of which can be independently compressed with
// LZS or left uncompressed. An entire cel can also be line decimated, where
// lines are deleted from the source bitmap at compression time and are
// reconstructed by decompression using line interpolation. Each cel also
// includes coordinates where it should be placed within the video frame,
// relative to the top-left corner of the frame.
//
// Audio coding is fixed-length, and all audio blocks except for the primer
// audio are the same size. Audio is encoded with Sierra SOL DPCM16 compression,
// and is split into two channels ('even' and 'odd'), each at a 11025Hz sample
// rate. The original signal is restored by interleaving samples from the two
// channels together. Channel packets are 'even' if they have an ''absolute
// position of audio'' that is evenly divisible by 2; otherwise, they are 'odd'.
// Because the channels use DPCM compression, there is an 8-byte runway at the
// start of every audio block that is never written to the output stream, which
// is used to move the signal to the correct location by the 9th sample.
//
// File header (v5/v6):
//
//  byte | description
//     0 | signature 0x16
//     1 | unused
//   2-5 | signature 'SOL\0'
//   6-7 | version (4, 5, and 6 are the only known versions)
//   8-9 | size of audio blocks
// 10-11 | primer is compressed flag
// 12-13 | unused
// 14-15 | total number of video frames
// 16-17 | embedded palette size, in bytes
// 18-19 | primer reserved size
// 20-21 | coordinate X-resolution (if 0, uses game coordinates)
// 22-23 | coordinate Y-resolution (if 0, uses game coordinates)
//    24 | if non-zero, Robot includes a palette
//    25 | if non-zero, Robot includes audio
// 26-27 | unused
// 28-29 | the frame rate, in frames per second
// 30-31 | coordinate conversion flag; if true, screen item coordinates
//       | from the robot should be used as-is with NO conversion when
//       | explicitly displaying a specific frame
// 32-33 | the maximum number of packets that can be skipped without causing
//       | audio drop-out
// 34-35 | the maximum possible number of cels that will be displayed in any
//       | frame of the robot
// 36-39 | the maximum possible size, in bytes, of the first fixed cel
// 40-43 | the maximum possible size, in bytes, of the second fixed cel
// 44-47 | the maximum possible size, in bytes, of the third fixed cel
// 48-51 | the maximum possible size, in bytes, of the fourth fixed cel
// 52-59 | unused
//
// If the ''file includes audio'' flag is false, seek ''primer reserved size''
// bytes from the end of the file header to get past a padding zone.
//
// If the ''file includes audio'' flag is true, and the ''primer reserved size''
// is not zero, the data immediately after the file header consists of an audio
// primer header plus compressed audio data:
//
// Audio primer header:
//
//  byte | description
//   0-3 | the size, in bytes, of the entire primer audio section
//   4-5 | the compression format of the primer audio (must be zero)
//   6-9 | the size, in bytes, of the "even" primer
// 10-13 | the size, in bytes, of the "odd" primer
//
// If the combined sizes of the even and odd primers do not match the ''primer
// reserved size'', the next header block can be found ''primer reserved size''
// bytes from the *start* of the audio primer header.
//
// Otherwise, if the Robot has audio, and the ''primer reserved size'' is zero,
// and the ''primer is compressed flag'' is set, the "even" primer size is
// 19922, the "odd" primer size is 21024, and the "even" and "odd" buffers
// should be zero-filled.
//
// Any other combination of these flags is an error.
//
// If the Robot has a palette, the next ''palette size'' bytes should be read
// as a SCI HunkPalette. Otherwise, seek ''palette size'' bytes from the current
// position to get to the frame index.
//
// The next section of the Robot is the video frame size index. In version 5
// robots, read ''total number of frames'' 16-bit integers to get the size of
// the compressed video for each frame. For version 6 robots, use 32-bit
// integers.
//
// The next section of the Robot is the packet size index (combined compressed
// size of video + audio for each frame). In version 5 Robots, read ''total
// number of frames'' 16-bit integers. In version 6 robots, use 32-bit integers.
//
// The next section of the Robot is the cue times index. Read 256 32-bit
// integers, which represent the number of ticks from the start of playback that
// the given cue point falls on.
//
// The next section of the Robot is the cue values index. Read 256 16-bit
// integers, which represent the actual cue values that will be passed back to
// the game engine when a cue is requested.
//
// Finally, to get to the first frame packet, seek from the current position to
// the start of the next 2048-byte-aligned sector.
//
// Frame packet:
//
//  byte | description
//  0..n | video data (size is in the ''video frame size index'')
// n+1.. | optional audio data (size is ''size of audio blocks'')
//
// Video data:
//
//  byte | description
//   0-2 | number of cels in the frame (max 10)
//  3..n | cels
//
// Cel:
//
//  0-17 | cel header
// 18..n | data chunks
//
// Cel header:
//
//  byte | description
//     0 | unused
//     1 | vertical scale factor, in percent decimation (100 = no decimation,
//       | 50 = 50% of lines were removed)
//   2-3 | cel width
//   4-5 | cel height
//   6-9 | unused
// 10-11 | cel x-position, in Robot coordinates
// 12-13 | cel y-position, in Robot coordinates
// 14-15 | cel total data chunk size, in bytes
// 16-17 | number of data chunks
//
// Cel data chunk:
//
//   0-9 | cel data chunk header
// 10..n | cel data
//
// Cel data chunk header:
//
//  byte | description
//   0-3 | compressed size
//   4-7 | decompressed size
//   8-9 | compression type (0 = LZS, 2 = uncompressed)
//
// Random frame seeking can be done by calculating the address of the frame
// packet by adding up the ''packet size index'' entries up to the current
// frame. This will normally disable audio playback, as audio data in a packet
// does not correspond to the video in the same packet.
//
// Audio data is placed immediately after the end of the video data in a packet,
// and consists of an audio header plus compressed audio data:
//
// Audio data:
//
//  byte | description
//   0-7 | audio data header
//  8-15 | DPCM runway
// 16..n | compressed audio data
//
// Audio data header:
//
//  byte | description
//   0-3 | absolute position of audio in the audio stream
//   4-7 | the size of the audio block, excluding the header
//
// When a block of audio is processed, first check to ensure that the
// decompressed audio block's `position * 2 + length * 4` runs past the end of
// the last packet of the same evenness/oddness. Discard the audio block
// entirely if data has already been written past the end of this block for this
// channel, or if the read head has already read past the end of this audio
// block.
//
// If the block is not discarded, apply DPCM decompression to the entire block,
// starting from beginning of the DPCM runway, using an initial sample value of
// 0. Then, copy every sample from the decompressed source outside of the DPCM
// runway into every *other* sample of the final audio buffer (1 -> 2, 2 -> 4,
// 3 -> 6, etc.).
//
// Finally, for any skipped samples where the opposing (even/odd) channel did
// not yet write, interpolate the skipped areas by adding together the
// neighbouring samples from this audio block and dividing by two. (This allows
// the audio quality to degrade to 11kHz in case it takes too long to decode all
// the frames in the stream). Interpolated samples must not be written on top of
// true data from the opposing channel. Audio from later packets must also not
// be written on top of data in the same channel that was already written by an
// earlier packet, in particular because the first 8 bytes of the next packet
// are garbage data used to move the waveform to the correct position (due to
// the use of DPCM compression).

#pragma mark -
#pragma mark RobotAudioStream

/**
 * A Robot audio stream is a simple loop buffer that accepts audio blocks from
 * the Robot engine.
 */
class RobotAudioStream : public Audio::AudioStream {
public:
	enum {
		/**
		 * The sample rate used for all robot audio.
		 */
		kRobotSampleRate = 22050,

		/**
		 * Multiplier for the size of a packet that is being expanded by writing
		 * to every other byte of the target buffer.
		 */
		kEOSExpansion    = 2
	};

	/**
	 * Playback state information. Used for framerate calculation.
	 */
	struct StreamState {
		/**
		 * The current position of the read head of the audio stream.
		 */
		int bytesPlaying;

		/**
		 * The sample rate of the audio stream. Always 22050.
		 */
		uint16 rate;

		/**
		 * The bit depth of the audio stream. Always 16.
		 */
		uint8 bits;
	};

	/**
	 * A single packet of compressed audio from a Robot data stream.
	 */
	struct RobotAudioPacket {
		/**
		 * Raw DPCM-compressed audio data.
		 */
		byte *data;

		/**
		 * The size of the compressed audio data,
		 * in bytes.
		 */
		int dataSize;

		/**
		 * The uncompressed, file-relative position of this audio packet.
		 */
		int position;

		RobotAudioPacket(byte *data_, const int dataSize_, const int position_) :
			data(data_), dataSize(dataSize_), position(position_) {}
	};

	RobotAudioStream(const int32 bufferSize);
	~RobotAudioStream() override;

	/**
	 * Adds a new audio packet to the stream.
	 * @returns `true` if the audio packet was fully consumed, otherwise
	 * `false`.
	 */
	bool addPacket(const RobotAudioPacket &packet);

	/**
	 * Prevents any additional audio packets from being added to the audio
	 * stream.
	 */
	void finish();

	/**
	 * Returns the current status of the audio stream.
	 */
	StreamState getStatus() const;

private:
	Common::Mutex _mutex;

	/**
	 * Loop buffer for playback. Contains decompressed 16-bit PCM samples.
	 */
	byte *_loopBuffer;

	/**
	 * The size of the loop buffer, in bytes.
	 */
	int32 _loopBufferSize;

	/**
	 * The position of the read head within the loop buffer, in bytes.
	 */
	int32 _readHead;

	/**
	 * The lowest file position that can be buffered, in uncompressed bytes.
	 */
	int32 _readHeadAbs;

	/**
	 * The highest file position that can be buffered, in uncompressed bytes.
	 */
	int32 _maxWriteAbs;

	/**
	 * The highest file position, in uncompressed bytes, that has been written
	 * to the stream. This is different from `_maxWriteAbs`, which is the
	 * highest uncompressed position which *can* be written right now.
	 */
	int32 _writeHeadAbs;

	/**
	 * The highest file position, in uncompressed bytes, that has been written
	 * to the even & odd sides of the stream.
	 *
	 * Index 0 corresponds to the 'even' side; index 1 corresponds to the 'odd'
	 * side.
	 */
	int32 _jointMin[2];

	/**
	 * When `true`, the stream is waiting for all primer blocks to be received
	 * before allowing playback to begin.
	 */
	bool _waiting;

	/**
	 * When `true`, the stream will accept no more audio blocks.
	 */
	bool _finished;

	/**
	 * The uncompressed position of the first packet of robot data. Used to
	 * decide whether all primer blocks have been received and the stream should
	 * be started.
	 */
	int32 _firstPacketPosition;

	/**
	 * Decompression buffer, used to temporarily store an uncompressed block of
	 * audio data.
	 */
	byte *_decompressionBuffer;

	/**
	 * The size of the decompression buffer, in bytes.
	 */
	int32 _decompressionBufferSize;

	/**
	 * The position of the packet currently in the decompression buffer. Used to
	 * avoid re-decompressing audio data that has already been decompressed
	 * during a partial packet read.
	 */
	int32 _decompressionBufferPosition;

	/**
	 * Calculates the absolute ranges for new fills into the loop buffer.
	 */
	void fillRobotBuffer(const RobotAudioPacket &packet, const int8 bufferIndex);

	/**
	 * Interpolates `numSamples` samples from the read head, if no true samples
	 * were written for one (or both) of the joint channels.
	 */
	void interpolateMissingSamples(const int32 numSamples);

#pragma mark -
#pragma mark RobotAudioStream - AudioStream implementation
public:
	int readBuffer(Audio::st_sample_t *outBuffer, int numSamples) override;
	bool isStereo() const override { return false; };
	int getRate() const override { return 22050; };
	bool endOfData() const override {
		Common::StackLock lock(_mutex);
		return _readHeadAbs >= _writeHeadAbs;
	};
	bool endOfStream() const override {
		Common::StackLock lock(_mutex);
		return _finished && endOfData();
	}
};

#pragma mark -
#pragma mark RobotDecoder

/**
 * RobotDecoder implements the logic required for Robot animations.
 */
class RobotDecoder {
public:
	RobotDecoder(SegManager *segMan);
	~RobotDecoder();

	GuiResourceId getResourceId() const {
		return _robotId;
	}

private:
	SegManager *_segMan;

	/**
	 * The ID of the currently loaded robot.
	 */
	GuiResourceId _robotId;

#pragma mark Constants
public:
	/**
	 * The playback status of the robot.
	 */
	enum RobotStatus {
		kRobotStatusUninitialized = 0,
		kRobotStatusPlaying       = 1,
		kRobotStatusEnd           = 2,
		kRobotStatusPaused        = 3
	};

	enum {
		// Special high value used to represent parameters that should be left
		// unchanged when calling `showFrame`
		kUnspecified = 50000
	};

private:
	enum {
		/**
		 * Maximum number of on-screen screen items.
		 */
		kScreenItemListSize    = 10,

		/**
		 * Maximum number of queued audio blocks.
		 */
		kAudioListSize         = 10,

		/**
		 * Maximum number of samples used for frame timing.
		 */
		kDelayListSize         = 10,

		/**
		 * Maximum number of cues.
		 */
		kCueListSize           = 256,

		/**
		 * Maximum number of 'fixed' cels that never change for the duration of
		 * a robot.
		 */
		kFixedCelListSize      = 4,

		/**
		 * The size of a hunk palette in the Robot stream.
		 */
		kRawPaletteSize        = 1200,

		/**
		 * The size of a frame of Robot data. This value was used to align the
		 * first block of data after the main Robot header to the next CD
		 * sector.
		 */
		kRobotFrameSize        = 2048,

		/**
		 * The size of a block of zero-compressed audio. Used to fill audio when
		 * the size of an audio packet does not match the expected packet size.
		 */
		kRobotZeroCompressSize = 2048,

		/**
		 * The size of the audio block header, in bytes. The audio block header
		 * consists of the compressed size of the audio in the record, plus the
		 * position of the audio in the compressed data stream.
		 */
		kAudioBlockHeaderSize  = 8,

		/**
		 * The size of a Robot cel header, in bytes.
		 */
		kCelHeaderSize         = 22,

		/**
		 * The maximum amount that the frame rate is allowed to drift from the
		 * nominal frame rate in order to correct for AV drift or slow playback.
		 */
		kMaxFrameRateDrift     = 1
	};

	/**
	 * The version number for the currently loaded robot.
	 *
	 * There are several known versions of robot:
	 *
	 * v2: before Nov 1994; no known examples
	 * v3: before Nov 1994; no known examples
	 * v4: Jan 1995; KQ7 1.65, PQ:SWAT demo
	 * v5: Mar 1995; SCI2.1 and SCI3 games
	 * v6: SCI3 games
	 */
	uint16 _version;

#pragma mark -
#pragma mark Initialisation
private:
	/**
	 * Sets up the read stream for the robot.
	 */
	void initStream(const GuiResourceId robotId);

	/**
	 * Sets up the initial values for playback control.
	 */
	void initPlayback();

	/**
	 * Sets up the initial values for audio decoding.
	 */
	void initAudio();

	/**
	 * Sets up the initial values for video rendering.
	 */
	void initVideo(const int16 x, const int16 y, const int16 scale, const reg_t plane, const bool hasPalette, const uint16 paletteSize);

	/**
	 * Sets up the robot's data record and cue positions.
	 */
	void initRecordAndCuePositions();

#pragma mark -
#pragma mark Playback
public:
	/**
	 * Opens a robot file for playback. Newly opened robots are paused by
	 * default.
	 */
	void open(const GuiResourceId robotId, const reg_t plane, const int16 priority, const int16 x, const int16 y, const int16 scale);

	/**
	 * Closes the currently open robot file.
	 */
	void close();

	/**
	 * Pauses the robot. Once paused, the audio for a robot is disabled until
	 * the end of playback.
	 */
	void pause();

	/**
	 * Resumes a paused robot.
	 */
	void resume();

	/**
	 * Moves robot to the specified frame and pauses playback.
	 *
	 * @note Called DisplayFrame in SSCI.
	 */
	void showFrame(const uint16 frameNo, const uint16 newX, const uint16 newY, const uint16 newPriority);

	/**
	 * Retrieves the value associated with the current cue point.
	 */
	int16 getCue() const;

	/**
	 * Gets the currently displayed frame.
	 */
	int16 getFrameNo() const;

	/**
	 * Gets the playback status of the player.
	 */
	RobotStatus getStatus() const;

private:
	/**
	 * The read stream containing raw robot data.
	 */
	Common::SeekableSubReadStreamEndian *_stream;

	/**
	 * The current status of the player.
	 */
	RobotStatus _status;

	typedef Common::Array<int> PositionList;

	/**
	 * A map of frame numbers to byte offsets within `_stream`.
	 */
	PositionList _recordPositions;

	/**
	 * The offset of the Robot file within a resource bundle.
	 */
	int32 _fileOffset;

	/**
	 * A list of cue times that is updated to prevent earlier cue values from
	 * being given to the game more than once.
	 */
	mutable int32 _cueTimes[kCueListSize];

	/**
	 * The original list of cue times from the raw Robot data.
	 */
	int32 _masterCueTimes[kCueListSize];

	/**
	 * The list of values to provide to a game when a cue value is requested.
	 */
	int32 _cueValues[kCueListSize];

	/**
	 * The current playback frame rate.
	 */
	int16 _frameRate;

	/**
	 * The nominal playback frame rate.
	 */
	int16 _normalFrameRate;

	/**
	 * The minimal playback frame rate. Used to correct for AV sync drift when
	 * the video is more than one frame ahead of the audio.
	 */
	int16 _minFrameRate;

	/**
	 * The maximum playback frame rate. Used to correct for AV sync drift when
	 * the video is more than one frame behind the audio.
	 */
	int16 _maxFrameRate;

	/**
	 * The maximum number of record blocks that can be skipped without causing
	 * audio to drop out.
	 */
	int16 _maxSkippablePackets;

	/**
	 * The currently displayed frame number.
	 */
	int _currentFrameNo;

	/**
	 * The last displayed frame number.
	 */
	int _previousFrameNo;

	/**
	 * The time, in ticks, when the robot was last started or resumed.
	 */
	int32 _startTime;

	/**
	 * The first frame displayed when the robot was resumed.
	 */
	int32 _startFrameNo;

	/**
	 * The last frame displayed when the robot was resumed.
	 */
	int32 _startingFrameNo;

	/**
	 * Seeks the raw data stream to the record for the given frame number.
	 */
	bool seekToFrame(const int frameNo);

	/**
	 * Sets the start time and frame of the robot when the robot is started or
	 * resumed.
	 */
	void setRobotTime(const int frameNo);

#pragma mark -
#pragma mark Timing
private:
	/**
	 * This class tracks the amount of time it takes for a frame of robot
	 * animation to be rendered. This information is used by the player to
	 * speculatively skip rendering of future frames to keep the animation in
	 * sync with the robot audio.
	 */
	class DelayTime {
	public:
		DelayTime(RobotDecoder *decoder);

		/**
		 * Starts performance timing.
		 */
		void startTiming();

		/**
		 * Ends performance timing.
		 */
		void endTiming();

		/**
		 * Returns whether or not timing is currently in progress.
		 */
		bool timingInProgress() const;

		/**
		 * Returns the median time, in ticks, of the currently stored timing
		 * samples.
		 */
		int predictedTicks() const;

	private:
		RobotDecoder *_decoder;

		/**
		 * The start time, in ticks, of the current timing loop. If no loop is
		 * in progress, the value is 0.
		 *
		 * @note This is slightly different than SSCI where the not-timing value
		 * was -1.
		 */
		uint32 _startTime;

		/**
		 * A sorted list containing the timing data for the last
		 * `kDelayListSize` frames, in ticks.
		 */
		int _delays[kDelayListSize];

		/**
		 * A list of monotonically increasing identifiers used to identify and
		 * replace the oldest sample in the `_delays` array when finishing the
		 * next timing operation.
		 */
		uint _timestamps[kDelayListSize];

		/**
		 * The identifier of the oldest timing.
		 */
		uint _oldestTimestamp;

		/**
		 * The identifier of the newest timing.
		 */
		uint _newestTimestamp;

		/**
		 * Sorts the list of timings.
		 */
		void sortList();
	};

	/**
	 * Calculates the next frame number that needs to be rendered, using the
	 * timing data collected by DelayTime.
	 */
	uint16 calculateNextFrameNo(const uint32 extraTicks = 0) const;

	/**
	 * Calculates and returns the number of frames that should be rendered in
	 * `ticks` time, according to the current target frame rate of the robot.
	 */
	uint32 ticksToFrames(const uint32 ticks) const;

	/**
	 * Gets the current game time, in ticks.
	 */
	uint32 getTickCount() const;

	/**
	 * The performance timer for the robot.
	 */
	DelayTime _delayTime;

#pragma mark -
#pragma mark Audio
private:
	enum {
		/**
		 * The number of ticks that should elapse between each AV sync check.
		 */
		kAudioSyncCheckInterval = 5 * 60 /* 5 seconds */
	};

	/**
	 * The status of the audio track of a Robot animation.
	 */
	enum RobotAudioStatus {
		kRobotAudioReady    = 1,
		kRobotAudioStopped  = 2,
		kRobotAudioPlaying  = 3,
		kRobotAudioPaused   = 4,
		kRobotAudioStopping = 5
	};

#pragma mark -
#pragma mark Audio - AudioList
private:
	/**
	 * This class manages packetized audio playback for robots.
	 */
	class AudioList {
	public:
		AudioList();

		/**
		 * Starts playback of robot audio.
		 */
		void startAudioNow();

		/**
		 * Stops playback of robot audio, allowing any queued audio to finish
		 * playing back.
		 */
		void stopAudio();

		/**
		 * Stops playback of robot audio immediately.
		 */
		void stopAudioNow();

		/**
		 * Submits as many blocks of audio as possible to the audio engine.
		 */
		void submitDriverMax();

		/**
		 * Adds a new AudioBlock to the queue.
		 *
		 * @param position The absolute position of the audio for the block, in
		 *                 compressed bytes.
		 * @param size The size of the buffer.
		 * @param buffer A pointer to compressed audio data that will be copied
		 *               into the new AudioBlock.
		 */
		void addBlock(const int position, const int size, const byte *buffer);

		/**
		 * Immediately stops any active playback and purges all audio data in
		 * the audio list.
		 */
		void reset();

		/**
		 * Pauses the robot audio channel in preparation for the first block of
		 * audio data to be read.
		 */
		void prepareForPrimer();

		/**
		 * Sets the audio offset which is used to offset the position of audio
		 * packets sent to the audio stream.
		 */
		void setAudioOffset(const int offset);

#pragma mark -
#pragma mark Audio - AudioList - AudioBlock

	private:
		/**
		 * AudioBlock represents a block of audio from the Robot's audio track.
		 */
		class AudioBlock {
		public:
			AudioBlock(const int position, const int size, const byte *const data);
			~AudioBlock();

			/**
			 * Submits the block of audio to the audio manager.
			 * @returns true if the block was fully read, or false if the block
			 * was not read or only partially read.
			 */
			bool submit(const int startOffset);

		private:
			/**
			 * The absolute position, in compressed bytes, of this audio block's
			 * audio data in the audio stream.
			 */
			int _position;

			/**
			 * The compressed size, in bytes, of this audio block's audio data.
			 */
			int _size;

			/**
			 * A buffer containing raw SOL-compressed audio data.
			 */
			byte *_data;
		};

		/**
		 * The list of compressed audio blocks submitted for playback.
		 */
		AudioBlock *_blocks[kAudioListSize];

		/**
		 * The number of blocks in `_blocks` that are ready to be submitted.
		 */
		uint8 _blocksSize;

		/**
		 * The index of the oldest submitted audio block.
		 */
		uint8 _oldestBlockIndex;

		/**
		 * The index of the newest submitted audio block.
		 */
		uint8 _newestBlockIndex;

		/**
		 * The offset used when sending packets to the audio stream.
		 */
		int _startOffset;

		/**
		 * The status of robot audio playback.
		 */
		RobotAudioStatus _status;

		/**
		 * Frees all audio blocks in the `_blocks` list.
		 */
		void freeAudioBlocks();
	};

	/**
	 * Whether or not this robot animation has
	 * an audio track.
	 */
	bool _hasAudio;

	/**
	 * The audio list for the current robot.
	 */
	AudioList _audioList;

	/**
	 * The size, in bytes, of a block of audio data, excluding the audio block
	 * header.
	 */
	uint16 _audioBlockSize;

	/**
	 * The expected size of a block of audio data, in bytes, excluding the audio
	 * block header.
	 */
	int16 _expectedAudioBlockSize;

	/**
	 * The number of compressed audio bytes that are needed per frame to fill
	 * the audio buffer without causing audio to drop out.
	 */
	int16 _audioRecordInterval;

	/**
	 * If true, primer audio buffers should be filled with silence instead of
	 * trying to read buffers from the Robot data.
	 */
	uint16 _primerZeroCompressFlag;

	/**
	 * The size, in bytes, of the primer audio in the Robot, including any extra
	 * alignment padding.
	 */
	uint16 _primerReservedSize;

	/**
	 * The combined size, in bytes, of the even and odd primer channels.
	 */
	int32 _totalPrimerSize;

	/**
	 * The absolute offset of the primer audio data in the robot data stream.
	 */
	int32 _primerPosition;

	/**
	 * The size, in bytes, of the even primer.
	 */
	int32 _evenPrimerSize;

	/**
	 * The size, in bytes, of the odd primer.
	 */
	int32 _oddPrimerSize;

	/**
	 * The absolute position in the audio stream of the first audio packet.
	 */
	int32 _firstAudioRecordPosition;

	/**
	 * A temporary buffer used to hold one frame of raw (DPCM-compressed) audio
	 * when reading audio records from the robot stream.
	 */
	byte *_audioBuffer;

	/**
	 * The next tick count when AV sync should be checked and framerate
	 * adjustments made, if necessary.
	 */
	uint32 _checkAudioSyncTime;

	/**
	 * Primes the audio buffer with the first frame of audio data.
	 *
	 * @note `primeAudio` was `InitAudio` in SSCI
	 */
	bool primeAudio(const uint32 startTick);

	/**
	 * Reads primer data from the robot data stream and puts it into the given
	 * buffers.
	 */
	bool readPrimerData(byte *outEvenBuffer, byte *outOddBuffer);

	/**
	 * Reads audio data for the given frame number into the given buffer.
	 *
	 * @param outAudioPosition The position of the audio, in compressed bytes,
	 *                         in the data stream.
	 * @param outAudioSize The size of the audio data, in compressed bytes.
	 */
	bool readAudioDataFromRecord(const int frameNo, byte *outBuffer, int &outAudioPosition, int &outAudioSize);

	/**
	 * Submits part of the audio packet of the given frame to the audio list,
	 * starting `startPosition` bytes into the audio.
	 */
	bool readPartialAudioRecordAndSubmit(const int startFrame, const int startPosition);

#pragma mark -
#pragma mark Rendering
public:
	/**
	 * Gets the plane used to render the robot.
	 */
	const reg_t getPlaneId() const {
		return _planeId;
	}

	/**
	 * Gets the origin of the robot.
	 */
	Common::Point getPosition() const {
		return _position;
	}

	/**
	 * Gets the scale of the robot.
	 */
	int16 getScale() const {
		return _scaleInfo.x;
	}

	/**
	 * Puts the current dimensions of the robot, in game script coordinates,
	 * into the given rect, and returns the total number of frames in the robot
	 * animation.
	 */
	uint16 getFrameSize(Common::Rect &outRect) const;

	/**
	 * Pumps the robot player for the next frame of video. This is the main
	 * rendering function.
	 */
	void doRobot();

	/**
	 * Submits any outstanding audio blocks that should be added to the queue
	 * before the robot frame becomes visible.
	 */
	void frameAlmostVisible();

	/**
	 * Evaluates frame drift and makes modifications to the player in order to
	 * ensure that future frames will arrive on time.
	 */
	void frameNowVisible();

	/**
	 * Scales a vertically compressed cel to its original uncompressed
	 * dimensions.
	 */
	void expandCel(byte *target, const byte* source, const int16 celWidth, const int16 celHeight) const;

	int16 getPriority() const;

	/**
	 * Sets the visual priority of the robot.
	 * @see Plane::_priority
	 */
	void setPriority(const int16 newPriority);

private:
	enum CompressionType {
		kCompressionLZS  = 0,
		kCompressionNone = 2
	};

	/**
	 * Describes the state of a Robot video cel.
	 */
	struct CelHandleInfo {
		/**
		 * The persistence level of Robot cels.
		 */
		enum CelHandleLifetime {
			kNoCel         = 0,
			kFrameLifetime = 1,
			kRobotLifetime = 2
		};

		/**
		 * A reg_t pointer to an in-memory bitmap containing the cel.
		 */
		reg_t bitmapId;

		/**
		 * The lifetime of the cel, either just for this frame or for the entire
		 * duration of the robot playback.
		 */
		CelHandleLifetime status;

		/**
		 * The size, in pixels, of the decompressed cel.
		 */
		int area;

		CelHandleInfo() : bitmapId(NULL_REG), status(kNoCel), area(0) {}
	};

	typedef Common::Array<ScreenItem *> RobotScreenItemList;
	typedef Common::Array<CelHandleInfo> CelHandleList;
	typedef Common::Array<int> VideoSizeList;
	typedef Common::Array<uint> MaxCelAreaList;
	typedef Common::Array<reg_t> FixedCelsList;
	typedef Common::Array<Common::Point> CelPositionsList;
	typedef Common::Array<byte> ScratchMemory;

	/**
	 * Renders a version 5/6 robot frame.
	 */
	void doVersion5(const bool shouldSubmitAudio = true);

	/**
	 * Creates screen items for a version 5/6 robot.
	 */
	void createCels5(const byte *rawVideoData, const int16 numCels, const bool usePalette);

	/**
	 * Creates a single screen item for a cel in a version 5/6 robot.
	 *
	 * Returns the size, in bytes, of the raw cel data.
	 */
	uint32 createCel5(const byte *rawVideoData, const int16 screenItemIndex, const bool usePalette);

	/**
	 * Preallocates memory for the next `numCels` cels in the robot data stream.
	 */
	void preallocateCelMemory(const byte *rawVideoData, const int16 numCels);

	/**
	 * The decompressor for LZS-compressed cels.
	 */
	DecompressorLZS _decompressor;

	/**
	 * The ID of the robot plane.
	 */
	reg_t _planeId;

	/**
	 * The origin of the robot animation, in screen coordinates.
	 */
	Common::Point _position;

	/**
	 * Global scaling applied to the robot.
	 */
	ScaleInfo _scaleInfo;

	/**
	 * The native resolution of the robot.
	 */
	int16 _xResolution, _yResolution;

	/**
	 * Whether or not the coordinates read from robot data are high resolution.
	 */
	bool _isHiRes;

	/**
	 * The maximum number of cels that will be rendered on any given frame in
	 * this robot. Used for preallocation of cel memory.
	 */
	int16 _maxCelsPerFrame;

	/**
	 * The maximum areas, in pixels, for each of the fixed cels in the robot.
	 * Used for preallocation of cel memory.
	 */
	MaxCelAreaList _maxCelArea;

	/**
	 * The hunk palette to use when rendering the current frame, if the
	 * `usePalette` flag was set in the robot header.
	 */
	uint8 *_rawPalette;

	/**
	 * A list of the raw video data sizes, in bytes, for each frame of the
	 * robot.
	 */
	VideoSizeList _videoSizes;

	/**
	 * A list of cels that will be present for the entire duration of the robot
	 * animation.
	 */
	FixedCelsList _fixedCels;

	/**
	 * A list of handles for each cel in the current frame.
	 */
	CelHandleList _celHandles;

	/**
	 * Scratch memory used to temporarily store decompressed cel data for
	 * vertically squashed cels.
	 */
	ScratchMemory _celDecompressionBuffer;

	/**
	 * The size, in bytes, of the squashed cel decompression buffer.
	 */
	int _celDecompressionArea;

	/**
	 * If true, the robot just started playing and is awaiting output for the
	 * first frame.
	 */
	bool _syncFrame;

	/**
	 * Scratch memory used to store the compressed robot video data for the
	 * current frame.
	 */
	ScratchMemory _doVersion5Scratch;

	/**
	 * When set to a non-negative value, forces the next call to doRobot to
	 * render the given frame number instead of whatever frame would have
	 * normally been rendered.
	 */
	mutable int _cueForceShowFrame;

	/**
	 * The plane where the robot animation will be drawn.
	 */
	Plane *_plane;

	/**
	 * A list of pointers to ScreenItems used by the robot.
	 */
	RobotScreenItemList _screenItemList;

	/**
	 * The positions of the various screen items in this robot, in screen
	 * coordinates.
	 */
	Common::Array<int16> _screenItemX, _screenItemY;

	/**
	 * The raw position values from the cel header for each screen item
	 * currently on-screen.
	 */
	Common::Array<int16> _originalScreenItemX, _originalScreenItemY;

	/**
	 * The duration of the current robot, in frames.
	 */
	uint16 _numFramesTotal;

	/**
	 * The screen priority of the video.
	 * @see ScreenItem::_priority
	 */
	int16 _priority;

	/**
	 * The amount of visual vertical compression applied to the current cel. A
	 * value of 100 means no compression; a value above 100 indicates how much
	 * the cel needs to be scaled along the y-axis to return to its original
	 * dimensions.
	 */
	uint8 _verticalScaleFactor;
};
} // end of namespace Sci
#endif
