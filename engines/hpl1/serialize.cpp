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

#include "serialize.h"

#include "hpl1/engine/engine.h"
#include "hpl1/engine/system/SerializeClass.h"

#include "hpl1/penumbra-overture/ButtonHandler.h"
#include "hpl1/penumbra-overture/GameArea.h"
#include "hpl1/penumbra-overture/GameDamageArea.h"
#include "hpl1/penumbra-overture/GameEnemy.h"
#include "hpl1/penumbra-overture/GameForceArea.h"
#include "hpl1/penumbra-overture/GameItem.h"
#include "hpl1/penumbra-overture/GameLadder.h"
#include "hpl1/penumbra-overture/GameLamp.h"
#include "hpl1/penumbra-overture/GameLink.h"
#include "hpl1/penumbra-overture/GameLiquidArea.h"
#include "hpl1/penumbra-overture/GameMessageHandler.h"
#include "hpl1/penumbra-overture/GameObject.h"
#include "hpl1/penumbra-overture/GameSaveArea.h"
#include "hpl1/penumbra-overture/GameScripts.h"
#include "hpl1/penumbra-overture/GameStickArea.h"
#include "hpl1/penumbra-overture/GameSwingDoor.h"
#include "hpl1/penumbra-overture/Init.h"
#include "hpl1/penumbra-overture/MapHandler.h"
#include "hpl1/penumbra-overture/Player.h"

#include "hpl1/penumbra-overture/AttackHandler.h"
#include "hpl1/penumbra-overture/Credits.h"
#include "hpl1/penumbra-overture/DeathMenu.h"
#include "hpl1/penumbra-overture/DemoEndText.h"
#include "hpl1/penumbra-overture/EffectHandler.h"
#include "hpl1/penumbra-overture/FadeHandler.h"
#include "hpl1/penumbra-overture/GameMusicHandler.h"
#include "hpl1/penumbra-overture/GraphicsHelper.h"
#include "hpl1/penumbra-overture/Inventory.h"
#include "hpl1/penumbra-overture/MapLoadText.h"
#include "hpl1/penumbra-overture/Notebook.h"
#include "hpl1/penumbra-overture/NumericalPanel.h"
#include "hpl1/penumbra-overture/PlayerHands.h"
#include "hpl1/penumbra-overture/PreMenu.h"
#include "hpl1/penumbra-overture/RadioHandler.h"
#include "hpl1/penumbra-overture/SaveHandler.h"
#include "hpl1/penumbra-overture/TriggerHandler.h"

