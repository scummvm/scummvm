
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

#ifndef BAGEL_BAGLIB_LINK_OBJECT_H
#define BAGEL_BAGLIB_LINK_OBJECT_H

#include "bagel/baglib/object.h"

namespace Bagel {

/**
 * CBagLinkObject is an object that can be place within the slide window.
 */
class CBagLinkObject : public CBagObject {
public:
	enum LinkType { LINK = 0, CLOSEUP = 1 };

private:
	CBofSize _size;
	CBofPoint _destLocation;
	CBofPoint _srcLocation;
	LinkType _linkType;
	int _fade;

public:
	CBagLinkObject();
	virtual ~CBagLinkObject();

	// Return UPDATED_OBJECT if the Object had members that are properly initialized/de-initialized
	ParseCodes setInfo(CBagIfstream &istr) override;

	bool runObject() override;

	CBofRect getRect() override;

	CBofSize getSize() const {
		return _size;
	}
	CBofPoint getDestLoc() const {
		return _destLocation;
	}
	CBofPoint getSrcLoc() const {
		return _srcLocation;
	}

	void setSize(const CBofSize &size) override {
		_size = size;
	}
	void setDstLoc(const CBofPoint &loc) {
		_destLocation = loc;
	}
	void setSrcLoc(const CBofPoint &loc) {
		_srcLocation = loc;
	}
};

} // namespace Bagel

#endif
