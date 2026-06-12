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

#ifndef MOHAWK_LIVINGBOOKS_ITEM_H
#define MOHAWK_LIVINGBOOKS_ITEM_H

#include "livingbooks_code.h"

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
	class SeekableReadStreamEndian;
	class MemoryReadStreamEndian;
}

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBPage;
struct LBScriptEntry;

class LBItem {
	friend class LBCode;

public:
	LBItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect);
	virtual ~LBItem();

	void readFrom(Common::SeekableReadStreamEndian *stream);
	void readData(uint16 type, uint16 size, byte *data);
	virtual void readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream);

	virtual void destroySelf(); // 0x2
	virtual void setEnabled(bool enabled); // 0x3
	virtual void setGlobalEnabled(bool enabled);
	virtual bool contains(Common::Point point); // 0x7
	virtual void update(); // 0x8
	virtual void draw() { } // 0x9
	virtual void handleKeyChar(Common::Point pos) { } // 0xA
	virtual void handleMouseDown(Common::Point pos); // 0xB
	virtual void handleMouseMove(Common::Point pos); // 0xC
	virtual void handleMouseUp(Common::Point pos); // 0xD
	virtual bool togglePlaying(bool playing, bool restart = false); // 0xF
	virtual void done(bool onlyNotify); // 0x10
	virtual void init(); // 0x11
	virtual void seek(uint16 pos) { } // 0x13
	virtual void seekToTime(uint32 time) { }
	virtual void setFocused(bool focused) { } // 0x14
	virtual void setVisible(bool visible); // 0x17
	virtual void setGlobalVisible(bool enabled);
	virtual void startPhase(uint phase); // 0x18
	virtual void stop(); // 0x19
	virtual void notify(uint16 data, uint16 from); // 0x1A
	virtual void load();
	virtual void unload();
	virtual void moveBy(const Common::Point &pos);
	virtual void moveTo(const Common::Point &pos);

	LBItem *clone(uint16 newId, const Common::String &newName);

	uint16 getId() { return _itemId; }
	const Common::String &getName() { return _desc; }
	const Common::Rect &getRect() { return _rect; }
	uint16 getSoundPriority() { return _soundMode; }
	bool isLoaded() { return _loaded; }
	bool isAmbient() { return _isAmbient; }

	Common::List<LBItem *>::iterator _iterator;

	// TODO: make private
	Common::HashMap<Common::String, LBValue, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;

protected:
	MohawkEngine_LivingBooks *_vm;
	LBPage *_page;

	void setNextTime(uint16 min, uint16 max);
	void setNextTime(uint16 min, uint16 max, uint32 start);

	Common::Rect _rect;
	Common::String _desc;
	uint16 _resourceId;
	uint16 _itemId;

	bool _loaded, _visible, _globalVisible, _playing, _enabled, _globalEnabled;

	uint32 _nextTime, _startTime;
	uint16 _loops;

	uint16 _phase, _timingMode, _delayMin, _delayMax;
	uint16 _loopMode, _periodMin, _periodMax;
	uint16 _controlMode, _soundMode;
	Common::Point _relocPoint;

	bool _isAmbient;
	bool _doHitTest;

	virtual LBItem *createClone();

	Common::Array<LBScriptEntry *> _scriptEntries;
	void runScript(uint event, uint16 data = 0, uint16 from = 0);
	int runScriptEntry(LBScriptEntry *entry);

	void runCommand(const Common::String &command);
	bool checkCondition(const Common::String &condition);

	LBScriptEntry *parseScriptEntry(uint16 type, uint16 &size, Common::MemoryReadStreamEndian *stream, bool isSubentry = false);
};

} // End of namespace Mohawk

#endif
