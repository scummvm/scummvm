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

#ifndef MTROPOLIS_SUBTITLES_H
#define MTROPOLIS_SUBTITLES_H

#include "common/array.h"
#include "common/error.h"
#include "common/str.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"

namespace Graphics {

class Font;
class ManagedSurface;

} // End of namespace Graphics

namespace MTropolis {

class Runtime;
class Window;

class SubtitleAssetMappingTable {
public:
	SubtitleAssetMappingTable();

	Common::ErrorCode load(const Common::String &filePath);

	const Common::String *findSubtitleSetForAssetID(uint32 assetID) const;
	const Common::String *findSubtitleSetForAssetName(const Common::String &assetName) const;

private:
	Common::HashMap<uint32, Common::String> _assetIDToSubtitleSet;
	Common::HashMap<Common::String, Common::String> _assetNameToSubtitleSet;
};

class SubtitleModifierMappingTable {
public:
	SubtitleModifierMappingTable();

	Common::ErrorCode load(const Common::String &filePath);

	const Common::String *findSubtitleSetForModifierGUID(uint32 guid) const;

private:
	Common::HashMap<uint32, Common::String> _guidToSubtitleSet;
};

class SubtitleSpeakerTable {
public:
	SubtitleSpeakerTable();

	Common::ErrorCode load(const Common::String &filePath);

	const Common::Array<Common::String> &getSpeakers() const;
	uint getSpeakerID(const Common::String &speakerName) const;

private:
	Common::Array<Common::String> _speakers;
	Common::HashMap<Common::String, uint> _speakerToID;
};

class SubtitleLineTable {
public:
	SubtitleLineTable();

	Common::ErrorCode load(const Common::String &filePath, const SubtitleSpeakerTable &speakerTable);

	struct LineRange {
		LineRange();

		uint linesStart;
		uint numLines;
	};

	enum LineClass {
		kLineClassDefault,
		kLineClassGameplay,
	};

	struct LineData {
		LineData();

		uint32 timeOffsetMSec;
		uint slot;
		uint durationMSec;
		Common::String textUTF8;
		uint speakerID;
		LineClass lineClass;
		double position;
	};

	const Common::Array<LineData> &getAllLines() const;
	const LineRange *getLinesForSubtitleSetID(const Common::String &subtitleSetID) const;

private:
	Common::Array<LineData> _lines;
	Common::HashMap<Common::String, LineRange> _lineRanges;
};


struct SubtitleTables {
	Common::SharedPtr<SubtitleAssetMappingTable> assetMapping;
	Common::SharedPtr<SubtitleModifierMappingTable> modifierMapping;
	Common::SharedPtr<SubtitleSpeakerTable> speakers;
	Common::SharedPtr<SubtitleLineTable> lines;
};

class SubtitleDisplayItem {
public:
	SubtitleDisplayItem(const Common::String &text, const Common::String &speaker, uint slot, double position);

	const Common::U32String &getText() const;
	const Common::U32String &getSpeaker() const;
	uint getSlot() const;
	double getPosition() const;

private:
	Common::U32String _text;
	Common::U32String _speaker;
	uint _slot;
	double _position;
};

class SubtitleRenderer {
public:
	explicit SubtitleRenderer(bool enableGameplaySubtitles);

	void addDisplayItem(const Common::SharedPtr<SubtitleDisplayItem> &item, uint duration);
	void removeDisplayItem(const SubtitleDisplayItem *item, bool immediately);

	bool update(uint64 currentTime);	// Updates the subtitle player, returns true if any changes occurred since last update

	void composite(Window &window) const;
	bool isDirty() const;

	bool isGameplaySubtitlesEnabled() const;

private:
	struct DisplayItem {
		DisplayItem();

		Common::SharedPtr<SubtitleDisplayItem> item;
		Common::SharedPtr<Graphics::ManagedSurface> surface;
		uint64 expireTime;
	};

	void render();
	static void splitLines(const Common::U32String &str, Common::Array<Common::U32String> &outLines);

	Common::Array<DisplayItem> _displayItems;

	Common::SharedPtr<Graphics::Font> _font;
	uint64 _lastTime;
	uint _nonImmediateDisappearTime;
	int _fontHeight;
	bool _isDirty;
	bool _enableGameplaySubtitles;
};

class SubtitlePlayer {
public:
	SubtitlePlayer(Runtime *runtime, const Common::String &subtitleSetID, const SubtitleTables &tables);
	~SubtitlePlayer();

	void update(uint64 prevTime, uint64 newTime);
	void stop();

private:
	void triggerSubtitleLine(const SubtitleLineTable::LineData &line);

	Common::Array<Common::SharedPtr<SubtitleDisplayItem> > _items;

	Common::SharedPtr<SubtitleSpeakerTable> _speakers;
	Common::SharedPtr<SubtitleLineTable> _lines;

	SubtitleLineTable::LineRange _lineRange;
	Runtime *_runtime;
};

} // End of namespace MTropolis

#endif
