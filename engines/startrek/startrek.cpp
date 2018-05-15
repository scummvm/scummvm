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
#include "graphics/surface.h"
#include "engines/util.h"
#include "video/qt_decoder.h"

#include "startrek/filestream.h"
#include "startrek/iwfile.h"
#include "startrek/lzss.h"
#include "startrek/startrek.h"

namespace StarTrek {

StarTrekEngine::StarTrekEngine(OSystem *syst, const StarTrekGameDescription *gamedesc) :
	Engine(syst),
	_gameDescription(gamedesc),
	_kirkObject(&_objectList[0]),
	_spockObject(&_objectList[1]),
	_mccoyObject(&_objectList[2]),
	_redshirtObject(&_objectList[3]) {

	DebugMan.addDebugChannel(kDebugSound, "sound", "Sound");
	DebugMan.addDebugChannel(kDebugGraphics, "graphics", "Graphics");

	_gfx = nullptr;
	_sound = nullptr;
	_macResFork = nullptr;

	_clockTicks = 0;

	_musicEnabled = true;
	_sfxEnabled = true;
	_word_467a6 = true;
	_musicWorking = true;
	_sfxWorking = true;
	_finishedPlayingSpeech = false;

	_mouseControllingShip = false;
	_keyboardControlsMouse = true;

	_inQuitGameMenu = false;

	_missionToLoad = "DEMON";
	_roomIndexToLoad = 0;
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

	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
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
	objectFunc1();
	initObjects();

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
		loadObjectAnim(i, filename, x, y, 256);
		_objectList[i].animationString[0] = '\0';
	}

	if (_missionToLoad.equalsIgnoreCase("feather") && name[4] == 'b') {
		loadObjectAnim(9, "qteleb", 0x61, 0x79, 0x100);
	}
	else if (_missionToLoad.equalsIgnoreCase("trial")) {
		if (name[4] == 'd') {
			loadObjectAnim(9, "qteled", 0x61, 0x79, 0x100);
		}
		/* TODO
		else if (word_51156 >= 3) {
			loadObjectAnim(9, "qteleb", 0x61, 0x79, 0x100);
		}
		*/
	}

	loadObjectAnim(8, "transc", 0, 0, 0x100);

	// TODO: redraw mouse and sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeinScreen();

	playSoundEffectIndex(0x0a);

	if (name.equalsIgnoreCase("teled"))
		playSoundEffectIndex(0x08);
	else
		playSoundEffectIndex(0x09);

	while (_objectList[0].field62 == 0) {
		TrekEvent event;
		if (popNextEvent(&event)) {
			if (event.type == TREKEVENT_TICK) {
				// TODO: redraw sprite_52c4e?
				_frameIndex++;
				updateObjectAnimations();
				_gfx->drawAllSprites();
			}
		}
	}

	// TODO: redraw sprite_52c4e?

	_gfx->drawAllSprites();
	_gfx->fadeoutScreen();
	objectFunc1();
	initObjects();
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

void StarTrekEngine::playSpeech(const Common::String &filename) {
	_sound->playSpeech(filename.c_str());
}

void StarTrekEngine::stopPlayingSpeech() {
	_sound->stopPlayingSpeech();
}

void StarTrekEngine::initObjects() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		_objectList[i] = Object();
	}
	for (int i = 0; i < MAX_OBJECTS / 2; i++)
		_objectBanFiles[i].reset();

	strcpy(_kirkObject->animationString, "kstnd");
	strcpy(_spockObject->animationString, "sstnd");
	strcpy(_mccoyObject->animationString, "mstnd");
	strcpy(_redshirtObject->animationString, "rstnd");
}

/**
 * Set an object's animation, position, and scale.
 */
int StarTrekEngine::loadObjectAnim(int objectIndex, const Common::String &animName, int16 x, int16 y, uint16 scale) {
	debugC(6, kDebugGraphics, "Load animation '%s' on object %d", animName.c_str(), objectIndex);

	Object *object;

	if (objectIndex == -1) {
		// TODO
	}
	else
		object = &_objectList[objectIndex];

	if (object->spriteDrawn) {
		releaseAnim(object);
		drawObjectToScreen(object, animName, x, y, scale, false);
	}
	else {
		drawObjectToScreen(object, animName, x, y, scale, true);
	}

	object->walkingIntoRoom = 0;
	object->field66 = 0;

	return objectIndex;
}

/**
 * Tries to make an object walk to a position.
 * Returns true if successful in initiating the walk.
 */
