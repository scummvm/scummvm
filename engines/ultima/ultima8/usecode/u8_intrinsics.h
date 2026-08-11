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

#ifndef ULTIMA8_USECODE_U8INTRINSICS_H
#define ULTIMA8_USECODE_U8INTRINSICS_H

#include "ultima/ultima8/usecode/intrinsics.h"

#include "ultima/ultima8/world/item.h"
#include "ultima/ultima8/usecode/uc_machine.h"
#include "ultima/ultima8/world/container.h"
#include "ultima/ultima8/world/actors/actor.h"
#include "ultima/ultima8/world/actors/main_actor.h"
#include "ultima/ultima8/kernel/kernel.h"
#include "ultima/ultima8/world/camera_process.h"
#include "ultima/ultima8/ultima8.h"
#include "ultima/ultima8/world/egg.h"
#include "ultima/ultima8/world/monster_egg.h"
#include "ultima/ultima8/world/current_map.h"
#include "ultima/ultima8/gfx/palette_fader_process.h"
#include "ultima/ultima8/world/sprite_process.h"
#include "ultima/ultima8/audio/music_process.h"
#include "ultima/ultima8/gumps/scroll_gump.h"
#include "ultima/ultima8/gumps/book_gump.h"
#include "ultima/ultima8/gumps/readable_gump.h"
#include "ultima/ultima8/gumps/target_gump.h"
#include "ultima/ultima8/world/actors/grant_peace_process.h"
#include "ultima/ultima8/world/fireball_process.h"
#include "ultima/ultima8/world/actors/heal_process.h"
#include "ultima/ultima8/gfx/inverter_process.h"
#include "ultima/ultima8/audio/audio_process.h"
#include "ultima/ultima8/games/game.h"

