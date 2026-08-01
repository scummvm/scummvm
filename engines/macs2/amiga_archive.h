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

#ifndef MACS2_AMIGA_ARCHIVE_H
#define MACS2_AMIGA_ARCHIVE_H

#include "common/archive.h"
#include "common/array.h"
#include "common/file.h"
#include "common/hashmap.h"
#include "common/mutex.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"

namespace Macs2 {

/**
 * Resource type tags used in Amiga Mdir / DataA entry headers.
 * Stored as big-endian ASCII pairs: "OO", "MM", "OS", "FF".
 */
enum AmigaResourceType : uint16 {
	kAmigaResOO = 0x4F4F, // object / graphics
	kAmigaResMM = 0x4D4D, // scene package (MXMM: planar BG + tables; not Protracker)
	kAmigaResOS = 0x4F53, // sound effect
	kAmigaResFF = 0x4646, // font
	kAmigaResUnknown = 0
};

struct AmigaDirEntry {
	AmigaResourceType type = kAmigaResUnknown;
	uint16 id = 0;
	uint16 disk = 0;
	uint32 offset = 0; // absolute offset of the 8-byte entry header in Data*
};

struct AmigaInfoData {
	uint16 cursorResourceIds[8] = {0}; // Talk..Disabled style slots from Info
	uint16 useInventoryCursorId = 0;
	uint16 sceneCount = 0;
	uint16 volumeCount = 0;
	uint16 mdirSize = 0;
	/** 16x 12-bit 0x0RGB UI colors from MXIN (Ghidra DAT_002379dc). */
	uint16 uiPaletteAmiga[16] = {0};
	/**
	 * Panel border line/corner color indices from MXIN (Ghidra g_awPanelBorderColorIndices).
	 * Demo: 18,19,20,21,22.
	 */
	uint16 panelBorderColorIndices[5] = {0};
	/**
	 * Panel darken luminance->color table (Ghidra g_awPanelDarkenColorIndices).
	 * Demo: 28,29,29,60,61,31,62,23. Indexed by (7 - (R4+G4+B4)/0x18).
	 */
	uint16 panelDarkenColorIndices[8] = {0};
	/**
	 * After title screen, g_bUsePanelPatternFill is overwritten from PREF MENUMODE
	 * (initAndRunTitleScreen @ 00234b42): 0=SHADE (darken remap), 1=GFX / 2=SOLID /
	 * 3=POINTS -> fill_ui_panel_pattern_tiles @ 00222332 (opaque 16x16 tiles).
	 * Demo PREF defaults to MENUMODE 2 (SOLID). Info MXIN +0x50 is only the
	 * pre-title default (0).
	 */
	uint8 prefMenuMode = 2;
	bool usePanelPatternFill = true;
	/**
	 * Starting MM resource id from MXIN (u32BE at Info offset 8).
	 * Demo Info stores 40 -> MM_0040 (intro). Script-visible scene id is mmId+1.
	 */
	uint16 startSceneResourceId = 0;
	bool loaded = false;
};

/**
 * Amiga MACS2 container: Mdir (MXDR) index + DataA/DataB/... (MXMF) volumes.
 *
 * Members are exposed as Common::Archive paths:
 *   "scene_table"          - first PP20 block (decompressed MXOO)
 *   "OO_0104", "MM_0004".. - typed resources (PP20 decompressed; MXMM raw)
 *   "Info"                 - raw MXIN game metadata (if present on disk)
 *
 * Based on FORMAT.md / extract_macs2 Amiga mode in scummvm-tools.
 */
class Macs2AmigaArchive : public Common::Archive {
public:
	Macs2AmigaArchive();
	~Macs2AmigaArchive() override;

	/**
	 * Open Mdir + DataA (and additional DataB.. volumes referenced by Mdir).
	 * Looks up files via SearchMan / the game directory.
	 */
	bool open();
	void close();
	bool isOpen() const { return !_entries.empty(); }

	uint16 getSceneCount() const { return _sceneCount; }
	uint16 getResourceCount() const { return _totalResources; }
	const AmigaInfoData &getInfo() const { return _info; }

	/** Decompressed resource stream, or nullptr if missing. Caller owns the stream. */
	Common::SeekableReadStream *createReadStreamForResource(AmigaResourceType type, uint16 id) const;

	/** Decompressed first MXMF PP20 block (scene/base table). */
	Common::SeekableReadStream *createSceneTableStream() const;

	bool hasResource(AmigaResourceType type, uint16 id) const;

	// Common::Archive
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	static Common::String makeResourceName(AmigaResourceType type, uint16 id);
	static bool parseResourceName(const Common::String &name, AmigaResourceType &type, uint16 &id);
	static AmigaResourceType typeFromTag(uint16 tag);
	static const char *typeToString(AmigaResourceType type);

	/**
	 * Decode an Amiga planar MXOO sprite body into chunky 8bpp pixels.
	 * Returns false if the resource is not a simple sprite.
	 */
	static bool decodePlanarSprite(const byte *mxoo, uint32 mxooSize, uint16 &width, uint16 &height, Common::Array<byte> &pixels);

private:
	struct Volume {
		Common::Path path;
		Common::ScopedPtr<Common::File> file;
		uint32 size = 0;
	};

	struct Entry {
		AmigaDirEntry dir;
		uint32 payloadSize = 0; // size field from the 8-byte Data* header
		Common::String name;
	};

	typedef Common::HashMap<Common::Path, Entry, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> EntryMap;

	bool openVolume(uint16 disk);
	bool loadMdir();
	bool loadInfo();
	/** Parse PREF MENUMODE into _info (Ghidra parsePrefResourceSettings -> title exit). */
	bool loadPrefMenuMode();
	Common::SeekableReadStream *readAndDecompressEntry(const Entry &entry) const;
	static byte *decompressPP20(const byte *src, uint32 srcLen, uint32 &outLen);
	static byte *decompressPayload(const byte *payload, uint32 payloadLen, uint32 &outLen);

	EntryMap _entries;
	Common::Array<Volume> _volumes;
	mutable Common::Mutex _mutex;

	uint16 _sceneCount = 0;
	uint16 _totalResources = 0;
	uint32 _firstBlockSize = 0;
	uint32 _firstBlockOffset = 14;
	AmigaInfoData _info;
};

} // End of namespace Macs2

#endif // MACS2_AMIGA_ARCHIVE_H