bool StarTrekEngine::objectWalkToPosition(int objectIndex, const Common::String &animFile, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	debugC(6, "Obj %d: walk from (%d,%d) to (%d,%d)", objectIndex, srcX, srcY, destX, destY);

	Object *object = &_objectList[objectIndex];

	object->walkingIntoRoom = 0;
	if (isPositionSolid(destX, destY))
		return false;

	if (object->spriteDrawn)
		releaseAnim(object);
	else
		_gfx->addSprite(&object->sprite);

	object->spriteDrawn = true;
	object->animType = 1;
	object->frameToStartNextAnim = _frameIndex + 1;
	strcpy(object->animationString2, animFile.c_str());

	object->dest.x = destX;
	object->dest.y = destY;
	object->field92 = 0;
	object->walkingIntoRoom = 0;

	object->iwDestPosition = -1;
	object->iwSrcPosition = -1;

	if (directPathExists(srcX, srcY, destX, destY)) {
		chooseObjectDirectionForWalking(object, srcX, srcY, destX, destY);
		updateObjectPositionWhileWalking(object, (object->granularPosX + 0x8000) >> 16, (object->granularPosY + 0x8000) >> 16);
		return true;
	}
	else {
		object->iwSrcPosition = _iwFile->getClosestKeyPosition(srcX, srcY);
		object->iwDestPosition = _iwFile->getClosestKeyPosition(destX, destY);

		if (object->iwSrcPosition == -1 || object->iwDestPosition == -1) {
			// No path exists; face south by default.
			strcat(object->animationString2, "S");
			object->direction = 'S';

			updateObjectPositionWhileWalking(object, srcX, srcY);
			initStandAnim(objectIndex);

			return false;
		}
		else {
			Common::Point iwSrc = _iwFile->_keyPositions[object->iwSrcPosition];
			chooseObjectDirectionForWalking(object, srcX, srcY, iwSrc.x, iwSrc.y);
			updateObjectPositionWhileWalking(object, (object->granularPosX + 0x8000) >> 16, (object->granularPosY + 0x8000) >> 16);
			return true;
		}
	}
}

