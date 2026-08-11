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

#include "mohawk/livingbooks.h"
#include "mohawk/resource.h"

#include "common/system.h"
#include "common/textconsole.h"

namespace Mohawk {

// SCRP - Animation sprites/background

LBAnimationNode::LBAnimationNode(MohawkEngine_LivingBooks *vm, LBAnimation *parent, uint16 scriptResourceId) : _vm(vm), _parent(parent) {
	loadScript(scriptResourceId);
}

LBAnimationNode::~LBAnimationNode() {
	for (uint32 i = 0; i < _scriptEntries.size(); i++)
		delete[] _scriptEntries[i].data;
}

void LBAnimationNode::loadScript(uint16 resourceId) {
	Common::SeekableReadStreamEndian *scriptStream = _vm->wrapStreamEndian(ID_SCRP, resourceId);

	reset();

	while (byte opcodeId = scriptStream->readByte()) {
		byte size = scriptStream->readByte();

		LBAnimScriptEntry entry;
		entry.opcode = opcodeId;
		entry.size = size;

		if (!size) {
			entry.data = nullptr;
		} else {
			entry.data = new byte[entry.size];
			scriptStream->read(entry.data, entry.size);
		}

		_scriptEntries.push_back(entry);
	}

	byte size = scriptStream->readByte();
	if (size != 0 || scriptStream->pos() != scriptStream->size())
		error("Failed to read script correctly");

	delete scriptStream;
}

void LBAnimationNode::draw(const Common::Rect &_bounds) {
	if (!_currentCel)
		return;

	// this is also checked in SetCel, below
	if (_currentCel > _parent->getNumResources())
		error("Animation cel %d was too high, this shouldn't happen!", _currentCel);

	int16 xOffset = _xPos + _bounds.left;
	int16 yOffset = _yPos + _bounds.top;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (!_vm->isPreMohawk()) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		xOffset -= offset.x;
		yOffset -= offset.y;
	}

	_vm->_gfx->copyOffsetAnimImageToScreen(resourceId, xOffset, yOffset);
}

void LBAnimationNode::reset() {
	// TODO: this causes stupid flickering
	//if (_currentCel)
	//	_vm->_needsRedraw = true;

	_currentCel = 0;
	_currentEntry = 0;
	_delay = 0;

	_xPos = 0;
	_yPos = 0;
}

