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

#include "harvester/dialogue/dialogue.h"
#include "harvester/dialogue/dialogue_session.h"

#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/npc/authority_dialogue.h"
#include "harvester/npc/beggar_dialogue.h"
#include "harvester/npc/boyle_dialogue.h"
#include "harvester/npc/buster_dialogue.h"
#include "harvester/npc/butcher_dialogue.h"
#include "harvester/npc/chessmaster_dialogue.h"
#include "harvester/npc/cloak_atnd_dialogue.h"
#include "harvester/npc/curator_dialogue.h"
#include "harvester/npc/dad_dialogue.h"
#include "harvester/npc/dark_woman_dialogue.h"
#include "harvester/npc/dwayne_dialogue.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/npc/edna_dialogue.h"
#include "harvester/npc/gladiator_dialogue.h"
#include "harvester/npc/fireman1_dialogue.h"
#include "harvester/npc/fireman2_dialogue.h"
#include "harvester/npc/hank_dialogue.h"
#include "harvester/npc/herrill_dialogue.h"
#include "harvester/npc/inquisitor_dialogue.h"
#include "harvester/npc/jimmy_dialogue.h"
#include "harvester/npc/johnson_dialogue.h"
#include "harvester/npc/karin_dialogue.h"
#include "harvester/npc/librarian_dialogue.h"
#include "harvester/npc/loomis_dialogue.h"
#include "harvester/npc/lodge_chef_dialogue.h"
#include "harvester/npc/mcknight_dialogue.h"
#include "harvester/npc/memb_dir_dialogue.h"
#include "harvester/npc/maint_man_dialogue.h"
#include "harvester/npc/madam_dialogue.h"
#include "harvester/npc/mom_dialogue.h"
#include "harvester/npc/mother_dialogue.h"
#include "harvester/npc/moynahan_dialogue.h"
#include "harvester/npc/mr_potts_dialogue.h"
#include "harvester/npc/mrs_potts_dialogue.h"
#include "harvester/npc/parsons_dialogue.h"
#include "harvester/npc/pastorelli_dialogue.h"
#include "harvester/npc/phelps_dialogue.h"
#include "harvester/npc/priest_dialogue.h"
#include "harvester/npc/pta_mom_dialogue.h"
#include "harvester/npc/ryder_dialogue.h"
#include "harvester/npc/sergeant_dialogue.h"
#include "harvester/npc/nude_man_dialogue.h"
#include "harvester/npc/stephanie_dialogue.h"
#include "harvester/npc/stub_dialogue.h"
#include "harvester/npc/sparky_dialogue.h"
#include "harvester/npc/swell_dialogue.h"
#include "harvester/npc/valet_dialogue.h"
#include "harvester/npc/vet_dialogue.h"
#include "harvester/npc/whaley_dialogue.h"
#include "harvester/npc/wasp_woman_dialogue.h"

