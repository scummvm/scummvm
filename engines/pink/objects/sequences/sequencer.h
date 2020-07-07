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

#ifndef PINK_SEQUENCER_H
#define PINK_SEQUENCER_H

#include "pink/objects/object.h"

namespace Pink {

class Sequence;
class SequenceContext;
class GamePage;
class SeqTimer;

struct SequenceActorState;

class Sequencer : public Object {
public:
	Sequencer(GamePage *page);
	~Sequencer() override;

	void toConsole() const override;
	void deserialize(Archive &archive) override;

public:
	void loadState(Archive &archive);
	void saveState(Archive &archive);

	bool isPlaying() { return _context != nullptr; }
	bool isSkipping() { return _isSkipping; }
	void update();

	void authorSequence(Sequence *sequence, bool loadingSave);
	void authorParallelSequence(Sequence *sequence, bool loadingSave);

	void skipSubSequence();
	void restartSequence();
	void skipSequence();

	void removeContext(SequenceContext *context);

	SequenceContext *findConflictingContextWith(SequenceContext *context);

	Sequence *findSequence(const Common::String &name);
	SequenceActorState *findState(const Common::String &name);

	GamePage *getPage() const { return _page; }

private:
	SequenceContext *_context;
	GamePage *_page;
	Common::Array<SequenceContext *> _parallelContexts;
	Array<Sequence *> _sequences;
	Array<SeqTimer *> _timers;
	uint _time;
	bool _isSkipping;
};

} // End of namespace Pink

#endif
