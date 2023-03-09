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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug-channels.h"

#include "asylum/console.h"

#include "asylum/puzzles/puzzles.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/inventory.h"
#include "asylum/resources/object.h"
#include "asylum/resources/script.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/text.h"

#include "asylum/views/insertdisc.h"
#include "asylum/views/menu.h"
#include "asylum/views/scene.h"
#include "asylum/views/video.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"

namespace Asylum {

extern int g_debugActors;
extern int g_debugDrawRects;
extern int g_debugObjects;
extern int g_debugPolygons;
extern int g_debugPolygonIndex;
extern int g_debugSceneRects;
extern int g_debugScrolling;

const ResourcePackId puzzleToScenes[17] = {
	kResourcePackTowerCells,           // VCR
	kResourcePackCourtyardAndChapel,   // Pipes
	kResourcePackInnocentAbandoned,    // TicTacToe
	kResourcePackInnocentAbandoned,    // Lock
	kResourcePackInvalid,              // Puzzle 5 has no event handler
	kResourcePackLaboratory,           // Wheel
	kResourcePackLaboratory,           // Board Salvation
	kResourcePackLaboratory,           // Board Youth
	kResourcePackLaboratory,           // Board Key Hides To
	kResourcePackMorgueAndCemetery,    // Writings
	kResourcePackInvalid,              // ?? (11)
	kResourcePackMorgueAndCemetery,    // Morgue Door
	kResourcePackMansion,              // Clock
	kResourcePackMorgueAndCemetery,    // Time Machine
	kResourcePackLostVillage,          // Fisherman
	kResourcePackHive,                 // Hive Machine
	kResourcePackHive                  // Hive Control
};

#define ED_LISTEND {-1, kObjectNone, kObjectNone, 0, 0}
static const struct EncounterData {
	int32 index;
	uint32 objectId1;
	uint32 objectId2;
	ActorIndex actorIndex;
	uint32 subIndex;
} encounterData[13][31] = {
	// TowerCells
	{
		{ 0, kObjectPreAlphaNut,            kObjectPreAlphaNut,            0, 0},
		{ 1, kObjectPreAlphaNut2,           kObjectPreAlphaNut2,           0, 0},
		{ 2, kObjectRocker,                 kObjectRocker,                 0, 0},
		{73, kObjectNone,                   kObjectNone,                   0, 0},
		ED_LISTEND
	},
	// InnocentAbandoned
	{
		{ 3, kObjectJessieTalks,            kObjectJessieStatusWhileUp,    0, 0},
		{ 4, kObjectBillyTalks,             kObjectBillyStatusUp,          0, 0},
		{ 5, kObjectMariaPointsLeft,        kObjectMarisStatusQuo,         0, 0},
		{ 6, kObjectNone,                   kObjectNone,                   1, 0},
		{ 7, kObjectTalkToBallBoy,          kObjectMarty02,                0, 0},
		{ 8, kObjectNone,                   kObjectNone,                   2, 0},
		{ 9, kObjectDennisTalk,             kObjectDennisStatusQuo,        0, 0},
		{ 9, kObjectDennisTalkHide,         kObjectDennisFoundSt,          0, 1},
		{10, kObjectEleenOnGround,          kObjectIleanStatusFrame,       0, 0},
		{10, kObjectEileenTalkingWithShove, kObjectStqEileenTalkWithShov,  0, 1},
		{10, kObjectEileenOnBench,          kObjectEileenOnBench,          0, 2},
		{11, kObjectSailorBoy,              kObjectSailorBoy,              0, 0},
		{12, kObjectNone,                   kObjectNone,                   2, 0},
		{12, kObjectSuckerTalks,            kObjectSuckerSittingStatusQuo, 0, 1},
		{13, kObjectFishingBoy,             kObjectFishingBoy,             0, 0},
		{14, kObjectMotherTalking,          kObjectMotherTalkingSq,        0, 0},
		{74, kObjectCarolsDiary,            kObjectCarolsDiary,            0, 0},
		{75, kObjectObituary,               kObjectObituary,               0, 0},
		{76, kObjectObituary,               kObjectObituary,               0, 0},
		{77, kObjectObituary,               kObjectObituary,               0, 0},
		{78, kObjectObituary,               kObjectObituary,               0, 0},
		ED_LISTEND
	},
	// CourtyardAndChapel
	{
		{32, kObjectNone,                   kObjectNone,                   1, 0},
		{33, kObjectNone,                   kObjectNone,                   2, 0},
		{34, kObjectNone,                   kObjectNone,                   3, 0},
		{35, kObjectNone,                   kObjectNone,                   4, 0},
		{36, kObjectNpc024TalkChurch,       kObjectNPC024Church,           0, 0},
		{36, kObjectNpc024DanceAway,        kObjectNpc024DanceAway,        0, 1},
		{36, kObjectNpc024TalkFount,        kObjectNPC024Fountain,         0, 2},
		{37, kObjectNpc025Talking,          kObjectNpc025Talking,          0, 0},
		{38, kObjectNPC026Talking,          kObjectNPC026TalkStatusQuo,    0, 0},
		{39, kObjectNPC027Dancing,          kObjectNPC027Dancing,          0, 0},
		{39, kObjectNpc027Talk,             kObjectNPC027Sit,              0, 1},
		{40, kObjectNPC028Dancing,          kObjectNPC028Dancing,          0, 0},
		{40, kObjectNpc028Talk,             kObjectNPC028Sit,              0, 1},
		{41, kObjectNpc029Dancing,          kObjectNpc029Dancing,          0, 0},
		{41, kObjectNpc029Talk,             kObjectNPC029Sit,              0, 1},
		{42, kObjectNone,                   kObjectNone,                   5, 0},
		{43, kObjectNpc000Talking,          kObjectDrMorgan,               0, 0},
		ED_LISTEND
	},
	// CircusOfFools
	{
		{44, kObjectNpc032TalkOutside,      kObjectNPC032StatusQuoOutside, 0, 0},
		{44, kObjectRingmasterTalkTent,     kObjectNPC032StatusQuoBigTop,  0, 1},
		{45, kObjectNpc033Talking,          kObjectNpc033SqNoBook,         0, 0},
		{46, kObjectTattooingStrongMan,     kObjectTattooingStrongMan,     0, 0},
		{46, kObjectTattooGuy034Talk,       kObjectTattooManStatusQuo,     0, 1},
		{47, kObjectStrongman035Talk,       kObjectStrongmanStatusQuo,     0, 0},
		{47, kObjectStrongmanStatusQuo2,    kObjectStrongmanStatusQuo2,    0, 1},
		{48, kObjectInfernoTalk036,         kObjectInfernoStatusQuo,       0, 0},
		{49, kObjectJugglerWithPin,         kObjectJugglerWithPin,         0, 0},
		{49, kObjectJuggler,                kObjectJuggler,                0, 1},
		{50, kObjectClown038Talk,           kObjectClownStatusQuo,         0, 0},
		{51, kObjectTrixie039Talk,          kObjectTrixieSq2,              0, 0},
		{52, kObjectSSimon040Talk,          kObjectSimonSq2,               0, 0},
		{53, kObjectNone,                   kObjectNone,                   1, 0},
		{54, kObjectNone,                   kObjectNone,                   2, 0},
		{55, kObjectNone,                   kObjectNone,                   3, 0},
		{56, kObjectFunTixStatusQuo,        kObjectFunTixStatusQuo,        0, 0},
		{57, kObjectFreakTixStatusQuoUp,    kObjectFreakTixStatusQuoUp,    0, 0},
		{57, kObjectFreakTixStatusQuoDown,  kObjectFreakTixStatusQuoDown,  0, 1},
		{58, kObjectFortTellerStatusQuo,    kObjectFortTellerStatusQuo,    0, 0},
		{59, kObjectRingTossStatusQuo,      kObjectRingTossStatusQuo,      0, 0},
		{59, kObjectKnockDownStatusQuo,     kObjectKnockDownStatusQuo,     0, 1},
		{59, kObjectPigShootStatusQuo,      kObjectPigShootStatusQuo,      0, 2},
		{60, kObjectPretzool048Talk,        kObjectPretZoolStatusQuo,      0, 0},
		{61, kObjectTimber049Talk,          kObjectTimberStatusQuo,        0, 0},
		{61, kObjectTimber049Talk,          kObjectNpc049Sq2,              0, 1},
		{62, kObjectTwins050Talk,           kObjectTwinsStatusQuo,         0, 0},
		{63, kObjectSean051Talk,            kObjectSeanStatusQuo,          0, 0},
		{64, kObjectMom052Talk,             kObjectMomAndPopStatusQuo,     0, 0},
		{65, kObjectPop053Talk,             kObjectMomAndPopStatusQuo,     0, 0},
		ED_LISTEND
	},
	// Laboratory
	{
		{79, kObjectNone,                   kObjectNone,                   0, 0},
		ED_LISTEND
	},
	// Hive
	{
		{67, kObjectGravinTalkCyber,        kObjectGravinTalkCyber,        0, 0},
		{67, kObjectGravinTalkGravins,      kObjectGravinSqGravins,        0, 1},
		{67, kObjectGravinTalkDoor,         kObjectGravinSqDoor,           0, 2},
		{67, kObjectGravinTalkGromnas,      kObjectGravinSqGromnas,        0, 3},
		{67, kObjectGravinWorkMachine,      kObjectGravinWorkMachine,      0, 4},
		{68, kObjectNpc062GritzaTalk,       kObjectNPC062GritzaStatusQuo,  0, 0},
		{69, kObjectNpc063GrundleTalk,      kObjectNPC063GrundleStatusQuo, 0, 0},
		{70, kObjectNpc064GrellaTalk,       kObjectNPC064GrellaStatusQuo,  0, 0},
		{71, kObjectNpc065Talk,             kObjectNPC065StatusQuo,        0, 0},
		{72, kObjectNPC066StatusQuo,        kObjectNPC066StatusQuo,        0, 0},
		ED_LISTEND
	},
	// MorgueAndCemetery
	{
		{15, kObjectBodyTalks,              kObjectBodyStat,               0, 0},
		{16, kObjectFreezerHallInterior,    kObjectFreezerHallInterior,    0, 0},
		{17, kObjectTreeTalks,              kObjectBlinks,                 0, 0},
		ED_LISTEND
	},
	// LostVillage
	{
		{ 6, kObjectNone,                   kObjectNone,                   1, 0},
		{18, kObjectOracleTalks,            kObjectOracleTalkStatus,       0, 0},
		{19, kObjectPixelForQueztza,        kObjectPixelForQueztza,        0, 0},
		{20, kObjectWitchTalks,             kObjectWitchDoctor,            0, 0},
		{20, kObjectDeadShamanStill,        kObjectDeadShamanStill,        0, 1},
		{21, kObjectStoneMaisonTalk,        kObjectStoneMaison,            0, 0},
		{22, kObjectStoneWifeTalks,         kObjectStoneWifeStatuQuo,      0, 0},
		{23, kObjectLittleGirlTalk,         kObjectGirlStatusQuo,          0, 0},
		{24, kObjectFishermansWTalks,       kObjectFishermanWidowStatusQuo,0, 0},
		{25, kObjectWitchWifeTalking,       kObjectWitchWifeTalking,       0, 0},
		{26, kObjectGhost2,                 kObjectGhost2,                 0, 0},
		{26, kObjectJumpDown,               kObjectJumpDown,               0, 1},
		{27, kObjectGhost6,                 kObjectGhost6,                 0, 0},
		{28, kObjectGhost3,                 kObjectGhost3,                 0, 0},
		{29, kObjectGhost1,                 kObjectGhost1,                 0, 0},
		{30, kObjectGhost4,                 kObjectGhost4,                 0, 0},
		{31, kObjectGhost5,                 kObjectGhost5,                 0, 0},
		ED_LISTEND
	},
	// Gauntlet
	{
		ED_LISTEND
	},
	// Mansion
	{
		ED_LISTEND
	},
	// Cave
	{
		ED_LISTEND
	},
	// Maze
	{
		ED_LISTEND
	},
	// MorgansLastGame
	{
		ED_LISTEND
	}
};

static const int32 itemIndices[][16] = {
	{61, 69},
	{107, 134, 104, 113, 110, 112, 117, 109, 108, 111, 106},
	{170, 182, 181, 172, 171, 169},
	{61, 64, 66, 67, 68, 69, 70, 78, 77},
	{197},
	{59, 81, 60, 84, 88, 54, 74, 139, 97, 121},
	{239, 234, 249, 250, 251, 263, 237, 253},
	{58, 59, 60, 111, 75, 76, 77, 78},
	{284, 285, 286, 329, 330, 331, 332, 322, 465},
	{91, 92, 93, 94, 95},
	{69, 70, 78}
};

Console::Console(AsylumEngine *engine) : _vm(engine), _insertDisc(engine), _resViewer(engine) {
	// Commands
	registerCmd("help",           WRAP_METHOD(Console, cmdHelp));

	registerCmd("ls",             WRAP_METHOD(Console, cmdListFiles));

	registerCmd("action",         WRAP_METHOD(Console, cmdShowAction));
	registerCmd("actions",        WRAP_METHOD(Console, cmdListActions));
	registerCmd("actors",         WRAP_METHOD(Console, cmdListActors));
	registerCmd("flags",          WRAP_METHOD(Console, cmdListFlags));
	registerCmd("object",         WRAP_METHOD(Console, cmdShowObject));
	registerCmd("objects",        WRAP_METHOD(Console, cmdListObjects));
	registerCmd("world",          WRAP_METHOD(Console, cmdShowWorldStats));

	registerCmd("video",          WRAP_METHOD(Console, cmdPlayVideo));
	registerCmd("script",         WRAP_METHOD(Console, cmdRunScript));
	registerCmd("show_script",    WRAP_METHOD(Console, cmdShowScript));
	registerCmd("kill_script",    WRAP_METHOD(Console, cmdKillScript));

	registerCmd("insertdisc",     WRAP_METHOD(Console, cmdInsertDisc));
	registerCmd("scene",          WRAP_METHOD(Console, cmdChangeScene));
	registerCmd("puzzle",         WRAP_METHOD(Console, cmdRunPuzzle));

	registerCmd("get_status",     WRAP_METHOD(Console, cmdGetStatus));
	registerCmd("set_status",     WRAP_METHOD(Console, cmdSetStatus));

	registerCmd("encounter",      WRAP_METHOD(Console, cmdRunEncounter));
	registerCmd("show_enc",       WRAP_METHOD(Console, cmdShowEncounter));

	registerCmd("items",          WRAP_METHOD(Console, cmdListItems));
	registerCmd("grab",           WRAP_METHOD(Console, cmdAddToInventory));
	registerCmd("throw",          WRAP_METHOD(Console, cmdRemoveFromInventory));

	registerCmd("palette",        WRAP_METHOD(Console, cmdSetPalette));
	registerCmd("view",           WRAP_METHOD(Console, cmdViewResource));
	registerCmd("draw_area",      WRAP_METHOD(Console, cmdDrawActionArea));

	registerCmd("toggle_flag",    WRAP_METHOD(Console, cmdToggleFlag));

	// Variables
	registerVar("show_actors",     &g_debugActors);
	registerVar("show_drawrects",  &g_debugDrawRects);
	registerVar("show_objects",    &g_debugObjects);
	registerVar("show_polygons",   &g_debugPolygons);
	registerVar("show_scenerects", &g_debugSceneRects);
	registerVar("use_scrolling",   &g_debugScrolling);
}

Console::~Console() {
}

//////////////////////////////////////////////////////////////////////////
// Help
//////////////////////////////////////////////////////////////////////////
bool Console::cmdHelp(int, const char **) {
	debugPrintf("Debug flags\n");
	debugPrintf("-----------\n");
	debugPrintf(" debugflag_list    - Lists the available debug flags and their status\n");
	debugPrintf(" debugflag_enable  - Enables a debug flag\n");
	debugPrintf(" debugflag_disable - Disables a debug flag\n");
	debugPrintf("\n");
	debugPrintf(" show_actors       - Show actors\n");
	debugPrintf(" show_objects      - Show objects\n");
	debugPrintf(" show_polygons     - Show polygons\n");
	debugPrintf(" show_drawrects    - Show drawing rects\n");
	debugPrintf(" use_scrolling     - Scroll scene using the mouse\n");
	debugPrintf("\n");
	debugPrintf("Commands\n");
	debugPrintf("--------\n");
	debugPrintf(" ls          - list engine files\n");
	debugPrintf("\n");
	debugPrintf(" actors      - show actors information\n");
	debugPrintf(" action      - show action information\n");
	debugPrintf(" actions     - list actions information\n");
	debugPrintf(" flags       - show flags\n");
	debugPrintf(" object      - inspect a particular object\n");
	debugPrintf(" objects     - show objects information\n");
	debugPrintf(" world       - show worldstats\n");
	debugPrintf("\n");
	debugPrintf(" video       - play a video\n");
	debugPrintf(" script      - run a script\n");
	debugPrintf(" scene       - change the scene\n");
	debugPrintf(" show_script - show script commands\n");
	debugPrintf(" kill_script - terminate a script\n");
	debugPrintf(" puzzle      - run a puzzle\n");
	debugPrintf(" insertdisc  - show Insert Disc screen\n");
	debugPrintf("\n");
	debugPrintf(" get_status  - get actor's status\n");
	debugPrintf(" set_status  - set actor's status\n");
	debugPrintf("\n");
	debugPrintf(" encounter   - run an encounter\n");
	debugPrintf(" show_enc    - show encounter commands\n");
	debugPrintf("\n");
	debugPrintf(" items       - list all grabbable objects\n");
	debugPrintf(" grab        - add an item to inventory\n");
	debugPrintf(" throw       - remove an item from inventory\n");
	debugPrintf("\n");
	debugPrintf(" palette     - set the screen palette\n");
	debugPrintf(" view        - view game resources\n");
	debugPrintf(" draw_area   - draw action area\n");
	debugPrintf("\n");
	debugPrintf(" toggle_flag - toggle a flag\n");
	debugPrintf("\n");

	return true;
}

//////////////////////////////////////////////////////////////////////////
// List commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdListFiles(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <filter> (use * for all)\n", argv[0]);
		return true;
	}

	Common::String filter(const_cast<char *>(argv[1]));

	Common::ArchiveMemberList list;
	int count = SearchMan.listMatchingMembers(list, filter);

	debugPrintf("Number of matches: %d\n", count);
	for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it)
		debugPrintf(" %s\n", (*it)->getName().c_str());

	return true;
}