namespace Harvester {

namespace {

static void syncDialogueSharedState(Common::Serializer &s, DialogueSharedState &state) {
	syncDialogueBool(s, state.boyleGascanApplicationState);
	syncDialogueBool(s, state.dialogueStateD2e98);
	syncDialogueBool(s, state.dialogueStateD2eb0);
	syncDialogueBool(s, state.dialogueStateD2eb8);
	syncDialogueBool(s, state.dialogueStateD2ebc);
	syncDialogueBool(s, state.dialogueStateD2ec0);
	syncDialogueBool(s, state.dialogueStateD2ec8);
	syncDialogueBool(s, state.dialogueStateD2eec);
	syncDialogueBool(s, state.dialogueStateD2ed0);
	syncDialogueBool(s, state.dialogueStateD2ef4);
	syncDialogueBool(s, state.dialogueStateD2f04);
	syncDialogueBool(s, state.dialogueStateD2ea4);
	syncDialogueBool(s, state.dialogueStateD2ea8);
	syncDialogueBool(s, state.dialogueStateD2f00);
	syncDialogueBool(s, state.karinKidnapedDialogueState);
	syncDialogueBool(s, state.discussedLodgeTopic);
	syncDialogueBool(s, state.dialogueStateD2f08);
	syncDialogueBool(s, state.waspWomanDialogueState);
	syncDialogueBool(s, state.momGoodCauseDay5State);
	syncDialogueBool(s, state.moynahanKarinKidnapedDiscussionState);
	syncDialogueBool(s, state.dwayneWhaleyDisciplineFollowupState);
	syncDialogueBool(s, state.dwayneDiscussedBoylesButton);
	syncDialogueBool(s, state.dwayneCompletedKarinAliveFollowup);
	syncDialogueBool(s, state.discussedMrPottsTuesdayNightAlibi);
	syncDialogueBool(s, state.discussedMrsPottsTuesdayNightAlibi);
	syncDialogueBool(s, state.confrontedMrPottsAboutSpyhole);
	syncDialogueInt(s, state.serializedTalkStateFlagD2cd8);
	syncDialogueInt(s, state.dwaynePendingKarinAliveFollowupState);
	syncDialogueInt(s, state.dialogueStateD2eac);
	syncDialogueInt(s, state.dialogueStateD2eb4);
	syncDialogueInt(s, state.discussedNoteCheckbookEvidence);
	syncDialogueInt(s, state.discussedTvDeedEvidence);
	syncDialogueInt(s, state.discussedLedgerEvidence);
	syncDialogueInt(s, state.discussedCasketPhotoEvidence);
	syncDialogueInt(s, state.discussedWhaleyHerrillPhoto);
	syncDialogueInt(s, state.discussedKarinPurse);
	syncDialogueInt(s, state.momFatherTopicState);
	syncDialogueInt(s, state.dadMeatPermissionState);
	syncDialogueInt(s, state.sergeantCompletedFirstTaskState);
	syncDialogueInt(s, state.dialogueStateD2f30);
}

} // End of anonymous namespace

DialogueSystem::DialogueSystem(HarvesterEngine &engine, Common::Point &mousePos)
	: _engine(engine), _mousePos(mousePos) {
	registerNpcHandlers();
}

DialogueSystem::~DialogueSystem() {
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		delete _npcHandlers[i].handler;
}

NpcDialogueHandler *DialogueSystem::findNpcHandler(const Common::String &npcName) const {
	for (const NpcHandlerRegistration &registration : _npcHandlers) {
		for (const char *name : registration.names) {
			if (npcName.equalsIgnoreCase(name))
				return registration.handler;
		}
	}
	return nullptr;
}

bool DialogueSystem::hasRoomNpcHandler(const Common::String &npcName) const {
	return findNpcHandler(npcName) != nullptr;
}

void DialogueSystem::registerNpcHandlers() {
	_npcHandlers.push_back({ new AuthorityDialogueHandler(), { "AUTHORITY", "AUTHORITY2" } });
	_npcHandlers.push_back({ new BeggarDialogueHandler(), { "BEGGAR" } });
	_npcHandlers.push_back({ new BoyleDialogueHandler(), { "BOYLE" } });
	_npcHandlers.push_back({ new BusterDialogueHandler(), { "BUSTER" } });
	_npcHandlers.push_back({ new ButcherDialogueHandler(), { "BUTCHER" } });
	_npcHandlers.push_back({ new DwayneDialogueHandler(), { "DWAYNE", "DWAYNE_DNALFT", "DWAYNE_ST_BEDRM" } });
	_npcHandlers.push_back({ new EdnaDialogueHandler(), { "EDNA" } });
	_npcHandlers.push_back({ new Fireman1DialogueHandler(), { "FIREMAN1" } });
	_npcHandlers.push_back({ new Fireman2DialogueHandler(), { "FIREMAN2" } });
	_npcHandlers.push_back({ new ChessmasterDialogueHandler(), { "CHESSMASTER" } });
	_npcHandlers.push_back({ new CloakAtndDialogueHandler(), { "CLOAK_ATND" } });
	_npcHandlers.push_back({ new CuratorDialogueHandler(), { "CURATOR" } });
	_npcHandlers.push_back({ new DadDialogueHandler(), { "DAD" } });
	_npcHandlers.push_back({ new DarkWomanDialogueHandler(), { "DARK_WOMAN" } });
	_npcHandlers.push_back({ new JimmyDialogueHandler(), { "JIMMY" } });
	_npcHandlers.push_back({ new InquisitorDialogueHandler(), { "INQUISITOR" } });
	_npcHandlers.push_back({ new KarinDialogueHandler(), { "KARIN", "KARIN_STANDING", "KARIN_CEM10", "KARIN_OFFICE" } });
	_npcHandlers.push_back({ new WaspWomanDialogueHandler(), { "WASP_WOMAN" } });
	_npcHandlers.push_back({ new GladiatorDialogueHandler(), { "GLADIATOR" } });
	_npcHandlers.push_back({ new LibrarianDialogueHandler(), { "LIBRARIAN" } });
	_npcHandlers.push_back({ new LoomisDialogueHandler(), { "LOOMIS" } });
	_npcHandlers.push_back({ new McknightDialogueHandler(), { "MCKNIGHT" } });
	_npcHandlers.push_back({ new MembDirDialogueHandler(), { "MEMB_DIR" } });
	_npcHandlers.push_back({ new MaintManDialogueHandler(), { "MAINT_MAN" } });
	_npcHandlers.push_back({ new MadamDialogueHandler(), { "MADAM" } });
	_npcHandlers.push_back({ new MomDialogueHandler(), { "MOM" } });
	_npcHandlers.push_back({ new MotherDialogueHandler(), { "MOTHER" } });
	_npcHandlers.push_back({ new MoynahanDialogueHandler(), { "MOYNAHAN", "MOYNAHAN_MBLM" } });
	_npcHandlers.push_back({ new MrPottsDialogueHandler(), { "MR_POTTS", "POTTS_FLESH", "MR_POTTS_CEM10", "MR_POTTS_HALL" } });
	_npcHandlers.push_back({ new MrsPottsDialogueHandler(), { "MRS_POTTS", "MRS_POTTS_ST_BEDRM" } });
	_npcHandlers.push_back({ new LodgeChefDialogueHandler(), { "LODGE_CHEF" } });
	_npcHandlers.push_back({ new NudeManDialogueHandler(), { "NUDE_MAN" } });
	_npcHandlers.push_back({ new ParsonsDialogueHandler(), { "PARSONS" } });
	_npcHandlers.push_back({ new PastorelliDialogueHandler(), { "PASTORELLI" } });
	_npcHandlers.push_back({ new PhelpsDialogueHandler(), { "PHELPS" } });
	_npcHandlers.push_back({ new PriestDialogueHandler(), { "PRIEST" } });
	_npcHandlers.push_back({ new RyderDialogueHandler(), { "RYDER", "RYDER_TV_OUT" } });
	_npcHandlers.push_back({ new ValetDialogueHandler(), { "VALET" } });
	_npcHandlers.push_back({ new VetDialogueHandler(), { "VET" } });
	_npcHandlers.push_back({ new SparkyDialogueHandler(), { "SPARKY" } });
	_npcHandlers.push_back({ new StephanieDialogueHandler(), { "STEPHANIE" } });
	_npcHandlers.push_back({ new SwellDialogueHandler(), { "SWELL" } });
	_npcHandlers.push_back({ new WhaleyDialogueHandler(), { "WHALEY" } });
	_npcHandlers.push_back({ new HankDialogueHandler(), { "HANK" } });
	_npcHandlers.push_back({ new HerrillDialogueHandler(), { "HERRILL", "HERRILL_LOG" } });
	_npcHandlers.push_back({ new JohnsonDialogueHandler(), { "JOHNSON" } });
	_npcHandlers.push_back({ new PtaMomDialogueHandler(), { "PTA_MOM1", "PTA_MOM2", "PTA_MOM3", "PTA_MOM4", "PTA_MOM5" } });

	_npcHandlers.push_back({ new SergeantDialogueHandler(), { "SERGEANT" } });
	_npcHandlers.push_back({ new StubNpcDialogueHandler("PTA_MOM"), { "PTA_MOM" } });
}

void DialogueSystem::resetRoomNpcDialogueState() {
	_sharedDialogueState = DialogueSharedState();
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		_npcHandlers[i].handler->resetState();
}

void DialogueSystem::syncRuntimeSaveState(Common::Serializer &s) {
	syncDialogueSharedState(s, _sharedDialogueState);
	for (uint i = 0; i < _npcHandlers.size(); ++i)
		_npcHandlers[i].handler->syncState(s);
	if (s.isLoading()) {
		for (uint i = 0; i < _npcHandlers.size(); ++i)
			_npcHandlers[i].handler->migrateSharedState(_sharedDialogueState);
	}
}

Common::Error DialogueSystem::runRoomNpcDialogue(const IndexedBitmap &backdrop, const byte *palette,
		float paletteBrightness, const ScriptExecutionContext &executionContext,
		const NpcRecord &npc, const Common::String &usedItemName,
		Flow &flow) {
	Script *script = _engine.getScript();
	Text *text = _engine.getText();
	if (!script || !text || !backdrop.isValid())
		return Common::kReadingFailed;
	NpcDialogueHandler *handler = findNpcHandler(npc.npcName);
	if (!handler) {
		debugC(1, kDebugDialogue,
			"Harvester: blocked room NPC dialogue npc='%s' reason='no handler'",
			npc.npcName.c_str());
		return Common::kNoError;
	}
	if (!script->isNamedNpcDeathTypeClear(npc.npcName)) {
		const NpcRecord *runtimeNpc = script->findRuntimeNpcRecord(npc.npcName);
		debugC(1, kDebugDialogue,
			"Harvester: blocked room NPC dialogue npc='%s' reason='death queued' damage_type=%d",
			npc.npcName.c_str(), runtimeNpc ? runtimeNpc->deathDamageType : -1);
		return Common::kNoError;
	}

	if (!usedItemName.empty() && script->addRuntimeObjectToInventory(usedItemName)) {
		debugC(2, kDebugDialogue, "Harvester: presented item moved to inventory object='%s' npc='%s'",
			usedItemName.c_str(), npc.npcName.c_str());
	}

	RoomNpcDialogueSession session(_engine, _mousePos, flow, backdrop, palette, paletteBrightness, npc);
	if (!session.isValid())
		return Common::kReadingFailed;
	// START_DIALOG may invoke an NPC assigned to another room.
	DialogueRuntime runtime(
		_engine, *script, *text, flow, executionContext, session.getGenericByeTopic(), session);

	return handler->handleDialogue(runtime, usedItemName, _sharedDialogueState);
}

} // End of namespace Harvester
