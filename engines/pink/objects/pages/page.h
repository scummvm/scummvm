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

#ifndef PINK_PAGE_H
#define PINK_PAGE_H

#include "pink/resource_mgr.h"
#include "pink/objects/module.h"

namespace Pink {

class Archive;
class Actor;
class LeadActor;
class WalkMgr;
class Sequencer;

class Page : public NamedObject {
public:
	~Page() override;
	void toConsole() const override;

	void load(Archive &archive) override;
	void init();
	void initPalette();

	Actor *findActor(const Common::String &name);
	LeadActor *getLeadActor() { return _leadActor; }

	Common::SafeSeekableSubReadStream *getResourceStream(const Common::String &fileName) { return _resMgr.getResourceStream(fileName); }

	virtual void clear();
	void pause(bool paused);

	PinkEngine *getGame() { return _resMgr.getGame(); }

	virtual Sequencer *getSequencer() { return nullptr; }
	virtual WalkMgr *getWalkMgr() { return nullptr; }
	virtual Module *getModule()  { return nullptr; }
	virtual const Module *getModule() const { return nullptr; }

	virtual bool checkValueOfVariable(const Common::String &variable, const Common::String &value) const { return 0; }
	virtual void setVariable(Common::String &variable, Common::String &value) {}

protected:

	Array<Actor *> _actors;
	ResourceMgr _resMgr;
	LeadActor *_leadActor;
};

} // End of namespace Pink

#endif