bool Console::cmdListActions(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Syntax: %s <index> (use nothing for all)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		for (uint32 i = 0; i < getWorld()->actions.size(); i++)
			debugPrintf("%s\n", getWorld()->actions[i]->toString().c_str());

	} else {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actions.size() - 1;

		if (maxIndex == -1) {
			debugPrintf("[error] No actions are present!\n");
			return true;
		}

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s\n", getWorld()->actions[index]->toString().c_str());
	}

	return true;
}

bool Console::cmdListActors(int argc, const char **argv) {
	if (argc != 1 && argc != 2 && argc != 4) {
		debugPrintf("Syntax: %s <index> (use nothing for all) (<x>, <y>)\n", argv[0]);
		return true;
	}

	if (argc == 1) {
		Actor *player = getScene()->getActor();

		for (uint32 i = 0; i < getWorld()->actors.size(); i++) {
			Actor *actor = getWorld()->actors[i];

			debugPrintf("%c", actor == player ? '*' : ' ');
			debugPrintf("%s\n", actor->toString().c_str());
		}
	} else if (argc == 2 || argc == 4) {
		int index = atoi(argv[1]);
		int maxIndex = getWorld()->actors.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		if (argc == 2) {
			debugPrintf("%s\n", getWorld()->actors[index]->toString(false).c_str());
			return true;
		}

		// Adjust actor coordinates
		int16 x = (int16)atoi(argv[2]);
		int16 y = (int16)atoi(argv[3]);

		// TODO add error handling

		*getWorld()->actors[index]->getPoint1() = Common::Point(x, y);
	}

	return true;
}