namespace Hpl1 {

#define SERIALIZE(klass, parent) serializeTempClasses[nSerializeTempClasses++] = cSerializeSavedClass(#klass, #parent, klass::mpSerialize_MemberFields, sizeof(klass), []() { return static_cast<iSerializable *>(new klass()); })
#define SERIALIZE_BASE(klass) serializeTempClasses[nSerializeTempClasses++] = cSerializeSavedClass(#klass, "", klass::mpSerialize_MemberFields, sizeof(klass), []() { return static_cast<iSerializable *>(new klass()); })
#define SERIALIZE_VIRTUAL(klass, parent) serializeTempClasses[nSerializeTempClasses++] = cSerializeSavedClass(#klass, #parent, klass::mpSerialize_MemberFields, sizeof(klass), nullptr)
#define SERIALIZE_BASE_VIRTUAL(klass) serializeTempClasses[nSerializeTempClasses++] = cSerializeSavedClass(#klass, "", klass::mpSerialize_MemberFields, sizeof(klass), nullptr)

using namespace hpl;

cSerializeSavedClass serializeTempClasses[1000];
int nSerializeTempClasses = 0;

void engineSerializeInit() {
	SERIALIZE_BASE_VIRTUAL(iSaveData);
	SERIALIZE_BASE(cScriptVar);
	SERIALIZE_BASE(cBoundingVolume);
	SERIALIZE(cSaveData_cSoundEntity, cSaveData_iEntity3D);
	SERIALIZE(cSaveData_cSubMeshEntity, cSaveData_iRenderable);
	SERIALIZE_BASE(cAreaEntity);
	SERIALIZE_BASE(cStartPosEntity);
	SERIALIZE_BASE(cSaveData_cWorld3D);
	SERIALIZE_VIRTUAL(cSaveData_iNode, iSaveData);
	SERIALIZE(cSaveData_cNode3D, cSaveData_iNode);
	SERIALIZE_BASE(cSaveData_cAnimationState);
	SERIALIZE(cSaveData_cMeshEntity, cSaveData_iRenderable);
	SERIALIZE_VIRTUAL(cSaveData_iEntity, iSaveData);
	SERIALIZE(cSaveData_cLight3DSpot, cSaveData_iLight3D);
	SERIALIZE_VIRTUAL(cSaveData_iEntity3D, cSaveData_iEntity);
	SERIALIZE_VIRTUAL(cSaveData_iLight3D, cSaveData_iRenderable);
	SERIALIZE(cSaveData_cLight3DPoint, cSaveData_iLight3D);
	SERIALIZE(cSaveData_iPhysicsJointHinge, cSaveData_iPhysicsJoint);
	SERIALIZE(cSaveData_iPhysicsBody, cSaveData_iEntity3D);
	SERIALIZE_BASE(cSaveData_iCollideShape);
	SERIALIZE(cSaveData_iPhysicsJointBall, cSaveData_iPhysicsJoint);
	SERIALIZE_BASE(cJointLimitEffect);
	SERIALIZE_VIRTUAL(cSaveData_iPhysicsJoint, iSaveData);
	SERIALIZE_BASE(cSaveData_iPhysicsController);
	SERIALIZE(cSaveData_iCharacterBody, iSaveData);
	SERIALIZE(cSaveData_iPhysicsJointSlider, cSaveData_iPhysicsJoint);
	SERIALIZE(cSaveData_iPhysicsJointScrew, cSaveData_iPhysicsJoint);
	SERIALIZE_VIRTUAL(cSaveData_iRenderable, cSaveData_iEntity3D);
	SERIALIZE(cSaveData_cBillboard, cSaveData_iRenderable);
	SERIALIZE(cSaveData_iParticleEmitter3D, cSaveData_iRenderable);
	SERIALIZE(cSaveData_cParticleSystem3D, cSaveData_iEntity3D);
	SERIALIZE_BASE(cSaveData_ParticleEmitter3D);
}

void penumbraOvertureSerializeInit() {
	SERIALIZE_BASE(cInventoryUseCallback);
	SERIALIZE_BASE(cInventoryPickupCallback);
	SERIALIZE_BASE(cInventoryCombineCallback);
	SERIALIZE_BASE(cSaveData_cInventory);
	SERIALIZE(cGameLiquidArea_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameLamp_SaveData, iGameEntity_SaveData);
	SERIALIZE_BASE(cEngineBody_SaveData);
	SERIALIZE_BASE(cEngineJointController_SaveData);
	SERIALIZE_BASE(cEngineJoint_SaveData);
	SERIALIZE_BASE(cEnginePSEmitter_SaveData);
	SERIALIZE_BASE(cEnginePS_SaveData);
	SERIALIZE_BASE(cEngineBeam_SaveData);
	SERIALIZE_BASE(cEngineSound_SaveData);
	SERIALIZE_BASE(cEngineLightAttachBB_SaveData);
	SERIALIZE_BASE(cEngineLight_SaveData);
	SERIALIZE(cGameLink_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameForceArea_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameObject_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameItem_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameStickArea_SaveData, iGameEntity_SaveData);
	SERIALIZE_BASE(cSaveData_cPlayer);
	SERIALIZE(cGameLadder_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameDamageArea_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameSaveArea_SaveData, iGameEntity_SaveData);
	SERIALIZE(cGameSwingDoor_SaveData, iGameEntity_SaveData);
	SERIALIZE_BASE(cGameEntityScript);
	SERIALIZE_BASE(cGameEntityAnimation_SaveData);
	SERIALIZE_BASE_VIRTUAL(iGameEntity_SaveData);
	SERIALIZE(cGameArea_SaveData, iGameEntity_SaveData);
	SERIALIZE_BASE(cMusic_GlobalSave);
	SERIALIZE_BASE(cGameMusic_GlobalSave);
	SERIALIZE_BASE(cGameMusicHandler_GlobalSave);
	SERIALIZE_BASE(cNotebookTask_GlobalSave);
	SERIALIZE_BASE(cNotebookNote_GlobalSave);
	SERIALIZE_BASE(cNotebook_GlobalSave);
	SERIALIZE_BASE(cInventorySlot_GlobalSave);
	SERIALIZE_BASE(cInventoryItem_GlobalSave);
	SERIALIZE_BASE(cInventory_GlobalSave);
	SERIALIZE_BASE(cSceneLoadedMap_GlobalSave);
	SERIALIZE_BASE(cMapHandlerLoadedMap_GlobalSave);
	SERIALIZE_BASE(cMapHandlerTimer_GlobalSave);
	SERIALIZE_BASE(cMapHandler_GlobalSave);
	SERIALIZE_BASE(cPlayer_GlobalSave_CameraPS);
	SERIALIZE_BASE(cPlayer_GlobalSave);
	SERIALIZE_BASE(cSavedWorld);
	SERIALIZE_BASE(cSavedGame);
	SERIALIZE_BASE(cGameTimer);
	SERIALIZE_BASE(cSaveGame_cGameCollideScript);
	SERIALIZE(iGameEnemy_SaveData, iGameEntity_SaveData);
	SERIALIZE_BASE(cEnemyPatrolNode);
}

void serializeInit() {
	nSerializeTempClasses = 0;
}

} // namespace Hpl1
