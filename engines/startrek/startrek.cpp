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

#include "base/plugins.h"
#include "base/version.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/macresman.h"

#include "graphics/pixelformat.h"
#include "graphics/surface.h"

#include "engines/util.h"
#include "video/qt_decoder.h"

#include "startrek/filestream.h"
#include "startrek/iwfile.h"
#include "startrek/lzss.h"
#include "startrek/room.h"
#include "startrek/startrek.h"

namespace StarTrek {

StarTrekEngine::StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc) :
	Engine(syst),
	_gameDescription(gamedesc),
	_randomSource("Star Trek"),
	_kirkActor(&_actorList[0]),
	_spockActor(&_actorList[1]),
	_mccoyActor(&_actorList[2]),
	_redshirtActor(&_actorList[3]) {

	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics");
	DebugMan.addDebugChannel(kDebugSavegame, "savegame", "Savegames");

	_gfx = nullptr;
	_sound = nullptr;
	_macResFork = nullptr;

	memset(_actionOnWalkCompletionInUse, 0, sizeof(_actionOnWalkCompletionInUse));

	_clockTicks = 0;

	_musicEnabled = true;
	_sfxEnabled = true;
	_word_467a6 = true;
	_musicWorking = true;
	_sfxWorking = true;
	_finishedPlayingSpeech = false;

	_lookActionBitmapIndex = 0;

	_mouseControllingShip = false;
	_keyboardControlsMouse = true;

	_inQuitGameMenu = false;
	_textDisplayMode = TEXTDISPLAY_SUBTITLES;
	_textboxVar2 = 0;
	_textboxVar6 = 0;
	_textboxHasMultipleChoices = false;

	_missionToLoad = "MUDD";
	_roomIndexToLoad = 0;

	for (int i = 0; i < NUM_OBJECTS; i++)
		_itemList[i] = g_itemList[i];
}

StarTrekEngine::~StarTrekEngine() {
	delete _gfx;
	delete _sound;
	delete _macResFork;
}

Common::Error StarTrekEngine::run() {
	_gfx = new Graphics(this);
	_sound = new Sound(this);

	if (getPlatform() == Common::kPlatformMacintosh) {
		_macResFork = new Common::MacResManager();
		if (!_macResFork->open("Star Trek Data"))
			error("Could not load Star Trek Data");
		assert(_macResFork->hasDataFork() && _macResFork->hasResFork());
	}

	const ::Graphics::PixelFormat format = ::Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, &format);
	initializeEventsAndMouse();

	_frameIndex = 0;

	_gameMode = -1;
	_lastGameMode = -1;

	runGameMode(GAMEMODE_AWAYMISSION);
	return Common::kNoError;
}

Common::Error StarTrekEngine::runGameMode(int mode) {
	_gameMode = mode;

	_sound->stopAllVocSounds();
	if (!_sound->_loopingAudioName.empty())
		_sound->playVoc(_sound->_loopingAudioName);

	if (_gameMode == GAMEMODE_START)
		_gameMode = GAMEMODE_BRIDGE;

	while (true) {
		TrekEvent event;

		if (_gameMode != _lastGameMode) {
			// Cleanup previous game mode
			switch (_lastGameMode) {
			case GAMEMODE_BRIDGE:
				//cleanupBridge();
				break;

			case GAMEMODE_AWAYMISSION:
				cleanupAwayMission();
				break;

			case GAMEMODE_BEAMDOWN:
			case GAMEMODE_BEAMUP:
				break;
			}

			_lastGameMode = _gameMode;

			// Load next game mode
			switch (_gameMode) {
			case GAMEMODE_BRIDGE:
				_sound->loadMusicFile("bridge");
				//initBridge();
				break;

			case GAMEMODE_AWAYMISSION:
				initAwayMission();
				break;

			case GAMEMODE_BEAMDOWN:
				_awayMission.redshirtDead = false;
				_sound->loadMusicFile("ground");
				runTransportSequence("teled");
				_gameMode = GAMEMODE_AWAYMISSION;
				continue; // Back to start of loop

			case GAMEMODE_BEAMUP:
				runTransportSequence("teleb");
				_gameMode = GAMEMODE_BRIDGE;
				//sub_15c61();
				_sound->stopAllVocSounds();
				_sound->playVoc("bridloop");
				continue; // Back to start of loop
			}
		}

		// Run current game mode
		switch (_gameMode) {
		case GAMEMODE_BRIDGE:
			popNextEvent(&event);
			//runBridge();
			break;

		case GAMEMODE_AWAYMISSION:
			runAwayMission();
			break;

		case GAMEMODE_BEAMDOWN:
		case GAMEMODE_BEAMUP:
			error("Can't be here.");
			break;
		}
	}

	return Common::kNoError;
}

void StarTrekEngine::runTransportSequence(const Common::String &name) {
	const uint16 crewmanTransportPositions[][2] = {
		{ 0x8e, 0x7c },
		{ 0xbe, 0x7c },
		{ 0x7e, 0x72 },
		{ 0xaa, 0x72 }
	};

	_sound->stopAllVocSounds();
	_gfx->fadeoutScreen();
	actorFunc1();
	initActors();

	SharedPtr<Bitmap> bgImage = _gfx->loadBitmap("transprt");
	_gfx->setBackgroundImage(bgImage);
	_gfx->clearPri();
	_gfx->loadPalette("palette");
	_gfx->drawDirectToScreen(bgImage);
	_system->updateScreen();

	for (int i = 0; i < (_awayMission.redshirtDead ? 3 : 4); i++) {
		Common::String filename = getCrewmanAnimFilename(i, name);
		int x = crewmanTransportPositions[i][0];
		int y = crewmanTransportPositions[i][1];
		loadActorAnim(i, filename, x, y, 256);
		_actorList[i].animationString[0] = '\0';
	}

	if (_missionToLoad.equalsIgnoreCase("feather") && name[4] == 'b') {
		loadActorAnim(9, "qteleb", 0x61, 0x79, 0x100);
	}
	else if (_missionToLoad.equalsIgnoreCase("trial")) {
		if (name[4] == 'd') {
			loadActorAnim(9, "qteled", 0x61, 0x79, 0x100);
		}
		/* TODO
		else if (word_51156 >= 3) {
			loadActorAnim(9, "qteleb", 0x61, 0x79, 0x100);
		}
		*/
	}

	loadActorAnim(8, "transc", 0, 0, 0x100);

	// TODO: redraw mouse and sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeinScreen();

	playSoundEffectIndex(0x0a);

	if (name.equalsIgnoreCase("teled"))
		playSoundEffectIndex(0x08);
	else
		playSoundEffectIndex(0x09);

	while (_actorList[0].field62 == 0) {
		TrekEvent event;
		if (popNextEvent(&event)) {
			if (event.type == TREKEVENT_TICK) {
				// TODO: redraw sprite_52c4e?
				_frameIndex++;
				updateActorAnimations();
				_gfx->drawAllSprites();
			}
		}
	}

	// TODO: redraw sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeoutScreen();
	actorFunc1();
	initActors();
}

