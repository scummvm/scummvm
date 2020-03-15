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

#ifndef CRYOMNI3D_VERSAILLES_DOCUMENTATION_H
#define CRYOMNI3D_VERSAILLES_DOCUMENTATION_H

#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "graphics/managed_surface.h"

namespace CryOmni3D {
class FontManager;
class MouseBoxes;
class Sprites;

class CryOmni3DEngine;

namespace Versailles {
class Versailles_Documentation {
public:
	Versailles_Documentation() : _engine(nullptr), _fontManager(nullptr), _sprites(nullptr),
		_messages(nullptr), _linksData(nullptr), _linksSize(0),
		_currentInTimeline(false), _currentMapLayout(false), _currentHasMap(false) { }
	~Versailles_Documentation() { delete [] _linksData; }

	void init(const Sprites *sprites, FontManager *fontManager, const Common::StringArray *messages,
	          CryOmni3DEngine *engine, const Common::String &allDocsFileName,
	          const Common::String &linksDocsFileName);
	void handleDocArea();
	void handleDocInGame(const Common::String &record);

private:
	Common::String docAreaHandleSummary();
	Common::String docAreaHandleTimeline();
	Common::String docAreaHandleGeneralMap();
	Common::String docAreaHandleCastleMap();
	uint docAreaHandleRecords(const Common::String &record);

	void docAreaPrepareNavigation();
	void docAreaPrepareRecord(Graphics::ManagedSurface &surface, MouseBoxes &boxes);
	uint docAreaHandleRecord(Graphics::ManagedSurface &surface, MouseBoxes &boxes,
	                         Common::String &nextRecord);

	void inGamePrepareRecord(Graphics::ManagedSurface &surface, MouseBoxes &boxes);
	uint inGameHandleRecord(Graphics::ManagedSurface &surface, MouseBoxes &boxes,
	                        Common::String &nextRecord);

	void setupRecordBoxes(bool inDocArea, MouseBoxes &boxes);
	void setupTimelineBoxes(MouseBoxes &boxes);
	void drawRecordData(Graphics::ManagedSurface &surface,
	                    const Common::String &text, const Common::String &title,
	                    const Common::String &subtitle, const Common::String &caption);
	void drawRecordBoxes(Graphics::ManagedSurface &surface, bool inDocArea, MouseBoxes &boxes);

	uint handlePopupMenu(const Graphics::ManagedSurface &surface,
	                     const Common::Point &anchor, bool rightAligned, uint itemHeight,
	                     const Common::StringArray &items);

	struct RecordInfo {
		uint id;
		uint position;
		uint size;
	};

	struct LinkInfo {
		Common::String record;
		Common::String title;
	};

	struct TimelineEntry {
		char year[8];
		uint x;
		uint y;
	};
	static const TimelineEntry kTimelineEntries[];

	char *getDocPartAddress(char *start, char *end, const char *patterns[]);
	const char *getDocTextAddress(char *start, char *end);
	const char *getRecordTitle(char *start, char *end);
	const char *getRecordSubtitle(char *start, char *end);
	const char *getRecordCaption(char *start, char *end);
	void getRecordHyperlinks(char *start, char *end, Common::StringArray &hyperlinks);

	Common::String getRecordTitle(const Common::String &record);
	Common::String getRecordData(const Common::String &record, Common::String &title,
	                             Common::String &subtitle, Common::String &caption,
	                             Common::StringArray &hyperlinks);
	void convertHyperlinks(const Common::StringArray &hyperlinks, Common::Array<LinkInfo> &links);

	void loadLinksFile();
	void getLinks(const Common::String &record, Common::Array<LinkInfo> &links);

	Common::String _allDocsFileName;
	Common::String _linksDocsFileName;

	static const uint kPopupMenuMargin = 5;

	CryOmni3DEngine *_engine;
	FontManager *_fontManager;
	const Sprites *_sprites;
	const Common::StringArray *_messages;

	bool _multilineAttributes;

	Common::StringArray _recordsOrdered;
	Common::HashMap<Common::String, RecordInfo> _records;
	char *_linksData;
	uint _linksSize;

	Common::Array<LinkInfo> _allLinks;

	Common::StringArray _visitTrace;
	Common::String _currentRecord;
	Common::String _categoryStartRecord;
	Common::String _categoryEndRecord;
	Common::String _categoryTitle;
	Common::Array<LinkInfo> _currentLinks;
	bool _currentInTimeline;
	bool _currentMapLayout;
	bool _currentHasMap;
};

} // End of namespace Versailles
} // End of namespace CryOmni3D

#endif
