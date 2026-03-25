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

#include "backends/audiocd/mds/mds-disc.h"
#include "backends/fs/iso/iso-abstract-fs.h"
#include "audio/decoders/raw.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/formats/iso9660.h"
#include "common/formats/iso9660archive.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "common/substream.h"
#include "common/textconsole.h"

// MDS binary format constants
//
// Reference: Alcohol 120% MDS format (reverse-engineered; widely documented).
// All multi-byte integers in MDS are little-endian.

static const char kMDSSignature[] = "MEDIA DESCRIPTOR";  // 16 bytes, no NUL in file
static constexpr int kMDSSigLen   = 16;

// MDS header layout (88 bytes total):
//   0x00  16 b  signature "MEDIA DESCRIPTOR"
//   0x10   1 b  version major
//   0x11   1 b  version minor
//   0x12   2 b  media type (uint16 LE): 0=CD-ROM, 1=CD-R, 4=DVD-ROM, 5=DVD-R
//   0x14   2 b  number of sessions (uint16 LE)
//   0x16–0x33   various fields (BCA length/offset, disc structures, unknowns)
//   0x34   4 b  disc structures offset (uint32 LE) -- usually 0 (not sessions!)
//   0x38–0x4F   more unknowns
//   0x50   4 b  session blocks offset (uint32 LE)  <-- sessions are here
//   0x54   4 b  DPM data offset (uint32 LE)
enum MDSHeaderOfs {
	kMDSOfsSignature   = 0x00, // 16 bytes
	kMDSOfsVersion     = 0x10, // 2 bytes (major, minor)
	kMDSOfsNumSessions = 0x14, // uint16 LE
	kMDSOfsSessionOfs  = 0x50, // uint32 LE: byte offset of first session block
	kMDSHeaderSize     = 0x58  // minimum size to read
};

// Session block layout (size = 0x18 = 24 bytes):
//   0x00   4 b  pregap start LBA (int32 LE)
//   0x04   4 b  session length in LBAs (int32 LE)
//   0x08   2 b  session number (uint16 LE)
//   0x0A   1 b  total number of track block entries (uint8), including lead-in TOC entries
//   0x0B   1 b  number of lead-in entries (uint8), e.g. 3 for points 0xA0/0xA1/0xA2
//   0x0C   2 b  first track number (uint16 LE)
//   0x0E   2 b  last track number (uint16 LE)
//   0x10   4 b  unknown / reserved
//   0x14   4 b  byte offset of first track block for this session (uint32 LE)
enum MDSSessionOfs {
	kSessionNumBlocks    = 0x0A, // uint8: total track block entries (including TOC)
	kSessionNumLeadIn    = 0x0B, // uint8: number of lead-in TOC entries
	kSessionFirstTrack   = 0x0C, // uint16 LE: first track number
	kSessionLastTrack    = 0x0E, // uint16 LE: last track number
	kSessionTrackOfs     = 0x14, // uint32 LE: offset of first track block
	kSessionBlockSize    = 0x18
};

// Track block layout (size = 0x50 = 80 bytes):
//   0x00   1 b  track mode (uint8): encodes track type + raw/cooked + subchannel
//   0x01   1 b  subchannel mode (uint8)
//   0x02   1 b  ADR/Control nibbles (uint8)
//   0x03   1 b  TNO (uint8): track number (0 for lead-in TOC entries)
//   0x04   1 b  point (uint8): track number for normal tracks, 0xA0-0xA2 for TOC
//   0x05   3 b  MSF address (minutes, seconds, frames)
//   0x08   1 b  zero
//   0x09   3 b  PMSF address (minutes, seconds, frames)
//   0x0C   4 b  offset to extra data block (uint32 LE): pregap + sector count
//   0x10   2 b  sector size in bytes (uint16 LE): 2048/2336/2352/2448
//   0x12  18 b  unknown
//   0x24   4 b  start sector LBA (uint32 LE)
//   0x28   8 b  byte offset into the MDF data file (uint64 LE)
//   0x30   4 b  unknown (file count)
//   0x34   4 b  footer offset (uint32 LE): offset to footer with MDF filename
//   0x38  24 b  unknown
enum MDSTrackOfs {
	kTrackMode        = 0x00, // uint8
	kTrackPoint       = 0x04, // uint8: track number for normal tracks, 0xA0-0xA2 for TOC
	kTrackExtraOfs    = 0x0C, // uint32 LE: offset to extra data block
	kTrackSectorSize  = 0x10, // uint16 LE
	kTrackStartLBA    = 0x24, // uint32 LE
	kTrackMDFOffset   = 0x28, // uint64 LE: byte offset into the MDF
	kTrackBlockSize   = 0x50
};