void StarTrekEngine::playSoundEffectIndex(int index) {
	switch (index) {
	case 0x04:
		_sound->playVoc("tricorde");
		break;
	case 0x05:
		_sound->playVoc("STDOOR1");
		break;
	case 0x06:
		_sound->playVoc("PHASSHOT");
		break;
	case 0x07:
		_sound->playMidiTrack(index);
		break;
	case 0x08:
		_sound->playVoc("TRANSDEM");
		break;
	case 0x09: // Beaming in?
		_sound->playVoc("TRANSMAT");
		break;
	case 0x0a: // Beaming out?
		_sound->playVoc("TRANSENE");
		break;
	case 0x10: // Menu selection sound
		_sound->playMidiTrack(index);
		break;
	case 0x22:
		_sound->playVoc("HAILING");
		break;
	case 0x24:
		_sound->playVoc("PHASSHOT");
		break;
	case 0x25:
		_sound->playVoc("PHOTSHOT");
		break;
	case 0x26:
		_sound->playVoc("HITSHIEL");
		break;
	case 0x27:
		_sound->playMidiTrack(index);
		break;
	case 0x28:
		_sound->playVoc("REDALERT");
		break;
	case 0x29:
		_sound->playVoc("WARP");
		break;
	default:
	    break;
	}
}

void StarTrekEngine::playMidiMusicTracks(int startTrack, int loopTrack) {
	_sound->playMidiMusicTracks(startTrack, loopTrack);
}

void StarTrekEngine::playSpeech(const Common::String &filename) {
	_sound->playSpeech(filename.c_str());
}

void StarTrekEngine::stopPlayingSpeech() {
	_sound->stopPlayingSpeech();
}

void StarTrekEngine::initActors() {
	for (int i = 0; i < NUM_ACTORS; i++) {
		_actorList[i] = Actor();
	}
	for (int i = 0; i < NUM_ACTORS / 2; i++)
		_actorBanFiles[i].reset();

	strcpy(_kirkActor->animationString, "kstnd");
	strcpy(_spockActor->animationString, "sstnd");
	strcpy(_mccoyActor->animationString, "mstnd");
	strcpy(_redshirtActor->animationString, "rstnd");
}

/**
 * Set an actor's animation, position, and scale.
 */
int StarTrekEngine::loadActorAnim(int actorIndex, const Common::String &animName, int16 x, int16 y, Fixed16 scale) {
	debugC(6, kDebugGraphics, "Load animation '%s' on actor %d", animName.c_str(), actorIndex);

	Actor *actor;

	if (actorIndex == -1) {
		// TODO
		warning("loadActorAnim: actor == -1");
	}
	else
		actor = &_actorList[actorIndex];

	if (actor->spriteDrawn) {
		releaseAnim(actor);
		drawActorToScreen(actor, animName, x, y, scale, false);
	}
	else {
		drawActorToScreen(actor, animName, x, y, scale, true);
	}

	actor->triggerActionWhenAnimFinished = false;
	actor->finishedAnimActionParam = 0;

	return actorIndex;
}

/**
 * Tries to make an actor walk to a position.
 * Returns true if successful in initiating the walk.
 */
bool StarTrekEngine::actorWalkToPosition(int actorIndex, const Common::String &animFile, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	debugC(6, "Obj %d: walk from (%d,%d) to (%d,%d)", actorIndex, srcX, srcY, destX, destY);

	Actor *actor = &_actorList[actorIndex];

	actor->triggerActionWhenAnimFinished = false;
	if (isPositionSolid(destX, destY))
		return false;

	if (actor->spriteDrawn)
		releaseAnim(actor);
	else
		_gfx->addSprite(&actor->sprite);

	actor->spriteDrawn = true;
	actor->animType = 1;
	actor->frameToStartNextAnim = _frameIndex + 1;
	strcpy(actor->animationString2, animFile.c_str());

	actor->dest.x = destX;
	actor->dest.y = destY;
	actor->field92 = 0;
	actor->triggerActionWhenAnimFinished = false;

	actor->iwDestPosition = -1;
	actor->iwSrcPosition = -1;

	if (directPathExists(srcX, srcY, destX, destY)) {
		chooseActorDirectionForWalking(actor, srcX, srcY, destX, destY);
		updateActorPositionWhileWalking(actor, (actor->granularPosX + 0x8000) >> 16, (actor->granularPosY + 0x8000) >> 16);
		return true;
	}
	else {
		actor->iwSrcPosition = _iwFile->getClosestKeyPosition(srcX, srcY);
		actor->iwDestPosition = _iwFile->getClosestKeyPosition(destX, destY);

		if (actor->iwSrcPosition == -1 || actor->iwDestPosition == -1) {
			// No path exists; face south by default.
			strcat(actor->animationString2, "S");
			actor->direction = 'S';

			updateActorPositionWhileWalking(actor, srcX, srcY);
			initStandAnim(actorIndex);

			return false;
		}
		else {
			Common::Point iwSrc = _iwFile->_keyPositions[actor->iwSrcPosition];
			chooseActorDirectionForWalking(actor, srcX, srcY, iwSrc.x, iwSrc.y);
			updateActorPositionWhileWalking(actor, (actor->granularPosX + 0x8000) >> 16, (actor->granularPosY + 0x8000) >> 16);
			return true;
		}
	}
}

void StarTrekEngine::updateActorAnimations() {
	for (int i = 0; i < NUM_ACTORS; i++) {
		Actor *actor = &_actorList[i];
		if (!actor->spriteDrawn)
			continue;

		switch (actor->animType) {
		case 0: // Not walking?
		case 2:
			if (_frameIndex >= actor->frameToStartNextAnim) {
				int nextAnimIndex = getRandomWord() & 3;
				actor->animFile->seek(18 + nextAnimIndex + actor->animFrame * 22, SEEK_SET);
				byte nextAnimFrame = actor->animFile->readByte();

				if (actor->animFrame != nextAnimFrame) {
					if (nextAnimFrame == actor->numAnimFrames - 1) {
						actor->field62++;
						if (actor->triggerActionWhenAnimFinished) {
							addAction(Action(ACTION_FINISHED_ANIMATION, actor->finishedAnimActionParam, 0, 0));
						}
					}
				}

				actor->animFrame = nextAnimFrame;
				if (actor->animFrame >= actor->numAnimFrames) {
					if (actor->animationString[0] == '\0')
						removeActorFromScreen(i);
					else
						initStandAnim(i);
				}
				else {
					Sprite *sprite = &actor->sprite;

					actor->animFile->seek(actor->animFrame * 22, SEEK_SET);
					char animFrameFilename[16];
					actor->animFile->read(animFrameFilename, 16);
					sprite->setBitmap(loadAnimationFrame(animFrameFilename, actor->scale));

					memset(actor->bitmapFilename, 0, 10);
					strncpy(actor->bitmapFilename, animFrameFilename, 9);

					actor->animFile->seek(10 + actor->animFrame * 22, SEEK_SET);
					uint16 xOffset = actor->animFile->readUint16();
					uint16 yOffset = actor->animFile->readUint16();
					uint16 basePriority = actor->animFile->readUint16();
					uint16 frames = actor->animFile->readUint16();

					sprite->pos.x = xOffset + actor->pos.x;
					sprite->pos.y = yOffset + actor->pos.y;
					sprite->drawPriority = _gfx->getPriValue(0, yOffset + actor->pos.y) + basePriority;
					sprite->bitmapChanged = true;

					actor->frameToStartNextAnim = frames + _frameIndex;
				}
			}
			break;
		case 1: // Walking
			if (_frameIndex < actor->frameToStartNextAnim)
				break;
			if (i == 0) // Kirk only
				checkTouchedLoadingZone(actor->pos.x, actor->pos.y);
			if (actor->field90 != 0) {
				Sprite *sprite = &actor->sprite;
				int loops;
				if (getActorScaleAtPosition((actor->granularPosY + 0x8000) >> 16) < 0xa0)
					loops = 1;
				else
					loops = 2;
				for (int k = 0; k < loops; k++) {
					if (actor->field90 == 0)
						break;
					actor->field90--;
					uint32 newX = actor->granularPosX + actor->speedX;
					uint32 newY = actor->granularPosY + actor->speedY;
					if ((actor->field90 & 3) == 0) {
						sprite->bitmap.reset();
						updateActorPositionWhileWalking(actor, (newX + 0x8000) >> 16, (newY + 0x8000) >> 16);
						actor->field92++;
					}

					actor->granularPosX = newX;
					actor->granularPosY = newY;
					actor->frameToStartNextAnim = _frameIndex;
				}
			}
			else { // actor->field90 == 0
				if (actor->iwSrcPosition == -1) {
					if (actor->triggerActionWhenAnimFinished) {
						actor->triggerActionWhenAnimFinished = false;
						addAction(Action(ACTION_FINISHED_WALKING, actor->finishedAnimActionParam & 0xff, 0, 0));
					}

					actor->sprite.bitmap.reset();
					updateActorPositionWhileWalking(actor, (actor->granularPosX + 0x8000) >> 16, (actor->granularPosY + 0x8000) >> 16);
					initStandAnim(i);
				}
				else { // actor->iwSrcPosition != -1
					if (actor->iwSrcPosition == actor->iwDestPosition) {
						actor->animationString2[strlen(actor->animationString2) - 1] = '\0';
						actor->iwDestPosition = -1;
						actor->iwSrcPosition = -1;
						chooseActorDirectionForWalking(actor, actor->pos.x, actor->pos.y, actor->dest.x, actor->dest.y);
					}
					else {
						int index = _iwFile->_iwEntries[actor->iwSrcPosition][actor->iwDestPosition];
						actor->iwSrcPosition = index;
						Common::Point dest = _iwFile->_keyPositions[actor->iwSrcPosition];
						actor->animationString2[strlen(actor->animationString2) - 1] = '\0';
						chooseActorDirectionForWalking(actor, actor->pos.x, actor->pos.y, dest.x, dest.y);
					}
				}
			}
			break;
		default:
			error("Invalid anim type.");
			break;
		}
	}
}

