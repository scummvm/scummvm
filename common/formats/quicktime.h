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

//
// Heavily based on ffmpeg code.
//
// Copyright (c) 2001 Fabrice Bellard.
// First version by Francois Revol revol@free.fr
// Seek function by Gael Chardon gael.dev@4now.net
//

#ifndef COMMON_QUICKTIME_H
#define COMMON_QUICKTIME_H

#include "common/array.h"
#include "common/scummsys.h"
#include "common/path.h"
#include "common/stream.h"
#include "common/rational.h"
#include "common/types.h"
#include "common/rect.h"

namespace Common {
	class MacResManager;

/**
 * @defgroup common_quicktime Quicktime file parser
 * @ingroup common
 *
 * @brief Parser for QuickTime/MPEG-4 files.
 *
 * @details File parser used in engines:
 *          - groovie
 *          - mohawk
 *          - mtropolis
 *          - sci
 * @{
 */

class QuickTimeParser {
public:
	QuickTimeParser();
	virtual ~QuickTimeParser();

	/**
	 * Load a QuickTime file
	 * @param filename	the filename to load
	 */
	bool parseFile(const Path &filename);

	/**
	 * Load a QuickTime file from a SeekableReadStream
	 * @param stream	the stream to load
	 * @param disposeFileHandle whether to delete the stream after use
	 */
	bool parseStream(SeekableReadStream *stream, DisposeAfterUse::Flag disposeFileHandle = DisposeAfterUse::YES);

	/**
	 * Close a QuickTime file
	 */
	void close();

	/**
	 * Flattens edit lists to a single edit containing the first edit contiguously through the last edit.
	 * Used to work around bad edit offsets.
	 */
	void flattenEditLists();

	/**
	 * Set the beginning offset of the video so we can modify the offsets in the stco
	 * atom of videos inside the Mohawk/mTropolis archives
	 * @param offset the beginning offset of the video
	 */
	void setChunkBeginOffset(uint32 offset) { _beginOffset = offset; }

	/**
	 * Returns the movie time scale
	 */
	uint32 getTimeScale() const { return _timeScale; }

	/** Find out if this parser has an open file handle */
	bool isOpen() const { return _fd != nullptr; }

	enum class QTVRType {
		OTHER,
		OBJECT,
		PANORAMA
	};

protected:
	// This is the file handle from which data is read from. It can be the actual file handle or a decompressed stream.
	SeekableReadStream *_fd;

	struct TimeToSampleEntry {
		int count;
		int duration; // media time
	};

	struct SampleToChunkEntry {
		uint32 first;
		uint32 count;
		uint32 id;
	};

	struct EditListEntry {
		uint32 trackDuration; // movie time
		uint32 timeOffset;    // movie time
		int32 mediaTime;      // media time
		Rational mediaRate;
	};

	struct Track;

	class SampleDesc {
	public:
		SampleDesc(Track *parentTrack, uint32 codecTag);
		virtual ~SampleDesc();

		uint32 getCodecTag() const { return _codecTag; }

		SeekableReadStream *_extraData;
		byte _objectTypeMP4;

	protected:
		Track *_parentTrack;
		uint32 _codecTag;
	};

	enum CodecType {
		CODEC_TYPE_MOV_OTHER,
		CODEC_TYPE_VIDEO,
		CODEC_TYPE_AUDIO,
		CODEC_TYPE_MIDI,
		CODEC_TYPE_PANO
	};

	enum class GraphicsMode {
		COPY				 = 0x0,   // Directly copy the source image over the destination.
		DITHER_COPY			 = 0x40,  // Dither the image (if needed), otherwise copy.
		BLEND				 = 0x20,  // Blend source and destination pixel colors using opcolor values.
		TRANSPARENT			 = 0x24,  // Replace destination with source if not equal to opcolor.
		STRAIGHT_ALPHA  	 = 0x100, // Blend source and destination pixels, with the proportion controlled by the alpha channel.
		PREMUL_WHITE_ALPHA   = 0x101, // Blend after removing pre-multiplied white from the source.
		PREMUL_BLACK_ALPHA	 = 0x102, // Blend after removing pre-multiplied black from the source.
		STRAIGHT_ALPHA_BLEND = 0x104, // Similar to straight alpha, but the alpha for each channel is combined with the corresponding opcolor channel.
		COMPOSITION			 = 0x103  // Render offscreen and then dither-copy to the main screen (tracks only).
	};

	struct PanoramaNode {
		uint32 nodeID;
		uint32 timestamp;
	};

	struct PanoramaInformation {
		String name;
		uint32 defNodeID;
		float defZoom;
		Array<PanoramaNode> nodes;
	};

	struct PanoSampleHeader {
		uint32 nodeID;

		float defHPan;
		float defVPan;
		float defZoom;

		// Constraints for this node; zero for default
		float minHPan;
		float minVPan;
		float maxHPan;
		float maxVPan;
		float minZoom;

		int32 nameStrOffset;
		int32 commentStrOffset;
	};

	enum class HotSpotType {
		undefined,
		anim,
		cnod,
		link,
		navg,
		soun,
	};

	struct PanoHotSpot {
		uint16 id;
		HotSpotType type;
		uint32 typeData; // for link and navg, the ID in the link and navg table

		// Canonical view for this hotspot
		float viewHPan;
		float viewVPan;
		float viewZoom;

		Rect rect;

		int32 mouseOverCursorID;
		int32 mouseDownCursorID;
		int32 mouseUpCursorID;

