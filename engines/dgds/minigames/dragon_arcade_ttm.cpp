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

#include "dgds/minigames/dragon_arcade_ttm.h"
#include "dgds/minigames/dragon_arcade.h"
#include "dgds/ads.h"
#include "dgds/drawing.h"
#include "dgds/sound.h"
#include "dgds/includes.h"

namespace Dgds {

Common::String ArcadeFloor::dump() {
	return Common::String::format("ArcadeFloor<x:%d-%d y:%d flg:%d>",
				x, x + width, yval, flag);
}


DragonArcadeTTM::DragonArcadeTTM(ArcadeNPCState *npcState) : _npcState(npcState),
	_currentTTMNum(0), _currentNPCRunningTTM(0), _drawXOffset(0), _drawYOffset(0),
	_startYOffset(0), _doingInit(false), _drawColBG(0), _drawColFG(0)
{
	ARRAYCLEAR(_shapes3);
}

void DragonArcadeTTM::clearDataPtrs() {
	for (int i = 0; i < 5; i++) {
		_ttmEnvs[i] = TTMEnviro();
	}
	// TODO: Is this used anywhere?
	// INT_39e5_3cb8 = -1;
}

int16 DragonArcadeTTM::load(const char *filename) {
	TTMEnviro *env = nullptr;
	int16 envNum;
	for (envNum = 0; envNum < ARRAYSIZE(_ttmEnvs); envNum++) {
		if (_ttmEnvs[envNum].scr == nullptr) {
			env = &_ttmEnvs[envNum];
			debug("Arcade TTM load %s into env %d", filename, envNum);
			break;
		}
	}
	if (!env)
		error("Trying to load too many TTMs in Dragon arcade");

	DgdsEngine *engine = DgdsEngine::getInstance();
	TTMParser dgds(engine->getResourceManager(), engine->getDecompressor());
	bool parseResult = dgds.parse(env, filename);
	if (!parseResult)
		error("Error loading dgds arcade script %s", filename);

	env->scr->seek(0);

	return envNum;
}

void DragonArcadeTTM::finishTTMParse(int16 envNum) {
	TTMEnviro &env = _ttmEnvs[envNum];

	if (!env.scr)
		error("DragonArcadeTTM::finishTTMParse: script env %d not loaded", envNum);

	// Discover the frame offsets
	uint16 op = 0;
	for (uint frame = 0; frame < env._totalFrames; frame++) {
		env._frameOffsets[frame] = env.scr->pos();
		op = env.scr->readUint16LE();
		while (op != 0x0ff0 && env.scr->pos() < env.scr->size()) {
			switch (op & 0xf) {
			case 0:
				break;
			case 0xf: {
				TTMInterpreter::readTTMStringVal(env.scr);
				break;
			}
			default:
				env.scr->skip((op & 0xf) * 2);
				break;
			}
			op = env.scr->readUint16LE();
		}
	}
	env.scr->seek(0);
}

int16 DragonArcadeTTM::runNextPage(int16 pageNum) {
	_shapes2[_currentTTMNum] = _shapes[_currentTTMNum];
	// TODO: what is this?
	//UINT_39e5_3ca2 = 0;

	if (pageNum < _ttmEnvs[_currentTTMNum]._totalFrames && pageNum > -1 &&
			_ttmEnvs[_currentTTMNum]._frameOffsets[pageNum] > -1) {
		return runScriptPage(pageNum);
	} else {
		return 0;
	}
}

int16 DragonArcadeTTM::handleOperation(TTMEnviro &env, int16 page, uint16 op, byte count, const int16 *ivals, const Common::String &sval) {
	DgdsEngine *engine = DgdsEngine::getInstance();
	Graphics::ManagedSurface &compBuffer = engine->_compositionBuffer;
	switch (op) {
	case 0x0020:
		// This doesn't seem explicitly handled in the original, but appears in
		// arcade sequence 2 - just ignore it??
		break;
	case 0x0070:
		// Do nothing.
		break;
	case 0x0080: // FREE SHAPE
		_allShapes[_shapes3[_currentTTMNum] * 5 + _currentTTMNum].reset();
		_shapes[_currentTTMNum].reset();
		break;
	case 0x1021: // SET DELAY
		engine->adsInterpreter()->setScriptDelay((int)(ivals[0] * MS_PER_FRAME));
		break;
	case 0x1031: // SET BRUSH
		//debug("Set brush %d for slot %d", ivals[0], _currentTTMNum);
		if (!_shapes2[_currentTTMNum]) {
			_brushes[_currentTTMNum].reset();
		} else {
			_brushes[_currentTTMNum] = Brush(_shapes2[_currentTTMNum], ivals[0]);
		}
		break;
	case 0x1051: // SET SHAPE
		_shapes3[_currentTTMNum] = ivals[0];
		//debug("Set img %d into slot %d", ivals[0], _currentTTMNum);
		_shapes[_currentTTMNum] = _allShapes[ivals[0] * 5 + _currentTTMNum];
		_shapes2[_currentTTMNum] = _allShapes[ivals[0] * 5 + _currentTTMNum];
		break;
	case 0x1061:
		// Do nothing (ignore arg)
		break;
	case 0x1101:
	case 0x1111:
		// Do nothing (ignore arg)
		break;
	case 0x1201:
		// This doesn't seem explicitly handled in the original, but appears in
		// arcade sequence 1 - just ignore it??
		break;
	case 0x2002: // SET COLORS
		_drawColFG = (byte)ivals[0];
		_drawColBG = (byte)ivals[1];
		break;
	case 0x2012: { // PLAY SOUND
		int16 sound;
		if (ivals[0] == 0 || ivals[0] == 1) {
			sound = 0x26;
		} else if (ivals[0] == 2) {
			sound = 0x4f;
		} else {
			break;
		}
		engine->_soundPlayer->playSFX(sound);
		break;
	}
	case 0x4504: { // SET NPC POS 1
		int16 x = _drawXOffset + ivals[0];
		int16 y = _drawYOffset + ivals[1] + 2;
		_npcState[_currentNPCRunningTTM].x_11 = x;
		_npcState[_currentNPCRunningTTM].x_12 = x + ivals[2];
		_npcState[_currentNPCRunningTTM].y_11 = y;
		_npcState[_currentNPCRunningTTM].y_12 = y + ivals[3];
		break;
	}
	case 0x4514: {// SET NPC POS 2
		int16 x = _drawXOffset + ivals[0];
		int16 y = _drawYOffset + ivals[1] + 2;
		_npcState[_currentNPCRunningTTM].x_21 = x;
		_npcState[_currentNPCRunningTTM].x_22 = x + ivals[2];
		_npcState[_currentNPCRunningTTM].y_21 = y;
		_npcState[_currentNPCRunningTTM].y_22 = y + ivals[3];
		break;
	}
	case 0xA0A4: { // DRAW LINE
		compBuffer.drawLine(_drawXOffset + ivals[0], _drawYOffset + ivals[1] + 2, _drawXOffset + ivals[2], _drawYOffset + ivals[3] + 2, _drawColFG);
		break;
	}
	case 0xA104: // DRAW FILLED RECT
		if (_doingInit) {
			ArcadeFloor data;
			data.x = (page - 1) * 320 + ivals[0];
			data.width = ivals[2];
			data.yval = (byte)ivals[1];
			data.flag = false;
			debug("Floor: %s", data.dump().c_str());
			_floorData.push_back(data);
		} else {
			const Common::Rect rect(Common::Point(ivals[0], ivals[1]), ivals[2], ivals[3]);
			compBuffer.fillRect(rect, _drawColFG);
		}
		break;
	case 0xA114: // DRAW EMPTY RECT
		if (_doingInit) {
			ArcadeFloor data;
			data.x = (page - 1) * 320 + ivals[0];
			data.width = ivals[2];
			data.yval = (byte)ivals[1];
			data.flag = true;
			debug("Floor: %s", data.dump().c_str());
			_floorData.push_back(data);
		} else {
			const Common::Rect r(Common::Point(ivals[0], ivals[1]), ivals[2] - 1, ivals[3] - 1);
			compBuffer.drawLine(r.left, r.top, r.right, r.top, _drawColFG);
			compBuffer.drawLine(r.left, r.bottom, r.right, r.bottom, _drawColFG);
			compBuffer.drawLine(r.left, r.top, r.left, r.bottom, _drawColFG);
			compBuffer.drawLine(r.right, r.top, r.right, r.bottom, _drawColFG);
		}
		break;
	case 0xA404: { // DRAW FILLED CIRCLE
		int16 r = ivals[3] / 2;
		Drawing::filledCircle(ivals[0], ivals[1], r, r, &compBuffer, _drawColFG, _drawColBG);
		break;
	}
	case 0xA424: { // DRAW EMPTY CIRCLE
		int16 r = ivals[3] / 2;
		Drawing::emptyCircle(ivals[0], ivals[1], r, r, &compBuffer, _drawColFG);
		break;
	}
	case 0xA502:
	case 0xA512:
	case 0xA522:
	case 0xA532: { // DRAW SHAPE
		if (_doingInit)
			break;

		ImageFlipMode flipMode = kImageFlipNone;
		if (op == 0xa512)
			flipMode = kImageFlipV;
		else if (op == 0xa522)
			flipMode = kImageFlipH;
		else if (op == 0xa532)
			flipMode = kImageFlipHV;

		// Only draw in the scroll area
		const Common::Rect drawWin(Common::Point(8, 8), SCREEN_WIDTH - 16, 117);
		if (_currentNPCRunningTTM == 0) {
			int16 x = ivals[0] + _npcState[0].x - 152;
			int16 y = ivals[1] + _startYOffset + 2;
			if (_brushes[_currentTTMNum].isValid())
				_brushes[_currentTTMNum].getShape()->drawBitmap(_brushes[_currentTTMNum].getFrame(), x, y, drawWin, compBuffer, flipMode);
			_npcState[0].y = y;
		} else {
			int16 x = ivals[0] + _drawXOffset;
			int16 y = ivals[1] + _drawYOffset + 2;
			if (_brushes[_currentTTMNum].isValid())
				_brushes[_currentTTMNum].getShape()->drawBitmap(_brushes[_currentTTMNum].getFrame(), x, y, drawWin, compBuffer, flipMode);
			_npcState[_currentNPCRunningTTM].x = x;
			_npcState[_currentNPCRunningTTM].y = y;
		}
		break;
	}
	case 0xF02F: {
		_shapes[_currentTTMNum].reset(new Image(engine->getResourceManager(), engine->getDecompressor()));
		_shapes[_currentTTMNum]->loadBitmap(sval);
		debug("Load img %s into slot %d", sval.c_str(), _currentTTMNum);
		_shapes2[_currentTTMNum] = _shapes[_currentTTMNum];
		_allShapes[_shapes3[_currentTTMNum] * 5 + _currentTTMNum] = _shapes[_currentTTMNum];
		break;
	}
	case 0x505F:
		// Do nothing (ignore arg)
		break;
	default:
		warning("Unsupported TTM opcode 0x%04x for Dragon arcade.", op);
		break;
	}
	return 0;
}

int16 DragonArcadeTTM::runScriptPage(int16 pageNum) {
	Common::SeekableReadStream *scr = _ttmEnvs[_currentTTMNum].scr;
	scr->seek(_ttmEnvs[_currentTTMNum]._frameOffsets[pageNum]);

	uint16 opcode = scr->readUint16LE();
	while (opcode != 0x0ff0 && opcode) {
		int16 ivals[4] { 0, 0, 0, 0 };
		Common::String sval;
		byte count = (byte)(opcode & 0xf);
		if (count <= 4) {
			for (int i = 0; i < count; i++)
				ivals[i] = scr->readUint16LE();
		} else if (count == 0xf) {
			sval = TTMInterpreter::readTTMStringVal(scr);
		} else {
			error("Unsupported TTM opcode 0x%04x with %d args for Dragon arcade.", opcode, count);
		}

		handleOperation(_ttmEnvs[_currentTTMNum], pageNum, opcode, count, ivals, sval);
		opcode = scr->readUint16LE();
	}
	return 1;

}

void DragonArcadeTTM::runPagesForEachNPC(int16 xScrollOffset) {
	for (_currentNPCRunningTTM = 19; _currentNPCRunningTTM > 0; _currentNPCRunningTTM--) {
		ArcadeNPCState &npcState = _npcState[_currentNPCRunningTTM];
		if (npcState.byte12) {
			npcState.x_21 = 0;
			npcState.x_11 = 0;
			npcState.x_22 = 0;
			npcState.x_12 = 0;
			npcState.y_21 = 0;
			npcState.y_11 = 0;
			npcState.y_22 = 0;
			npcState.y_12 = 0;
			_drawXOffset = npcState.xx - xScrollOffset * 8 - 152;
			_drawYOffset = npcState.yy;
			_currentTTMNum = npcState.ttmNum;

			// The original does this comparison, but it seems like a bug (should be &&)
			// We could correct the check, but better to maintain bug compatibility.
			// if (_drawXOffset > -20 || _drawXOffset < 340)
			runNextPage(npcState.ttmPage);
		}
	}
}


void DragonArcadeTTM::freePages(uint16 num) {
	delete _ttmEnvs[num].scr;
	_ttmEnvs[num] = TTMEnviro();
}

void DragonArcadeTTM::freeShapes() {
	_shapes3[_currentTTMNum] = 0;
	_shapes[_currentTTMNum].reset();
	_shapes2[_currentTTMNum].reset();
	for (int i = 0; i < 6; i++) {
		_allShapes[i * 5 + _currentTTMNum].reset();
	}
}

} // end namespace Dgds