void StarTrekEngine::removeActorFromScreen(int actorIndex) {
	Actor *actor = &_actorList[actorIndex];

	if (actor->spriteDrawn != 1)
		return;

	debugC(6, kDebugGraphics, "Stop drawing actor %d", actorIndex);

	Sprite *sprite = &actor->sprite;
	sprite->field16 = true;
	sprite->bitmapChanged = true;
	_gfx->drawAllSprites();
	_gfx->delSprite(sprite);
	releaseAnim(actor);
}

void StarTrekEngine::actorFunc1() {
	for (int i = 0; i < NUM_ACTORS; i++) {
		if (_actorList[i].spriteDrawn == 1) {
			removeActorFromScreen(i);
		}
	}

	for (int i = 0; i < NUM_ACTORS / 2; i++) {
		_actorBanFiles[i].reset();
	}
}

void StarTrekEngine::drawActorToScreen(Actor *actor, const Common::String &_animName, int16 x, int16 y, Fixed16 scale, bool addSprite) {
	Common::String animFilename = _animName;
	if (_animName.hasPrefixIgnoreCase("stnd") /* && word_45d20 == -1 */) // TODO
		animFilename += 'j';
	memcpy(actor->animFilename, _animName.c_str(), sizeof(actor->animFilename));

	actor->animType = 2;
	actor->animFile = loadFile(animFilename + ".anm");
	actor->numAnimFrames = actor->animFile->size() / 22;
	actor->animFrame = 0;
	actor->pos.x = x;
	actor->pos.y = y;
	actor->field62 = 0;
	actor->scale = scale;

	actor->animFile->seek(16, SEEK_SET);
	actor->frameToStartNextAnim = actor->animFile->readUint16() + _frameIndex;

	char firstFrameFilename[10];
	actor->animFile->seek(0, SEEK_SET);
	actor->animFile->read(firstFrameFilename, 10);

	Sprite *sprite = &actor->sprite;
	if (addSprite)
		_gfx->addSprite(sprite);

	sprite->setBitmap(loadAnimationFrame(firstFrameFilename, scale));
	memset(actor->bitmapFilename, 0, sizeof(char) * 10);
	strncpy(actor->bitmapFilename, firstFrameFilename, sizeof(char) * 9);

	actor->scale = scale;

	actor->animFile->seek(10, SEEK_SET);
	uint16 xOffset = actor->animFile->readUint16();
	uint16 yOffset = actor->animFile->readUint16();
	uint16 basePriority = actor->animFile->readUint16();

	sprite->pos.x = xOffset + actor->pos.x;
	sprite->pos.y = yOffset + actor->pos.y;
	sprite->drawPriority = _gfx->getPriValue(0, yOffset + actor->pos.y) + basePriority;
	sprite->bitmapChanged = true;

	actor->spriteDrawn = 1;
}

void StarTrekEngine::releaseAnim(Actor *actor) {
	switch (actor->animType) {
	case 0:
	case 2:
		actor->sprite.bitmap.reset();
		actor->animFile.reset();
		break;
	case 1:
		actor->sprite.bitmap.reset();
		break;
	default:
		error("Invalid anim type");
		break;
	}

	actor->spriteDrawn = 0;
}

void StarTrekEngine::initStandAnim(int actorIndex) {
	Actor *actor = &_actorList[actorIndex];

	if (!actor->spriteDrawn)
		error("initStandAnim: dead anim");

	////////////////////
	// sub_239d2
	const char *directions = "nsew";

	if (actorIndex >= 0 && actorIndex <= 3) {
		int8 dir = _awayMission.crewDirectionsAfterWalk[actorIndex];
		if (dir != -1) {
			actor->direction = directions[dir];
			_awayMission.crewDirectionsAfterWalk[actorIndex] = -1;
		}
	}
	// end of sub_239d2
	////////////////////

	Common::String animName;
	if (actor->direction != 0)
		animName = Common::String(actor->animationString) + (char)actor->direction;
	else // Default to facing south
		animName = Common::String(actor->animationString) + 's';

	uint16 scale = getActorScaleAtPosition(actor->pos.y);
	loadActorAnim(actorIndex, animName, actor->pos.x, actor->pos.y, scale);
	actor->animType = 0;
}

void StarTrekEngine::updateActorPositionWhileWalking(Actor *actor, int16 x, int16 y) {
	actor->scale = getActorScaleAtPosition(y);
	Common::String animName = Common::String::format("%s%02d", actor->animationString2, actor->field92 & 7);
	actor->sprite.setBitmap(loadAnimationFrame(animName, actor->scale));

	memset(actor->bitmapFilename, 0, 10);
	strncpy(actor->bitmapFilename, animName.c_str(), 9);

	Sprite *sprite = &actor->sprite;
	sprite->drawPriority = _gfx->getPriValue(0, y);
	sprite->pos.x = x;
	sprite->pos.y = y;
	sprite->bitmapChanged = true;

	actor->frameToStartNextAnim = _frameIndex;
	actor->pos.x = x;
	actor->pos.y = y;
}

/**
 * Chooses a value for the actor's speed and direction, based on a source position and
 * a destination position it's walking to.
 */