// Extra data block offsets (relative to the extra block start)
enum MDSExtraOfs {
	kExtraPregap      = 0x00, // uint32 LE
	kExtraSectorCount = 0x04, // uint32 LE
	kExtraBlockSize   = 0x08
};

// MDS track-mode byte values (byte 0x00 of a track block).
// Encodes track type, raw/cooked, and subchannel presence.
enum MDSRawMode {
	kRawModeAudio       = 0xA9, // Audio (2352)
	kRawModeAudioSub    = 0xE9, // Audio + subchannel (2448)
	kRawModeMode1       = 0xAA, // Mode 1 cooked (2048)
	kRawModeMode1RawSub = 0xEA, // Mode 1 raw + subchannel (2448)
	kRawModeMode2       = 0xAB, // Mode 2 (2336)
	kRawModeMode2RawSub = 0xEB, // Mode 2 raw + subchannel (2448)
	kRawModeMode2Form1  = 0xEC, // Mode 2 Form 1 (2048)
	kRawModeMode2Form2  = 0xED  // Mode 2 Form 2 (2328)
};

// Sector data layout for raw Mode 1 sectors (2352 bytes)
// [0-11]  sync pattern  (12 bytes)
// [12-14] address MSF   (3 bytes)
// [15]    mode (0x01)   (1 byte)
// [16-2063] user data   (2048 bytes)
// [2064-2351] ECC/EDC   (288 bytes)
static constexpr uint32 kRawSectorSize     = 2352;
static constexpr uint32 kRawSubSectorSize  = 2448; // 2352 raw + 96 subchannel
static constexpr uint32 kLogicalSectorSize = 2048;
static constexpr uint32 kSyncHeaderSize    = 16;   // sync (12) + address (3) + mode (1)
MDSDisc::MDSDisc() : _open(false) {}

void MDSDisc::close() {
	_tracks.clear();
	_mdfPath.clear();
	_open = false;
}

static Common::String swapExtension(const Common::String &path, const char *newExt) {
	auto dot = path.rfind('.');
	if (dot != Common::String::npos) {
		return path.substr(0, dot) + newExt;
	}
	return path + newExt;
}

static Common::Path resolveToMDS(const Common::Path &path) {
	const auto str = path.toString(Common::Path::kNativeSeparator);
	if (!str.hasSuffixIgnoreCase(".mdf")) {
		return path;
	}
	return Common::Path(swapExtension(str, ".mds"), Common::Path::kNativeSeparator);
}

bool MDSDisc::parseMDSStream(Common::SeekableReadStream *stream, const char *context) {
	const int64 fileSize = stream->size();
	if (fileSize < kMDSHeaderSize) {
		warning("MDSDisc: %s too small (%lld bytes, need at least %d)",
		        context, static_cast<long long>(fileSize), kMDSHeaderSize);
		return false;
	}

	Common::ScopedPtr<byte, Common::ArrayDeleter<byte>> buf(new byte[static_cast<uint32>(fileSize)]);
	stream->seek(0);
	if (stream->read(buf.get(), static_cast<uint32>(fileSize)) != static_cast<uint32>(fileSize)) {
		warning("MDSDisc: read failed for %s", context);
		return false;
	}

	if (memcmp(buf.get() + kMDSOfsSignature, kMDSSignature, kMDSSigLen) != 0) {
		warning("MDSDisc: invalid MDS signature in %s", context);
		return false;
	}

	const uint8  verMajor    = buf.get()[kMDSOfsVersion];
	const uint8  verMinor    = buf.get()[kMDSOfsVersion + 1];
	const uint16 numSessions = READ_LE_UINT16(buf.get() + kMDSOfsNumSessions);
	const uint32 sessionOfs  = READ_LE_UINT32(buf.get() + kMDSOfsSessionOfs);
	debug(2, "MDSDisc: version %u.%u, %u session(s), session block at 0x%04x",
	      verMajor, verMinor, numSessions, sessionOfs);

	if (!readSessionBlocks(buf.get(), static_cast<uint32>(fileSize), sessionOfs, numSessions)
	    || _tracks.empty()) {
		warning("MDSDisc: no tracks found in %s", context);
		return false;
	}

	return true;
}

