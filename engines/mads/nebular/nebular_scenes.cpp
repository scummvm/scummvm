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
#include "mads/nebular/nebular_scenes5.h"
#include "mads/nebular/nebular_scenes6.h"
#include "mads/nebular/nebular_scenes7.h"
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
	// Scene group #1 (ship, ocean, cave)
	case 101:	// Ship, cockpit
		return new Scene101(vm);
	case 102:	// Ship, dining room
		return new Scene102(vm);
	case 103:	// Ship, engine room
		return new Scene103(vm);
	case 104:	// Ocean, northwest cliff
		return new Scene104(vm);
	case 105:	// Ocean, northeast cliff with mine
		return new Scene105(vm);
	case 106:	// Ocean, outside ship
		return new Scene106(vm);
	case 107:	// Ocean, bushes
		return new Scene107(vm);
	case 108:	// Ocean, southwest cliff
		return new Scene108(vm);
	case 109:	// Ocean, tunnel
		return new Scene109(vm);
	case 110:	// Ocean, cave with tunnel
		return new Scene110(vm);
	case 111:	// Cave with pool and opening
		return new Scene111(vm);
	case 112:	// cutscene, looking at view screen
		return new Scene112(vm);

	// Scene group #2 (island)
	case 201:	// outside teleporter
		return new Scene201(vm);
	case 202:	// village
		return new Scene202(vm);
	case 203:	// tree with Rhotunda (fat woman)
		return new Scene203(vm);
	case 205:	// village
		return new Scene205(vm);
	case 207:	// outside witch doctor's hut
		return new Scene207(vm);
	case 208:	// pit with leaves (trap)
		return new Scene208(vm);
	case 209:	// palm tree and bamboo plant
		return new Scene209(vm);
	case 210:	// outside native woman's hut
		return new Scene210(vm);
	case 211:	// palm tree with monkey
		return new Scene211(vm);
	case 212:	// outside cave
		return new Scene212(vm);
	case 213:	// inside teleporter
		return new Scene213(vm);
	case 214:	// inside witch doctor's hut
		return new Scene214(vm);
	case 215:	// inside native woman's hut
		return new Scene215(vm);
	case 216:	// cutscene, monitor showing Rex and native woman
		return new Scene216(vm);

	// Scene group #3 (women's base, cell block)
	case 301:	// outside teleporter (before chaos)
		return new Scene301(vm);
	case 302:	// room with statue (before chaos)
		return new Scene302(vm);
	case 303:	// western corridor (before chaos)
		return new Scene303(vm);
	case 304:	// crossing with traffic light (before chaos)
		return new Scene304(vm);
	case 307:	// Rex's cell (before chaos)
		return new Scene307(vm);
	case 308:	// sauropod's cell (before chaos)
		return new Scene308(vm);
	case 309:	// multihand monster's cell (before chaos)
		return new Scene309(vm);
	case 310:	// empty cell (before chaos)
		return new Scene310(vm);
	case 311:	// warden's desk (before chaos)
		return new Scene311(vm);
	case 313:	// air shaft overview
		return new Scene313(vm);
	case 316:	// Gender Bender
		return new Scene316(vm);
	case 318:	// doctor's gurney
		return new Scene318(vm);
	case 319:	// doctor Slache closeup (lying on the gurney)
		return new Scene319(vm);
	case 320:	// warden's desk closeup / monitors
		return new Scene320(vm);
	case 321:	// gender bender sex change sequence
		return new Scene321(vm);
	case 322:	// inside teleporter
		return new Scene322(vm);
	case 351:	// outside teleporter (after chaos)
		return new Scene351(vm);
	case 352:	// room with statue (after chaos)
		return new Scene352(vm);
	case 353:	// western corridor (after chaos)
		return new Scene353(vm);
	case 354:	// crossing with traffic light (after chaos)
		return new Scene354(vm);
	case 357:	// Rex's cell (after chaos)
		return new Scene357(vm);
	case 358:	// sauropod's cell (after chaos)
		return new Scene358(vm);
	case 359:	// multihand monster's cell (after chaos)
		return new Scene359(vm);
	case 360:	// empty cell (after chaos)
		return new Scene360(vm);
	case 361:	// warden's desk (after chaos)
		return new Scene361(vm);
	case 366:	// air shaft ending at Gender Bender
		return new Scene366(vm);
	case 387:	// air shaft ending at cell
		return new Scene387(vm);
	case 388:	// air shaft ending at sauropod's cell
		return new Scene388(vm);
	case 389:	// air shaft ending at multihand monster's cell (before chaos)
		return new Scene389(vm);
	case 390:	// air shaft ending at cell
		return new Scene390(vm);
	case 391:	// air shaft ending at warden's desk
		return new Scene391(vm);
	case 399:	// air shaft ending at multihand monster's cell (after chaos)
		return new Scene399(vm);

	// Scene group #4 (women's base)
	case 401:	// outside bar
		return new Scene401(vm);
	case 402:	// inside bar
		return new Scene402(vm);
	case 405:	// outside armory
		return new Scene405(vm);
	case 406:	// outside storage room
		return new Scene406(vm);
	case 407:	// eastern corridor
		return new Scene407(vm);
	case 408:	// inside armory
		return new Scene408(vm);
	case 409:	// inside female only teleporter
		return new Scene409(vm);
	case 410:	// inside storage room
		return new Scene410(vm);
	case 411:	// lab
		return new Scene411(vm);
	case 413:	// outside female only teleporter
		return new Scene413(vm);

	// Scene group #5 (men's city, lower floor)
	case 501:	// outside car
		return new Scene501(vm);
	case 502:	// inside male only teleporter
		return new Scene502(vm);
	case 503:	// guard tower
		return new Scene503(vm);
	case 504:	// inside car
		return new Scene504(vm);
	case 505:	// car view screen
		return new Scene505(vm);
	case 506:	// shopping street
		return new Scene506(vm);
	case 507:	// inside software house
		return new Scene507(vm);
	case 508:	// laser cannon
		return new Scene508(vm);
	case 511:	// outside pleasure dome
		return new Scene511(vm);
	case 512:	// inside pleasure dome
		return new Scene512(vm);
	case 513:	// outside mall
		return new Scene513(vm);
	case 515:	// overview
		return new Scene515(vm);
	case 551:	// outside teleporter (with skeleton)
		return new Scene551(vm);

	// Scene group #6 (men's city, upper floor)
	case 601:	// outside Bruce's house
		return new Scene601(vm);
	case 602:	// Bruce's house, living room
		return new Scene602(vm);
	case 603:	// Bruce's house, bedroom
		return new Scene603(vm);
	case 604:	// viewport
		return new Scene604(vm);
	case 605:	// viewport closeup
		return new Scene605(vm);
	case 607:	// outside Abdul's garage
		return new Scene607(vm);
	case 608:	// inside Abdul's garage
		return new Scene608(vm);
	case 609:	// outside Buckluster video store
		return new Scene609(vm);
	case 610:	// inside Buckluster video store
		return new Scene610(vm);
	case 611:	// back alley
		return new Scene611(vm);
	case 612:	// expressway / maintenance building
		return new Scene612(vm);
	case 620:	// cutscene, viewport glass breaking
		return new Scene620(vm);

	// Scene group #7 (submerged men's city / upper floor)
	case 701:	// outside elevator (after city is submerged)
		return new Scene701(vm);
	case 702:	// outside teleporter (after city is submerged)
		return new Scene702(vm);
	case 703:	// water
		return new Scene703(vm);
	case 704:	// water, building in the distance
		return new Scene704(vm);
	case 705:	// water, outside building
		return new Scene705(vm);
	case 706:	// inside building, pedestral room, outside teleporter
		return new Scene706(vm);
	case 707:	// teleporter
		return new Scene707(vm);
	case 710:	// looking at pedestral room through binoculars
		return new Scene710(vm);
	case 711:	// inside teleporter
		return new Scene711(vm);
	case 751:	// outside elevator (before city is submerged)
		return new Scene751(vm);
	case 752:	// outside teleporter (before city is submerged)
		return new Scene752(vm);

	// Scene group #8
	case 801:	// control room, outside teleporter
		return new Scene801(vm);
	case 802:	// launch pad with destroyed ship
		return new Scene802(vm);
	case 803:	// empty launch pad
		return new Scene803(vm);
	case 804:	// inside Rex's ship - cockpit
		return new Scene804(vm);
	case 805:	// service panel
		return new Scene805(vm);
	case 807:	// teleporter
		return new Scene807(vm);
	case 808:	// antigrav control
		return new Scene808(vm);
	case 810:	// cutscene: Rex's ship leaving the planet
		return new Scene810(vm);

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

