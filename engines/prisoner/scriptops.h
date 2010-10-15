#ifndef PRISONER_SCRIPTOPS_H
#define PRISONER_SCRIPTOPS_H

#include "common/array.h"
#include "common/func.h"

#include "prisoner/prisoner.h"

namespace Prisoner {

class Opcode {
public:
	Opcode(Common::Functor1<Script*, int16> *call, Common::String name)
		: _call(call), _name(name) {}
public:
	Common::Functor1<Script*, int16> *_call;
	Common::String _name;
};

// Macros for convenience and clarity
#define ARG_INT16(name) \
	int16 name = script->readInt16(); \
	debug(1, "const() " #name " = %d", name);
#define ARG_EVALUATE(name) \
	int16 name = evaluate(script); \
	debug(1, "evaluate() " #name " = %d", name);
#define ARG_STRING(name) \
	Common::String name = script->readString(); \
	debug(1, "string() " #name " = %s", name.c_str());
#define ARG_PAKNAME(name, languageSpecific) \
	Common::String name = readPakName(script, languageSpecific); \
	debug(1, "pakName() " #name " = %s", name.c_str());
#define ARG_PAKNAME_S(name, languageSpecific) \
	Common::String name = "S" + readPakName(script, languageSpecific); \
	debug(1, "pakNameS() " #name " = %s", name.c_str());

class ScriptOpcodes {
public:
	ScriptOpcodes(PrisonerEngine *vm) : _vm(vm) {}
	void setupOpcodes();
	int16 execOpcode(Script *script, byte opcode);
protected:
	PrisonerEngine *_vm;
	Common::Array<const Opcode*> _opcodes;

	int16 evaluate(Script *script);
	Common::String readPakName(Script *script, bool languageSpecific);

	int16 op_returnNegOne(Script *script);
	int16 op_setBackground(Script *script);
	int16 op_startScript(Script *script);
	int16 op_jumpRel(Script *script);
	int16 op_getGlobalVar(Script *script);
	int16 op_do(Script *script);
	int16 op_true(Script *script);
	int16 op_lt(Script *script);
	int16 op_gt(Script *script);
	int16 op_lte(Script *script);
	int16 op_gte(Script *script);
	int16 op_boolAnd(Script *script);
	int16 op_boolOr(Script *script);
	int16 op_add(Script *script);
	int16 op_sub(Script *script);
	int16 op_mul(Script *script);
	int16 op_execTwo(Script *script);
	int16 op_getModuleVar(Script *script);
	int16 op_loadConstWord(Script *script);
	int16 op_if(Script *script);
	int16 op_sleep(Script *script);
	int16 op_incVar(Script *script);
	int16 op_decVar(Script *script);
	int16 op_setGlobalVar(Script *script);
	int16 op_setModuleVar(Script *script);
	int16 op_setPalette(Script *script);
	int16 op_clearBackground(Script *script);
	int16 op_loadSceneSound(Script *script);
	int16 op_setSoundVolume(Script *script);
	int16 op_stopScript(Script *script);
	int16 op_loadMusic(Script *script);
	int16 op_setMusicVolume(Script *script);
	int16 op_gotoScene(Script *script);
	int16 op_sub_22310(Script *script);
	int16 op_waitForInput(Script *script);
	int16 op_sub_2234D(Script *script);
	int16 op_random(Script *script);
	int16 op_paletteFunc(Script *script);
	int16 op_copyPalette(Script *script);
	int16 op_setScriptContinueFlag(Script *script);
	int16 op_clearScriptContinueFlag(Script *script);
	int16 op_setSomeArrayValue(Script *script);
	int16 op_syncScript(Script *script);
	int16 op_false(Script *script);
	int16 op_getSysVar(Script *script);
	int16 op_addPathNode(Script *script);
	int16 op_addPathEdge(Script *script);
	int16 op_addPathPolygon(Script *script);
	int16 op_actorWalkToPathNode(Script *script);
	int16 op_setActorDirection(Script *script);
	int16 op_addActorScreenText(Script *script);
	int16 op_setVar(Script *script);
	int16 op_addActorAtPathNode(Script *script);
	int16 op_setMainActor(Script *script);
	int16 op_addActorZone(Script *script);
	int16 op_addZoneAction(Script *script);
	int16 op_addTextZone(Script *script);
	int16 op_addSceneItem(Script *script);
	int16 op_addItemZone(Script *script);
	int16 op_startDialog(Script *script);
	int16 op_registerInventoryItem(Script *script);
	int16 op_addInventoryItemCombination(Script *script);
	int16 op_getSomeArrayValue(Script *script);
	int16 op_loadDialog(Script *script);
	int16 op_addItemToInventory(Script *script);
	int16 op_enableDialogKeyword(Script *script);
	int16 op_disableDialogKeyword(Script *script);
	int16 op_sub_23259(Script *script);
	int16 op_sub_2328A(Script *script);
	int16 op_unloadDialog(Script *script);
	int16 op_getActorX(Script *script);
	int16 op_getActorY(Script *script);
	int16 op_setActorX(Script *script);
	int16 op_setActorY(Script *script);
	int16 op_addActorAtPos(Script *script);
	int16 op_setBackgroundObjects(Script *script);
	int16 op_actorEntersScene(Script *script);
	int16 op_addExitZoneAction(Script *script);
	int16 op_addZoneAction90(Script *script);
	int16 op_removeItemFromInventory(Script *script);
	int16 op_removeActor(Script *script);
	int16 op_clearZoneAction(Script *script);
	int16 op_path207E6(Script *script);
	int16 op_getValue(Script *script);
	int16 op_actorPutAtPos(Script *script);
	int16 op_actorPutAtPathNode(Script *script);
	int16 op_addZone(Script *script);
	int16 op_removeZone(Script *script);
	int16 op_not(Script *script);
	int16 op_actorAssignPathWalker(Script *script);
	int16 op_setUserInput(Script *script);
	int16 op_startPaletteTask(Script *script);
	int16 op_playActorAnimation(Script *script);
	int16 op_setActorSpriteFrameListIndexIfIdle(Script *script);
	int16 op_actorAnimation20CC7(Script *script);
	int16 op_resetMessageValues(Script *script);
	int16 op_playSound(Script *script);
	int16 op_unloadSound(Script *script);
	int16 op_togglePathSystem(Script *script);
	int16 op_startMusicE(Script *script);
	int16 op_unloadMusic(Script *script);
	int16 op_stopSound(Script *script);
	int16 op_stopMusic(Script *script);
	int16 op_isSoundPlaying(Script *script);
	int16 op_isMusicPlaying(Script *script);
	int16 op_addActorFrameSound(Script *script);
	int16 op_setZoneActionScript(Script *script);
	int16 op_interactActorMessage(Script *script);
	int16 op_setScriptZoneEnterLeaveFlag(Script *script);
	int16 op_setMouseAddXY(Script *script);
	int16 op_playLoopingSound(Script *script);
	int16 op_removeActorFrameSound(Script *script);
	int16 op_setActorFrameSound(Script *script);
	int16 op_playActorAnimationAtPos(Script *script);
	int16 op_startMusic(Script *script);
	int16 op_setBackgroundCameraLocked(Script *script);
	int16 op_removeInventoryItemCombination(Script *script);
	int16 op_getVar(Script *script);
	int16 op_addLooseScreenText(Script *script);
	int16 op_actorWalkToPoint(Script *script);
	int16 op_resetCurrInventoryItemCursor(Script *script);
	int16 op_interactScreenMessage(Script *script);
	int16 op_nop(Script *script);
	int16 op_addScreenText2(Script *script);
	int16 op_actorText21704(Script *script);
	int16 op_actorAnimation218A1(Script *script);
	int16 op_startModuleScript(Script *script);
	int16 op_death(Script *script);
	int16 op_loadModuleSound(Script *script);
	int16 op_playSoundSync(Script *script);
	int16 op_setActorFontColors(Script *script);
	int16 op_setTextDisplayColor(Script *script);
	int16 op_loadInventoryItemsAnimation(Script *script);
	int16 op_playMux(Script *script);
	int16 op_setCameraFollowsActor(Script *script);
	int16 op_actor21C78(Script *script);
	int16 op_actor21C89(Script *script);
	int16 op_waitForActorAnimation(Script *script);
	int16 op_addActorAltAnimation(Script *script);
	int16 op_unloadActorAltAnimation(Script *script);
	int16 op_sub_21D2D(Script *script);
	int16 op_setActorAltAnimationAtPos(Script *script);
	int16 op_actor21E0D(Script *script);
	int16 op_addZoneAction91(Script *script);
	int16 op_setUpdateDirtyRectsFlag(Script *script);
	int16 op_sub_21FD9(Script *script);
	int16 op_sub_220B8(Script *script);
	int16 op_autoSave(Script *script);
	int16 op_setMainActorValid(Script *script);

};

}
#endif
