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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/resources.h"
#include "mads/scene.h"
#include "mads/nebular/game_nebular.h"
#include "mads/nebular/nebular_scenes.h"
#include "mads/nebular/nebular_scenes1.h"
#include "mads/nebular/nebular_scenes2.h"
#include "mads/nebular/nebular_scenes3.h"
#include "mads/nebular/nebular_scenes4.h"
#include "mads/nebular/nebular_scenes8.h"

namespace MADS {

namespace Nebular {

SceneLogic *SceneFactory::createScene(MADSEngine *vm) {
	Scene &scene = vm->_game->_scene;

	scene.addActiveVocab(NOUN_DROP);
	scene.addActiveVocab(NOUN_DOLLOP);
	scene.addActiveVocab(NOUN_DASH);
	scene.addActiveVocab(NOUN_SPLASH);
	scene.addActiveVocab(NOUN_ALCOHOL);

	switch (scene._nextSceneId) {
	// Scene group #1
	case 101:
		return new Scene101(vm);
	case 102:
		return new Scene102(vm);
	case 103:
		return new Scene103(vm);
	case 104:
		return new Scene104(vm);
	case 105:
		return new Scene105(vm);
	case 106:
		return new Scene106(vm);
	case 107:
		return new Scene107(vm);
	case 108:
		return new Scene108(vm);
	case 109:
		return new Scene109(vm);
	case 110:
		return new Scene110(vm);
	case 111:
		return new Scene111(vm);
	case 112:
		return new Scene112(vm);

	// Scene group #2
	case 201:
		return new Scene201(vm);
	case 202:
		return new Scene202(vm);
	case 203:
		return new Scene203(vm);
	case 205:
		return new Scene205(vm);
	case 207:
		return new Scene207(vm);
	case 208:
		return new Scene208(vm);
	case 209:
		return new Scene209(vm);
	case 210:
		return new Scene210(vm);
	case 211:
		return new Scene211(vm);
	case 212:
		return new Scene212(vm);
	case 213:
		return new Scene213(vm);
	case 214:
		return new Scene214(vm);
	case 215:
		return new Scene215(vm);
	case 216:
		return new Scene216(vm);

	// Scene group #3
	case 301:
		return new Scene301(vm);
	case 302:
		return new Scene302(vm);
	case 303:
		return new Scene303(vm);
	case 304:
		return new Scene304(vm);
	case 307:
		return new Scene307(vm);
	case 308:
		return new Scene308(vm);
	case 309:
		return new Scene309(vm);
	case 310:
		return new Scene310(vm);
	case 311:
		return new Scene311(vm);
	case 313:
		return new Scene313(vm);
	case 316:
		return new Scene316(vm);
	case 318:
		return new Scene318(vm);
	case 319:
		return new Scene319(vm);
	case 320:
		return new Scene320(vm);
	case 321:
		return new Scene321(vm);
	case 322:
		return new Scene322(vm);
	case 351:
		return new Scene351(vm);
	case 352:
		return new Scene352(vm);
	case 353:
		return new Scene353(vm);
	case 354:
		return new Scene354(vm);
	case 357:
		return new Scene357(vm);
	case 358:
		return new Scene358(vm);
	case 359:
		return new Scene359(vm);
	case 360:
		return new Scene360(vm);
	case 361:
		return new Scene361(vm);
	case 366:
		return new Scene366(vm);
	case 387:
		return new Scene387(vm);
	case 388:
		return new Scene388(vm);
	case 389:
		return new Scene389(vm);
	case 390:
		return new Scene390(vm);
	case 391:
		return new Scene391(vm);
	case 399:
		return new Scene399(vm);

	// Scene group #4
	case 401:
		return new Scene401(vm);
	case 402:
		return new Scene402(vm);
	case 405:
		return new Scene405(vm);
	case 406:
		return new Scene406(vm);
	case 407:
		return new Scene407(vm);
	case 408:
		return new Scene408(vm);
	case 409:
		return new Scene409(vm);
	case 410:
		return new Scene410(vm);
	case 411:
		return new Scene411(vm);
	case 413:
		return new Scene413(vm);

	// Scene group #5
	case 502:
		return new Scene502(vm);
	case 501:
		// TODO
	case 503:
		// TODO
	case 504:
		// TODO
	case 505:
		// TODO
	case 506:
		// TODO
	case 507:
		// TODO
	case 508:
		// TODO
	case 511:
		// TODO
	case 512:
		// TODO
	case 513:
		// TODO
	case 515:
		// TODO
	case 551:
		// TODO
		// Fall-through for unimplemented scenes in group #5
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #6
	case 601:
		// TODO
	case 602:
		// TODO
	case 603:
		// TODO
	case 604:
		// TODO
	case 605:
		// TODO
	case 607:
		// TODO
	case 608:
		// TODO
	case 609:
		// TODO
	case 610:
		// TODO
	case 611:
		// TODO
	case 612:
		// TODO
	case 620:
		// TODO
		// Fall-through for unimplemented scenes in group #6
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #7
	case 701:
		// TODO
	case 702:
		// TODO
	case 703:
		// TODO
	case 704:
		// TODO
	case 705:
		// TODO
	case 706:
		// TODO
	case 707:
		// TODO
	case 708:
		// TODO
	case 710:
		// TODO
	case 711:
		// TODO
	case 751:
		// TODO
	case 752:
		// TODO
		// Fall-through for unimplemented scenes in group #7
		error("Invalid scene %d called", scene._nextSceneId);

	// Scene group #8
	case 801:
		// TODO
	case 802:
		// TODO
	case 803:
		// TODO
		// Fall-through for unimplemented scenes in group #8
		error("Invalid scene %d called", scene._nextSceneId);
	case 804:
		return new Scene804(vm);
	case 805:
		// TODO
	case 806:
		// TODO
	case 807:
		// TODO
	case 808:
		// TODO
	case 810:
		// TODO
		// Fall-through for unimplemented scenes in group #8
		error("Invalid scene %d called", scene._nextSceneId);

	default:
		error("Invalid scene %d called", scene._nextSceneId);
	}
}

/*------------------------------------------------------------------------*/

NebularScene::NebularScene(MADSEngine *vm) : SceneLogic(vm),
		_globals(static_cast<GameNebular *>(vm->_game)->_globals),
		_game(*static_cast<GameNebular *>(vm->_game)),
		_action(vm->_game->_scene._action) {
}

Common::String NebularScene::formAnimName(char sepChar, int suffixNum) {
	return Resources::formatName(_scene->_currentSceneId, sepChar, suffixNum,
		EXT_NONE, "");
}

/*------------------------------------------------------------------------*/

void SceneInfoNebular::loadCodes(MSurface &depthSurface, int variant) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(variant + 1);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoNebular::loadCodes(MSurface &depthSurface, Common::SeekableReadStream *stream) {
	byte *destP = depthSurface.getData();
	byte *endP = depthSurface.getBasePtr(0, depthSurface.h);

	byte runLength = stream->readByte();
	while (destP < endP && runLength > 0) {
		byte runValue = stream->readByte();

		// Write out the run length
		Common::fill(destP, destP + runLength, runValue);
		destP += runLength;

		// Get the next run length
		runLength = stream->readByte();
	}

	if (destP < endP)
		Common::fill(destP, endP, 0);
}

/*------------------------------------------------------------------------*/

int SceneTeleporter::teleporterAddress(int code, bool working) {
	int limit = working ? 6 : 10;

	for (int i = 0; i < limit; i++) {
		if (code == _globals[kTeleporterCode + i])
			return _globals[kTeleporterRoom + i];
	}

	return -1;
}

Common::Point SceneTeleporter::teleporterComputeLocation() {
	Common::Point result;

	switch (_buttonTyped) {
	case 0:
		result = Common::Point(179, 200);
		break;

	case 1:
		result = Common::Point(166, 170);
		break;

	case 2:
		result = Common::Point(179, 170);
		break;

	case 3:
		result = Common::Point(192, 170);
		break;

	case 4:
		result = Common::Point(166, 180);
		break;

	case 5:
		result = Common::Point(179, 180);
		break;

	case 6:
		result = Common::Point(192, 180);
		break;

	case 7:
		result = Common::Point(166, 190);
		break;

	case 8:
		result = Common::Point(179, 190);
		break;

	case 9:
		result = Common::Point(192, 190);
		break;

	case 10:
		result = Common::Point(194, 200);
		break;

	case 11:
		result = Common::Point(164, 200);
		break;

	default:
		error("teleporterComputeLocation() - Unexpected button pressed");
	}

	return result;
}

void SceneTeleporter::teleporterHandleKey() {
	switch (_game._trigger) {
	case 0: {
		_game._player._stepEnabled = false;
		Common::Point msgPos = teleporterComputeLocation();
		_handSequenceId = _scene->_sequences.startReverseCycle(_handSpriteId, false, 4, 2, 0, 0);
		_scene->_sequences.setMsgPosition(_handSequenceId, msgPos);
		_scene->_sequences.setDepth(_handSequenceId, 2);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_LOOP, 0, 1);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 2);

		if (_globals[kMeteorologistWatch] == 0)
			_vm->_events->hideCursor();

		}
		break;