bool MDSDisc::open(const Common::Path &path) {
	close();

	const Common::Path mdsPath = resolveToMDS(path);
	debug(2, "MDSDisc: opening '%s'", mdsPath.toString().c_str());

	Common::ScopedPtr<Common::SeekableReadStream> mdsStream(
		Common::FSNode(mdsPath).createReadStream());
	if (!mdsStream) {
		warning("MDSDisc: cannot open '%s'", mdsPath.toString().c_str());
		return false;
	}

	if (!parseMDSStream(mdsStream.get(), mdsPath.toString().c_str())) {
		return false;
	}

	_mdfPath = swapExtension(mdsPath.toString(Common::Path::kNativeSeparator), ".mdf");
	debug(2, "MDSDisc: successfully parsed %d track(s)", static_cast<int>(_tracks.size()));
	_open = true;
	return true;
}

bool MDSDisc::open(Common::SeekableReadStream *stream) {
	close();

	if (!stream) {
		warning("MDSDisc: null stream");
		return false;
	}

	if (!parseMDSStream(stream, "stream")) {
		return false;
	}

	_open = true;
	return true;
}

bool MDSDisc::readSessionBlocks(const byte *buf, uint32 bufSize,
                                 uint32 sessionOffset, uint16 numSessions) {
	if (sessionOffset + kSessionBlockSize > bufSize) {
		return false;
	}

	for (uint16 s = 0; s < numSessions; s++) {
		const byte *sess = buf + sessionOffset + s * kSessionBlockSize;
		if (static_cast<uint32>(sess - buf) + kSessionBlockSize > bufSize) {
			break;
		}

		const uint8  numBlocks  = sess[kSessionNumBlocks];
		const uint8  numLeadIn  = sess[kSessionNumLeadIn];
		const uint16 firstTrack = READ_LE_UINT16(sess + kSessionFirstTrack);
		const uint16 lastTrack  = READ_LE_UINT16(sess + kSessionLastTrack);
		const uint32 trackOfs   = READ_LE_UINT32(sess + kSessionTrackOfs);
		debug(5, "MDSDisc: session %u: tracks %u-%u, %u block(s) (%u lead-in), track block at 0x%04x",
		      static_cast<uint32>(s) + 1, static_cast<uint32>(firstTrack), static_cast<uint32>(lastTrack),
		      static_cast<uint32>(numBlocks), static_cast<uint32>(numLeadIn), trackOfs);

		for (uint8 t = 0; t < numBlocks; t++) {
			if (trackOfs + kTrackBlockSize > bufSize) {
				break;
			}
			const byte *tr = buf + trackOfs + t * kTrackBlockSize;
			if (static_cast<uint32>(tr - buf) + kTrackBlockSize > bufSize) {
				break;
			}

			// Skip TOC lead-in entries (points 0xA0, 0xA1, 0xA2)
			const uint8 point = tr[kTrackPoint];
			if (point >= 0xA0) {
				debug(5, "MDSDisc:   skipping TOC entry point=0x%02x", point);
				continue;
			}

			TrackInfo info{};
			const uint8 rawMode = tr[kTrackMode];

			// Map MDS mode byte to our enum.
			// MDS encodes track type, raw/cooked, and subchannel presence
			// in a single byte.
			switch (rawMode) {
			case kRawModeAudio:       // Audio (2352)
			case kRawModeAudioSub:    // Audio + subchannel (2448)
				info.mode = kModeAudio;
				break;
			case kRawModeMode1:       // Mode1 cooked (2048)
				info.mode = kModeMode1;
				break;
			case kRawModeMode1RawSub: // Mode1 raw + subchannel (2448)
				info.mode = kModeMode1Raw;
				break;
			case kRawModeMode2:       // Mode2 (2336)
			case kRawModeMode2RawSub: // Mode2 raw + subchannel (2448)
			case kRawModeMode2Form1:  // Mode2 Form1 (2048)
			case kRawModeMode2Form2:  // Mode2 Form2 (2328)
				info.mode = kModeMode2;
				break;
			default:
				// For unknown mode values, guess from sector size
				info.mode = kModeUnknown;
				break;
			}

			info.sectorSize  = READ_LE_UINT16(tr + kTrackSectorSize);
			info.mdfOffset   = READ_LE_UINT64(tr + kTrackMDFOffset);
			info.startLba    = READ_LE_UINT32(tr + kTrackStartLBA);

			// Sector count and pregap are in the extra data block, not the track block
			const uint32 extraOfs = READ_LE_UINT32(tr + kTrackExtraOfs);
			if (extraOfs != 0 && extraOfs + kExtraBlockSize <= bufSize) {
				info.pregapSectors = READ_LE_UINT32(buf + extraOfs + kExtraPregap);
				info.sectorCount   = READ_LE_UINT32(buf + extraOfs + kExtraSectorCount);
			} else {
				info.pregapSectors = 0;
				info.sectorCount   = 0;
			}

			// Track number: use Point field (offset 0x04) for the track number;
			// TNO (offset 0x03) is 0 for lead-in and normal track entries alike
			info.number = tr[kTrackPoint];
			if (info.number == 0) {
				info.number = static_cast<int>(_tracks.size()) + 1;
			}

			// Sanity: skip track entries that don't look like real tracks
			if (info.sectorSize == 0 || info.sectorSize > 2448) {
				debug(5, "MDSDisc: skipping track entry with invalid sector size %u", static_cast<uint32>(info.sectorSize));
				continue;
			}

			debug(5, "MDSDisc:   track %2d: mode=%d sectorSize=%4u startLBA=%6u sectors=%u pregap=%u mdfOffset=%llu",
			      info.number, info.mode, static_cast<uint32>(info.sectorSize),
			      info.startLba, info.sectorCount, info.pregapSectors,
			      static_cast<unsigned long long>(info.mdfOffset));

			_tracks.push_back(info);
		}
	}
	return true;
}

