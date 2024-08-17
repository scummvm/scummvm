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

#include "dgds/dragon_arcade_ttm.h"

namespace Dgds {

DragonArcadeTTM::DragonArcadeTTM() {
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
				byte ch[2];
				do {
					ch[0] = env.scr->readByte();
					ch[1] = env.scr->readByte();
				} while (ch[0] != 0 && ch[1] != 0);
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
	//UINT_39e5_3ca2 = 0;

	if (pageNum < _ttmEnvs[_currentTTMNum]._totalFrames && pageNum > -1 &&
	_ttmEnvs[_currentTTMNum]._frameOffsets[pageNum] > -1) {
		return runScriptPage(pageNum);
	} else {
		return 0;
	}
}

int16 DragonArcadeTTM::runScriptPage(int16 pageNum) {
	Common::SeekableReadStream *scr = _ttmEnvs[_currentTTMNum].scr;
	scr->seek(_ttmEnvs[_currentTTMNum]._frameOffsets[pageNum]);

	uint16 opcode = scr->readUint16LE();
	if (opcode == 0x0ff0)
		return 1;

	error("DragonArcadeTTM::runScriptPage: TODO: Implement me");
}

void DragonArcadeTTM::freePages(uint16 num) {
	delete _ttmEnvs[num].scr;
	_ttmEnvs[num] = TTMEnviro();
}

void DragonArcadeTTM::freeShapes() {
	_shapes3[_currentTTMNum].reset();
	_shapes[_currentTTMNum].reset();
	for (int i = 0; i < 6; i++) {
		_allShapes[i * 5 + _currentTTMNum].reset();
	}
}

} // end namespace Dgds