void StarTrekEngine::chooseActorDirectionForWalking(Actor *actor, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	actor->granularPosX = srcX << 16;
	actor->granularPosY = srcY << 16;

	int16 distX = destX - srcX;
	int16 distY = destY - srcY;
	int16 absDistX = abs(distX);
	int16 absDistY = abs(distY);

	if (absDistX > absDistY) {
		char d;
		if (distX > 0)
			d = 'E';
		else
			d = 'W';

		// Append direction to animation string
		actor->animationString2[strlen(actor->animationString2) + 1] = '\0';
		actor->animationString2[strlen(actor->animationString2)] = d;

		actor->direction = d;
		actor->field90 = absDistX;

		if (distX != 0) {
			if (distX > 0)
				actor->speedX = 1 << 16;
			else
				actor->speedX = -1 << 16; // 0xffff0000

			actor->speedY = (distY << 16) / absDistX;
		}
	}
	else {
		char d;
		if (distY > 0)
			d = 'S';
		else
			d = 'N';

		// Append direction to animation string
		actor->animationString2[strlen(actor->animationString2) + 1] = '\0';
		actor->animationString2[strlen(actor->animationString2)] = d;

		actor->direction = d;
		actor->field90 = absDistY;

		if (distY != 0) {
			if (distY > 0)
				actor->speedY = 1 << 16;
			else
				actor->speedY = -1 << 16; // 0xffff0000

			actor->speedX = (distX << 16) / absDistY;
		}
	}
}

/**
 * Returns true if an actor can walk directly from a source position to a destination
 * position without running into unwalkable terrain.
 */
bool StarTrekEngine::directPathExists(int16 srcX, int16 srcY, int16 destX, int16 destY) {
	int32 distX = destX - srcX;
	int32 distY = destY - srcY;

	int32 absDistX = abs(distX);
	int32 absDistY = abs(distY);

	int32 distCounter;
	Fixed32 speedX, speedY;

	if (absDistX > absDistY) {
		distCounter = absDistX;

		if (distCounter == 0)
			return true;

		speedY = (distY << 16) / absDistX;

		if (distX > 0)
			speedX = 1 << 16;
		else
			speedX = -1 << 16;
	}
	else { // absDistX <= absDistY
		distCounter = absDistY;

		if (distCounter == 0)
			return true;

		speedX = (distX << 16) / absDistY;

		if (distY > 0)
			speedY = 1 << 16;
		else
			speedY = -1 << 16;
	}

	Fixed32 fixedX = srcX << 16;
	Fixed32 fixedY = srcY << 16;

	if (isPositionSolid((fixedX + 0x8000) >> 16, (fixedY + 0x8000) >> 16))
		return false;

	while (distCounter-- > 0) {
		fixedX += speedX;
		fixedY += speedY;

		if (isPositionSolid((fixedX + 0x8000) >> 16, (fixedY + 0x8000) >> 16))
			return false;
	}

	return true;
}

int StarTrekEngine::findObjectAt(int x, int y) {
	Sprite *sprite = _gfx->getSpriteAt(x, y);

	if (sprite != nullptr) {
		if (sprite == &_inventoryIconSprite)
			return OBJECT_INVENTORY_ICON;
		else if (sprite == &_itemIconSprite)
			return _awayMission.activeObject;

		for (int i = 0; i < NUM_ACTORS; i++) {
			Actor *actor = &_actorList[i];
			if (sprite == &actor->sprite)
				return i;
		}

		error("findObject: Clicked on an unknown sprite");
	}

	_objectHasWalkPosition = false;
	int actionBit = 1 << (_awayMission.activeAction - 1);
	int offset = _room->getFirstHotspot();

	while (offset != _room->getHotspotEnd()) {
		uint16 word = _room->readRdfWord(offset);
		if (word & 0x8000) {
			if ((word & actionBit) && isPointInPolygon((int16 *)(_room->_rdfData + offset + 6), x, y)) {
				int actorIndex = _room->readRdfWord(offset + 6);
				_objectHasWalkPosition = true;
				 _objectWalkPosition.x = _room->readRdfWord(offset + 2);
				 _objectWalkPosition.y = _room->readRdfWord(offset + 4);
				return actorIndex;
			}

			int numVertices = _room->readRdfWord(offset + 8);
			offset = offset + 10 + numVertices * 4;
		}
		else {
			if (isPointInPolygon((int16 *)(_room->_rdfData + offset), x, y)) {
				int actorIndex = _room->readRdfWord(offset);
				return actorIndex;
			}

			int numVertices = _room->readRdfWord(offset + 2);
			offset = offset + 4 + numVertices * 4;
		}
	}

	return -1;
}

/**
 * Loads a bitmap for the animation frame with the given scale.
 */
SharedPtr<Bitmap> StarTrekEngine::loadAnimationFrame(const Common::String &filename, Fixed16 scale) {
	SharedPtr<Bitmap> bitmapToReturn;

	char basename[5];
	strncpy(basename, filename.c_str()+1, 4);
	basename[4] = '\0';

	char c = filename[0];
	if ((strcmp(basename, "stnd") == 0 || strcmp(basename, "tele") == 0)
			&& (c == 'm' || c == 's' || c == 'k' || c == 'r')) {
		if (c == 'm') {
			// Mccoy has the "base" animations for all crewmen
			bitmapToReturn = _gfx->loadBitmap(filename);
		}
		else {
			// All crewman other than mccoy copy the animation frames from mccoy, change
			// the colors of the uniforms, and load an "xor" file to redraw the face.

			// TODO: The ".$bm" extension is a "virtual file"? Caches the changes to the
			// file made here?
			// bitmapToReturn = _gfx->loadBitmap(filename + ".$bm");

			if (bitmapToReturn == nullptr) {
				Common::String mccoyFilename = filename;
				mccoyFilename.setChar('m', 0);
				SharedPtr<Bitmap> bitmap = _gfx->loadBitmap(mccoyFilename);

				uint16 width = bitmap->width;
				uint16 height = bitmap->height;

				bitmapToReturn = SharedPtr<Bitmap>(new Bitmap(width, height));
				bitmapToReturn->xoffset = bitmap->xoffset;
				bitmapToReturn->yoffset = bitmap->yoffset;

				// Change uniform color
				int16 colorShift;
				switch (c) {
				case 'k': // Kirk
					colorShift = 8;
					break;
				case 'r': // Redshirt
					colorShift = -8;
					break;
				case 's': // Spock
					colorShift = 0;
					break;
				}

				if (colorShift == 0) {
					memcpy(bitmapToReturn->pixels, bitmap->pixels, width * height);
				}
				else {
					byte *src = bitmap->pixels;
					byte *dest = bitmapToReturn->pixels;
					byte baseUniformColor = 0xa8;

					for (int i = 0; i < width * height; i++) {
						byte b = *src++;
						if (b >= baseUniformColor && b < baseUniformColor + 8)
							*dest++ = b + colorShift;
						else
							*dest++ = b;
					}
				}

				// Redraw face with xor file
				SharedPtr<FileStream> xorFile = loadFile(filename + ".xor");
				xorFile->seek(0, SEEK_SET);
				uint16 xoffset = bitmap->xoffset - xorFile->readUint16();
				uint16 yoffset = bitmap->yoffset - xorFile->readUint16();
				uint16 xorWidth = xorFile->readUint16();
				uint16 xorHeight = xorFile->readUint16();

				byte *dest = bitmapToReturn->pixels + yoffset * bitmap->width + xoffset;

				for (int i = 0; i < xorHeight; i++) {
					for (int j = 0; j < xorWidth; j++)
						*dest++ ^= xorFile->readByte();
					dest += (bitmap->width - xorWidth);
				}
			}
		}
	}
	else {
		// TODO: when loading a bitmap, it passes a different argument than is standard to
		// the "file loading with cache" function...
		bitmapToReturn = _gfx->loadBitmap(filename);
	}

	if (scale != 256) {
		bitmapToReturn = scaleBitmap(bitmapToReturn, scale);
	}

	return bitmapToReturn;
}


