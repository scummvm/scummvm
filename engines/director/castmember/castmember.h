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

#include "common/rect.h"

#include "director/archive.h"
#include "director/stxt.h"

#include "director/lingo/lingo-object.h"

namespace Graphics {
class MacWidget;
}

namespace Common {
class SeekableReadStream;
class SeekableReadStreamEndian;
class MemoryWriteStream;
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

	virtual CastMember *duplicate(Cast *cast, uint16 castId);

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
	virtual void updateFromWidget(Graphics::MacWidget *widget, bool spriteEditable) {}
	virtual Common::Rect getInitialRect() { return _initialRect; }

	virtual void setColors(uint32 *fgcolor, uint32 *bgcolor) { return; }
	virtual uint32 getForeColor() { return 0; }
	virtual void setForeColor(uint32 fgCol) { return; }
	virtual uint32 getBackColor() { return 0; }
	virtual void setBackColor(uint32 bgCol) { return; }

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;
	bool setProp(const Common::String &propName, const Datum &value, bool force = false) override;
	bool hasField(int field) override;
	Datum getField(int field) override;
	bool setField(int field, const Datum &value) override;

	// release the control to widget, this happens when we are changing sprites. Because we are having the new cast member and the old one shall leave
	void releaseWidget() { _widget = nullptr; }

	virtual Common::String formatInfo() { return Common::String(); };

	// Return the default bounding box of the cast member. The origin is at the registration offset.
	virtual Common::Rect getBbox();
	// Return the bounding box of the cast member, assuming a stretched width and height value.
	// The origin is at the registration offset.
	virtual Common::Rect getBbox(int16 currentWidth, int16 currentHeight);
	// Return the default registration offset. Offset is relative to the top-left corner of the widget.
	virtual Common::Point getRegistrationOffset() { return Common::Point(0, 0); }
	// Return the registration offset, assuming a stretched width and height value.
	// Offset is relative to the top-left corner of the widget.
	virtual Common::Point getRegistrationOffset(int16 currentWidth, int16 currentHeight) { return Common::Point(0, 0); }

	virtual CollisionTest isWithin(const Common::Rect &bbox, const Common::Point &pos, InkType ink) { return bbox.contains(pos) ? kCollisionYes : kCollisionNo; }

	// When writing the 'CASt' resource, the general structure is the same for all the CastMembers
	// Three parts to a 'CASt' resource (header + _info_, _data_)
	// The headers, are common, the _info_ writing is handled by the Cast class, so no worries there
	// So, the only thing that differs is the _data_, for which we'll have separate implementations for each CastMember
	uint32 writeCAStResource(Common::MemoryWriteStream *writeStream, uint32 offset);
	uint32 getCastInfoSize();
	uint32 getCastResourceSize();
	virtual void writeCastData(Common::MemoryWriteStream *writeStream);
	virtual uint32 getCastDataSize();

	CastType _type;
	Common::Rect _initialRect;
	Common::Rect _boundingRect;
	Common::Array<Resource> _children;

	bool _hilite;
	bool _erase;
	int _purgePriority;
	uint32 _size;

	/* Data fields used when saving the Cast Member */
	uint32 _castDataSize;
	uint8 _flags1;

protected:
	Cast *_cast;
	uint16 _castId;
	// a link to the widget we created, we may use it later
	Graphics::MacWidget *_widget;
	bool _loaded;
	bool _modified;
	bool _isChanged;
	bool _needsReload;
};

struct EditInfo {
	Common::Rect rect;
	int32 selStart;
	int32 selEnd;
	byte version;
	byte rulerFlag;
	bool valid;

	EditInfo(): valid(false) {}
};

struct CastMemberInfo {
	uint32 unk1;
	uint32 unk2;
	uint32 flags;
	uint16 count;
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

	// There just has to be a better solution
	// It is not rare to find these strings in the CastMemberInfo
	Common::Array<byte> unknown1;
	Common::Array<byte> unknown2;
	Common::Array<byte> unknown3;
	Common::Array<byte> unknown4;
	Common::Array<byte> unknown5;
	Common::Array<byte> unknown6;
	Common::Array<byte> unknown7;

	CastMemberInfo() : autoHilite(false), scriptId(0) {}
};

} // End of namespace Director

#endif