bool Console::cmdListFlags(int argc, const char **argv) {
	if (argc != 1 && argc != 2) {
		debugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
		return true;
	}

	// Show all flags
	if (argc == 1) {
		for (int32 i = 0; i < 1512; i++) {
			debugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));

			if ((i + 1) % 10 == 0)
				debugPrintf("\n");
		}
		debugPrintf("\n");
	} else {
		int32 type = atoi(argv[1]);

		if (type != 0 && type != 1) {
			debugPrintf("Syntax: %s <type> (nothing: all  -  1: show set flags  -  0: show unset flags)\n", argv[0]);
			return true;
		}

		// Show only set/unset flags
		int count = 0;
		for (int32 i = 0; i < 1512; i++) {
			if (_vm->isGameFlagSet((GameFlag)i) == (bool)type) {
				debugPrintf("%04d: %d    ", i, _vm->isGameFlagSet((GameFlag)i));
				++count;
			}

			if ((count + 1) % 10 == 0)
				debugPrintf("\n");
		}
		debugPrintf("\n\n%s flags: %d\n", (type ? "Set" : "Unset"), count);
	}

	return true;
}

bool Console::cmdShowWorldStats(int, const char **) {
	debugPrintf("WorldStats\n");
	debugPrintf("----------\n");
	debugPrintf("%s", getWorld()->toString().c_str());

	return true;
}

