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

namespace Petka {

class Screen;

class QVisibleObject {
public:
	QVisibleObject();
	virtual ~QVisibleObject() {};

	virtual void draw() {};

public:
	int32 _resourceId;
	int32 _z;
};


class QMessageObject : public QVisibleObject {
public:
	QMessageObject();

	virtual void processMessage(const QMessage &msg);

public:
	int32 _x;
	int32 _y;
	int32 _field14;
	int32 _field18;
	int32 _field20;
	int32 _field24;
	int32 _field28;
	int32 _isShown;
	int32 _animate;
	uint16 _id;
	int8 _status;
	Common::String _name;
	Common::String _nameOnScreen;
	int32 _dialogColor;
	Common::Array<QReaction> _reactions;
};

class QObjectBG : public QMessageObject {
public:
	void processMessage(const QMessage &msg) override;
	void draw() override;

public:
	int _showMap;
	int _fxId;
	int _musicId;
};

class QObject : public QMessageObject {

};

class QObjectCase : public QObject {

};

class QObjectStar : public QObject {

};

class QObjectCursor : public QObject {

};

} // End of naespace Petka

#endif