namespace Ultima {
namespace Ultima8 {

// Ultima 8 Intrinsics
const Intrinsic U8Intrinsics[] = {
	// 0x000
	TargetGump::I_target,
	0, //U
	Item::I_touch,
	Item::I_getX,
	Item::I_getY,
	Item::I_getZ,
	Item::I_getCX,
	Item::I_getCY,
	Item::I_getCZ,
	0, //U
	0, //U
	0, //U
	Item::I_getPoint,
	Item::I_getShape,
	Item::I_setShape,
	Item::I_getFrame,
	// 0x010
	Item::I_setFrame,
	Item::I_getQuality,
	Item::I_getUnkEggType,
	Item::I_getQuantity,
	0, //U
	Item::I_getContainer,
	Item::I_getRootContainer,
	0, //U
	0, //U
	Item::I_getQ,
	Item::I_setQ,
	Item::I_setQuality,
	0, //U
	Item::I_setQuantity,
	Item::I_getFamily,
	Item::I_getTypeFlag,
	// 0x020
	Item::I_getStatus,
	Item::I_orStatus,
	Item::I_andStatus,
	Item::I_getFootpadData,
	0, //U
	Item::I_overlaps,
	Item::I_overlapsXY,
	Item::I_isOn,
	0, //U
	0, //U
	0, //U
	Item::I_ascend,
	Item::I_getWeight,
	Item::I_getWeightIncludingContents,
	Item::I_getSurfaceWeight,
	0, //U
	// 0x030
	0, //U
	Item::I_legalCreateAtCoords,
	Item::I_create,
	Item::I_legalCreateAtPoint,
	Item::I_legalCreateInCont,
	Item::I_push,
	Item::I_popToCoords,
	Item::I_popToContainer,
	Item::I_pop,
	Item::I_popToEnd,
	Item::I_destroy,
	Container::I_removeContents,
	Container::I_destroyContents,
	Item::I_isExplosive,
	Item::I_move,
	0, //U
	// 0x040
	Item::I_legalMoveToPoint,
	Item::I_legalMoveToContainer,
	Actor::I_isNPC,
	0, //U
	Item::I_hurl,
	Item::I_shoot,
	Item::I_fall,
	Item::I_grab,
	0, //U
	Item::I_bark,
	Item::I_ask,
	Item::I_getSliderInput,
	Item::I_openGump,
	Item::I_closeGump,
	0, //U
	0, //U
	// 0x050
	Item::I_getMapArray,
	0, //U
	Item::I_setMapArray,
	Item::I_receiveHit,
	Item::I_explode,
	Item::I_canReach,
	Item::I_getRange,
	0, //U
	Item::I_getDirToCoords,
	Item::I_getDirFromCoords,
	Item::I_getDirToItem,
	Item::I_getDirFromItem,
	Item::I_look,
	Item::I_use,
	0, //U
	0, //U
	// 0x060
	0, //U
	Item::I_gotHit,
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	Item::I_enterFastArea,
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	Item::I_guardianBark,
	BookGump::I_readBook,
	ScrollGump::I_readScroll,
	// 0x070
	ReadableGump::I_readGrave,
	ReadableGump::I_readPlaque,
	Egg::I_getEggXRange,
	Egg::I_getEggYRange,
	Egg::I_setEggXRange,
	Egg::I_setEggYRange,
	Egg::I_getEggId,
	Egg::I_setEggId,
	0, //U
	MonsterEgg::I_monsterEggHatch,
	MonsterEgg::I_getMonId,
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	// 0x080
	Actor::I_isBusy,
	Actor::I_areEnemiesNear,
	Actor::I_isInCombat,
	Actor::I_setInCombat,
	Actor::I_clrInCombat,
	Actor::I_setTarget,
	Actor::I_getTarget,
	Actor::I_setAlignment,
	Actor::I_getAlignment,
	Actor::I_setEnemyAlignment,
	Actor::I_getEnemyAlignment,
	Actor::I_isEnemy,
	Actor::I_isDead,
	Actor::I_setDead,
	Actor::I_clrDead,
	Actor::I_isImmortal,
	// 0x090
	Actor::I_setImmortal,
	Actor::I_clrImmortal,
	Actor::I_isWithstandDeath,
	Actor::I_setWithstandDeath,
	Actor::I_clrWithstandDeath,
	Actor::I_isFeignDeath,
	Actor::I_setFeignDeath,
	Actor::I_clrFeignDeath,
	0, //U
	0, //U
	0, //U
	0, //U
	Actor::I_getDir,
	Actor::I_getMap,
	Actor::I_teleport,
	Actor::I_doAnim,
	// 0x0A0
	Actor::I_getLastAnimSet,
	Actor::I_pathfindToPoint,
	Actor::I_pathfindToItem,
	Actor::I_getStr,
	Actor::I_getInt,
	Actor::I_getDex,
	Actor::I_getHp,
	Actor::I_getMana,
	Actor::I_setStr,
	Actor::I_setInt,
	Actor::I_setDex,
	Actor::I_setHp,
	Actor::I_setMana,
	Actor::I_createActor,
	Actor::I_setActivity,
	Actor::I_setAirWalkEnabled,
	// 0x0B0
	Actor::I_getAirWalkEnabled,
	Actor::I_schedule,
	Actor::I_getEquip,
	Actor::I_setEquip,
	Ultima8Engine::I_closeItemGumps,
	CameraProcess::I_scrollTo,
	UCMachine::I_urandom,
	UCMachine::I_rndRange,
	GrantPeaceProcess::I_castGrantPeace,
	UCMachine::I_numToStr,
	0, //U
	MusicProcess::I_playMusic,
	UCMachine::I_getName, //temp
	Item::I_igniteChaos,
	CameraProcess::I_setCenterOn,
	CameraProcess::I_moveTo,
	// 0x0C0
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	0, //U
	CameraProcess::I_startQuake,
	CameraProcess::I_stopQuake,
	InverterProcess::I_invertScreen,
	0, //U
	Kernel::I_getNumProcesses,
	Kernel::I_resetRef,
	MainActor::I_teleportToEgg,
	Kernel::I_resetRef,
	0, // setRef
	Ultima8Engine::I_getAvatarInStasis,
	// 0x0D0
	Ultima8Engine::I_setAvatarInStasis,
	Item::I_getEtherealTop,
	Ultima8Engine::I_getCurrentTimerTick,
	0, //U (canGetThere)
	CurrentMap::I_canExistAt,
	SpriteProcess::I_createSprite,
	SpriteProcess::I_createSprite,
	Item::I_getFamilyOfType,
	Ultima8Engine::I_getTimeInGameHours,
	Ultima8Engine::I_getTimeInMinutes,
	Ultima8Engine::I_getTimeInSeconds,
	Ultima8Engine::I_setTimeInGameHours,
	0, // U (SetTimeInMinutes)
	0, // U (SetTimeInSeconds)
	PaletteFaderProcess::I_fadeToBlack,
	PaletteFaderProcess::I_fadeFromBlack,
	// 0x0E0
	PaletteFaderProcess::I_fadeToPaletteTransform,
	PaletteFaderProcess::I_lightningBolt,
	PaletteFaderProcess::I_fadeToWhite,
	PaletteFaderProcess::I_fadeFromWhite,
	Game::I_playEndgame,
	HealProcess::I_feedAvatar,
	MainActor::I_accumulateStrength,
	MainActor::I_accumulateIntelligence,
	MainActor::I_accumulateDexterity,
	MainActor::I_clrAvatarInCombat,
	MainActor::I_setAvatarInCombat,
	MainActor::I_isAvatarInCombat,
	AudioProcess::I_playSFX,        // int16
	AudioProcess::I_playSFX,        // int16 uint8
	AudioProcess::I_playSFX,        // int16 uint16 ObjID
	AudioProcess::I_playAmbientSFX, // int16              Unused (but implemented)
	// 0x0F0
	AudioProcess::I_playAmbientSFX, // int16 uint8        Unused (but implemented)
	AudioProcess::I_playAmbientSFX, // int16 uint16 ObjID
	AudioProcess::I_isSFXPlaying,
	AudioProcess::I_setVolumeSFX,
	AudioProcess::I_stopSFX,        // int16
	AudioProcess::I_stopSFX,        // int16 ObjID
	0, //U
	0, //U
	MusicProcess::I_stopMusic,
	0, //U
	0, //U
	FireballProcess::I_TonysBalls,
	Ultima8Engine::I_avatarCanCheat,
	Ultima8Engine::I_makeAvatarACheater,
	UCMachine::I_true, // isGameRunning
	0, //U
	// 0x100
	0, //U
	0  //U
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
