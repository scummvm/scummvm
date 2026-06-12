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

#ifndef MOHAWK_LIVINGBOOKS_ITEMTYPES_H
#define MOHAWK_LIVINGBOOKS_ITEMTYPES_H

#include "mohawk/livingbooks_item.h"

#include "common/array.h"
#include "common/rect.h"

namespace Common {
	class MemoryReadStreamEndian;
}

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBPage;

class LBSoundItem : public LBItem {
public:
	LBSoundItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBSoundItem() override;

	void update() override;
	bool togglePlaying(bool playing, bool restart) override;
	void stop() override;

protected:
	LBItem *createClone() override;

	bool _running;
};

struct GroupEntry {
	uint entryId;
	uint entryType;
};

class LBGroupItem : public LBItem {
public:
	LBGroupItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) override;

	void destroySelf() override;
	void setEnabled(bool enabled) override;
	void setGlobalEnabled(bool enabled) override;
	bool contains(Common::Point point) override;
	bool togglePlaying(bool playing, bool restart) override;
	// 0x12
	void seek(uint16 pos) override;
	void setVisible(bool visible) override;
	void setGlobalVisible(bool visible) override;
	void startPhase(uint phase) override;
	void stop() override;
	void load() override;
	void unload() override;
	void moveBy(const Common::Point &pos) override;
	void moveTo(const Common::Point &pos) override;

protected:
	LBItem *createClone() override;

	bool _starting;

	Common::Array<GroupEntry> _groupEntries;
};

class LBPaletteItem : public LBItem {
public:
	LBPaletteItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBPaletteItem() override;

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) override;

	bool togglePlaying(bool playing, bool restart) override;
	void update() override;

protected:
	LBItem *createClone() override;

	uint16 _fadeInPeriod, _fadeInStep, _drawStart, _drawCount;
	uint32 _fadeInStart, _fadeInCurrent;
	byte *_palette;
};

struct LiveTextWord {
	Common::Rect bounds;
	uint16 soundId;

	uint16 itemType;
	uint16 itemId;
};

struct LiveTextPhrase {
	uint16 wordStart, wordCount;
	uint16 highlightStart, highlightEnd;
	uint16 startId, endId;
};

class LBLiveTextItem : public LBItem {
public:
	LBLiveTextItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) override;

	bool contains(Common::Point point) override;
	void update() override;
	void draw() override;
	void handleMouseDown(Common::Point pos) override;
	bool togglePlaying(bool playing, bool restart) override;
	void stop() override;
	void notify(uint16 data, uint16 from) override;

protected:
	LBItem *createClone() override;

	void paletteUpdate(uint16 word, bool on);
	void drawWord(uint word, uint yPos);

	uint16 _currentPhrase, _currentWord;

	byte _backgroundColor[4];
	byte _foregroundColor[4];
	byte _highlightColor[4];
	uint16 _paletteIndex;

	Common::Array<LiveTextWord> _words;
	Common::Array<LiveTextPhrase> _phrases;
};

class LBPictureItem : public LBItem {
public:
	LBPictureItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);

	void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) override;

	bool contains(Common::Point point) override;
	void draw() override;
	void init() override;

protected:
	LBItem *createClone() override;
};

class LBMovieItem : public LBItem {
public:
	LBMovieItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBMovieItem() override;

	void update() override;
	bool togglePlaying(bool playing, bool restart) override;

protected:
	LBItem *createClone() override;
};

class LBMiniGameItem : public LBItem {
public:
	LBMiniGameItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBMiniGameItem() override;

	bool togglePlaying(bool playing, bool restart) override;

protected:
	LBItem *createClone() override;
};

class LBProxyItem : public LBItem {
public:
	LBProxyItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBProxyItem() override;

	void load() override;
	void unload() override;

protected:
	LBItem *createClone() override;

	class LBPage *_page;
};

} // End of namespace Mohawk

#endif
