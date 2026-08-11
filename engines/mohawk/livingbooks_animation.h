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

#ifndef MOHAWK_LIVINGBOOKS_ANIMATION_H
#define MOHAWK_LIVINGBOOKS_ANIMATION_H

#include "mohawk/livingbooks_constants.h"
#include "mohawk/livingbooks_item.h"
#include "mohawk/sound.h"

#include "common/array.h"
#include "common/rect.h"

namespace Mohawk {

class MohawkEngine_LivingBooks;
class LBPage;
class LBAnimation;

struct LBAnimScriptEntry {
	byte opcode;
	byte size;
	byte *data;
};

class LBAnimationNode {
public:
	LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId);
	~LBAnimationNode();

	void draw(const Common::Rect &_bounds);
	void reset();
	NodeState update(bool seeking = false);
	bool transparentAt(int x, int y);

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimation *_parent;

	void loadScript(uint16 resourceId);
	uint _currentEntry;
	Common::Array<LBAnimScriptEntry> _scriptEntries;

	uint _currentCel;
	int16 _xPos, _yPos;
	uint32 _delay;
};

class LBAnimationItem;

class LBAnimation {
public:
	LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId);
	~LBAnimation();

	void draw();
	bool update();

	void start();
	void seek(uint16 pos);
	void seekToTime(uint32 time);
	void stop();

	void playSound(uint16 resourceId);
	bool soundPlaying(uint16 resourceId, const Common::String &cue);

	bool transparentAt(int x, int y);

	void setTempo(uint16 tempo);

	uint getNumResources() { return _shapeResources.size(); }
	uint16 getResource(uint num) { return _shapeResources[num]; }
	Common::Point getOffset(uint num) { return _shapeOffsets[num]; }

	uint32 getCurrentFrame() { return _currentFrame; }

	uint16 getParentId();

protected:
	MohawkEngine_LivingBooks *_vm;
	LBAnimationItem *_parent;

	Common::Rect _bounds, _clip;
	Common::Array<LBAnimationNode *> _nodes;

	uint16 _tempo;

	uint16 _currentSound;
	CueList _cueList;

	uint32 _lastTime, _currentFrame;
	bool _running;

	void loadShape(uint16 resourceId);
	Common::Array<uint16> _shapeResources;
	Common::Array<Common::Point> _shapeOffsets;
};

class LBAnimationItem : public LBItem {
public:
	LBAnimationItem(MohawkEngine_LivingBooks *_vm, LBPage *page, Common::Rect rect);
	~LBAnimationItem() override;

	void setEnabled(bool enabled) override;
	bool contains(Common::Point point) override;
	void update() override;
	void draw() override;
	bool togglePlaying(bool playing, bool restart) override;
	void done(bool onlyNotify) override;
	void init() override;
	void seek(uint16 pos) override;
	void seekToTime(uint32 time) override;
	void startPhase(uint phase) override;
	void stop() override;

protected:
	LBItem *createClone() override;

	LBAnimation *_anim;
	bool _running;
};

} // End of namespace Mohawk

#endif