bool Console::cmdShowObject(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s [id|idx] <target>\n", argv[0]);
		return true;
	}

	if (Common::String(argv[1]) == "id") {
		int id = atoi(argv[2]);
		for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
			if (getWorld()->objects[i]->getId() == id) {
				debugPrintf("%s", getWorld()->objects[i]->toString(false).c_str());
				return true;
			}
		}
		debugPrintf("No object with id %d found\n", id);
	} else if (Common::String(argv[1]) == "idx") {
		int index = atoi(argv[2]);
		int maxIndex = getWorld()->objects.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s", getWorld()->objects[index]->toString(false).c_str());

	} else {
		debugPrintf("[error] valid options are 'id' and 'idx'\n");
	}

	return true;
}

bool Console::cmdShowAction(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s [id|idx] <target>\n", argv[0]);
		return true;
	}

	if (Common::String(argv[1]) == "id") {
		int id = atoi(argv[2]);
		for (uint32 i = 0; i < getWorld()->actions.size(); i++) {
			if (getWorld()->actions[i]->id == id) {
				debugPrintf("%s", getWorld()->actions[i]->toString().c_str());
				return true;
			}
		}
		debugPrintf("No action with id %d found\n", id);
	} else if (Common::String(argv[1]) == "idx") {
		int index = atoi(argv[2]);
		int maxIndex = getWorld()->actions.size() - 1;

		if (index < 0 || index > maxIndex) {
			debugPrintf("[error] index should be between 0 and %d\n", maxIndex);
			return true;
		}

		debugPrintf("%s", getWorld()->actions[index]->toString().c_str());

	} else {
		debugPrintf("[error] valid options are 'id' and 'idx'\n");
	}

	return true;
}

