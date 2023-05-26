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

#ifndef DIRECTOR_CASTMEMBER_CASTMEMBER_H
#define DIRECTOR_CASTMEMBER_CASTMEMBER_H

#include "graphics/font.h"

#include "director/archive.h"
#include "director/sprite.h"
#include "director/stxt.h"

#include "director/lingo/lingo-object.h"

namespace Graphics {
class MacWidget;
}

namespace Common {
class SeekableReadStream;
class SeekableReadStreamEndian;
}

namespace Director {

struct CastMemberInfo;
class Channel;
struct Resource;

class CastMember : public Object<CastMember> {
public:
	CastMember(Cast *cast, uint16 castId, Common::SeekableReadStreamEndian &stream);
	CastMember(Cast *cast, uint16 castId);
	virtual ~CastMember() {}

	Cast *getCast() { return _cast; }
	uint16 getID() { return _castId; }
	CastMemberInfo *getInfo();

	virtual void load();
	virtual void unload();
	bool isLoaded() { return _loaded; }

	virtual bool isEditable() { return false; }
	virtual void setEditable(bool editable) {}
	virtual bool isModified() { return _modified; }
	void setModified(bool modified);
	virtual Graphics::MacWidget *createWidget(Common::Rect &bbox, Channel *channel, SpriteType spriteType) { return nullptr; }
	virtual void updateWidget(Graphics::MacWidget *widget, Channel *channel) {}
	virtual void updateFromWidget(Graphics::MacWidget *widget) {}
	virtual Common::Rect getInitialRect() { return _initialRect; }

	virtual void setColors(uint32 *fgcolor, uint32 *bgcolor) { return; }
	virtual uint32 getForeColor() { return 0; }
	virtual void setForeColor(uint32 fgCol) { return; }
	virtual uint32 getBackColor() { return 0; }
	virtual void setBackColor(uint32 bgCol) { return; }

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	bool setProp(const Common::String &propName, const Datum &value) override;
	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	// release the control to widget, this happens when we are changing sprites. Because we are having the new cast member and the old one shall leave
	void releaseWidget() { _widget = nullptr; }

	virtual Common::String formatInfo() { return Common::String(); };

	virtual Common::Rect getBbox();
	virtual Common::Rect getBbox(int16 width, int16 height);
	virtual Common::Point getRegistrationOffset() { return Common::Point(0, 0); }
	virtual Common::Point getRegistrationOffset(int16 width, int16 height) { return Common::Point(0, 0); }

	CastType _type;
	Common::Rect _initialRect;
	Common::Rect _boundingRect;
	Common::Array<Resource> _children;

	bool _hilite;
	bool _erase;
	int _purgePriority;
	uint32 _size;
	uint8 _flags1;

protected:
	Cast *_cast;
	uint16 _castId;
	// a link to the widget we created, we may use it later
	Graphics::MacWidget *_widget;
	bool _loaded;
	bool _modified;
	bool _isChanged;
};

struct EditInfo {
	Common::Rect rect;
	int32 selStart;
	int32 selEnd;
	byte version;
	byte rulerFlag;
};

struct CastMemberInfo {
	bool autoHilite;
	uint32 scriptId;
	Common::String script;
	Common::String name;
	Common::String directory;
	Common::String fileName;
	Common::String type;
	EditInfo scriptEditInfo;
	FontStyle scriptStyle;
	EditInfo textEditInfo;
	Common::String modifiedBy;
	Common::String comments;

	CastMemberInfo() : autoHilite(false), scriptId(0) {}
};

} // End of namespace Director

#endif