		int32 nameStrOffset;
		int32 commentStrOffset;
	};

	struct PanoHotSpotTable {
		Array<PanoHotSpot> hotSpots;
	};

	struct PanoStringTable {
		String strings;

		String getString(int32 offset) const;
	};

	struct PanoLink {
		uint16 id;
		uint16 toNodeID;

		// Values to set at the destination node
		float toHPan;
		float toVPan;
		float toZoom;

		int32 nameStrOffset;
		int32 commentStrOffset;
	};

	struct PanoLinkTable {
		Array<PanoLink> links;
	};

	struct PanoNavigation {
		uint16 id;

		uint32 hPan;
		uint32 vPan;
		uint32 zoom;

		Rect rect; // Starting rect for zoom out transitions

		// Values to set at the destination node
		int32 nameStrOffset;
		int32 commentStrOffset;
	};

	struct PanoNavigationTable {
		Array<PanoNavigation> navs;
	};

	struct PanoTrackSample {
		PanoSampleHeader hdr;
		PanoHotSpotTable hotSpotTable;
		PanoStringTable strTable;
		PanoLinkTable linkTable;
		PanoNavigationTable navTable;
	};

	struct Track {
		Track();
		~Track();

		uint32 chunkCount;
		uint32 *chunkOffsets;
		int timeToSampleCount;
		TimeToSampleEntry *timeToSample;
		uint32 sampleToChunkCount;
		SampleToChunkEntry *sampleToChunk;
		uint32 sampleSize;
		uint32 sampleCount;
		uint32 *sampleSizes;
		uint32 keyframeCount;
		uint32 *keyframes;
		int32 timeScale; // media time

		uint16 width;
		uint16 height;
		CodecType codecType;

		Array<SampleDesc *> sampleDescs;

		Common::Array<EditListEntry> editList;

		uint32 frameCount;    // from stts
		uint32 duration;      // movie time
		uint32 mediaDuration; // media time
		Rational scaleFactorX;
		Rational scaleFactorY;

		Common::String volume;
		Common::String filename;
		Common::String path;
		Common::String directory;
		int16 nlvlFrom;
		int16 nlvlTo;

		PanoramaInformation panoInfo;
		Array<PanoTrackSample> panoSamples;

		GraphicsMode graphicsMode; // Transfer mode
		uint16 opcolor[3];         // RGB values used in the transfer mode specified by graphicsMode.

		uint16 soundBalance; // Controls the sound mix between the computer's two speakers, usually set to 0.

		uint targetTrack;
	};

	enum class MovieType {
		kStandardObject = 1,
		kOldNavigableMovieScene,
		kObjectInScene
	};

	struct Navigation {
		uint16 columns = 1;
		uint16 rows = 1;
		uint16 loop_size = 0;      // Number of frames shot at each position
		uint16 frame_duration = 1;

		MovieType movie_type = MovieType::kStandardObject;

		uint16 loop_ticks = 0;	 // Number of ticks before next frame of loop is displayed

		float field_of_view = 1.0f;

		float startHPan = 1.0f;
		float startVPan = 1.0f;
		float endHPan = 1.0f;
		float endVPan = 1.0f;
		float initialHPan = 1.0f;
		float initialVPan = 1.0f;
	};

	virtual SampleDesc *readSampleDesc(Track *track, uint32 format, uint32 descSize) = 0;

	uint32 _timeScale;      // movie time
	uint32 _duration;       // movie time
	Rational _scaleFactorX;
	Rational _scaleFactorY;
	Array<Track *> _tracks;
	Navigation _nav;
	QTVRType _qtvrType;
	uint16 _winX;
	uint16 _winY;

	Track *_panoTrack;

	void init();

private:
	struct Atom {
		uint32 type;
		uint32 offset;
		uint32 size;
	};

	struct ParseTable {
		int (QuickTimeParser::*func)(Atom atom);
		uint32 type;
	};

	DisposeAfterUse::Flag _disposeFileHandle;
	const ParseTable *_parseTable;
	uint32 _beginOffset;
	MacResManager *_resFork;
	bool _foundMOOV;

	void initParseTable();

	bool parsePanoramaAtoms();

	int readDefault(Atom atom);
	int readLeaf(Atom atom);
	int readDREF(Atom atom);
	int readELST(Atom atom);
	int readHDLR(Atom atom);
	int readMDHD(Atom atom);
	int readMOOV(Atom atom);
	int readMVHD(Atom atom);
	int readTKHD(Atom atom);
	int readTRAK(Atom atom);
	int readSMHD(Atom atom);
	int readSTCO(Atom atom);
	int readSTSC(Atom atom);
	int readSTSD(Atom atom);
	int readSTSS(Atom atom);
	int readSTSZ(Atom atom);
	int readSTTS(Atom atom);
	int readVMHD(Atom atom);
	int readCMOV(Atom atom);
	int readWAVE(Atom atom);
	int readESDS(Atom atom);
	int readSMI(Atom atom);
	int readCTYP(Atom atom);
	int readWLOC(Atom atom);
	int readNAVG(Atom atom);
	int readGMIN(Atom atom);
	int readPINF(Atom atom);

	int readPHDR(Atom atom);
	int readPHOT(Atom atom);
	int readSTRT(Atom atom);
	int readPLNK(Atom atom);
	int readPNAV(Atom atom);
};

/** @} */

} // End of namespace Common

#endif
