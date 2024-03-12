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

#ifndef MM1_VIEWS_MAPS_PRISONERS_H
#define MM1_VIEWS_MAPS_PRISONERS_H

#include "mm/mm1/views/text_view.h"
#include "mm/mm1/data/character.h"

namespace MM {
namespace MM1 {
namespace Views {
namespace Interactions {

class Prisoner : public TextView {
private:
	Common::String _line1;
	byte _flag;
	Alignment _freeAlignment;
	Alignment _leaveAlignment;
protected:
	virtual void flee() {}
public:
	Prisoner(const Common::String &name, const Common::String &line1,
		byte flag, Alignment freeAlignment, Alignment leaveAlignment);
	virtual ~Prisoner() {}

	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	void timeout() override;
};

class ChildPrisoner : public Prisoner {
public:
	ChildPrisoner();
	virtual ~ChildPrisoner() {}
};

class ManPrisoner : public Prisoner {
public:
	ManPrisoner();
	virtual ~ManPrisoner() {}
};

class CloakedPrisoner : public Prisoner {
public:
	CloakedPrisoner();
	virtual ~CloakedPrisoner() {
	}
};

class DemonPrisoner : public Prisoner {
public:
	DemonPrisoner();
	virtual ~DemonPrisoner() {
	}
};

class MutatedPrisoner : public Prisoner {
public:
	MutatedPrisoner();
	virtual ~MutatedPrisoner() {
	}
};

class MaidenPrisoner : public Prisoner {
protected:
	void flee() override;
public:
	MaidenPrisoner();
	virtual ~MaidenPrisoner() {
	}
};

class VirginPrisoner : public TextView {
public:
	VirginPrisoner();
	virtual ~VirginPrisoner() {
	}
	void draw() override;
	bool msgKeypress(const KeypressMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace MM1
} // namespace MM

#endif