	case 1:
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_SPRITE, 3, 3);
		if (_buttonTyped <= 9) {
			if (_digitCount < 4) {
				_curCode *= 10;
				_curCode += _buttonTyped;
				_digitCount++;
				_msgText = "";
				_msgText.format("%d", _curCode);
				if (_digitCount < 4)
					_msgText += "_";

				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(32);
			}
		} else if (_buttonTyped == 11) {
			_digitCount = 0;
			_curCode = 0;
			_msgText = "_";
			if (_scene->_currentSceneId != 711)
				_vm->_sound->command(33);
		} else if (_digitCount == 4) {
			if (_scene->_currentSceneId != 711)
				_finishedCodeCounter = 1;

			if (teleporterAddress(_curCode, true) > 0) {
				_vm->_palette->setEntry(252, 0, 63, 0);
				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(34);
			} else {
				_vm->_palette->setEntry(252, 63, 0, 0);
				if (_scene->_currentSceneId != 711)
					_vm->_sound->command(35);
			}
		}

		if (_scene->_currentSceneId != 711) {
			if (_curMessageId >= 0)
				_scene->_kernelMessages.remove(_curMessageId);
			_curMessageId = _scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, 9999999, _msgText);
		}
		break;

	case 2:
		if (_finishedCodeCounter == 1) {
			_finishedCodeCounter++;

			if (_globals[kMeteorologistWatch] != 0)
				_scene->_nextSceneId = 202;
			else {
				_vm->_events->showCursor();
				int destination = teleporterAddress(_curCode, true);

				if (destination > 0) {
					_globals[kTeleporterCommand] = 2;
					_scene->_nextSceneId = _teleporterSceneId;
					_globals[kTeleporterDestination] = destination;
				} else {
					_globals[kTeleporterCommand] = 4;
					_scene->_nextSceneId = _teleporterSceneId;
				}
			}
		} else if (_globals[kMeteorologistWatch] != 0)
			_scene->_sequences.addTimer(30, 230 + _meteorologistCurPlace);

		break;

	case 3:
		if (!_finishedCodeCounter) {
			if (_globals[kMeteorologistWatch] == 0) {
				_game._player._stepEnabled = true;
				_vm->_events->showCursor();
			}
		}
		break;

	default:
		break;
	}
}