MDSDisc::TrackInfo MDSDisc::getTrack(int trackNo) const {
	for (const auto &t : _tracks) {
		if (t.number == trackNo) {
			return t;
		}
	}
	TrackInfo empty{};
	empty.number = -1;
	empty.mode   = kModeUnknown;
	return empty;
}

const MDSDisc::TrackInfo *MDSDisc::findDataTrack() const {
	for (const auto &t : _tracks) {
		if (t.mode == kModeMode1 || t.mode == kModeMode1Raw || t.mode == kModeMode2) {
			return &t;
		}
	}
	return nullptr;
}

// Sector-stripping stream
//
// Presents a view of MDF sectors with per-sector header and trailer bytes
// stripped on-the-fly.  This avoids loading entire tracks into memory.
//
// For audio tracks with subchannel (2448 → 2352): headerSkip=0,  usable=2352
// For raw Mode 1 data tracks      (2352 → 2048): headerSkip=16, usable=2048
// For raw+sub Mode 1 data tracks  (2448 → 2048): headerSkip=16, usable=2048

/** Describes how to decode raw sectors from an MDF file. */
struct SectorLayout {
	uint32 sectorCount;    ///< Number of sectors
	uint32 physSectorSize; ///< Raw bytes per sector in the MDF
	uint32 headerSkip;     ///< Bytes to skip at start of each sector
	uint32 usableBytes;    ///< Usable data bytes per sector
};