bool Console::cmdListObjects(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s [onscreen|*]\n", argv[0]);
		return true;
	}

	if (argc == 2) {
		if (Common::String(argv[1]) == "onscreen") {
			for (uint32 i = 0; i < getWorld()->objects.size(); i++) {
				if (getWorld()->objects[i]->isOnScreen()) {
					debugPrintf("%s", getWorld()->objects[i]->toString().c_str());
				}
			}

			debugPrintf("Total: %d\n", getWorld()->objects.size());

		} else if (Common::String(argv[1]) == "*") {
			for (uint32 i = 0; i < getWorld()->objects.size(); i++)
				debugPrintf("%s", getWorld()->objects[i]->toString().c_str());

			debugPrintf("Total: %d\n", getWorld()->objects.size());

		} else {
			debugPrintf("[error] valid options are 'onscreen' and '*'\n");
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Video / Scene / Script commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdPlayVideo(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <video number>\n", argv[0]);
		return true;
	}

	int index = atoi(argv[1]);

	// Check if the video exists
	char filename[20];
	const char *extension;

	if (_vm->checkGameVersion("Steam"))
		extension = "_smk.ogv";
	else if (_vm->isAltDemo())
		extension = ".avi";
	else
		extension = ".smk";

	snprintf(filename, 20, "mov%03d%s", index, extension);
	if (!SearchMan.hasFile(filename)) {
		debugPrintf("[Error] Movie %d does not exists\n", index);
		return true;
	}

	_vm->_delayedVideoIndex = index;

	return false;
}

bool Console::cmdGetStatus(int argc, const char **argv) {
	Actor *actor;

	if (argc == 1) {
		actor = getScene()->getActor();
	} else if (argc == 2) {
		int32 index = atoi(argv[1]);

		if (index < 0 || index >= (int32)getWorld()->actors.size()) {
			debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", index, getWorld()->actors.size() - 1);
			return true;
		}

		actor = getScene()->getActor(index);
	} else {
		debugPrintf("Syntax: %s (<actor index>)\n", argv[0]);
		return true;
	}

	debugPrintf("%s's status = %d\n", actor->getName(), actor->getStatus());
	return true;
}

bool Console::cmdSetStatus(int argc, const char **argv) {
	Actor *actor;
	int32 status;

	if (argc == 2) {
		actor = getScene()->getActor();
		status = atoi(argv[1]);
	} else if (argc == 3) {
		int32 index = atoi(argv[1]);

		if (index < 0 || index >= (int32)getWorld()->actors.size()) {
			debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", index, getWorld()->actors.size() - 1);
			return true;
		}

		actor = getScene()->getActor(index);
		status = atoi(argv[2]);
	} else {
		debugPrintf("Syntax: %s (<actor index>) <status>\n", argv[0]);
		return true;
	}

	if (status <= 0 || status > 21) {
		debugPrintf("[Error] Invalid status (was: %d - valid: [1-21])\n", status);
		return true;
	}

	actor->changeStatus((ActorStatus)status);
	return false;
}

bool Console::cmdShowScript(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <script index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check parameters
	if (index < 0 || index >= (int32)getWorld()->numScripts) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, getWorld()->numScripts - 1);
		return true;
	}

	int32 lines = getScript()->_scripts[index].commands[0].numLines;
	for (uint8 i = 0; i <= lines; i++) {
		Asylum::ScriptManager::ScriptEntry *cmd = &getScript()->_scripts[index].commands[i];

		debugPrintf("%02d: [0x%02X] %s (%d, %d, %d, %d, %d, %d, %d, %d, %d)\n",
			i, cmd->opcode, getScript()->_opcodes[cmd->opcode]->name,
			cmd->param1, cmd->param2, cmd->param3, cmd->param4, cmd->param5,
			cmd->param6, cmd->param7, cmd->param8, cmd->param9);
	}

	return true;
}