/**
 * Called when the "get" action is first selected. Returns a selected object.
 * This behaves like other menus in that it holds game execution, but no actual menu pops
 * up; it just waits for the player to select something on the screen.
 */
int StarTrekEngine::selectObjectForUseAction() {
	while (true) {
		if (!(_awayMission.crewDownBitset & (1 << OBJECT_KIRK)))
			showInventoryIcons(false);

		TrekEvent event;

		while (true) {
			if (!getNextEvent(&event))
				continue;

			if (event.type == TREKEVENT_TICK) {
				updateMouseBitmap();
				_gfx->drawAllSprites();
				_sound->checkLoopMusic();
			}
			else if (event.type == TREKEVENT_LBUTTONDOWN) {
				removeNextEvent();
				break;
			}
			else if (event.type == TREKEVENT_MOUSEMOVE) {
			}
			else if (event.type == TREKEVENT_RBUTTONDOWN) {
				// Allow this to be processed by main away mission loop
				break;
			}
			else if (event.type == TREKEVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE
						|| event.kbd.keycode == Common::KEYCODE_w
						|| event.kbd.keycode == Common::KEYCODE_t
						|| event.kbd.keycode == Common::KEYCODE_u
						|| event.kbd.keycode == Common::KEYCODE_g
						|| event.kbd.keycode == Common::KEYCODE_l
						|| event.kbd.keycode == Common::KEYCODE_SPACE
						|| event.kbd.keycode == Common::KEYCODE_F2) {
					// Allow these buttons to be processed by main away mission loop
					break;
				}
				else if (event.kbd.keycode == Common::KEYCODE_i) {
					removeNextEvent();
					break;
				}
				else if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER || event.kbd.keycode == Common::KEYCODE_F1) {
					// Simulate left-click
					removeNextEvent();
					event.type = TREKEVENT_LBUTTONDOWN;
					break;
				}
			}

			removeNextEvent();
		}

		if (event.type == TREKEVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_i) {
			hideInventoryIcons();
			int clickedObject = showInventoryMenu(50, 50, true);
			if (clickedObject == -1)
				continue;
			return clickedObject;
		}
		else if (event.type == TREKEVENT_LBUTTONDOWN) {
			int clickedObject = findObjectAt(_gfx->getMousePos());
			hideInventoryIcons();

			if (clickedObject == -1)
				continue;
			else if (isObjectUnusable(clickedObject, ACTION_USE))
				continue;
			else if (clickedObject == OBJECT_INVENTORY_ICON) {
				clickedObject = showInventoryMenu(50, 50, false);
				if (clickedObject == -1)
					continue;
				else
					return clickedObject;
			}
			else if (clickedObject <= OBJECT_REDSHIRT)
				return clickedObject;
			else if (isObjectUnusable(OBJECT_KIRK, ACTION_USE))
				continue;
			else if (_room->actionHasCode(Action(ACTION_USE, OBJECT_KIRK, clickedObject, 0))
					|| _room->actionHasCode(Action(ACTION_GET, clickedObject, 0, 0))
					|| _room->actionHasCode(Action(ACTION_WALK, clickedObject, 0, 0))) {
				_awayMission.activeObject = OBJECT_KIRK;
				_awayMission.passiveObject = clickedObject;
				_awayMission.activeAction = ACTION_USE;
				clickedObject = OBJECT_KIRK;
				if (!walkActiveObjectToHotspot())
					addAction(Action(_awayMission.activeAction, _awayMission.activeObject, _awayMission.passiveObject, 0));
				return clickedObject;
			}
			else
				continue;
		}
		else {
			hideInventoryIcons();
			return -1;
		}
	}
}

Common::String StarTrekEngine::getCrewmanAnimFilename(int actorIndex, const Common::String &basename) {
	const char *crewmanChars = "ksmr";
	assert(actorIndex >= 0 && actorIndex < 4);
	return crewmanChars[actorIndex] + basename;
}

/**
 * Checks whether to change the mouse bitmap to have the red outline.
 */
void StarTrekEngine::updateMouseBitmap() {
	const bool worksOnCrewmen[] = { // True if the action reacts with crewmen
		false, // ACTION_WALK
		true,  // ACTION_USE
		false, // ACTION_GET
		true,  // ACTION_LOOK
		true   // ACTION_TALK
	};
	const bool worksOnActors[] = { // True if the action reacts with other objects
		false, // ACTION_WALK
		true,  // ACTION_USE
		true,  // ACTION_GET
		true,  // ACTION_LOOK
		true   // ACTION_TALK
	};
	const bool worksOnHotspots[] = { // True if the action reacts with hotspots
		false, // ACTION_WALK
		true,  // ACTION_USE
		true,  // ACTION_GET
		true,  // ACTION_LOOK
		false  // ACTION_TALK
	};

	Common::Point mousePos = _gfx->getMousePos();
	int selected = findObjectAt(mousePos.x, mousePos.y);
	int action = _awayMission.activeAction;
	assert(action >= 1 && action <= 5);

	bool withRedOutline;

	if (selected >= 0 && selected <= 3 && worksOnCrewmen[action - 1])
		withRedOutline = true;
	else if (selected > 3 && selected < NUM_ACTORS && worksOnActors[action - 1])
		withRedOutline = true;
	else if (selected >= NUM_ACTORS && selected < HOTSPOTS_END && worksOnHotspots[action - 1])
		withRedOutline = true;
	else
		withRedOutline = false;

	chooseMouseBitmapForAction(action, withRedOutline);
}

/**
 * Checks whether to walk a crewman to a hotspot (the last one obtained from
 * a "findObjectAt" call).
 */
bool StarTrekEngine::walkActiveObjectToHotspot() {
	if (!_objectHasWalkPosition)
		return false;

	int objectIndex;
	if (_awayMission.activeAction != ACTION_USE)
		objectIndex = OBJECT_KIRK;
	else if (_awayMission.activeObject <= OBJECT_REDSHIRT)
		objectIndex = _awayMission.activeObject;
	else if (_awayMission.activeObject >= ITEMS_START && _awayMission.activeObject <= ITEMS_END) { // FIXME: "<= ITEMS_END" doesn't make sense?
		if (_awayMission.activeObject == OBJECT_ISTRICOR)
			objectIndex = OBJECT_SPOCK;
		else if (_awayMission.activeObject == OBJECT_IMTRICOR)
			objectIndex = OBJECT_MCCOY;
		else
			objectIndex = OBJECT_KIRK;
	}
	else // This is the original error message...
		error("Jay didn't think about pmcheck");

	byte finishedAnimActionParam = false;
	bool walk = false;

	if (_awayMission.activeAction == ACTION_WALK)
		walk = true;
	else {
		// If this action has code defined for it in this room, buffer the action to be
		// done after the object finished walking there.
		Action action(_awayMission.activeAction, _awayMission.activeObject, 0, 0);
		if (_awayMission.activeAction == ACTION_USE)
			action.b2 = _awayMission.passiveObject;

		if (_room->actionHasCode(action)) {
			for (int i = 0; i < MAX_BUFFERED_WALK_ACTIONS; i++) {
				if (!_actionOnWalkCompletionInUse[i]) {
					finishedAnimActionParam = i + 0xe0;
					_actionOnWalkCompletionInUse[i] = true;
					_actionOnWalkCompletion[i] = action;
					walk = true;
					break;
				}
			}
		}
	}

	if (walk) {
		Actor *actor = &_actorList[objectIndex];
		Common::String anim = getCrewmanAnimFilename(objectIndex, "walk");
		actorWalkToPosition(objectIndex, anim, actor->pos.x, actor->pos.y, _objectWalkPosition.x, _objectWalkPosition.y);
		if (finishedAnimActionParam != 0) {
			actor->triggerActionWhenAnimFinished = true;
			actor->finishedAnimActionParam = finishedAnimActionParam;
		}
		_objectHasWalkPosition = false;
		return true;
	}
	else {
		_objectHasWalkPosition = false;
		return false;
	}
}