class SectorStrippingStream : public Common::SeekableReadStream {
public:
	SectorStrippingStream(Common::SeekableReadStream *mdf,
	                      uint64 baseOffset, const SectorLayout &layout)
		: _mdf(mdf), _baseOffset(baseOffset),
		  _physSectorSize(layout.physSectorSize), _headerSkip(layout.headerSkip),
		  _usableBytes(layout.usableBytes),
		  _pos(0), _logicalSize(static_cast<int64>(layout.sectorCount) * layout.usableBytes) {}

	int64 size() const override { return _logicalSize; }
	int64 pos() const override { return _pos; }
	bool eos() const override { return _pos >= _logicalSize; }

	bool seek(int64 offset, int whence = SEEK_SET) override {
		switch (whence) {
		case SEEK_SET: _pos = offset; break;
		case SEEK_CUR: _pos += offset; break;
		case SEEK_END: _pos = _logicalSize + offset; break;
		}
		if (_pos < 0) {
			_pos = 0;
		}
		if (_pos > _logicalSize) {
			_pos = _logicalSize;
		}
		return true;
	}

	uint32 read(void *dataPtr, uint32 dataSize) override {
		auto *dst = static_cast<byte *>(dataPtr);
		uint32 bytesRead = 0;

		while (dataSize > 0 && _pos < _logicalSize) {
			const auto sector = static_cast<uint32>(_pos / _usableBytes);
			const auto offsetInSector = static_cast<uint32>(_pos % _usableBytes);
			const uint32 toRead = MIN(dataSize, _usableBytes - offsetInSector);

			const uint64 physOffset = _baseOffset
			    + static_cast<uint64>(sector) * _physSectorSize
			    + _headerSkip + offsetInSector;
			_mdf->seek(static_cast<int64>(physOffset));
			const uint32 got = _mdf->read(dst, toRead);

			dst += got;
			_pos += got;
			bytesRead += got;
			dataSize -= got;

			if (got < toRead) {
				break;
			}
		}
		return bytesRead;
	}

private:
	Common::ScopedPtr<Common::SeekableReadStream> _mdf;
	uint64 _baseOffset;
	uint32 _physSectorSize;
	uint32 _headerSkip;
	uint32 _usableBytes;
	int64  _pos;
	int64  _logicalSize;
};

Common::SeekableReadStream *MDSDisc::openMDF() const {
	Common::SeekableReadStream *mdf =
		Common::FSNode(Common::Path(_mdfPath, Common::Path::kNativeSeparator)).createReadStream();
	if (!mdf) {
		warning("MDSDisc: cannot open MDF '%s'", _mdfPath.c_str());
	}
	return mdf;
}

/**
 * Create a stream over a range of MDF sectors, choosing the lightest
 * representation automatically:
 *
 *  - If physSectorSize == usableBytes the sectors need no transformation
 *    and a plain SafeSeekableSubReadStream is returned.
 *  - Otherwise a SectorStrippingStream strips headerSkip leading bytes
 *    and any trailing bytes from each physical sector on-the-fly.
 *
 * If sectorCount is 0 it is estimated from the MDF file size.
 * Ownership of @p mdf is transferred to the returned stream.
 */
Common::SeekableReadStream *MDSDisc::createTrackStream(
		Common::SeekableReadStream *mdf, uint64 baseOffset,
		const SectorLayout &layout) const {
	SectorLayout l = layout;
	if (l.sectorCount == 0) {
		l.sectorCount = static_cast<uint32>((mdf->size() - baseOffset) / l.physSectorSize);
		debug(5, "MDSDisc: sector count not in header, estimated %u from file size", l.sectorCount);
	}

	if (l.physSectorSize == l.usableBytes) {
		const uint64 end64 = baseOffset + static_cast<uint64>(l.sectorCount) * l.usableBytes;
		if (end64 > UINT32_MAX) {
			warning("MDSDisc: track exceeds 4 GB substream limit");
			delete mdf;
			return nullptr;
		}
		const auto begin = static_cast<uint32>(baseOffset);
		const auto end   = static_cast<uint32>(end64);
		debug(5, "MDSDisc: plain substream [%u, %u)", begin, end);
		return new Common::SafeSeekableSubReadStream(mdf, begin, end,
		                                              DisposeAfterUse::YES);
	}

	debug(5, "MDSDisc: sector-stripping stream: %u sectors (%u -> %u bytes)",
	      l.sectorCount, l.physSectorSize, l.usableBytes);
	return new SectorStrippingStream(mdf, baseOffset, l);
}