bool Console::cmdRunScript(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s <script index> <actor index>\n", argv[0]);
		return true;
	}

	int32 index = atoi(argv[1]);
	int32 actor = atoi(argv[2]);

	// Check parameters
	if (index < 0 || index >= (int32)getScript()->_scripts.size()) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, (int32)getScript()->_scripts.size() - 1);
		return true;
	}

	if (actor < 0 || actor >= (int32)getWorld()->actors.size()) {
		debugPrintf("[Error] Invalid actor index (was: %d - valid: [0-%d])\n", actor, getWorld()->actors.size() - 1);
	}

	getScript()->queueScript(index, actor);

	return false;
}

bool Console::cmdKillScript(int argc, const char **argv) {
	if (argc == 1) {
		debugPrintf("Scripts running:\n");
		for (uint32 i = 0; i < ARRAYSIZE(getScript()->_queue.entries); i++)
			if (getScript()->_queue.entries[i].scriptIndex > 0)
				debugPrintf(" %d\n", getScript()->_queue.entries[i].scriptIndex);

		return true;
	}

	int32 index = atoi(argv[1]);
	for (uint32 i = 0; i < ARRAYSIZE(getScript()->_queue.entries); i++)
		if (getScript()->_queue.entries[i].scriptIndex == index) {
			getScript()->removeFromQueue(i);
			break;
		}

	return false;
}

bool Console::cmdChangeScene(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <scene number>\n", argv[0]);
		for (int i = 0; i < 13; i++)
			debugPrintf("        %-2d  %s\n", i + 5, getText()->get(MAKE_RESOURCE(kResourcePackText, i + 1812)));
		return true;
	}

	ResourcePackId index = (ResourcePackId)atoi(argv[1]);

	// Check if the scene exists
	char filename[20];
	snprintf(filename, 20, "scn.%03d", index);
	if (!SearchMan.hasFile(filename)) {
		debugPrintf("[Error] Scene %d does not exists\n", index);
		return true;
	}

	_vm->_delayedSceneIndex = index;
	_vm->_puzzles->reset();

	getMenu()->setGameStarted();
	getSaveLoad()->resetVersion();

	return false;
}

bool Console::cmdInsertDisc(int argc, const char **argv) {
	if (argc < 2) {
		debugPrintf("Syntax: %s (1|2|3)\n", argv[0]);
		return true;
	}

	int cdNumber = CLIP<int>(atoi(argv[1]), 1, 3);
	_insertDisc.setCdNumber(cdNumber);
	_insertDisc.setEventHandler(_vm->getEventHandler());
	_vm->switchEventHandler(&_insertDisc);

	return false;
}