NodeState LBAnimationNode::update(bool seeking) {
	if (_currentEntry == _scriptEntries.size())
		return kLBNodeDone;

	if (_delay > 0 && --_delay)
		return kLBNodeRunning;

	while (_currentEntry < _scriptEntries.size()) {
		LBAnimScriptEntry &entry = _scriptEntries[_currentEntry];
		_currentEntry++;
		debug(5, "Running script entry %d of %d", _currentEntry, _scriptEntries.size());

		switch (entry.opcode) {
		case kLBAnimOpPlaySound:
		case kLBAnimOpWaitForSound:
		case kLBAnimOpReleaseSound:
		case kLBAnimOpResetSound:
			{
			uint16 soundResourceId = READ_BE_UINT16(entry.data);

			if (!soundResourceId) {
				error("Unhandled named wave file, tell clone2727 where you found this");
				break;
			}

			Common::String cue;
			uint pos = 2;
			while (pos < entry.size) {
				char in = entry.data[pos];
				if (!in)
					break;
				pos++;
				cue += in;
			}
			if (pos == entry.size)
				error("Cue in sound kLBAnimOp wasn't null-terminated");

			switch (entry.opcode) {
			case kLBAnimOpPlaySound:
				if (seeking)
					break;
				debug(4, "a: PlaySound(%0d)", soundResourceId);
				_parent->playSound(soundResourceId);
				break;
			case kLBAnimOpWaitForSound:
				if (seeking)
					break;
				debug(4, "b: WaitForSound(%0d)", soundResourceId);
				if (!_parent->soundPlaying(soundResourceId, cue))
					break;
				_currentEntry--;
				return kLBNodeWaiting;
			case kLBAnimOpReleaseSound:
				debug(4, "c: ReleaseSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			case kLBAnimOpResetSound:
				debug(4, "d: ResetSound(%0d)", soundResourceId);
				// TODO
				_vm->_sound->stopSound(soundResourceId);
				break;
			default:
				break;
			}
			}
			break;

		case kLBAnimOpSetTempo:
		case kLBAnimOpSetTempoDiv:
			{
			assert(entry.size == 2);
			uint16 tempo = (int16)READ_BE_UINT16(entry.data);

			// TODO: LB 3 uses fixed-point here.
			if (entry.opcode == kLBAnimOpSetTempo) {
				debug(4, "3: SetTempo(%d)", tempo);
				// TODO: LB 3 uses (tempo * 1000) / 60, while
				// the original divides the system time by 16.
				_parent->setTempo(tempo * 16);
			} else {
				// LB 3.0+ only.
				debug(4, "E: SetTempoDiv(%d)", tempo);
				_parent->setTempo(1000 / tempo);
			}

			}
			break;

		case kLBAnimOpWait:
			assert(entry.size == 0);
			debug(5, "6: Wait()");
			return kLBNodeRunning;

		case kLBAnimOpMoveTo:
			{
			assert(entry.size == 4);
			int16 x = (int16)READ_BE_UINT16(entry.data);
			int16 y = (int16)READ_BE_UINT16(entry.data + 2);
			debug(4, "5: MoveTo(%d, %d)", x, y);

			_xPos = x;
			_yPos = y;
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpDrawMode:
			{
			assert(entry.size == 2);
			uint16 mode = (int16)READ_BE_UINT16(entry.data);
			debug(4, "9: DrawMode(%d)", mode);

			// TODO
			}
			break;

		case kLBAnimOpSetCel:
			{
			assert(entry.size == 2);
			uint16 cel = (int16)READ_BE_UINT16(entry.data);
			debug(4, "7: SetCel(%d)", cel);

			_currentCel = cel;
			if (_currentCel > _parent->getNumResources())
				error("SetCel set current cel to %d, but we only have %d cels", _currentCel, _parent->getNumResources());
			_vm->_needsRedraw = true;
			}
			break;

		case kLBAnimOpNotify:
			{
			assert(entry.size == 2);
			uint16 data = (int16)READ_BE_UINT16(entry.data);

			if (seeking)
				break;

			debug(4, "2: Notify(%d)", data);
			_vm->notifyAll(data, _parent->getParentId());
			}
			break;

		case kLBAnimOpSleepUntil:
			{
			assert(entry.size == 4);
			uint32 frame = READ_BE_UINT32(entry.data);
			debug(4, "8: SleepUntil(%d)", frame);

			if (frame > _parent->getCurrentFrame()) {
				// *not* kLBNodeWaiting
				_currentEntry--;
				return kLBNodeRunning;
			}
			}
			break;

		case kLBAnimOpDelay:
			{
			assert(entry.size == 4);
			uint32 delay = READ_BE_UINT32(entry.data);
			debug(4, "f: Delay(%d)", delay);
			_delay = delay;
			return kLBNodeRunning;
			}
			break;

		default:
			error("Unknown opcode id %02x (size %d)", entry.opcode, entry.size);
			break;
		}
	}

	return kLBNodeRunning;
}

bool LBAnimationNode::transparentAt(int x, int y) {
	if (!_currentCel)
		return true;

	uint16 resourceId = _parent->getResource(_currentCel - 1);

	if (!_vm->isPreMohawk()) {
		Common::Point offset = _parent->getOffset(_currentCel - 1);
		x += offset.x;
		y += offset.y;
	}

	// TODO: only check pixels if necessary
	return _vm->_gfx->imageIsTransparentAt(resourceId, true, x - _xPos, y - _yPos);
}

LBAnimation::LBAnimation(MohawkEngine_LivingBooks *vm, LBAnimationItem *parent, uint16 resourceId) : _vm(vm), _parent(parent) {
	Common::SeekableReadStreamEndian *aniStream = _vm->wrapStreamEndian(ID_ANI, resourceId);

	// ANI records in the Wanderful sampler are 32 bytes, extra bytes are just NULs
	if (aniStream->size() != 30 && aniStream->size() != 32)
		warning("ANI Record size mismatch");

	uint16 version = aniStream->readUint16();
	if (version != 1)
		warning("ANI version not 1");

	_bounds = _vm->readRect(aniStream);
	_clip = _vm->readRect(aniStream);
	// TODO: what is colorId for?
	uint32 colorId = aniStream->readUint32();
	uint32 sprResourceId = aniStream->readUint32();
	uint32 sprResourceOffset = aniStream->readUint32();

	debug(5, "ANI bounds: (%d, %d), (%d, %d)", _bounds.left, _bounds.top, _bounds.right, _bounds.bottom);
	debug(5, "ANI clip: (%d, %d), (%d, %d)", _clip.left, _clip.top, _clip.right, _clip.bottom);
	debug(5, "ANI color id: %d", colorId);
	debug(5, "ANI SPRResourceId: %d, offset %d", sprResourceId, sprResourceOffset);
	if (aniStream->size() == 32) {
		debug(5, "ANI extra bytes: (%d)", aniStream->readUint16());
	}

	if (aniStream->pos() != aniStream->size())
		error("Still %d bytes at the end of anim stream", (int)(aniStream->size() - aniStream->pos()));

	delete aniStream;

	if (sprResourceOffset)
		error("Cannot handle non-zero ANI offset yet");

	Common::SeekableReadStreamEndian *sprStream = _vm->wrapStreamEndian(ID_SPR, sprResourceId);

	uint16 numBackNodes = sprStream->readUint16();
	uint16 numFrontNodes = sprStream->readUint16();
	uint32 shapeResourceID = sprStream->readUint32();
	uint32 shapeResourceOffset = sprStream->readUint32();
	uint32 scriptResourceID = sprStream->readUint32();
	uint32 scriptResourceOffset = sprStream->readUint32();
	uint32 scriptResourceLength = sprStream->readUint32();
	debug(5, "SPR# stream: %d front, %d background", numFrontNodes, numBackNodes);
	debug(5, "Shape ID %d (offset 0x%04x), script ID %d (offset 0x%04x, length %d)", shapeResourceID, shapeResourceOffset,
		scriptResourceID, scriptResourceOffset, scriptResourceLength);

	Common::Array<uint16> scriptIDs;
	for (uint16 i = 0; i < numFrontNodes; i++) {
		uint32 unknown1 = sprStream->readUint32();
		uint32 unknown2 = sprStream->readUint32();
		uint32 unknown3 = sprStream->readUint32();
		uint16 scriptID = sprStream->readUint32();
		uint32 unknown4 = sprStream->readUint32();
		uint32 unknown5 = sprStream->readUint32();
		scriptIDs.push_back(scriptID);
		debug(6, "Front node %d: script ID %d", i, scriptID);
		if (unknown1 != 0 || unknown2 != 0 || unknown3 != 0 || unknown4 != 0 || unknown5 != 0)
			error("Anim node %d had non-zero unknowns %08x, %08x, %08x, %08x, %08x",
				i, unknown1, unknown2, unknown3, unknown4, unknown5);
	}

	if (numBackNodes)
		error("Ignoring %d back nodes", numBackNodes);

	if (sprStream->pos() != sprStream->size())
		error("Still %d bytes at the end of sprite stream", (int)(sprStream->size() - sprStream->pos()));

	delete sprStream;

	loadShape(shapeResourceID);

	_nodes.push_back(new LBAnimationNode(_vm, this, scriptResourceID));
	for (uint16 i = 0; i < scriptIDs.size(); i++)
		_nodes.push_back(new LBAnimationNode(_vm, this, scriptIDs[i]));

	_currentFrame = 0;
	_currentSound = 0xffff;
	_running = false;
	_tempo = 1;
}

LBAnimation::~LBAnimation() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		delete _nodes[i];
	if (_currentSound != 0xffff)
		_vm->_sound->stopSound(_currentSound);
}

void LBAnimation::loadShape(uint16 resourceId) {
	if (resourceId == 0)
		return;

	Common::SeekableReadStreamEndian *shapeStream = _vm->wrapStreamEndian(ID_SHP, resourceId);

	if (_vm->isPreMohawk()) {
		if (shapeStream->size() < 6)
			error("V1 SHP Record size too short (%d)", (int)shapeStream->size());

		uint16 u0 = shapeStream->readUint16();
		if (u0 != 3)
			error("V1 SHP Record u0 is %04x, not 3", u0);

		uint16 u1 = shapeStream->readUint16();
		if (u1 != 0)
			error("V1 SHP Record u1 is %04x, not 0", u1);

		uint16 idCount = shapeStream->readUint16();
		debug(8, "V1 SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 2) + 6)
			error("V1 SHP Record size mismatch (%d)", (int)shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			debug(8, "V1 SHP: BMAP Resource Id %d: %d", i, _shapeResources[i]);
		}
	} else {
		uint16 idCount = shapeStream->readUint16();
		debug(8, "SHP: idCount: %d", idCount);

		if (shapeStream->size() != (idCount * 6) + 2)
			error("SHP Record size mismatch (%d)", (int)shapeStream->size());

		for (uint16 i = 0; i < idCount; i++) {
			_shapeResources.push_back(shapeStream->readUint16());
			int16 x = shapeStream->readSint16();
			int16 y = shapeStream->readSint16();
			_shapeOffsets.push_back(Common::Point(x, y));
			debug(8, "SHP: tBMP Resource Id %d: %d, at (%d, %d)", i, _shapeResources[i], x, y);
		}
	}

	for (uint16 i = 0; i < _shapeResources.size(); i++)
		_vm->_gfx->preloadImage(_shapeResources[i]);

	delete shapeStream;
}

void LBAnimation::draw() {
	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->draw(_bounds);
}

bool LBAnimation::update() {
	if (!_running)
		return false;

	if (_vm->_system->getMillis() <= _lastTime + (uint32)_tempo)
		return false;

	// the second check is to try 'catching up' with lagged animations, might be crazy
	if (_lastTime == 0 || (_vm->_system->getMillis()) > _lastTime + (uint32)(_tempo * 2))
		_lastTime = _vm->_system->getMillis();
	else
		_lastTime += _tempo;

	if (_currentSound != 0xffff && !_vm->_sound->isPlaying(_currentSound)) {
		_currentSound = 0xffff;
	}

	NodeState state = kLBNodeDone;
	for (uint32 i = 0; i < _nodes.size(); i++) {
		NodeState s = _nodes[i]->update();
		if (s == kLBNodeWaiting) {
			state = kLBNodeWaiting;
			if (i != 0)
				warning("non-primary node was waiting");
			break;
		}
		if (s == kLBNodeRunning)
			state = kLBNodeRunning;
	}

	if (state == kLBNodeRunning) {
		_currentFrame++;
	} else if (state == kLBNodeDone) {
		if (_currentSound == 0xffff) {
			_running = false;
			return true;
		}
	}

	return false;
}

void LBAnimation::start() {
	_lastTime = 0;
	_running = true;
}

void LBAnimation::seek(uint16 pos) {
	_lastTime = 0;
	_currentFrame = 0;

	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}

	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->reset();

	for (uint16 n = 0; n < pos; n++) {
		bool ranSomething = false;
		// nodes don't wait while seeking
		for (uint32 i = 0; i < _nodes.size(); i++)
			ranSomething |= (_nodes[i]->update(true) != kLBNodeDone);

		_currentFrame++;

		if (!ranSomething) {
			_running = false;
			break;
		}
	}
}

void LBAnimation::seekToTime(uint32 time) {
	_lastTime = 0;
	_currentFrame = 0;

	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}

	for (uint32 i = 0; i < _nodes.size(); i++)
		_nodes[i]->reset();

	uint32 elapsed = 0;
	while (elapsed <= time) {
		bool ranSomething = false;
		// nodes don't wait while seeking
		for (uint32 i = 0; i < _nodes.size(); i++)
			ranSomething |= (_nodes[i]->update(true) != kLBNodeDone);

		elapsed += _tempo;
		_currentFrame++;

		if (!ranSomething) {
			_running = false;
			break;
		}
	}
}

