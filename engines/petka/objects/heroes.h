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

#ifndef PETKA_HEROES_H
#define PETKA_HEROES_H

#include "common/ptr.h"
#include "petka/objects/object.h"

namespace Petka {

class Walk;

class QObjectPetka : public QObject {
public:
	QObjectPetka();
	void processMessage(const QMessage &msg) override;
	void initSurface();

	void walk(int x, int y);
	void stopWalk();
	void updateWalk();
	void setReactionAfterWalk(uint index, QReaction *reaction, QMessageObject *sender, bool deleteReaction);

	void draw() override;
	bool isInPoint(Common::Point p) override;
	void update(int time) override;
	void setPos(Common::Point p, bool ) override;

	double calcPerspective(int y);

	void updateZ() override;

	void sub_408940();

private:
	virtual void recalcOffset();

public:
	int _field7C;
	int _surfW;
	int _surfH;
	int _x_;
	int _y_;
	// int _surfId;
	int _imageId;
	double _k;
	Common::ScopedPtr<Walk> _walk;
	int _destX;
	int _destY;
	bool _isWalking;
	QReaction *_heroReaction;
	QMessageObject *_sender;
	int _fieldB4;
};

class QObjectChapayev : public QObjectPetka {
public:
	QObjectChapayev();

	void recalcOffset() override {}

};

} // End of namespace Petka

#endif