void StarTrekEngine::showInventoryIcons(bool showItem) {
	const char *crewmanFilenames[] = {
		"ikirk",
		"ispock",
		"imccoy",
		"iredshir"
	};

	Common::String itemFilename;

	if (showItem) {
		int i = _awayMission.activeObject;
		if (i >= OBJECT_KIRK && i <= OBJECT_REDSHIRT)
			itemFilename = crewmanFilenames[i];
		else {
			assert(i >= ITEMS_START && i < ITEMS_END);
			Item *item = &_itemList[i - ITEMS_START];
			itemFilename = item->name;
		}
	}

	if (itemFilename.empty())
		_inventoryIconSprite.pos.x = 10;
	else {
		_gfx->addSprite(&_itemIconSprite);
		_itemIconSprite.drawMode = 2;
		_itemIconSprite.pos.x = 10;
		_itemIconSprite.pos.y = 10;
		_itemIconSprite.drawPriority = 15;
		_itemIconSprite.drawPriority2 = 8;
		_itemIconSprite.setBitmap(_gfx->loadBitmap(itemFilename));

		_inventoryIconSprite.pos.x = 46;
	}

	_gfx->addSprite(&_inventoryIconSprite);

	_inventoryIconSprite.pos.y = 10;
	_inventoryIconSprite.drawMode = 2;
	_inventoryIconSprite.drawPriority = 15;
	_inventoryIconSprite.drawPriority2 = 8;
	_inventoryIconSprite.setBitmap(_gfx->loadBitmap("inv00"));
}

/**
 * Return true if an object is unselectable with the given action?
 */
bool StarTrekEngine::isObjectUnusable(int object, int action) {
	if (action == ACTION_LOOK)
		return false;
	if (object == OBJECT_REDSHIRT && _awayMission.redshirtDead)
		return true;
	if (object >= OBJECT_KIRK && object <= OBJECT_REDSHIRT && (_awayMission.crewDownBitset & (1 << object)))
		return true;
	if (object == OBJECT_IMTRICOR && (_awayMission.crewDownBitset & (1 << OBJECT_MCCOY)))
		return true;
	if (object == OBJECT_ISTRICOR && (_awayMission.crewDownBitset & (1 << OBJECT_SPOCK)))
		return true;
	return false;
}

void StarTrekEngine::hideInventoryIcons() {
	// Clear these sprites from the screen
	if (_itemIconSprite.drawMode == 2)
		_itemIconSprite.dontDrawNextFrame();
	if (_inventoryIconSprite.drawMode == 2)
		_inventoryIconSprite.dontDrawNextFrame();

	_gfx->drawAllSprites();

	if (_itemIconSprite.drawMode == 2) {
		_gfx->delSprite(&_itemIconSprite);
		_itemIconSprite.drawMode = 0;
		_itemIconSprite.bitmap.reset();
	}

	if (_inventoryIconSprite.drawMode == 2) {
		_gfx->delSprite(&_inventoryIconSprite);
		_inventoryIconSprite.drawMode = 0;
		_inventoryIconSprite.bitmap.reset();
	}
}

/**
 * When a crewman is collapsed, they get once a timer reaches 0.
 */
void StarTrekEngine::updateCrewmanGetupTimers() {
	if (_awayMission.crewDownBitset == 0)
		return;
	for (int i = OBJECT_KIRK; i <= OBJECT_REDSHIRT; i++) {
		Actor *actor = &_actorList[i];

		if (!(_awayMission.crewDownBitset & (1 << i)))
			continue;

		_awayMission.crewGetupTimers[i]--;
		if (_awayMission.crewGetupTimers[i] <= 0) {
			Common::String anim = getCrewmanAnimFilename(i, "getu");
			int8 dir = _awayMission.crewDirectionsAfterWalk[i];
			char d;
			if (dir == -1) {
				d = actor->direction;
			}
			else {
				const char *dirs = "nsew";
				uint16 scale = getActorScaleAtPosition(actor->sprite.pos.y);
				d = dirs[dir];

				int16 xOffset = 0, yOffset = 0;
				if (d == 'n') {
					xOffset = -24;
					yOffset = -8;
				}
				else if (d == 'w') {
					xOffset = -35;
					yOffset = -12;
				}
				actor->sprite.pos.x += (scale * xOffset) >> 8;
				actor->sprite.pos.y += (scale * yOffset) >> 8;
			}

			anim += (char)d;
			loadActorAnimWithRoomScaling(i, anim, actor->sprite.pos.x, actor->sprite.pos.y);
			_awayMission.crewDownBitset &= ~(1 << i);
		}
	}
}