void StarTrekEngine::updateObjectAnimations() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		Object *object = &_objectList[i];
		if (!object->spriteDrawn)
			continue;

		switch (object->animType) {
		case 0: // Not walking?
		case 2:
			if (_frameIndex >= object->frameToStartNextAnim) {
				int nextAnimIndex = 0; // TODO: "chooseNextAnimFrame" function
				object->animFile->seek(18 + nextAnimIndex + object->animFrame * 22, SEEK_SET);
				byte nextAnimFrame = object->animFile->readByte();

				if (object->animFrame != nextAnimFrame) {
					if (nextAnimFrame == object->numAnimFrames - 1) {
						object->field62++;
						if (object->walkingIntoRoom != 0) {
							addCommand(Command(COMMAND_FINISHED_BEAMING_IN, object->field66, 0, 0));
						}
					}
				}

				object->animFrame = nextAnimFrame;
				if (object->animFrame >= object->numAnimFrames) {
					if (object->animationString[0] == '\0')
						removeObjectFromScreen(i);
					else
						initStandAnim(i);
				}
				else {
					Sprite *sprite = &object->sprite;

					object->animFile->seek(object->animFrame * 22, SEEK_SET);
					char animFrameFilename[16];
					object->animFile->read(animFrameFilename, 16);
					sprite->setBitmap(loadAnimationFrame(animFrameFilename, object->scale));

					memset(object->animationString4, 0, 10);
					strncpy(object->animationString4, animFrameFilename, 9);

					object->animFile->seek(10 + object->animFrame * 22, SEEK_SET);
					uint16 xOffset = object->animFile->readUint16();
					uint16 yOffset = object->animFile->readUint16();
					uint16 basePriority = object->animFile->readUint16();
					uint16 frames = object->animFile->readUint16();

					sprite->pos.x = xOffset + object->pos.x;
					sprite->pos.y = yOffset + object->pos.y;
					sprite->drawPriority = _gfx->getPriValue(0, yOffset + object->pos.y) + basePriority;
					sprite->bitmapChanged = true;

					object->frameToStartNextAnim = frames + _frameIndex;
				}
			}
			break;
		case 1: // Walking
			if (_frameIndex < object->frameToStartNextAnim)
				break;
			if (i == 0) // Kirk only
				checkTouchedLoadingZone(object->pos.x, object->pos.y);
			if (object->field90 != 0) {
				Sprite *sprite = &object->sprite;
				int loops;
				if (getObjectScaleAtPosition((object->granularPosY + 0x8000) >> 16) < 0xa0)
					loops = 1;
				else
					loops = 2;
				for (int k = 0; k < loops; k++) {
					if (object->field90 == 0)
						break;
					object->field90--;
					uint32 newX = object->granularPosX + object->speedX;
					uint32 newY = object->granularPosY + object->speedY;
					if ((object->field90 & 3) == 0) {
						sprite->bitmap.reset();
						updateObjectPositionWhileWalking(object, (newX + 0x8000) >> 16, (newY + 0x8000) >> 16);
						object->field92++;
					}

					object->granularPosX = newX;
					object->granularPosY = newY;
					object->frameToStartNextAnim = _frameIndex;
				}
			}
			else { // object->field90 == 0
				if (object->iwSrcPosition == -1) {
					if (object->walkingIntoRoom != 0) {
						object->walkingIntoRoom = 0;
						addCommand(Command(FINISHED_ENTERING_ROOM, object->field66 & 0xff, 0, 0));
					}

					object->sprite.bitmap.reset();
					updateObjectPositionWhileWalking(object, (object->granularPosX + 0x8000) >> 16, (object->granularPosY + 0x8000) >> 16);
					initStandAnim(i);
				}
				else { // object->iwSrcPosition != -1
					if (object->iwSrcPosition == object->iwDestPosition) {
						object->animationString2[strlen(object->animationString2) - 1] = '\0';
						object->iwDestPosition = -1;
						object->iwSrcPosition = -1;
						chooseObjectDirectionForWalking(object, object->pos.x, object->pos.y, object->dest.x, object->dest.y);
					}
					else {
						int index = _iwFile->_iwEntries[object->iwSrcPosition][object->iwDestPosition];
						object->iwSrcPosition = index;
						Common::Point dest = _iwFile->_keyPositions[object->iwSrcPosition];
						object->animationString2[strlen(object->animationString2) - 1] = '\0';
						chooseObjectDirectionForWalking(object, object->pos.x, object->pos.y, dest.x, dest.y);
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

void StarTrekEngine::removeObjectFromScreen(int objectIndex) {
	Object *object = &_objectList[objectIndex];

	if (object->spriteDrawn != 1)
		return;

	debugC(6, kDebugGraphics, "Stop drawing object %d", objectIndex);

	Sprite *sprite = &object->sprite;
	sprite->field16 = true;
	sprite->bitmapChanged = true;
	_gfx->drawAllSprites();
	_gfx->delSprite(sprite);
	releaseAnim(object);
}

void StarTrekEngine::objectFunc1() {
	for (int i = 0; i < MAX_OBJECTS; i++) {
		if (_objectList[i].spriteDrawn == 1) {
			removeObjectFromScreen(i);
		}
	}

	for (int i = 0; i < MAX_OBJECTS / 2; i++) {
		_objectBanFiles[i].reset();
	}
}

void StarTrekEngine::drawObjectToScreen(Object *object, const Common::String &_animName, int16 x, int16 y, uint16 scale, bool addSprite) {
	Common::String animFilename = _animName;
	if (_animName.hasPrefixIgnoreCase("stnd") /* && word_45d20 == -1 */) // TODO
		animFilename += 'j';
	memcpy(object->animationString3, _animName.c_str(), sizeof(object->animationString3));

	object->animType = 2;
	object->animFile = loadFile(animFilename + ".anm");
	object->numAnimFrames = object->animFile->size() / 22;
	object->animFrame = 0;
	object->pos.x = x;
	object->pos.y = y;
	object->field62 = 0;
	object->scale = scale;

	object->animFile->seek(16, SEEK_SET);
	object->frameToStartNextAnim = object->animFile->readUint16() + _frameIndex;

	char firstFrameFilename[10];
	object->animFile->seek(0, SEEK_SET);
	object->animFile->read(firstFrameFilename, 10);

	Sprite *sprite = &object->sprite;
	if (addSprite)
		_gfx->addSprite(sprite);

	sprite->setBitmap(loadAnimationFrame(firstFrameFilename, scale));
	memset(object->animationString4, 0, sizeof(char) * 10);
	strncpy(object->animationString4, firstFrameFilename, sizeof(char) * 9);

	object->scale = scale;

	object->animFile->seek(10, SEEK_SET);
	uint16 xOffset = object->animFile->readUint16();
	uint16 yOffset = object->animFile->readUint16();
	uint16 basePriority = object->animFile->readUint16();

	sprite->pos.x = xOffset + object->pos.x;
	sprite->pos.y = yOffset + object->pos.y;
	sprite->drawPriority = _gfx->getPriValue(0, yOffset + object->pos.y) + basePriority;
	sprite->bitmapChanged = true;

	object->spriteDrawn = 1;
}

void StarTrekEngine::releaseAnim(Object *object) {
	switch (object->animType) {
	case 0:
	case 2:
		object->sprite.bitmap.reset();
		object->animFile.reset();
		break;
	case 1:
		object->sprite.bitmap.reset();
		break;
	default:
		error("Invalid anim type");
		break;
	}

	object->spriteDrawn = 0;
}

void StarTrekEngine::initStandAnim(int objectIndex) {
	Object *object = &_objectList[objectIndex];

	if (!object->spriteDrawn)
		error("initStandAnim: dead anim");

	////////////////////
	// sub_239d2
	const char *directions = "nsew";

	if (objectIndex >= 0 && objectIndex <= 3) {
		int8 dir = _awayMission.field25[objectIndex];
		if (dir != -1) {
			object->direction = directions[dir];
			_awayMission.field25[objectIndex] = -1;
		}
	}
	// end of sub_239d2
	////////////////////

	Common::String animName;
	if (object->direction != 0)
		animName = Common::String(object->animationString) + (char)object->direction;
	else // Default to facing south
		animName = Common::String(object->animationString) + 's';

	uint16 scale = getObjectScaleAtPosition(object->pos.y);
	loadObjectAnim(objectIndex, animName, object->pos.x, object->pos.y, scale);
	object->animType = 0;
}

void StarTrekEngine::updateObjectPositionWhileWalking(Object *object, int16 x, int16 y) {
	object->scale = getObjectScaleAtPosition(y);
	Common::String animName = Common::String::format("%s%02d", object->animationString2, object->field92 & 7);
	object->sprite.setBitmap(loadAnimationFrame(animName, object->scale));

	memset(object->animationString4, 0, 10);
	strncpy(object->animationString4, animName.c_str(), 9);

	Sprite *sprite = &object->sprite;
	sprite->drawPriority = _gfx->getPriValue(0, y);
	sprite->pos.x = x;
	sprite->pos.y = y;
	sprite->bitmapChanged = true;

	object->frameToStartNextAnim = _frameIndex;
	object->pos.x = x;
	object->pos.y = y;
}

/**
 * Chooses a value for the object's speed and direction, based on a source position and
 * a destination position it's walking to.
 */
void StarTrekEngine::chooseObjectDirectionForWalking(Object *object, int16 srcX, int16 srcY, int16 destX, int16 destY) {
	object->granularPosX = srcX << 16;
	object->granularPosY = srcY << 16;

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
		object->animationString2[strlen(object->animationString2) + 1] = '\0';
		object->animationString2[strlen(object->animationString2)] = d;

		object->direction = d;
		object->field90 = absDistX;

		if (distX != 0) {
			if (distX > 0)
				object->speedX = 1 << 16;
			else
				object->speedX = -1 << 16; // 0xffff0000

			object->speedY = (distY << 16) / absDistX;
		}
	}
	else {
		char d;
		if (distY > 0)
			d = 'S';
		else
			d = 'N';

		// Append direction to animation string
		object->animationString2[strlen(object->animationString2) + 1] = '\0';
		object->animationString2[strlen(object->animationString2)] = d;

		object->direction = d;
		object->field90 = absDistY;

		if (distY != 0) {
			if (distY > 0)
				object->speedY = 1 << 16;
			else
				object->speedY = -1 << 16; // 0xffff0000

			object->speedX = (distX << 16) / absDistY;
		}
	}
}

/**
 * Returns true if an object can walk directly from a source position to a destination
 * position without running into unwalkable terrain.
 */
bool StarTrekEngine::directPathExists(int16 srcX, int16 srcY, int16 destX, int16 destY) {
	int32 distX = destX - srcX;
	int32 distY = destY - srcY;

	int32 absDistX = abs(distX);
	int32 absDistY = abs(distY);

	int32 distCounter;
	FixedInt speedX, speedY;

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

	FixedInt fixedX = srcX << 16;
	FixedInt fixedY = srcY << 16;

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

/**
 * Loads a bitmap for the animation frame with the given scale.
 */
SharedPtr<Bitmap> StarTrekEngine::loadAnimationFrame(const Common::String &filename, uint16 scale) {
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

Common::String StarTrekEngine::getCrewmanAnimFilename(int objectIndex, const Common::String &basename) {
	const char *crewmanChars = "ksmr";
	assert(objectIndex >= 0 && objectIndex < 4);
	return crewmanChars[objectIndex] + basename;
}

/**
 * A scale of 256 is the baseline.
 */
SharedPtr<Bitmap> StarTrekEngine::scaleBitmap(SharedPtr<Bitmap> bitmap, uint16 scale) {
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

} // End of namespace StarTrek
