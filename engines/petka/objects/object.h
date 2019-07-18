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

#ifndef PETKA_OBJECT_H
#define PETKA_OBJECT_H

#include "petka/base.h"

namespace Petka {

extern void processSavedReaction(QReaction **reaction, QMessageObject *sender);

class QVisibleObject {
public:
	QVisibleObject();
	virtual ~QVisibleObject() {};

	virtual void draw() {};
	virtual void update(int time) {};
	virtual void updateZ() {};
	virtual void show(bool v) {};
	virtual void setPos(int x, int y) {};
	virtual bool isInPoint(int x, int y) { return false; }

public:
	int32 _resourceId;
	uint32 _z;
};

class Sound;

class QMessageObject : public QVisibleObject {
public:
	QMessageObject();

	void show(bool v) override;
	virtual void processMessage(const QMessage &msg);

public:
	int32 _x;
	int32 _y;
	int32 _field14;
	int32 _field18;
	int32 _time;
	int32 _field20;
	int32 _field24;
	int32 _field28;
	int32 _isShown;
	int32 _animate;
	int _updateZ;
	int _field_38;
	int _isActive;
	int _startSound;
	int _hasSound;
	int _notLoopedSound;
	Sound *_sound;
	int8 _status;
	uint16 _id;
	Common::String _name;
	Common::String _nameOnScreen;
	int32 _dialogColor;
	Common::Array<QReaction> _reactions;
	QReaction *_reaction;
	int16 _reactionResId;
};


class QObject : public QMessageObject {
public:
	QObject();

	void draw() override;
	void update(int time) override;
	void updateZ() override;
	bool isInPoint(int x, int y) override;
	void setPos(int x, int y) override;
	void show(bool v) override;
};

extern QReaction *g_dialogReaction;

} // End of namespace Petka

#endif
