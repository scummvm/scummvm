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

#ifndef PETKA_Q_MESSAGE_OBJECT_H
#define PETKA_Q_MESSAGE_OBJECT_H

#include "petka/base.h"

namespace Common {
	class SeekableReadStream;
}

namespace Petka {

class QVisibleObject {
public:
	QVisibleObject();
protected:
	int32 _resourceId;
	int32 _z;
};

class QMessageObject : public QVisibleObject {
public:
	QMessageObject();
	void deserialize(Common::SeekableReadStream &stream);
	void setDialogColor(int32 color);
	void setNameOnScreen(const Common::String &name);

protected:
	int16 _id;
	Common::String _name;
	Common::String _nameOnScreen;
	int32 _dialogColor;
	Common::Array<QReaction> _reactions;
};

} // End of namespace Petka

#endif