Common::SeekableReadStream *MDSDisc::openDataTrackStream() const {
	if (!_open) {
		return nullptr;
	}

	const TrackInfo *dataTrack = findDataTrack();
	if (!dataTrack) {
		warning("MDSDisc: no data track found");
		return nullptr;
	}

	debug(2, "MDSDisc: opening data track %d from MDF '%s'", dataTrack->number, _mdfPath.c_str());

	Common::SeekableReadStream *mdf = openMDF();
	if (!mdf) {
		return nullptr;
	}

	return createTrackStream(mdf, dataTrack->mdfOffset,
	                         SectorLayout{dataTrack->sectorCount, dataTrack->sectorSize,
	                                      kSyncHeaderSize, kLogicalSectorSize});
}

Common::SeekableReadStream *MDSDisc::openAudioTrackStream(int trackNo) const {
	if (!_open) {
		return nullptr;
	}

	const TrackInfo track = getTrack(trackNo);
	if (track.number < 0 || track.mode != kModeAudio) {
		warning("MDSDisc: track %d is not an audio track", trackNo);
		return nullptr;
	}

	debug(2, "MDSDisc: opening audio track %d from MDF '%s'", trackNo, _mdfPath.c_str());

	Common::SeekableReadStream *mdf = openMDF();
	if (!mdf) {
		return nullptr;
	}

	uint64 baseOffset  = track.mdfOffset;
	uint32 sectorCount = track.sectorCount;

	// Skip pregap sectors (silence before the actual audio content).
	// On a real CD, the player seeks to index 1 (after pregap) when you
	// play a track, so frame 0 = start of actual content.
	if (track.pregapSectors > 0 && track.pregapSectors < sectorCount) {
		debug(2, "MDSDisc: audio track %d: skipping %u pregap sectors", trackNo, track.pregapSectors);
		baseOffset  += static_cast<uint64>(track.pregapSectors) * track.sectorSize;
		sectorCount -= track.pregapSectors;
	}

	return createTrackStream(mdf, baseOffset,
	                         SectorLayout{sectorCount, track.sectorSize,
	                                      0, kRawSectorSize});
}

Audio::SeekableAudioStream *MDSDisc::openAudioStream(int trackNo) const {
	Common::SeekableReadStream *rawStream = openAudioTrackStream(trackNo);
	if (!rawStream) {
		return nullptr;
	}

	// Red Book CD-DA: 44100 Hz, 16-bit signed little-endian, stereo
	return Audio::makeRawStream(rawStream, 44100,
		Audio::FLAG_16BITS | Audio::FLAG_STEREO | Audio::FLAG_LITTLE_ENDIAN,
		DisposeAfterUse::YES);
}

Common::SeekableReadStream *openDiscImageDataStream(const Common::Path &imagePath) {
	const auto pathStr = imagePath.toString();

	if (pathStr.hasSuffixIgnoreCase(".mds") || pathStr.hasSuffixIgnoreCase(".mdf")) {
		MDSDisc disc;
		if (disc.open(imagePath)) {  // open() handles .mdf → .mds redirect
			return disc.openDataTrackStream();
		}
		warning("Cannot open MDS disc image '%s'", pathStr.c_str());
		return nullptr;
	}

	// Plain .iso — open directly
	Common::SeekableReadStream *stream =
		Common::FSNode(imagePath).createReadStream();
	if (!stream) {
		warning("Cannot open ISO '%s'", pathStr.c_str());
	}
	return stream;
}

bool isDiscImageFile(const Common::String &name) {
	return name.hasSuffixIgnoreCase(".iso") ||
	       name.hasSuffixIgnoreCase(".mds") ||
	       name.hasSuffixIgnoreCase(".mdf");
}