void SceneInfoNebular::loadCodes(BaseSurface &depthSurface, int variant) {
	File f(Resources::formatName(RESPREFIX_RM, _sceneId, ".DAT"));
	MadsPack codesPack(&f);
	Common::SeekableReadStream *stream = codesPack.getItemStream(variant + 1);

	loadCodes(depthSurface, stream);

	delete stream;
	f.close();
}

void SceneInfoNebular::loadCodes(BaseSurface &depthSurface, Common::SeekableReadStream *stream) {
	byte *destP = (byte *)depthSurface.getPixels();
	byte *endP = (byte *)depthSurface.getBasePtr(0, depthSurface.h);

	byte runLength = stream->readByte();
	while (destP < endP && runLength > 0) {
		byte runValue = stream->readByte();

		// Write out the run length
		Common::fill(destP, MIN(endP, destP + runLength), runValue);
		destP += runLength;

		// Get the next run length
		runLength = stream->readByte();
	}

	if (destP < endP)
		Common::fill(destP, endP, 0);
}

/*------------------------------------------------------------------------*/

SceneTeleporter::SceneTeleporter(MADSEngine *vm) : NebularScene(vm) {
	_buttonTyped = -1;
	_curCode = -1;
	_digitCount = -1;
	_curMessageId = -1;
	_handSpriteId = -1;
	_handSequenceId = -1;
	_finishedCodeCounter = -1;
	_meteorologistNextPlace = -1;
	_meteorologistCurPlace = -1;
	_teleporterSceneId = -1;
}

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
		_handSequenceId = _scene->_sequences.startPingPongCycle(_handSpriteId, false, 4, 2, 0, 0);
		_scene->_sequences.setPosition(_handSequenceId, msgPos);
		_scene->_sequences.setDepth(_handSequenceId, 2);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_LOOP, 0, 1);
		_scene->_sequences.addSubEntry(_handSequenceId, SEQUENCE_TRIGGER_EXPIRE, 0, 2);

		if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
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

				Common::String format = "%01d";
				format.setChar('0' + _digitCount, 2);
				_msgText = Common::String::format(format.c_str(), _curCode);
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
			_curMessageId = _scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, INDEFINITE_TIMEOUT, _msgText);
		}
		break;

	case 2:
		if (_finishedCodeCounter == 1) {
			_finishedCodeCounter++;

			if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
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
		} else if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
			_scene->_sequences.addTimer(30, 230 + _meteorologistCurPlace);

		break;

	case 3:
		if (!_finishedCodeCounter) {
			if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL) {
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
	_game._player._stepEnabled = (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL);
	_scene->_kernelMessages._talkFont = _vm->_font->getFont(FONT_TELE);
	_scene->_textSpacing = 0;
	_curCode   = 0;
	_digitCount = 0;
	_finishedCodeCounter       = 0;
	_curMessageId        = -1;
	_msgText = "_";

	if (_scene->_priorSceneId == RETURNING_FROM_DIALOG)
		_scene->_priorSceneId = _globals[kTeleporterDestination];

	if (_scene->_priorSceneId < 101)
		_scene->_priorSceneId = 201;

	_globals[kTeleporterDestination] = _scene->_priorSceneId;
	_vm->_palette->setEntry(252, 63, 63, 0);
	_vm->_palette->setEntry(253, 0,  0, 0);
	_teleporterSceneId = _scene->_priorSceneId;
	if (_teleporterSceneId == 202)
		_teleporterSceneId = 201;

	int codeVal = 0;
	for (int i = 0; i < 10; i++) {
		if (_teleporterSceneId == _globals[kTeleporterRoom + i])
			codeVal = _globals[kTeleporterCode + i];

		if (_globals[kTeleporterRoom + i] == 301)
			_meteorologistNextPlace = _globals[kTeleporterCode + i];
	}

	Common::String msgText2 = Common::String::format("#%.4d", codeVal);

	if (_scene->_currentSceneId != 711) {
		_scene->_kernelMessages.add(Common::Point(133, 34), 0, 32, 0, INDEFINITE_TIMEOUT, msgText2);
		_scene->_kernelMessages.add(Common::Point(143, 61), 0xFDFC, 16, 0, INDEFINITE_TIMEOUT, _msgText);
	}

	_meteorologistCurPlace = 0;

	if (_globals[kMeteorologistWatch] != METEOROLOGIST_NORMAL)
		_scene->_sequences.addTimer(30, 230);

	_vm->_sound->command(36);
}

bool SceneTeleporter::teleporterActions() {
	bool retVal = false;

	if (_action.isAction(VERB_PRESS) || _action.isAction(VERB_PUSH)) {
		static int _buttonList[12] = { NOUN_0_KEY, NOUN_1_KEY, NOUN_2_KEY, NOUN_3_KEY, NOUN_4_KEY, NOUN_5_KEY, NOUN_6_KEY, NOUN_7_KEY, NOUN_8_KEY, NOUN_9_KEY, NOUN_SMILE_KEY, NOUN_FROWN_KEY };
		for (int i = 0; i < 12; i++) {
			if (_action._activeAction._objectNameId == _buttonList[i])
				_buttonTyped = i;
		}
		teleporterHandleKey();
		retVal = true;
	}

	if (_action.isAction(VERB_EXIT_FROM, NOUN_DEVICE)) {
		_globals[kTeleporterCommand] = 3;
		_scene->_nextSceneId = _teleporterSceneId;
		retVal = true;
	}

	return (retVal);
}

void SceneTeleporter::teleporterStep() {
	if (_globals[kMeteorologistWatch] == METEOROLOGIST_NORMAL)
		return;

	if (_game._trigger >= 230) {
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
		_buttonTyped = digit;
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
