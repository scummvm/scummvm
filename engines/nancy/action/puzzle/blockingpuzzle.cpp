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

#include "common/random.h"
#include "common/system.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/blockingpuzzle.h"

namespace Nancy {
namespace Action {

static const uint32 kRecoilShakeDurationMs = 400;

BlockingPuzzle::~BlockingPuzzle() {
	g_system->setShakePos(0, 0);
}

void BlockingPuzzle::MovieDescription::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, name);
	startFrame = stream.readSint16LE();
	lastFrame = stream.readSint16LE();
	minPauseMs = stream.readSint32LE();
	maxPauseMs = stream.readSint32LE();
	pauseChance = stream.readByte();
}

void BlockingPuzzle::readData(Common::SeekableReadStream &stream) {
	_opponentHealthIndex = stream.readSint16LE();
	_playerHealthIndex = stream.readSint16LE();
	_field4 = stream.readSint16LE();
	_field6 = stream.readSint16LE();
	_field8 = stream.readByte();
	_field9 = stream.readByte();
	readFilename(stream, _imageName);
	for (uint i = 0; i < 4; ++i) {
		readRect(stream, _controlRects[i]);
	}
	_point.x = stream.readSint32LE();
	_point.y = stream.readSint32LE();
	_field51 = stream.readSint16LE();
	readRect(stream, _movieSrc);
	readRect(stream, _movieDest);
	_flag = stream.readByte();

	_fullBlockSounds.readData(stream);
	_partBlockSounds.readData(stream);
	_hitSounds.readData(stream);

	int16 numCells = stream.readSint16LE();
	_grid.resize(numCells);
	for (int16 i = 0; i < numCells; ++i) {
		GridCell &cell = _grid[i];
		cell.id = stream.readByte();
		readRect(stream, cell.rect);
		for (uint j = 0; j < 8; ++j) {
			cell.params[j] = stream.readSint16LE();
		}
		int16 numNeighbors = stream.readSint16LE();
		cell.neighbors.resize(numNeighbors);
		for (int16 j = 0; j < numNeighbors; ++j) {
			cell.neighbors[j] = stream.readSint16LE();
		}
	}

	_introMovie.readData(stream);
	int16 numIntro = stream.readSint16LE();
	_introSequence.resize(numIntro);
	for (int16 i = 0; i < numIntro; ++i) {
		readFilename(stream, _introSequence[i].name);
		_introSequence[i].weight = stream.readSint16LE();
	}

	int16 numMoves = stream.readSint16LE();
	_moves.resize(numMoves);
	for (int16 i = 0; i < numMoves; ++i) {
		AttackMove &move = _moves[i];
		move.cellID = stream.readSint16LE();
		move.field2 = stream.readSint16LE();
		move.windup.readData(stream);
		move.field_c = stream.readSint16LE();
		move.field_d = stream.readSint16LE();
		move.field_e = stream.readSint16LE();
		move.attack.readData(stream);
		move.strikeFrame = stream.readSint16LE();
		move.response.readData(stream);
		int16 numIdle = stream.readSint16LE();
		move.idleNames.resize(numIdle);
		for (int16 j = 0; j < numIdle; ++j) {
			readFilename(stream, move.idleNames[j]);
		}
		move.idleWeight = stream.readSint16LE();
	}
}

void BlockingPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	// The attack-telegraph box is a sprite in this sheet (source _controlRects[0]).
	g_nancy->_resource->loadImage(_imageName, _overlayImage);
	_overlayImage.setTransparentColor(_drawSurface.getTransparentColor());

	// The eight params are four recoil-vector pairs, chosen by block outcome.
	_cells.resize(_grid.size());
	for (uint i = 0; i < _grid.size(); ++i) {
		const GridCell &g = _grid[i];
		RuntimeCell &c = _cells[i];
		c.id = g.id;
		c.rect = g.rect;
		c.base = Common::Point(g.params[0], g.params[1]);
		c.full = Common::Point(g.params[2], g.params[3]);
		c.hit = Common::Point(g.params[4], g.params[5]);
		c.partial = Common::Point(g.params[6], g.params[7]);
		c.neighbors = g.neighbors;
	}

	// Health lives in the shared value table (indices _opponentHealthIndex/_playerHealthIndex); start at 0.
	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (table) {
		if (_opponentHealthIndex != 0xff) {
			table->setSingleValue(_opponentHealthIndex, 0);
		}
		if (_playerHealthIndex != 0xff) {
			table->setSingleValue(_playerHealthIndex, 0);
		}
	}

	_playerBlockCell = 0;
	_activeMove = -1;
	playNextMovie();
}

bool BlockingPuzzle::playMovie(const Common::Path &name) {
	if (name.empty() || !_moviePlayer.loadFile(name) || _moviePlayer.getFrameCount() <= 0) {
		if (!name.empty()) {
			warning("BlockingPuzzle: couldn't load movie '%s'", name.toString().c_str());
		}
		return false;
	}
	_moviePlayer.playRange(0, _moviePlayer.getFrameCount() - 1);
	redraw();
	return true;
}

