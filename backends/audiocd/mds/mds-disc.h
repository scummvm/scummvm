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

/**
 * @file mds-disc.h
 * Parser for Alcohol 120% MDS/MDF disc image pairs.
 *
 * MDS (Media Descriptor file) stores the track layout; MDF holds the raw
 * sector data.  MDSDisc parses the MDS and opens streams into the MDF for
 * individual tracks.
 *
 * Usage:
 * @code
 *   MDSDisc disc;
 *   if (disc.open(Common::Path("game.mds"))) {
 *       // Get ISO 9660 data stream (Mode 1 track, 2048-byte sectors)
 *       Common::SeekableReadStream *data = disc.openDataTrackStream();
 *       auto fs = Common::makeSharedPtr(new Common::ISO9660FileSystem(data));
 *
 *       // Get raw PCM audio for track 2
 *       Common::SeekableReadStream *audio = disc.openAudioTrackStream(2);
 *   }
 * @endcode
 */

#ifndef BACKENDS_AUDIOCD_MDS_MDS_DISC_H
#define BACKENDS_AUDIOCD_MDS_MDS_DISC_H

#include "audio/audiostream.h"
#include "common/archive.h"
#include "common/array.h"
#include "common/fs.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

/**
 * MDS/MDF disc image parser.
 *
 * Reads an Alcohol 120% MDS descriptor and provides streams into the
 * companion MDF raw-sector file.
 */
class MDSDisc {
public:
	/** Track mode (data encoding). */
	enum TrackMode {
		kModeAudio    = 0x00, ///< Red Book audio (2352 bytes/sector)
		kModeMode1    = 0x01, ///< Mode 1 data, 2048-byte user data sectors
		kModeMode1Raw = 0x02, ///< Mode 1 raw, 2352-byte sectors (sync header present)
		kModeMode2    = 0x03, ///< Mode 2 Form 1 raw
		kModeUnknown  = 0xFF
	};

	/** Metadata for a single track on the disc. */
	struct TrackInfo {
		int      number;        ///< 1-based track number
		TrackMode mode;         ///< Track encoding mode
		uint16   sectorSize;    ///< Raw bytes per sector in the MDF
		uint32   startLba;      ///< Starting LBA of the track (75 frames/sec)
		uint64   mdfOffset;     ///< Byte offset into the MDF file
		uint32   sectorCount;   ///< Number of sectors in the track
		uint32   pregapSectors; ///< Number of pregap sectors to skip
	};

	MDSDisc();

	/**
	 * Open and parse an MDS file.
	 * @param mdsPath  Path to the .mds descriptor file.
	 * @return True if parsing succeeded.
	 */
	bool open(const Common::Path &mdsPath);

	/**
	 * Open and parse MDS data from an in-memory stream.
	 *
	 * This overload reads the MDS descriptor from @p stream instead of
	 * from a file on disk.  The companion MDF path is not set, so
	 * openDataTrackStream() and openAudioTrackStream() will not work;
	 * use this for metadata queries (getTrackCount(), getTrack()).
	 *
	 * @param stream  Seekable stream containing the MDS descriptor.
	 *                The caller retains ownership.
	 * @return True if parsing succeeded.
	 */
	bool open(Common::SeekableReadStream *stream);

	/** Close and release all resources. */
	void close();

	/** Returns true if the MDS was opened and parsed successfully. */
	bool isOpen() const { return _open; }

	/** Number of tracks on the disc. */
	int getTrackCount() const { return static_cast<int>(_tracks.size()); }

	/**
	 * Get metadata for a track.
	 * @param trackNo  1-based track number.
	 */
	TrackInfo getTrack(int trackNo) const;

	/**
	 * Open a seekable stream for the data track (Mode 1).
	 *
	 * The returned stream presents 2048-byte logical sectors regardless of
	 * whether the MDF stores raw 2352-byte sectors; the 16-byte sync header
	 * and 288-byte ECC/EDC are stripped automatically.
	 *
	 * @return A new SeekableReadStream for the data track, or @c nullptr
	 *         if no data track is found or on error. The caller takes
	 *         ownership of the returned stream.
	 */
	Common::SeekableReadStream *openDataTrackStream() const;

	/**
	 * Open a seekable raw-PCM stream for an audio track.
	 *
	 * The returned stream contains interleaved 16-bit signed stereo PCM at
	 * 44100 Hz (Red Book CD audio), reading directly from the MDF.
	 *
	 * @param trackNo  1-based track number (must be an audio track).
	 * @return A new SeekableReadStream, or @c nullptr on error or if
	 *         trackNo is not an audio track. The caller takes ownership.
	 */
	Common::SeekableReadStream *openAudioTrackStream(int trackNo) const;