bool Console::cmdRunEncounter(int argc, const char **argv) {
	int i, j, subIndex = 0;
	const EncounterData *data;

	// Check that we are inside a scene
	if (!getScene()) {
		debugPrintf("[Error] Cannot run an encounter outside of a scene\n");
		return true;
	}

	if (argc < 2) {
		debugPrintf("Syntax: %s <encounter index> (<encounter subindex>)\n", argv[0]);

		j = 0;
		while (true) {
			data = &encounterData[getWorld()->chapter - 1][j];
			if (data->index == -1)
				break;

			debugPrintf("        %-2d", data->index);
			if (data->subIndex)
				debugPrintf(" %d ", data->subIndex);
			else
				debugPrintf("   ");

			Object *object1 = getWorld()->getObjectById((ObjectId)data->objectId1),
				   *object2 = getWorld()->getObjectById((ObjectId)data->objectId2);
			debugPrintf("%-23s | %-23s", object1 ? object1->getName() : "NONE", object2 ? object2->getName() : "NONE");
			debugPrintf("\n");

			j++;
		}
		return true;
	}

	// Check if index is valid
	int index = atoi(argv[1]);
	if (argc > 2)
		subIndex = atoi(argv[2]);

	j = 0;
	while (true) {
		data = &encounterData[getWorld()->chapter - 1][j];
		i = data->index;

		if (i == -1)
			break;

		if (i == index && subIndex == (int)data->subIndex)
			break;

		j++;
	}

	if (i == -1) {
		debugPrintf("[Error] No encounter with index %d in this chapter\n", index);
		return true;
	}

	_vm->encounter()->run(index,
	                      (ObjectId)data->objectId1,
	                      (ObjectId)data->objectId2,
	                      data->actorIndex);

	return false;
}

bool Console::cmdShowEncounter(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <encounter index>\n", argv[0]);
		return true;
	}

	// Check index is valid
	int32 index = atoi(argv[1]);
	if (index < 0 || index >= (int32)getEncounter()->_items.size()) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, getEncounter()->_items.size() - 1);
		return true;
	}

	int32 i = 0;
	ResourceId resourceId = getEncounter()->_items[index].scriptResourceId;
	do {
		Encounter::ScriptEntry entry = getEncounter()->getScriptEntry(resourceId, i);

		if (entry.opcode > 25)
			break;
		debugPrintf("%3d %s\n", i, entry.toString().c_str());
	} while (++i);

	return true;
}

bool Console::cmdRunPuzzle(int argc, const char **argv) {
	if (argc != 2) {
		debugPrintf("Syntax: %s <puzzle index>\n", argv[0]);
		debugPrintf("        0   VCR\n");
		debugPrintf("        1   Pipes\n");
		debugPrintf("        2   TicTacToe\n");
		debugPrintf("        3   Lock\n");
		debugPrintf("        4   N/A\n");
		debugPrintf("        5   Wheel\n");
		debugPrintf("        6   BoardSalvation\n");
		debugPrintf("        7   BoardYouth\n");
		debugPrintf("        8   BoardKeyHidesTo\n");
		debugPrintf("        9   Writings\n");
		debugPrintf("        10  Unknown\n");
		debugPrintf("        11  MorgueDoor\n");
		debugPrintf("        12  Clock\n");
		debugPrintf("        13  TimerMachine\n");
		debugPrintf("        14  Fisherman\n");
		debugPrintf("        15  HiveMachine\n");
		debugPrintf("        16  HiveControl\n");
		return true;
	}

	int32 index = atoi(argv[1]);

	// Check index is valid
	if (index < 0 || index >= ARRAYSIZE(puzzleToScenes)) {
		debugPrintf("[Error] Invalid index (was: %d - valid: [0-%d])\n", index, ARRAYSIZE(_vm->_puzzles->_puzzles));
		return true;
	}

	EventHandler *puzzle = getPuzzles()->getPuzzle((uint32)index);
	if (puzzle == nullptr) {
		debugPrintf("[Error] This puzzle does not exists (%d)", index);
		return true;
	}

	getScreen()->clear();
	getScreen()->clearGraphicsInQueue();

	// Save previous scene
	_vm->_previousScene = getScene();

	// Load puzzle scene
	_vm->_scene = new Scene(_vm);
	getScene()->load(puzzleToScenes[index]);

	_vm->switchEventHandler(puzzle);

	return false;
}

bool Console::cmdListItems(int argc, const char **argv) {
	ChapterIndex chapter = getWorld()->chapter;
	uint32 maxIndex;
	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!itemIndices[chapter - 1][maxIndex])
			break;
	}

	int32 offset, actorType = getWorld()->actorType;
	if (actorType == kActorMax)
		offset = 83;
	else if (actorType == kActorSarah)
		offset = 586;
	else if (actorType == kActorCyclops)
		offset = 743;
	else if (actorType == kActorAztec)
		offset = 893;
	else
		return true;

	for (uint32 i = 0; i < maxIndex; i++) {
		char *text = getText()->get(MAKE_RESOURCE(kResourcePackText, offset + itemIndices[chapter - 1][i]));
		text += 4;
		debugPrintf("%02d: %s\n", i + 1, text);
	}

	return true;
}