int BlockingPuzzle::findMoveByMovie(const Common::Path &name) const {
	for (uint i = 0; i < _moves.size(); ++i) {
		if (!_moves[i].response.name.empty() && _moves[i].response.name == name) {
			return (int)i;
		}
	}
	return -1;
}

const BlockingPuzzle::RuntimeCell *BlockingPuzzle::cellByID(int id) const {
	for (uint i = 0; i < _cells.size(); ++i) {
		if (_cells[i].id == id) {
			return &_cells[i];
		}
	}
	return nullptr;
}

void BlockingPuzzle::updateRecoil() {
	if (_recoilStartMs == 0) {
		return;
	}

	uint32 elapsed = g_system->getMillis() - _recoilStartMs;
	if (elapsed >= kRecoilShakeDurationMs) {
		g_system->setShakePos(0, 0);
		_recoilStartMs = 0;
		return;
	}

	// Amplitude falls to zero across the window; the sign flips a few times a
	// second so the screen buzzes rather than slides.
	int scale = 100 - (int)(elapsed * 100 / kRecoilShakeDurationMs);
	int sign = ((elapsed / 40) & 1) ? -1 : 1;
	g_system->setShakePos(_recoilAmp.x * scale * sign / 100,
		_recoilAmp.y * scale * sign / 100);
}

// Picks the next clip by weighted random; whichever move owns it becomes active.
void BlockingPuzzle::playNextMovie() {
	_resolved = false;
	_activeMove = -1;

	Common::Path pick = _introMovie.name;
	if (!_introSequence.empty()) {
		int total = 0;
		for (uint i = 0; i < _introSequence.size(); ++i) {
			if (_introSequence[i].weight > 0) {
				total += _introSequence[i].weight;
			}
		}

		if (total > 0) {
			int roll = g_nancy->_randomSource->getRandomNumber(total - 1);
			int cumulative = 0;
			for (uint i = 0; i < _introSequence.size(); ++i) {
				if (_introSequence[i].weight <= 0) {
					continue;
				}
				cumulative += _introSequence[i].weight;
				if (roll < cumulative) {
					pick = _introSequence[i].name;
					break;
				}
			}
		} else {
			pick = _introSequence[g_nancy->_randomSource->getRandomNumber(_introSequence.size() - 1)].name;
		}
	}

	if (!playMovie(pick)) {
		return;
	}

	_activeMove = findMoveByMovie(pick);
	if (_activeMove >= 0) {
		// Off-screen fighter's windup sound; the frame player is silent.
		playSoundBlock(_moves[_activeMove].windup);
	}
}

int BlockingPuzzle::resolveBlock(int attackCell, int blockCell, Common::Point &recoil) const {
	recoil = Common::Point(0, 0);

	const RuntimeCell *attack = nullptr;
	const RuntimeCell *block = nullptr;
	for (uint i = 0; i < _cells.size(); ++i) {
		if (_cells[i].id == attackCell) {
			attack = &_cells[i];
		}
		if (_cells[i].id == blockCell) {
			block = &_cells[i];
		}
	}

	if (!attack) {
		return kHit;
	}
	recoil += attack->base;

	if (attackCell == blockCell) {
		recoil += attack->full;
		return kFullBlock;
	}

	// A guard on an adjacent direction is a partial block.
	if (block) {
		for (uint i = 0; i < block->neighbors.size(); ++i) {
			if (block->neighbors[i] == attackCell) {
				recoil += attack->partial;
				return kPartialBlock;
			}
		}
	}

	recoil += attack->hit;
	return kHit;
}

void BlockingPuzzle::applyDamage(const Common::Point &recoil) {
	// recoil.x/.y are the damage to _opponentHealthIndex/_playerHealthIndex.
	TableData *table = (TableData *)NancySceneState.getPuzzleData(TableData::getTag());
	if (!table) {
		return;
	}
	if (_opponentHealthIndex != 0xff) {
		table->setSingleValue(_opponentHealthIndex, table->getSingleValue(_opponentHealthIndex) + recoil.x);
	}
	if (_playerHealthIndex != 0xff) {
		table->setSingleValue(_playerHealthIndex, table->getSingleValue(_playerHealthIndex) + recoil.y);
	}
}

int BlockingPuzzle::cellAtPoint(const Common::Point &mousePos) const {
	for (uint i = 0; i < _cells.size(); ++i) {
		if (_cells[i].rect.isEmpty()) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_cells[i].rect).contains(mousePos)) {
			return _cells[i].id;
		}
	}
	return 0;
}