int StarTrekEngine::showInventoryMenu(int x, int y, bool restoreMouse) {
	const int ITEMS_PER_ROW = 5;

	Common::Point oldMousePos = _gfx->getMousePos();
	bool keyboardControlledMouse = _keyboardControlsMouse;
	_keyboardControlsMouse = false;

	int itemIndex = 0;
	int numItems = 0;

	char itemNames[NUM_OBJECTS][10];
	Common::Point itemPositions[NUM_OBJECTS];
	int16 itemIndices[NUM_OBJECTS];

	while (itemIndex < NUM_OBJECTS) {
		if (_itemList[itemIndex].have) {
			strcpy(itemNames[numItems], _itemList[itemIndex].name);

			int16 itemX = (numItems % ITEMS_PER_ROW) * 32 + x;
			int16 itemY = (numItems / ITEMS_PER_ROW) * 32 + y;
			itemPositions[numItems] = Common::Point(itemX, itemY);
			itemIndices[numItems] = _itemList[itemIndex].field2;

			numItems++;
		}
		itemIndex++;
	}

	Sprite itemSprites[NUM_OBJECTS];

	for (int i = 0; i < numItems; i++) {
		_gfx->addSprite(&itemSprites[i]);

		itemSprites[i].drawMode = 2;
		itemSprites[i].pos.x = itemPositions[i].x;
		itemSprites[i].pos.y = itemPositions[i].y;
		itemSprites[i].drawPriority = 15;
		itemSprites[i].drawPriority2 = 8;
		itemSprites[i].setBitmap(_gfx->loadBitmap(itemNames[i]));
	}

	chooseMousePositionFromSprites(itemSprites, numItems, -1, 4);
	bool displayMenu = true;
	int lastItemIndex = -1;

	while (displayMenu) {
		_sound->checkLoopMusic();

		TrekEvent event;
		if (!getNextEvent(&event))
			continue;

		switch (event.type) {
		case TREKEVENT_TICK: {
			Common::Point mousePos = _gfx->getMousePos();
			itemIndex = getMenuButtonAt(itemSprites, numItems, mousePos.x, mousePos.y);
			if (itemIndex != lastItemIndex) {
				if (lastItemIndex != -1) {
					drawMenuButtonOutline(itemSprites[lastItemIndex].bitmap, 0);
					itemSprites[lastItemIndex].bitmapChanged = true;
				}
				if (itemIndex != -1) {
					drawMenuButtonOutline(itemSprites[itemIndex].bitmap, 15);
					itemSprites[itemIndex].bitmapChanged = true;
				}
				lastItemIndex = itemIndex;
			}
			_gfx->drawAllSprites();
			break;
		}

		case TREKEVENT_LBUTTONDOWN:
exitWithSelection:
			displayMenu = false;
			break;

		case TREKEVENT_RBUTTONDOWN:
exitWithoutSelection:
			displayMenu = false;
			lastItemIndex = -1;
			break;

		case TREKEVENT_KEYDOWN:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_F2:
				goto exitWithoutSelection;

			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
			case Common::KEYCODE_F1:
				goto exitWithSelection;

			case Common::KEYCODE_HOME:
			case Common::KEYCODE_KP7:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 4);
				break;

			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
			case Common::KEYCODE_PAGEUP:
			case Common::KEYCODE_KP9:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 2);
				break;

			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 1);
				break;

			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 0);
				break;

			case Common::KEYCODE_END:
			case Common::KEYCODE_KP1:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 5);
				break;

			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
			case Common::KEYCODE_PAGEDOWN:
			case Common::KEYCODE_KP3:
				chooseMousePositionFromSprites(itemSprites, numItems, lastItemIndex, 3);
				break;

			default:
				break;
			}
			break;

		default:
			break;
		}

		removeNextEvent();
	}

	playSoundEffectIndex(0x10);
	if (lastItemIndex >= 0)
		drawMenuButtonOutline(itemSprites[lastItemIndex].bitmap, 0);

	for (int i = 0; i < numItems; i++)
		itemSprites[i].dontDrawNextFrame();

	_gfx->drawAllSprites();

	for (int i = 0; i < numItems; i++) {
		itemSprites[i].bitmap.reset();
		_gfx->delSprite(&itemSprites[i]);
	}

	if (lastItemIndex >= 0) {
		lastItemIndex = itemIndices[lastItemIndex];
	}

	if (restoreMouse)
		_gfx->warpMouse(oldMousePos.x, oldMousePos.y);

	_keyboardControlsMouse = keyboardControlledMouse;
	return lastItemIndex;
}

/**
 * A scale of 256 is the baseline.
 */
SharedPtr<Bitmap> StarTrekEngine::scaleBitmap(SharedPtr<Bitmap> bitmap, Fixed16 scale) {
	int scaledWidth  = (bitmap->width  * scale) >> 8;
	int scaledHeight = (bitmap->height * scale) >> 8;
	int origWidth  = bitmap->width;
	int origHeight = bitmap->height;

	if (scaledWidth < 1)
		scaledWidth = 1;
	if (scaledHeight < 1)
		scaledHeight = 1;

	SharedPtr<Bitmap> scaledBitmap(new Bitmap(scaledWidth, scaledHeight));
	scaledBitmap->xoffset = (bitmap->xoffset * scale) >> 8;
	scaledBitmap->yoffset = (bitmap->yoffset * scale) >> 8;

	// sub_344a5(scaledWidth, origWidth);

	origHeight--;
	scaledHeight--;

	byte *src = bitmap->pixels;
	byte *dest = scaledBitmap->pixels;

	if (scale <= 256) {
		int16 var2e = 0;
		uint16 var30 = scaledHeight << 1;
		uint16 var32 = (scaledHeight - origHeight) << 1;
		uint16 origRow = 0;

		while (origRow <= origHeight) {
			if (var2e < 0) {
				var2e += var30;
			}
			else {
				var2e += var32;
				scaleBitmapRow(src, dest, origWidth, scaledWidth);
				dest += scaledWidth;
			}

			src += bitmap->width;
			origRow++;
		}
	}
	else {
		int16 var2e = (origHeight << 1) - scaledHeight;
		uint16 var30 = origHeight << 1;
		uint16 var32 = (origHeight - scaledHeight) << 1;
		uint16 srcRowChanged = true;
		origWidth = bitmap->width;
		uint16 scaledRow = 0;
		byte *rowData = new byte[scaledWidth];

		while (scaledRow++ <= scaledHeight) {
			if (srcRowChanged) {
				scaleBitmapRow(src, rowData, origWidth, scaledWidth);
				srcRowChanged = false;
			}

			memcpy(dest, rowData, scaledWidth);
			dest += scaledWidth;

			if (var2e < 0) {
				var2e += var30;
			}
			else {
				var2e += var32;
				src += origWidth;
				srcRowChanged = true;
			}
		}

		delete[] rowData;
	}

	return scaledBitmap;
}

/**
 * This takes a row of an unscaled bitmap, and copies it to a row of a scaled bitmap.
 * This was heavily optimized in the original game (manually constructed an unrolled
 * loop).
 */
void StarTrekEngine::scaleBitmapRow(byte *src, byte *dest, uint16 origWidth, uint16 scaledWidth) {
	if (origWidth >= scaledWidth) {
		int16 var2 = (scaledWidth << 1) - origWidth;
		uint16 var4 = scaledWidth << 1;
		uint16 var6 = (scaledWidth - origWidth) << 1;
		uint16 varE = 0;
		uint16 varA = 0;
		uint16 var8 = origWidth;
		uint16 di = 0;

		while (var8-- != 0) {
			if (var2 < 0) {
				var2 += var4;
			}
			else {
				var2 += var6;
				if (di != 0) {
					if (varE != 0) {
						*(dest - 1) = *src++;
						varE = 0;
						di--;
					}
					src += di;
					di = 0;
				}
				*dest++ = *src;
				varE = 1;
			}

			di++;
			varA++;
		}
	}
	else {
		int16 var2 = ((origWidth - 1) << 1) - (scaledWidth - 1);
		uint16 var4 = (origWidth - 1) << 1;
		uint16 var6 = ((origWidth - 1) - (scaledWidth - 1)) << 1;
		uint16 varA = 0;
		uint16 var8 = scaledWidth;
		uint16 di = 0;

		while (var8-- != 0) {
			if (di != 0) {
				src += di;
				di = 0;
			}
			*dest++ = *src;

			if (var2 < 0)
				var2 += var4;
			else {
				var2 += var6;
				di++;
			}

			varA++;
		}
	}
}

/**
 * TODO:
 *   - Should return nullptr on failure to open a file?
 *   - This is supposed to cache results, return same FileStream on multiple accesses.
 *   - This is supposed to read from a "patches" folder which overrides files in the
 *     packed blob.
 */
