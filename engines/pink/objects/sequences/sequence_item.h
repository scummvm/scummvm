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

#ifndef PINK_SEQUENCE_ITEM_H
#define PINK_SEQUENCE_ITEM_H

#include "pink/objects/object.h"

namespace Pink {

class Sequence;

class SequenceItem : public Object {
public:
	void deserialize(Archive &archive) override;

	void toConsole() const override;

	virtual bool execute(uint segment, Sequence *sequence, bool loadingSave);
	virtual bool isLeader();
	virtual void skip(Sequence *sequence) {};

	const Common::String &getActor() const { return _actor; }

protected:
	Common::String _actor;
	Common::String _action;
};

class SequenceItemLeader : public SequenceItem {
public:
	void toConsole() const override;
	bool isLeader() override;
};

class SequenceItemLeaderAudio : public SequenceItemLeader {
public:
	SequenceItemLeaderAudio()
		: _sample(0) {}

	void deserialize(Archive &archive) override;
	void toConsole() const override;

	uint32 getSample() { return _sample; }

private:
	uint32 _sample;
};

class SequenceItemDefaultAction : public SequenceItem {
public:
	void toConsole() const override;

	bool execute(uint segment, Sequence *sequence, bool loadingSave) override;
	void skip(Sequence *sequence) override;
};

} // End of namespace Pink

#endif