void LBAnimation::stop() {
	_running = false;
	if (_currentSound != 0xffff) {
		_vm->_sound->stopSound(_currentSound);
		_currentSound = 0xffff;
	}
}

void LBAnimation::playSound(uint16 resourceId) {
	_currentSound = resourceId;
	_vm->_sound->playSound(_currentSound, Audio::Mixer::kMaxChannelVolume, false, &_cueList);
}

bool LBAnimation::soundPlaying(uint16 resourceId, const Common::String &cue) {
	if (_currentSound != resourceId)
		return false;
	if (!_vm->_sound->isPlaying(_currentSound))
		return false;

	if (cue.empty())
		return true;

	uint samples = _vm->_sound->getNumSamplesPlayed(_currentSound);
	for (uint i = 0; i < _cueList.pointCount; i++) {
		if (_cueList.points[i].sampleFrame > samples)
			break;
		if (_cueList.points[i].name == cue)
			return false;
	}

	return true;
}

bool LBAnimation::transparentAt(int x, int y) {
	for (uint32 i = 0; i < _nodes.size(); i++)
		if (!_nodes[i]->transparentAt(x - _bounds.left, y - _bounds.top))
			return false;

	return true;
}

void LBAnimation::setTempo(uint16 tempo) {
	_tempo = tempo;
}

