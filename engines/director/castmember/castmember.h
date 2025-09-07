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
	void setProp(const Common::String &propName, const Datum &value, bool force = false) override;
	bool hasField(int field) override;
	Datum getField(int field) override;
	void setField(int field, const Datum &value) override;

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
	uint32 writeCAStResource(Common::SeekableWriteStream *writeStream);
	uint32 getCastInfoSize();
	uint32 getCastResourceSize();
	virtual void writeCastData(Common::SeekableWriteStream *writeStream);
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
	// This index is the index that it appears in the original movie archive
	int16 _index;

protected:
	Cast *_cast;
	// This is the id of the cast member, this id is unique to only cast members
	// Basically the cast members are given ids starting from _castArrayStart to _castArrayEnd
	// e.g. 0, 1, 2, 3, 4
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
	void read(Common::ReadStreamEndian *stream);
	void write(Common::WriteStream *stream);

	Common::String toString() {
		return Common::String::format("rect: [%s] selStart: %d selEnd: %d version: %d rulerFlag: %d valid: %d",
			rect.toString().c_str(), selStart, selEnd, version, rulerFlag, valid);
	}
};

struct CastMemberInfo {
	// Header
	uint32 unk1;
	uint32 unk2;
	uint32 flags;
	uint16 count;
	bool autoHilite;
	uint32 scriptId;

	// List items
	Common::String script;     // 0       (removed on protecting)
	Common::String name;
	Common::String directory;
	Common::String fileName;
	Common::String fileType;	// 4     pre-D5
	Common::String propInit;	// 4     post-D5
	EditInfo scriptEditInfo;	// 5      (removed on protecting)
	FontStyle scriptStyle;		//        (removed on protecting)
	EditInfo textEditInfo;		//        (removed on protecting)
	EditInfo rteEditInfo;		//        (removed on protecting)
	byte xtraGuid[16];			// 9
	Common::String xtraDisplayName;
	Common::Array<byte> bpTable; //       (removed on protecting)
	Common::Rect32 xtraRect;		// Rect32
	Common::Rect scriptRect;	//        (removed on protecting)
	Common::Array<byte> dvWindowInfo; //  (removed on protecting)
	byte guid[16];				// 15   Seems to be a GUID
	Common::String mediaFormatName; // 16 Used by DV cast members to store the media format name
	uint32 creationTime;		//        (removed on protecting)
	uint32 modifiedTime;		//        (removed on protecting)
	Common::String modifiedBy;	//        (removed on protecting)
	Common::String comments;	// 20     (removed on protecting, but could be retained)
	uint32 imageQuality;		// 21

	CastMemberInfo() : autoHilite(false), scriptId(0) {
		memset(xtraGuid, 0, 16);
		memset(guid, 0, 16);
		creationTime = 0;
		modifiedTime = 0;
		imageQuality = 0;
	}
};

} // End of namespace Director

#endif