	/**
	 * Open a decoded audio stream for an audio track, ready for playback.
	 *
	 * @param trackNo  1-based track number (must be an audio track).
	 * @return A new SeekableAudioStream, or @c nullptr on error or if
	 *         trackNo is not an audio track. The caller takes ownership.
	 */
	Audio::SeekableAudioStream *openAudioStream(int trackNo) const;

private:
	Common::SeekableReadStream *openMDF() const;
	const TrackInfo *findDataTrack() const;
	Common::SeekableReadStream *createTrackStream(
		Common::SeekableReadStream *mdf, uint64 baseOffset,
		const struct SectorLayout &layout) const;
	bool readSessionBlocks(const byte *buf, uint32 bufSize,
	                       uint32 sessionOffset, uint16 numSessions);
	bool parseMDSStream(Common::SeekableReadStream *stream, const char *context);

	Common::String              _mdfPath;   ///< Path to the .mdf data file
	Common::Array<TrackInfo>    _tracks;    ///< Parsed track list (sorted by track number)
	bool                        _open;
};

/**
 * Open a data track stream from a disc image file (.mds, .mdf, or .iso).
 *
 * For .mds/.mdf files, parses the MDS descriptor and extracts the data track
 * from the companion .mdf.  For .iso files, opens the file directly.
 * A .mdf path is automatically redirected to its companion .mds.
 *
 * @param imagePath  Path to the .mds, .mdf, or .iso file.
 * @return A seekable stream for the data track, or nullptr on failure.
 *         The caller takes ownership.
 */
Common::SeekableReadStream *openDiscImageDataStream(const Common::Path &imagePath);

/**
 * Returns true if @p name has a disc image file extension (.iso, .mds, or .mdf).
 */
bool isDiscImageFile(const Common::String &name);

/**
 * Open a disc image as a virtual ISO 9660 filesystem node.
 *
 * Chains openDiscImageDataStream() → ISO9660FileSystem → makeISOFSNodeRoot().
 *
 * @param imagePath  Path to the .mds, .mdf, or .iso file.
 * @return An FSNode wrapping the ISO root directory, or an invalid node on failure.
 */
Common::FSNode openDiscImageFSNode(const Common::Path &imagePath);

/**
 * If @p node is a disc image file (.iso/.mds/.mdf), open it as a virtual
 * ISO filesystem and return an FSNode for the ISO root directory.
 * Otherwise return @p node unchanged.
 *
 * @param node          The FSNode to resolve (may be a directory or disc image).
 * @param isoImagePath  If non-null, receives the path of the disc image file,
 *                      or remains unchanged if @p node was a plain directory.
 */
Common::FSNode resolveDiscImageNode(const Common::FSNode &node,
                                     Common::String *isoImagePath = nullptr);

/**
 * Save disc-image-specific config keys (iso_path, iso_subdir) for a game
 * that was detected inside a disc image.
 *
 * Sets "path" to the real parent directory of the image, "iso_path" to the
 * image file itself, and "iso_subdir" to any subdirectory within the ISO
 * where the game data lives.
 *
 * @param isoImagePath  Path to the disc image file (as returned by
 *                      resolveDiscImageNode()).
 * @param resolvedDir   The FSNode used for detection (virtual ISO directory).
 * @param domain        The config domain for the game.
 */
void saveDiscImageConfig(const Common::String &isoImagePath,
                         const Common::FSNode &resolvedDir,
                         const Common::String &domain);

/**
 * Open a disc image as a Common::Archive for use with SearchMan.
 *
 * Chains openDiscImageDataStream() → ISO9660FileSystem → ISO9660Archive.
 *
 * @param imagePath  Path to the .mds, .mdf, or .iso file.
 * @param subdir     Optional subdirectory within the ISO to use as root.
 * @return An owned Archive pointer, or nullptr on failure.
 */
Common::Archive *openDiscImageArchive(const Common::Path &imagePath,
                                       const Common::String &subdir = Common::String());

/**
 * If the active config has an "iso_path" key, open the disc image as a
 * virtual ISO 9660 filesystem and replace @p dir with the ISO root node.
 */
void resolveDiscImageDir(Common::FSNode &dir);

/**
 * If the active config has an "iso_path" key, mount the disc image into
 * SearchMan so that game engines can read files from inside the image.
 */
void mountDiscImageInSearchMan();

#endif // BACKENDS_AUDIOCD_MDS_MDS_DISC_H