bool Console::cmdAddToInventory(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Syntax: %s <item index> (<count>)\n", argv[0]);
		return true;
	}

	uint32 index = atoi(argv[1]), count = argc == 3 ? atoi(argv[2]) : 0, maxIndex;

	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!getWorld()->inventoryIconsActive[maxIndex])
			break;
	}

	if (index > maxIndex || !index) {
		debugPrintf("[Error] index should be between 1 and %d\n", maxIndex);
		return true;
	}

	getScene()->getActor()->inventory.add(index, count);

	return true;
}

bool Console::cmdRemoveFromInventory(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Syntax: %s <item index> (<count>)\n", argv[0]);
		return true;
	}

	uint32 index = atoi(argv[1]), count = argc == 3 ? atoi(argv[2]) : 0, maxIndex;

	for (maxIndex = 0; maxIndex < 16; maxIndex++) {
		if (!getWorld()->inventoryIconsActive[maxIndex])
			break;
	}

	if (index > maxIndex || !index) {
		debugPrintf("[Error] index should be between 1 and %d\n", maxIndex);
		return true;
	}

	getScene()->getActor()->inventory.remove(index, count);

	return true;
}

bool Console::cmdSetPalette(int argc, const char **argv) {
	if (argc != 3) {
		debugPrintf("Syntax: %s <pack> <index>\n", argv[0]);
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = atoi(argv[2]);

	// Check resource pack
	if (pack < 0 || pack > 18) {
		debugPrintf("[Error] Invalid resource pack (was: %d - valid: [0-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		debugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	// Try loading resource
	ResourceId id = MAKE_RESOURCE((uint32)pack, index);

	ResourceEntry *entry = getResource()->get(id);
	if (!entry) {
		debugPrintf("[Error] Invalid resource (0x%X)\n", id);
		return true;
	}

	getScreen()->setPalette(id);

	return true;
}

bool Console::cmdViewResource(int argc, const char **argv) {
	if (argc != 2 && argc != 3) {
		debugPrintf("Syntax: %s <pack> (<index>)\n", argv[0]);
		debugPrintf("\nControls:\n");
		debugPrintf("        Space/Backspace - next/previous resource\n");
		debugPrintf("        Enter           - toggle animation\n");
		debugPrintf("        PageDown/PageUp - next/previous palette\n");
		debugPrintf("        Arrow keys      - scroll the image\n");
		debugPrintf("        Escape          - quit\n");
		return true;
	}

	int32 pack = atoi(argv[1]);
	int32 index = pack < 18 ? 0 : 8;
	if (argc > 2)
		index = atoi(argv[2]);

	// Check resource pack
	if (pack < 1 || (pack > 1 && pack < 5)|| pack > 18) {
		debugPrintf("[Error] Invalid resource pack (was: %d - valid: [1,5-18])\n", pack);
		return true;
	}

	// Check index
	if (index < 0) {
		debugPrintf("[Error] Invalid index (was: %d - valid: > 0)\n", index);
		return true;
	}

	ResourceId resourceId = MAKE_RESOURCE((uint32)pack, index);

	if (_resViewer.setResourceId(resourceId)) {
		_resViewer.setEventHandler(_vm->getEventHandler());
		_vm->switchEventHandler(&_resViewer);
		return false;
	} else {
		debugPrintf("[Error] Could not load resource 0x%X\n", resourceId);
		return true;
	}
}

bool Console::cmdDrawActionArea(int argc, const char **argv) {
	if (argc == 1) {
		if (g_debugPolygonIndex) {
			g_debugPolygonIndex = 0;
			return false;
		} else {
			debugPrintf("Syntax: %s (<area_index>)\n", argv[0]);
			return true;
		}
	}

	int areaIndex = getWorld()->getActionAreaIndexById(atoi(argv[1]));
	if (areaIndex == -1) {
		debugPrintf("No such area\n");
		return true;
	}

	ActionArea *area = getWorld()->actions[areaIndex];
	if (area->polygonIndex)
		g_debugPolygonIndex = area->polygonIndex;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Flags commands
//////////////////////////////////////////////////////////////////////////
bool Console::cmdToggleFlag(int argc, const char **argv) {
	if (argc != 2 || atoi(argv[1]) > 1512 || atoi(argv[1]) < 0) {
		debugPrintf("Syntax: <value> between 0 and 1512\n");
		return true;
	}

	_vm->toggleGameFlag((GameFlag)atoi(argv[1]));
	debugPrintf("Flag %d == %d\n", atoi(argv[1]), _vm->isGameFlagSet((GameFlag)atoi(argv[1])));

	return true;
}

} // End of namespace Asulym