void SceneTeleporter::teleporterEnter() {
	_game._player._visible   = false;
	_game._player._stepEnabled = (_globals[kMeteorologistWatch] == 0);
	_scene->_kernelMessages._talkFont = _vm->_font->getFont(FONT_TELE);
	_scene->_textSpacing = 0;
	_curCode   = 0;
	_digitCount = 0;
	_finishedCodeCounter       = 0;
	_curMessageId        = -1;
	_msgText = "_";

	if (_scene->_priorSceneId == -2)
		_scene->_priorSceneId = _globals[kTeleporterDestination];

	if (_scene->_priorSceneId < 101)
		_scene->_priorSceneId = 201;

	_globals[kTeleporterDestination] = _scene->_priorSceneId;
	_vm->_palette->setEntry(252, 63, 63, 0);
	_vm->_palette->setEntry(253, 0,  0, 0);
	_teleporterSceneId = _scene->_priorSceneId;
	if (_teleporterSceneId == 202)
		_teleporterSceneId = 201;

	int tmpVal = 0;
	for (int i = 0; i < 10; i++) {
		if (_teleporterSceneId == _globals[kTeleporterRoom + i])
			tmpVal = _globals[kTeleporterRoom + i];

		if (_globals[kTeleporterRoom + i] == 301)
			_meteorologistNextPlace = _globals[kTeleporterCode + i];
	}

	Common::String msgText2 = Common::String::format("#%d", tmpVal);

	if (_scene->_currentSceneId != 711) {
		_scene->_kernelMessages.add(Common::Point(133, 34), 0, 32, 0, 9999999, msgText2);
		_scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, 9999999, _msgText);
	}

	_meteorologistCurPlace = 0;

	if (_globals[kMeteorologistWatch] != 0)
		_scene->_sequences.addTimer(30, 230);

	_vm->_sound->command(36);
}

bool SceneTeleporter::teleporterActions() {
	bool retVal = false;
	static int _buttonList[12] = { 0x1D0, 0x1D1, 0x1D2, 0x1D3, 0x1D4, 0x1D5, 0x1D6, 0x1D7, 0x1D8, 0x1D9, 0x1DB, 0x1DA };

	if (_action.isAction(0x11A) || _action.isAction(VERB_PUSH)) {
		for (int i = 0; i < 12; i++) {
			if (_action._activeAction._objectNameId == _buttonList[i])
				_buttonTyped = i;
		}
		teleporterHandleKey();
		retVal = true;
	}

	if (_action.isAction(0x1CE, 0x1CF)) {
		_globals[kTeleporterCommand] = 3;
		_scene->_nextSceneId = _teleporterSceneId;
		retVal = true;
	}

	return (retVal);
}

void SceneTeleporter::teleporterStep() {
	if ((_globals[kMeteorologistWatch] != 0) && (_game._trigger >= 230)) {
		int place = _game._trigger - 230;
		int digit;

		if (place < 4) {
			digit = _meteorologistNextPlace;
			for (int i = 0; i < (3 - place); i++)
				digit = digit / 10;

			digit   = digit % 10;
		} else {
			digit   = 10;
		}
		_buttonTyped              = digit;
		_meteorologistCurPlace = place + 1;
		_game._trigger = -1;
	}

	if (_game._trigger) {
		if (_game._trigger == -1)
			_game._trigger = 0;
		teleporterHandleKey();
	}
}

} // End of namespace Nebular

} // End of namespace MADS