SharedPtr<FileStream> StarTrekEngine::loadFile(Common::String filename, int fileIndex) {
	filename.toUppercase();

	Common::String basename, extension;

	bool bigEndian = getPlatform() == Common::kPlatformAmiga;

	for (int i=filename.size()-1; ; i--) {
		if (filename[i] == '.') {
			basename = filename;
			extension = filename;
			basename.replace(i, filename.size()-i, "");
			extension.replace(0, i+1, "");
			break;
		}
	}

	// FIXME: don't know if this is right, or if it goes here
	while (!basename.empty() && basename.lastChar() == ' ') {
		basename.erase(basename.size() - 1, 1);
	}

	filename = basename + '.' + extension;

	// The Judgment Rites demo has its files not in the standard archive
	if (getGameType() == GType_STJR && (getFeatures() & GF_DEMO)) {
		Common::File *file = new Common::File();
		if (!file->open(filename.c_str()))
			error("Could not find file \'%s\'", filename.c_str());
		return SharedPtr<FileStream>(new FileStream(file, bigEndian));
	}

	Common::SeekableReadStream *indexFile = 0;

	if (getPlatform() == Common::kPlatformAmiga) {
		indexFile = SearchMan.createReadStreamForMember("data000.dir");
		if (!indexFile)
			error("Could not open data000.dir");
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		indexFile = _macResFork->getResource("Directory");
		if (!indexFile)
			error("Could not find 'Directory' resource in 'Star Trek Data'");
	} else {
		indexFile = SearchMan.createReadStreamForMember("data.dir");
		if (!indexFile)
			error("Could not open data.dir");
	}
	
	uint32 indexOffset = 0;
	bool foundData = false;
	uint16 fileCount = 1;
	uint16 uncompressedSize = 0;
	
	while (!indexFile->eos() && !indexFile->err()) {
		Common::String testfile;
		for (byte i = 0; i < 8; i++) {
			char c = indexFile->readByte();
			if (c)
				testfile += c;
		}
		testfile += '.';

		for (byte i = 0; i < 3; i++)
			testfile += indexFile->readByte();

		if (getFeatures() & GF_DEMO && getPlatform() == Common::kPlatformDOS) {
			indexFile->readByte(); // Always 0?
			fileCount = indexFile->readUint16LE(); // Always 1
			indexOffset = indexFile->readUint32LE();
			uncompressedSize = indexFile->readUint16LE();
		} else {
			if (getPlatform() == Common::kPlatformAmiga)
				indexOffset = (indexFile->readByte() << 16) + (indexFile->readByte() << 8) + indexFile->readByte();	
			else 
				indexOffset = indexFile->readByte() + (indexFile->readByte() << 8) + (indexFile->readByte() << 16);
			
			if (indexOffset & (1 << 23)) {
				fileCount = (indexOffset >> 16) & 0x7F;
				indexOffset = indexOffset & 0xFFFF;
				assert(fileCount > 1);
			} else {
				fileCount = 1;
			}
		}

		if (filename.matchString(testfile)) {
			foundData = true;
			break;
		}
	}

	delete indexFile;

	if (!foundData) {
		// Files can be accessed "sequentially" if their filenames are the same except for
		// the last character being incremented by one.
		if ((basename.lastChar() >= '1' && basename.lastChar() <= '9') ||
				(basename.lastChar() >= 'B' && basename.lastChar() <= 'Z')) {
			basename.setChar(basename.lastChar()-1, basename.size()-1);
			return loadFile(basename + "." + extension, fileIndex+1);
		} else
			error("Could not find file \'%s\'", filename.c_str());
	}

	if (fileIndex >= fileCount)
		error("Tried to access file index %d for file '%s' which doesn't exist.", fileIndex, filename.c_str());

	Common::SeekableReadStream *dataFile = 0;
	Common::SeekableReadStream *dataRunFile = 0; // FIXME: Amiga & Mac need this implemented

	if (getPlatform() == Common::kPlatformAmiga) {
		dataFile = SearchMan.createReadStreamForMember("data.000");
		if (!dataFile)
			error("Could not open data.000");
	} else if (getPlatform() == Common::kPlatformMacintosh) {
		dataFile = _macResFork->getDataFork();
		if (!dataFile)
			error("Could not get 'Star Trek Data' data fork");
	} else {
		dataFile = SearchMan.createReadStreamForMember("data.001");
		if (!dataFile)
			error("Could not open data.001");
		dataRunFile = SearchMan.createReadStreamForMember("data.run");
		if (!dataFile)
			error("Could not open data.run");
	}

	if (getFeatures() & GF_DEMO && getPlatform() == Common::kPlatformDOS) {
		assert(fileCount == 1); // Sanity check...
		Common::SeekableReadStream *stream = dataFile->readStream(uncompressedSize);
		delete dataFile;
		delete dataRunFile;
		return SharedPtr<FileStream>(new FileStream(stream, bigEndian));
	} else {
		if (fileCount != 1) {
			dataRunFile->seek(indexOffset);

			indexOffset = dataRunFile->readByte() + (dataRunFile->readByte() << 8) + (dataRunFile->readByte() << 16);
			//indexOffset &= 0xFFFFFE;

			for (uint16 i = 0; i < fileIndex; i++) {
				uint16 size = dataRunFile->readUint16LE();
				indexOffset += size;
			}
		}
		dataFile->seek(indexOffset);

		uncompressedSize = (getPlatform() == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();
		uint16 compressedSize = (getPlatform() == Common::kPlatformAmiga) ? dataFile->readUint16BE() : dataFile->readUint16LE();

		Common::SeekableReadStream *stream = decodeLZSS(dataFile->readStream(compressedSize), uncompressedSize);

		delete dataFile;
		delete dataRunFile;
		return SharedPtr<FileStream>(new FileStream(stream, bigEndian));
	}

	// We should not get to this point...
	error("Could not find data for \'%s\'", filename.c_str());

	return SharedPtr<FileStream>();
}

void StarTrekEngine::playMovie(Common::String filename) {
	if (getPlatform() == Common::kPlatformMacintosh)
		playMovieMac(filename);
	else
		error("Interplay MVE not yet supported");
}

void StarTrekEngine::playMovieMac(Common::String filename) {
	// Swap to 16bpp mode
	initGraphics(512, 384, NULL);

	Video::QuickTimeDecoder *qtDecoder = new Video::QuickTimeDecoder();

	if (!qtDecoder->loadFile(filename))
		error("Could not open '%s'", filename.c_str());

	bool continuePlaying = true;

	qtDecoder->start();

	while (!qtDecoder->endOfVideo() && !shouldQuit() && continuePlaying) {
		if (qtDecoder->needsUpdate()) {
			const ::Graphics::Surface *frame = qtDecoder->decodeNextFrame();

			if (frame) {
				::Graphics::Surface *convertedFrame = frame->convertTo(_system->getScreenFormat());
				_system->copyRectToScreen(convertedFrame->getPixels(), convertedFrame->pitch, 0, 0, convertedFrame->w, convertedFrame->h);
				_system->updateScreen();
				convertedFrame->free();
				delete convertedFrame;
			}
		}

		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event))
			if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
				continuePlaying = false;

		g_system->delayMillis(10);
	}

	delete qtDecoder;

	// Swap back to 8bpp mode
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
}

uint16 StarTrekEngine::getRandomWord() {
	return _randomSource.getRandomNumber(0xffff);
}

/**
 * ".txt" files are just lists of strings. This traverses the file to get a particular
 * string index.
 */
Common::String StarTrekEngine::getLoadedText(int textIndex) {
	SharedPtr<FileStream> txtFile = loadFile(_txtFilename + ".txt");

	byte *data = txtFile->_data;
	while (textIndex != 0) {
		while (*(data++) != '\0');
		textIndex--;
	}

	return (char *)data;
}

} // End of namespace StarTrek