uint16 LBAnimation::getParentId() {
	return _parent->getId();
}

LBAnimationItem::LBAnimationItem(MohawkEngine_LivingBooks *vm, LBPage *page, Common::Rect rect) : LBItem(vm, page, rect) {
	_anim = nullptr;
	_running = false;
	debug(3, "new LBAnimationItem");
}

LBAnimationItem::~LBAnimationItem() {
	delete _anim;
}

void LBAnimationItem::setEnabled(bool enabled) {
	if (_running) {
		if (enabled && _globalEnabled && !_loaded)
			_anim->start();
		else if (_loaded && !enabled && _enabled && _globalEnabled)
			_anim->stop();
	}

	return LBItem::setEnabled(enabled);
}

bool LBAnimationItem::contains(Common::Point point) {
	if (!LBItem::contains(point))
		return false;

	if (!_doHitTest)
		return true;

	return !_anim->transparentAt(point.x, point.y);
}

void LBAnimationItem::update() {
	if (_loaded && _enabled && _globalEnabled && _running) {
		bool wasDone = _anim->update();
		if (wasDone) {
			_running = false;
			done(true);
		}
	}

	LBItem::update();
}

bool LBAnimationItem::togglePlaying(bool playing, bool restart) {
	if (playing) {
		if (_loaded && _enabled && _globalEnabled) {
			if (restart)
				seek(1);
			_running = true;
			_anim->start();
		}

		return _running;
	}

	return LBItem::togglePlaying(playing, restart);
}

void LBAnimationItem::done(bool onlyNotify) {
	if (!onlyNotify) {
		_anim->stop();
	}

	LBItem::done(onlyNotify);
}

void LBAnimationItem::init() {
	_anim = new LBAnimation(_vm, this, _resourceId);

	LBItem::init();
}

void LBAnimationItem::stop() {
	if (_running) {
		_anim->stop();
		seek(0xFFFF);
	}

	_running = false;

	LBItem::stop();
}

void LBAnimationItem::seek(uint16 pos) {
	_anim->seek(pos);
}

void LBAnimationItem::seekToTime(uint32 time) {
	_anim->seekToTime(time);
}

void LBAnimationItem::startPhase(uint phase) {
	if (phase == _phase)
		seek(1);

	LBItem::startPhase(phase);
}

void LBAnimationItem::draw() {
	if (!_visible || !_globalVisible)
		return;

	_anim->draw();
}

LBItem *LBAnimationItem::createClone() {
	LBAnimationItem *item = new LBAnimationItem(_vm, _page, _rect);
	item->_anim = new LBAnimation(_vm, item, _resourceId);
	return item;
}

} // End of namespace Mohawk
