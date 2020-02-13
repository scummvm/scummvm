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

#ifndef PINK_SEQUENCE_H
#define PINK_SEQUENCE_H

#include "pink/sound.h"
#include "pink/objects/sequences/sequence_item.h"

namespace Pink {

class Sequencer;
class SequenceItem;
class SequenceContext;

class Sequence : public NamedObject {
public:
	Sequence();
	~Sequence() override;

	void deserialize(Archive &archive) override ;
	void toConsole() const override;

public:
	virtual void init(bool loadingSave);

	virtual void start(bool loadingSave);
	virtual void end();
	virtual void restart();

	void forceEnd();

	virtual void update();

	virtual void skipSubSequence();
	virtual void skip();

	void allowSkipping() { _canBeSkipped = true; }
	bool isSkippingAllowed() { return _canBeSkipped; }

	SequenceContext *getContext() const { return _context; }
	Sequencer *getSequencer() const { return _sequencer; }
	Common::Array<SequenceItem *> &getItems() { return _items; }

	void setContext(SequenceContext *context) { _context = context; }

protected:
	SequenceContext *_context;
	Sequencer *_sequencer;
	Array<SequenceItem *> _items;
	bool _canBeSkipped;
};

class SequenceAudio : public Sequence {
public:
	SequenceAudio()
		: _leader(nullptr) {}

	void deserialize(Archive &archive) override;
	void toConsole() const override;

	void init(bool loadingSave) override;
	void start(bool loadingSave) override;
	void end() override;

	void update() override;
	void restart() override;

	void skipSubSequence() override {}
	void skip() override;

private:
	SequenceItemLeaderAudio *_leader;
	Common::String _soundName;
	Sound _sound;
};

} // End of namespace Pink

#endif
