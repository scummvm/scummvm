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

#ifndef MOHAWK_LIVINGBOOKS_H
#define MOHAWK_LIVINGBOOKS_H

#include "mohawk/mohawk.h"
#include "mohawk/console.h"
#include "mohawk/livingbooks_graphics.h"
#include "mohawk/livingbooks_constants.h"
#include "mohawk/sound.h"
#include "mohawk/video.h"

#include "common/formats/ini-file.h"
#include "common/rect.h"
#include "common/queue.h"
#include "common/random.h"

#include "livingbooks_code.h"

#include "mohawk/livingbooks_itemscript.h"
#include "mohawk/livingbooks_item.h"
#include "mohawk/livingbooks_itemtypes.h"
#include "mohawk/livingbooks_animation.h"
#include "mohawk/livingbooks_page.h"

namespace Common {
	class SeekableReadStreamEndian;
	class MemoryReadStreamEndian;
}

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBPage;
class LBGraphics;
class LBAnimation;

bool tryOpenPage(Archive *archive, const Common::String &fileName);

struct NotifyEvent {
	NotifyEvent(uint t, uint p) : type(t), param(p), newUnknown(0), newMode(0), newPage(0), newSubpage(0) { }
	uint type;
	uint param;

	// kLBNotifyChangeMode
	uint16 newUnknown;
	uint16 newMode;
	uint16 newPage;
	uint16 newSubpage;
	Common::String newCursor;
};

enum DelayedEventType {
	kLBDelayedEventDestroy = 0,
	kLBDelayedEventSetNotVisible = 1,
	kLBDelayedEventDone = 2
};

struct DelayedEvent {
	DelayedEvent(LBItem *i, DelayedEventType t) : item(i), type(t) { }
	LBItem *item;
	DelayedEventType type;
};

class MohawkEngine_LivingBooks : public MohawkEngine {
protected:
	Common::Error run() override;

public:
	MohawkEngine_LivingBooks(OSystem *syst, const MohawkGameDescription *gamedesc);
	~MohawkEngine_LivingBooks() override;

	Common::RandomSource *_rnd;

	VideoManager *_video;
	Sound *_sound;
	LBGraphics *_gfx;
	bool _needsRedraw, _needsUpdate;

	void addNotifyEvent(NotifyEvent event);

	Common::SeekableReadStreamEndian *wrapStreamEndian(uint32 tag, uint16 id);
	Common::String readString(Common::ReadStream *stream);
	Common::Rect readRect(Common::ReadStreamEndian *stream);

	void addArchive(Archive *archive);
	void removeArchive(Archive *archive);
	void addItem(LBItem *item);
	void removeItems(const Common::Array<LBItem *> &items);

	LBItem *getItemById(uint16 id);
	LBItem *getItemByName(Common::String name);

	void setFocus(LBItem *focus);
	void setEnableForAll(bool enable, LBItem *except = 0);
	void notifyAll(uint16 data, uint16 from);
	void queueDelayedEvent(DelayedEvent event);

	bool playSound(LBItem *source, uint16 resourceId);
	void lockSound(LBItem *owner, bool lock);

	bool isBigEndian() const { return getGameType() != GType_LIVINGBOOKSV1 || getPlatform() == Common::kPlatformMacintosh; }
	bool isPreMohawk() const;

	LBMode getCurMode() { return _curMode; }

	bool tryLoadPageStart(LBMode mode, uint page);
	bool loadPage(LBMode mode, uint page, uint subpage);
	void prevPage();
	void nextPage();

	// TODO: make private
	Common::HashMap<Common::String, LBValue, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _variables;

	// helper functions, also used by LBProxyItem
	Common::String getFileNameFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover);
	Archive *createArchive() const;

private:
	Common::INIFile _bookInfoFile;

	Common::Path getBookInfoFileName() const;
	void loadBookInfo(const Common::Path &filename);

	Common::String stringForMode(LBMode mode);

	bool _readOnly, _introDone;
	LBMode _curMode;
	uint16 _curPage, _curSubPage;
	uint16 _phase;
	LBPage *_page;
	Common::Array<LBItem *> _items;
	Common::List<LBItem *> _orderedItems;
	Common::Queue<DelayedEvent> _eventQueue;
	LBItem *_focus;
	void destroyPage();
	void updatePage();

	uint16 _lastSoundOwner, _lastSoundId;
	uint16 _lastSoundPriority;
	uint16 _soundLockOwner;
	uint16 _maxSoundPriority;

	void loadSHP(uint16 resourceId);

	bool tryDefaultPage();

	void handleUIMenuClick(uint controlId);
	void handleUIPoetryMenuClick(uint controlId);
	void handleUIQuitClick(uint controlId);
	void handleUIOptionsClick(uint controlId);

	Common::Queue<NotifyEvent> _notifyEvents;
	void handleNotify(NotifyEvent &event);

	uint16 _screenWidth;
	uint16 _screenHeight;
	uint16 _numLanguages;
	uint16 _numPages;
	Common::String _title;
	Common::String _copyright;
	bool _poetryMode;

	uint16 _curLanguage;
	uint16 _curSelectedPage;
	bool _alreadyShowedIntro;

	// String Manipulation Functions
	Common::String removeQuotesFromString(const Common::String &string, Common::String &leftover);
	Common::String convertMacFileName(const Common::String &string);
	Common::String convertWinFileName(const Common::String &string);

	// Configuration File Functions
	Common::String getStringFromConfig(const Common::String &section, const Common::String &key);
	Common::String getStringFromConfig(const Common::String &section, const Common::String &key, Common::String &leftover);
	int getIntFromConfig(const Common::String &section, const Common::String &key);

	void pauseEngineIntern(bool) override;
};

} // End of namespace Mohawk

#endif