// Alpha-blends an overlay-sheet sprite over the movie frame (per-pixel, RGB-keyed).
void BlockingPuzzle::drawTelegraph(const Common::Rect &srcRect, const Common::Point &destPos, byte alpha) {
	if (srcRect.isEmpty() || !_overlayImage.getBounds().contains(srcRect)) {
		return;
	}

	byte tr, tg, tb;
	g_nancy->_graphics->getInputPixelFormat().colorToRGB(g_nancy->_graphics->getTransColor(), tr, tg, tb);

	for (int y = 0; y < srcRect.height(); ++y) {
		int destY = destPos.y + y;
		if (destY < 0 || destY >= _drawSurface.h) {
			continue;
		}
		for (int x = 0; x < srcRect.width(); ++x) {
			int destX = destPos.x + x;
			if (destX < 0 || destX >= _drawSurface.w) {
				continue;
			}

			byte a, r, g, b;
			_overlayImage.format.colorToARGB(_overlayImage.getPixel(srcRect.left + x, srcRect.top + y), a, r, g, b);
			if (a == 0 || (r == tr && g == tg && b == tb)) {
				continue;
			}

			byte da, dr, dg, db;
			_drawSurface.format.colorToARGB(_drawSurface.getPixel(destX, destY), da, dr, dg, db);

			// Source-over, so the box shows over both the character and the
			// transparent movie background (not just the opaque silhouette).
			int srcA = a * alpha / 255;
			int dstA = da * (255 - srcA) / 255;
			int outA = srcA + dstA;
			if (outA == 0) {
				continue;
			}
			byte outR = (byte)((r * srcA + dr * dstA) / outA);
			byte outG = (byte)((g * srcA + dg * dstA) / outA);
			byte outB = (byte)((b * srcA + db * dstA) / outA);
			_drawSurface.setPixel(destX, destY, _drawSurface.format.ARGBToColor((byte)outA, outR, outG, outB));
		}
	}
}

void BlockingPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	if (_moviePlayer.isVideoLoaded()) {
		_moviePlayer.drawFrame(_drawSurface, Common::Point(_movieDest.left, _movieDest.top));
	}

	// Telegraph the target cell with the box sprite, fading bright to pale up to the
	// strike frame.
	if (_activeMove >= 0 && !_resolved && _moviePlayer.isVideoLoaded() && !_overlayImage.empty()) {
		const RuntimeCell *cell = cellByID(_moves[_activeMove].cellID);
		if (cell && !cell->rect.isEmpty()) {
			int strike = _moves[_activeMove].strikeFrame;
			int frame = _moviePlayer.getCurrentFrame();
			byte alpha = 255;
			if (strike > 0 && frame > 0) {
				alpha = (byte)(64 + 191 * CLIP(strike - frame, 0, strike) / strike);
			}
			drawTelegraph(_controlRects[0], Common::Point(cell->rect.left, cell->rect.top), alpha);
		}
	}

	_needsRedraw = true;
}

void BlockingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		updateRecoil();

		if (!_moviePlayer.isVideoLoaded()) {
			playNextMovie();
			break;
		}

		// Resolve the block when the attack clip reaches its strike frame: score the
		// outcome, apply damage, play the attack and reaction sounds.
		if (_activeMove >= 0 && !_resolved &&
			_moviePlayer.getCurrentFrame() >= _moves[_activeMove].strikeFrame) {
			_resolved = true;
			playSoundBlock(_moves[_activeMove].attack);

			Common::Point recoil;
			int outcome = resolveBlock(_moves[_activeMove].cellID, _playerBlockCell, recoil);
			applyDamage(recoil);

			// Jolt the screen on impact; direction/strength track the recoil vector.
			_recoilStartMs = g_system->getMillis();
			_recoilAmp = Common::Point(CLIP<int>(ABS(recoil.x) / 8, 0, 12),
				CLIP<int>(ABS(recoil.y) / 8, 0, 12));

			switch (outcome) {
			case kFullBlock:
				playSoundBlock(_fullBlockSounds);
				break;
			case kPartialBlock:
				playSoundBlock(_partBlockSounds);
				break;
			case kHit:
				playSoundBlock(_hitSounds);
				break;
			default:
				break;
			}
		}

		bool changed = _moviePlayer.update();

		if (!_moviePlayer.isRangePlaying()) {
			playNextMovie();
		} else if (changed) {
			redraw();
		}
		break;
	}
	default:
		break;
	}
}

void BlockingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Hovering a cell guards that direction (mouse-over, not click).
	int cell = cellAtPoint(input.mousePos);
	_playerBlockCell = cell;
	if (cell != 0) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
	}
}

void BlockingPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	// Pick a random one of the block's names.
	uint index = block.names.size() > 1 ?
		g_nancy->_randomSource->getRandomNumber(block.names.size() - 1) : 0;
	if (block.names[index].empty() || block.names[index] == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = block.names[index];
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);

	// The fighters' lines are CVTX captions keyed by the played sound's name
	// (autotext searched first, then convo), shown as the fight goes on.
	Common::String caption = resolveSubtitleText(desc.name, Common::String(), "AUTOTEXT");
	if (caption.empty()) {
		caption = resolveSubtitleText(desc.name, Common::String(), "CONVO");
	}
	if (!caption.empty()) {
		showSubtitle(caption);
	}
}

} // End of namespace Action
} // End of namespace Nancy
