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

#ifndef PINK_HANDLER_H
#define PINK_HANDLER_H

#include "common/str-array.h"

#include "pink/objects/object.h"

namespace Pink {

class Condition;
class SideEffect;
class LeadActor;
class Actor;

class Handler : public Object {
public:
	~Handler() override;
	void deserialize(Archive &archive) override;
	virtual void handle(Actor *actor);
	bool isSuitable(const Actor *actor) const;

protected:
	void executeSideEffects(Actor *actor);

	Array<Condition *> _conditions;
	Array<SideEffect *> _sideEffects;
};

class Sequence;
class Sequencer;

class HandlerSequences : public Handler {
public:
	void deserialize(Archive &archive) override;
	void handle(Actor *actor) override;

protected:
	virtual void authorSequence(Sequencer *sequencer, Sequence *sequence);

protected:
	StringArray _sequences;
};

class HandlerStartPage : public HandlerSequences {
	void authorSequence(Sequencer *sequencer, Sequence *sequence) override;
};

class HandlerLeftClick : public HandlerSequences {
public:
	void toConsole() const override;
};

class HandlerUseClick : public HandlerSequences {
public:
	void deserialize(Archive &archive) override;
	void toConsole() const override;

	const Common::String &getInventoryItem() const { return _inventoryItem; }
	const Common::String &getRecipient() const { return _recipient; }

private:
	Common::String _inventoryItem;
	Common::String _recipient;
};

class HandlerTimerActions : public Handler {
public:
	void toConsole() const override;
	void deserialize(Archive &archive) override;
	void handle(Actor *actor) override;

private:
	StringArray _actions;
};

class HandlerTimerSequences : public HandlerSequences {
	void authorSequence(Sequencer *sequencer, Sequence *sequence) override;
};

} // End of namespace Pink

#endif