static Common::SharedPtr<Common::ISO9660FileSystem> openDiscImageISO(const Common::Path &imagePath) {
	Common::SeekableReadStream *dataStream = openDiscImageDataStream(imagePath);
	if (!dataStream) {
		return nullptr;
	}
	auto isoFS = Common::SharedPtr<Common::ISO9660FileSystem>(new Common::ISO9660FileSystem(dataStream));
	if (!isoFS->isOpen()) {
		return nullptr;
	}
	return isoFS;
}

Common::FSNode openDiscImageFSNode(const Common::Path &imagePath) {
	auto isoFS = openDiscImageISO(imagePath);
	if (!isoFS) {
		return Common::FSNode();
	}
	return makeISOFSNodeRoot(isoFS, imagePath.toString());
}

Common::Archive *openDiscImageArchive(const Common::Path &imagePath,
                                       const Common::String &subdir) {
	auto isoFS = openDiscImageISO(imagePath);
	if (!isoFS) {
		return nullptr;
	}
	return new Common::ISO9660Archive(isoFS, subdir);
}

Common::FSNode resolveDiscImageNode(const Common::FSNode &node,
                                     Common::String *isoImagePath) {
	if (node.isDirectory()) return node;

	if (!isDiscImageFile(node.getName()))
		return node;

	const Common::Path imagePath = node.getPath();
	Common::FSNode isoRoot = openDiscImageFSNode(imagePath);
	if (!isoRoot.isDirectory()) return node;
	if (isoImagePath) *isoImagePath = imagePath.toString();
	return isoRoot;
}

void resolveDiscImageDir(Common::FSNode &dir) {
	if (ConfMan.hasKey("iso_path")) {
		Common::Path isoPath = ConfMan.getPath("iso_path");
		Common::FSNode isoRoot = openDiscImageFSNode(isoPath);
		if (isoRoot.isDirectory()) {
			dir = isoRoot;
			debug(2, "resolveDiscImageDir: resolved '%s'",
			      isoPath.toString().c_str());
		}
	}
}

void mountDiscImageInSearchMan() {
	if (ConfMan.hasKey("iso_path")) {
		Common::Path isoPath = ConfMan.getPath("iso_path");
		Common::String subdir = ConfMan.hasKey("iso_subdir")
		    ? ConfMan.get("iso_subdir") : Common::String();
		Common::Archive *isoArchive = openDiscImageArchive(isoPath, subdir);
		if (isoArchive) {
			SearchMan.add("game-iso", isoArchive, 0, true);
			debug(1, "Mounted ISO '%s' (subdir: '%s') into SearchMan",
			    isoPath.toString().c_str(), subdir.c_str());
		} else {
			warning("Failed to open disc image '%s'", isoPath.toString().c_str());
		}
	}
}

void saveDiscImageConfig(const Common::String &isoImagePath,
                         const Common::FSNode &resolvedDir,
                         const Common::String &domain) {
	if (isoImagePath.empty())
		return;

	Common::FSNode isoNode(Common::Path::fromConfig(isoImagePath));
	// Store the parent directory as "path" (real directory that exists),
	// and the image itself as "iso_path".
	ConfMan.setPath("path", isoNode.getParent().getPath(), domain);
	ConfMan.setPath("iso_path", isoNode.getPath(), domain);
	// If the game lives in a subdirectory of the ISO, record that too.
	Common::String isoVirtPath = resolvedDir.getPath().toString();
	Common::String isoRoot     = isoNode.getPath().toString();
	if (isoVirtPath.size() > isoRoot.size() + 1) {
		// Extract the subdirectory component
		Common::String subdir = isoVirtPath.substr(isoRoot.size() + 1);
		// Strip trailing slashes
		while (!subdir.empty() && (subdir.lastChar() == '/' || subdir.lastChar() == '\\'))
			subdir.deleteLastChar();
		if (!subdir.empty())
			ConfMan.set("iso_subdir", subdir, domain);
	}
}
