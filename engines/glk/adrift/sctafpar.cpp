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

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/jumps.h"

namespace Glk {
namespace Adrift {

/*
 * Module notes:
 *
 * o Adds new "types" to jAsea's property descriptor: 'M' for multiline
 *   strings, 'Z', 'F'/'T', and 'E' for defaulted integers, booleans, and
 *   strings, 'i', 'b', and 's' for ignored integers, booleans, and strings,
 *   and '{...}' and '|...|' for "special" descriptions and version fixups
 *   that can't be described as things stand.
 *
 * o Adds new 'G' expression test, to check Global boolean.
 *
 * o The stack "adjustment" stuff is a bit of a bother.
 */

/* Assorted definitions and constants. */
static const sc_char NUL = '\0';
enum {
	PARSE_TEMP_LENGTH = 256,
	PARSE_MAX_DEPTH = 32
};

/* Multiline separator sequences for the various versions supported. */
enum { SEPARATOR_SIZE = 3 };
static const sc_byte V400_SEPARATOR[SEPARATOR_SIZE] = {0xbd, 0xd0, 0x00};
static const sc_byte V390_SEPARATOR[SEPARATOR_SIZE] = {0x2a, 0x2a, 0x00};
static const sc_byte V380_SEPARATOR[SEPARATOR_SIZE] = {0x2a, 0x2a, 0x00};


/*
 * Tables of properties descriptors.  These strings define the structure of
 * a TAF file.  Field keys are:
 *
 * $,#,B,M   - string, integer, boolean, and multiline properties
 * E,F,T,Z   - string, integer, and boolean properties not in the TAF;
 *             set to "", FALSE, TRUE and zero on parsing (version < 4)
 * i,s,b     - string, integer, and boolean in the TAF, but not stored
 * [num]     - arrays of property, fixed size to num
 * V         - variable sized array of property, size in input file
 * W         - like V, but size - 1 in input file (version < 4)
 * <class>   - class of property, separate parse target (recurse)
 * ?[!]expr: - conditional property based on expr
 * G[!]expr: - conditional property based on expr using globals
 * |...|     - fixup specials for versions < 4
 * {special} - because some things just defy description
 */
struct sc_parse_schema_t {
	const sc_char *const class_name;
	const sc_char *const descriptor;
};

/* Version 4.0 TAF file properties descriptor table. */
static const sc_parse_schema_t V400_PARSE_SCHEMA[] = {
	{
		"_GAME_",
		"<HEADER>Header <GLOBAL>Globals V<ROOM>Rooms V<OBJECT>Objects V<TASK>Tasks"
		" V<EVENT>Events V<NPC>NPCs V<ROOM_GROUP>RoomGroups V<SYNONYM>Synonyms"
		" V<VARIABLE>Variables V<ALR>ALRs BCustomFont ?BCustomFont:$FontNameSize"
		" $CompileDate"
	},
	{
		"HEADER",
		"MStartupText #StartRoom MWinText"
	},
	{
		"GLOBAL",
		"$GameName $GameAuthor $DontUnderstand #Perspective BShowExits #WaitTurns"
		" BDispFirstRoom BBattleSystem #MaxScore $PlayerName BPromptName $PlayerDesc"
		" #Task ?!#Task=0:$AltDesc #Position #ParentObject #PlayerGender"
		" #MaxSize #MaxWt ?GBattleSystem:<BATTLE>Battle BEightPointCompass bNoDebug"
		" BNoScoreNotify BNoMap bNoAutoComplete bNoControlPanel bNoMouse BSound"
		" BGraphics <RESOURCE>IntroRes <RESOURCE>WinRes BStatusBox $StatusBoxText"
		" iUnk1 iUnk2 BEmbedded"
	},
	{
		"BATTLE",
		"iStaminaLo iStaminaHi iStrengthLo iStrengthHi iAccuracyLo iAccuracyHi"
		" iDefenseLo iDefenseHi iAgilityLo iAgilityHi iRecovery"
	},
	{
		"ROOM",
		"$Short $Long ?GEightPointCompass:[12]<ROOM_EXIT>Exits"
		" ?!GEightPointCompass:[8]<ROOM_EXIT>Exits <RESOURCE>Res V<ROOM_ALT>Alts"
		" ?!GNoMap:bHideOnMap"
	},
	{
		"ROOM_EXIT",
		"{V400_ROOM_EXIT:#Dest_#Var1_#Var2_#Var3}"
	},
	{
		"ROOM_ALT",
		"$M1 #Type <RESOURCE>Res1 $M2 #Var2 <RESOURCE>Res2 #HideObjects $Changed"
		" #Var3 #DisplayRoom"
	},
	{
		"RESOURCE",
		"?GSound:$SoundFile,#SoundLen,ZSoundOffset"
		" ?GGraphics:$GraphicFile,#GraphicLen,ZGraphicOffset {V400_RESOURCE}"
	},
	{
		"OBJECT",
		"$Prefix $Short V$Alias BStatic $Description #InitialPosition #Task"
		" BTaskNotDone $AltDesc ?BStatic:<ROOM_LIST1>Where BContainer BSurface"
		" #Capacity ?!BStatic:BWearable,#SizeWeight,#Parent"
		" ?BStatic:{OBJECT:#Parent} #Openable ?#Openable=5:#Key ?#Openable=6:#Key"
		" ?#Openable=7:#Key #SitLie ?!BStatic:BEdible BReadable ?BReadable:$ReadText"
		" ?!BStatic:BWeapon #CurrentState ?!#CurrentState=0:$States,BStateListed"
		" BListFlag <RESOURCE>Res1 <RESOURCE>Res2 ?GBattleSystem:<OBJ_BATTLE>Battle"
		" $InRoomDesc #OnlyWhenNotMoved"
	},
	{
		"OBJ_BATTLE",
		"iProtectionValue iHitValue iMethod iAccuracy"
	},
	{
		"ROOM_LIST1",
		"#Type {ROOM_LIST1}"
	},
	{
		"TASK",
		"V$Command $CompleteText $ReverseMessage $RepeatText $AdditionalMessage"
		" #ShowRoomDesc BRepeatable BReversible V$ReverseCommand <ROOM_LIST0>Where"
		" $Question ?$Question:$Hint1,$Hint2 V<TASK_RESTR>Restrictions"
		" V<TASK_ACTION>Actions $RestrMask <RESOURCE>Res"
	},
	{
		"TASK_RESTR",
		"#Type ?#Type=0:#Var1,#Var2,#Var3 ?#Type=1:#Var1,#Var2 ?#Type=2:#Var1,#Var2"
		" ?#Type=3:#Var1,#Var2,#Var3 ?#Type=4:#Var1,#Var2,#Var3,$Var4 $FailMessage"
	},
	{
		"TASK_ACTION",
		"#Type ?#Type=0:#Var1,#Var2,#Var3 ?#Type=1:#Var1,#Var2,#Var3"
		" ?#Type=2:#Var1,#Var2 ?#Type=3:#Var1,#Var2,#Var3,$Expr,#Var5"
		" ?#Type=4:#Var1 ?#Type=5:#Var1,#Var2 ?#Type=6:#Var1,#Var2,#Var3"
		" ?#Type=7:iVar1,iVar2,iVar3"
	},
	{
		"ROOM_LIST0",
		"#Type {ROOM_LIST0}"
	},
	{
		"EVENT",
		"$Short #StarterType ?#StarterType=2:#StartTime,#EndTime"
		" ?#StarterType=3:#TaskNum #RestartType BTaskFinished #Time1 #Time2"
		" $StartText $LookText $FinishText <ROOM_LIST0>Where #PauseTask"
		" BPauserCompleted #PrefTime1 $PrefText1 #ResumeTask BResumerCompleted"
		" #PrefTime2 $PrefText2 #Obj2 #Obj2Dest #Obj3 #Obj3Dest #Obj1 #Obj1Dest"
		" #TaskAffected [5]<RESOURCE>Res"
	},
	{
		"NPC",
		"$Name $Prefix V$Alias $Descr #StartRoom $AltText #Task V<TOPIC>Topics"
		" V<WALK>Walks BShowEnterExit ?BShowEnterExit:$EnterText,$ExitText"
		" $InRoomText #Gender [4]<RESOURCE>Res ?GBattleSystem:<NPC_BATTLE>Battle"
	},
	{
		"NPC_BATTLE",
		"iAttitude iStaminaLo iStaminaHi iStrengthLo iStrengthHi iAccuracyLo"
		" iAccuracyHi iDefenseLo iDefenseHi iAgilityLo iAgilityHi iSpeed"
		" iKilledTask iRecovery iStaminaTask"
	},
	{
		"TOPIC",
		"$Subject $Reply #Task $AltReply"
	},
	{
		"WALK",
		"#NumStops BLoop #StartTask #CharTask #MeetObject #ObjectTask #StoppingTask"
		" #MeetChar $ChangedDesc {WALK:#Rooms_#Times}"
	},
	{
		"ROOM_GROUP",
		"$Name {ROOM_GROUP:[]BList}"
	},
	{
		"SYNONYM",
		"$Replacement $Original"
	},
	{
		"VARIABLE",
		"$Name #Type $Value"
	},
	{
		"ALR",
		"$Original $Replacement"
	},
	{NULL, NULL}
};

/* Version 3.9 TAF file properties descriptor table. */
static const sc_parse_schema_t V390_PARSE_SCHEMA[] = {
	{
		"_GAME_",
		"<HEADER>Header <GLOBAL>Globals V<ROOM>Rooms V<OBJECT>Objects V<TASK>Tasks"
		" V<EVENT>Events V<NPC>NPCs V<ROOM_GROUP>RoomGroups V<SYNONYM>Synonyms"
		" V<VARIABLE>Variables V<ALR>ALRs BCustomFont ?BCustomFont:$FontNameSize"
		" $CompileDate sPassword"
	},
	{
		"HEADER",
		"MStartupText #StartRoom MWinText"
	},
	{
		"GLOBAL",
		"$GameName $GameAuthor $DontUnderstand #Perspective BShowExits #WaitTurns"
		" BDispFirstRoom BBattleSystem #MaxScore $PlayerName BPromptName $PlayerDesc"
		" #Task ?!#Task=0:$AltDesc #Position #ParentObject #PlayerGender"
		" #MaxSize #MaxWt ?GBattleSystem:<BATTLE>Battle BEightPointCompass bNoDebug"
		" BNoScoreNotify BNoMap bNoAutoComplete bNoControlPanel bNoMouse"
		" BSound BGraphics <RESOURCE>IntroRes <RESOURCE>WinRes FStatusBox"
		" EStatusBoxText iUnk1 iUnk2 FEmbedded"
	},
	{
		"BATTLE",
		"iStamina iStrength iDefense"
	},
	{
		"ROOM",
		"$Short $Long $LastDesc ?GEightPointCompass:[12]<ROOM_EXIT>Exits"
		" ?!GEightPointCompass:[8]<ROOM_EXIT>Exits $AddDesc1 #Task1 $AddDesc2 #Task2"
		" #Obj $AltDesc #TypeHideObjects <RESOURCE>Res <RESOURCE>LastRes"
		" <RESOURCE>Task1Res <RESOURCE>Task2Res <RESOURCE>AltRes"
		" ?!GNoMap:bHideOnMap |V390_ROOM:_Alts_|"
	},
	{
		"ROOM_EXIT",
		"{V390_V380_ROOM_EXIT:#Dest_#Var1_#Var2_ZVar3}"
	},
	{
		"RESOURCE",
		"?GSound:$SoundFile,ZSoundLen,ZSoundOffset"
		" ?GGraphics:$GraphicFile,ZGraphicLen,ZGraphicOffset"
	},
	{
		"OBJECT",
		"$Prefix $Short"
		" [1]$Alias BStatic $Description #InitialPosition #Task BTaskNotDone"
		" $AltDesc ?BStatic:<ROOM_LIST1>Where BContainer BSurface #Capacity"
		" ?!BStatic:BWearable,#SizeWeight,#Parent ?BStatic:{OBJECT:#Parent}"
		" #Openable |V390_OBJECT:_Openable_,Key| #SitLie ?!BStatic:BEdible BReadable"
		" ?BReadable:$ReadText ?!BStatic:BWeapon ZCurrentState FListFlag"
		" <RESOURCE>Res1 <RESOURCE>Res2 ?GBattleSystem:<OBJ_BATTLE>Battle"
		" EInRoomDesc ZOnlyWhenNotMoved"
	},
	{
		"OBJ_BATTLE",
		"iProtectionValue iHitValue iMethod"
	},
	{
		"ROOM_LIST1",
		"#Type {ROOM_LIST1}"
	},
	{
		"TASK",
		"W$Command $CompleteText $ReverseMessage $RepeatText $AdditionalMessage"
		" #ShowRoomDesc BRepeatable BReversible W$ReverseCommand <ROOM_LIST0>Where"
		" $Question ?$Question:$Hint1,$Hint2 V<TASK_RESTR>Restrictions"
		" V<TASK_ACTION>Actions |V390_TASK:$RestrMask| <RESOURCE>Res"
	},
	{
		"TASK_RESTR",
		"#Type ?#Type=0:#Var1,#Var2,#Var3 ?#Type=1:#Var1,#Var2 ?#Type=2:#Var1,#Var2"
		" ?#Type=3:#Var1,#Var2,#Var3 ?#Type=4:#Var1,#Var2,#Var3,EVar4"
		",|V390_TASK_RESTR:Var1>0?#Var1++| $FailMessage"
	},
	{
		"TASK_ACTION",
		"#Type |V390_TASK_ACTION:Type>4?#Type++| ?#Type=0:#Var1,#Var2,#Var3"
		" ?#Type=1:#Var1,#Var2,#Var3 ?#Type=2:#Var1,#Var2"
		" ?#Type=3:#Var1,#Var2,#Var3,|V390_TASK_ACTION:$Expr_#Var5|"
		" ?#Type=4:#Var1 ?#Type=6:#Var1,ZVar2,ZVar3 ?#Type=7:iVar1,iVar2,iVar3"
	},
	{
		"ROOM_LIST0",
		"#Type {ROOM_LIST0}"
	},
	{
		"EVENT",
		"$Short #StarterType ?#StarterType=2:#StartTime,#EndTime"
		" ?#StarterType=3:#TaskNum #RestartType BTaskFinished #Time1 #Time2"
		" $StartText $LookText $FinishText <ROOM_LIST0>Where #PauseTask"
		" BPauserCompleted #PrefTime1 $PrefText1 #ResumeTask BResumerCompleted"
		" #PrefTime2 $PrefText2 #Obj2 #Obj2Dest #Obj3 #Obj3Dest #Obj1 #Obj1Dest"
		" #TaskAffected [5]<RESOURCE>Res"
	},
	{
		"NPC",
		"$Name $Prefix [1]$Alias $Descr #StartRoom $AltText #Task V<TOPIC>Topics"
		" V<WALK>Walks BShowEnterExit ?BShowEnterExit:$EnterText,$ExitText"
		" $InRoomText #Gender [4]<RESOURCE>Res ?GBattleSystem:<NPC_BATTLE>Battle"
	},
	{
		"NPC_BATTLE",
		"iAttitude iStamina iStrength iDefense iSpeed iKilledTask"
	},
	{
		"TOPIC",
		"$Subject $Reply #Task $AltReply"
	},
	{
		"WALK",
		"#NumStops BLoop #StartTask #CharTask #MeetObject #ObjectTask #StoppingTask"
		" ZMeetChar $ChangedDesc {WALK:#Rooms_#Times}"
	},
	{
		"ROOM_GROUP",
		"$Name {ROOM_GROUP:[]BList}"
	},
	{
		"SYNONYM",
		"$Replacement $Original"
	},
	{
		"VARIABLE",
		"$Name ZType $Value"
	},
	{
		"ALR",
		"$Original $Replacement"
	},
	{NULL, NULL}
};

/* Version 3.8 TAF file properties descriptor table. */
static const sc_parse_schema_t V380_PARSE_SCHEMA[] = {
	{
		"_GAME_",
		"<HEADER>Header <GLOBAL>Globals V<ROOM>Rooms V<OBJECT>Objects V<TASK>Tasks"
		" V<EVENT>Events V<NPC>NPCs V<ROOM_GROUP>RoomGroups V<SYNONYM>Synonyms"
		" FCustomFont $CompileDate sPassword |V380_GLOBAL:_MaxScore_|"
		" |V380_OBJECT:_InitialPositions_|"
	},
	{
		"HEADER",
		"MStartupText #StartRoom MWinText"
	},
	{
		"GLOBAL",
		"$GameName $GameAuthor #MaxCarried |V380_MaxSize_MaxWt_| $DontUnderstand"
		" #Perspective BShowExits #WaitTurns FDispFirstRoom FBattleSystem"
		" EPlayerName FPromptName EPlayerDesc ZTask ZPosition ZParentObject"
		" ZPlayerGender FEightPointCompass TNoScoreNotify FSound FGraphics"
		" FStatusBox EStatusBoxText FEmbedded"
	},
	{
		"ROOM",
		"$Short $Long $LastDesc [8]<ROOM_EXIT>Exits $AddDesc1 #Task1 $AddDesc2"
		" #Task2 #Obj $AltDesc #TypeHideObjects |V380_ROOM:_Alts_|"
	},
	{
		"ROOM_EXIT",
		"{V390_V380_ROOM_EXIT:#Dest_#Var1_#Var2_ZVar3}"
	},
	{
		"OBJECT",
		"$Prefix $Short [1]$Alias BStatic $Description #InitialPosition #Task"
		" BTaskNotDone $AltDesc ?BStatic:<ROOM_LIST1>Where #SurfaceContainer"
		" FSurface ?#SurfaceContainer=2:TSurface FContainer"
		" ?#SurfaceContainer=1:TContainer #Capacity |V380_OBJECT:#Capacity*10+2|"
		" ?!BStatic:BWearable,#SizeWeight,#Parent ?BStatic:{OBJECT:#Parent}"
		" #Openable |V380_OBJECT:_Openable_,Key| #SitLie ?!BStatic:BEdible BReadable"
		" ?BReadable:$ReadText ?!BStatic:BWeapon ZCurrentState FListFlag"
		" EInRoomDesc ZOnlyWhenNotMoved"
	},
	{
		"ROOM_LIST1",
		"#Type {ROOM_LIST1}"
	},
	{
		"TASK",
		"W$Command $CompleteText $ReverseMessage $RepeatText $AdditionalMessage"
		" #ShowRoomDesc BRepeatable #Score BSingleScore [6]<TASK_MOVE>Movements"
		" BReversible W$ReverseCommand #WearObj1 #WearObj2 #HoldObj1 #HoldObj2"
		" #HoldObj3 #Obj1 #Task BTaskNotDone $TaskMsg $HoldMsg $WearMsg $CompanyMsg"
		" BNotInSameRoom #NPC $Obj1Msg #Obj1Room <ROOM_LIST0>Where BKillsPlayer"
		" BHoldingSameRoom $Question ?$Question:$Hint1,$Hint2 #Obj2"
		" ?!#Obj2=0:#Obj2Var1,#Obj2Var2,$Obj2Msg BWinGame |V380_TASK:_Actions_|"
		" |V380_TASK:_Restrictions_|"
	},
	{
		"TASK_MOVE",
		"#Var1 #Var2 #Var3"
	},
	{
		"ROOM_LIST0",
		"#Type {ROOM_LIST0}"
	},
	{
		"EVENT",
		"$Short #StarterType ?#StarterType=2:#StartTime,#EndTime"
		" ?#StarterType=3:#TaskNum #RestartType BTaskFinished #Time1 #Time2"
		" $StartText $LookText $FinishText <ROOM_LIST0>Where #PauseTask"
		" BPauserCompleted #PrefTime1 $PrefText1 #ResumeTask BResumerCompleted"
		" #PrefTime2 $PrefText2 #Obj2 #Obj2Dest #Obj3 #Obj3Dest #Obj1 #Obj1Dest"
		" #TaskAffected"
	},
	{
		"NPC",
		"$Name $Prefix [1]$Alias $Descr #StartRoom $AltText #Task V<TOPIC>Topics"
		" V<WALK>Walks BShowEnterExit ?BShowEnterExit:$EnterText,$ExitText"
		" $InRoomText ZGender"
	},
	{
		"TOPIC",
		"$Subject $Reply #Task $AltReply"
	},
	{
		"WALK",
		"#NumStops BLoop #StartTask #CharTask #MeetObject"
		" ?!#MeetObject=0:|V380_WALK:_MeetObject_| #ObjectTask ZMeetChar"
		" {WALK:#Rooms_#Times} ZStoppingTask EChangedDesc"
	},
	{
		"ROOM_GROUP",
		"$Name {ROOM_GROUP:[]BList}"
	},
	{
		"SYNONYM",
		"$Replacement $Original"
	},
	{NULL, NULL}
};


/*
 * parse_select_schema()
 *
 * Select one of the parse schemata based on a TAF file.
 */
static const sc_parse_schema_t *parse_select_schema(sc_tafref_t taf) {
	/* Switch based on the TAF file version. */
	switch (taf_get_version(taf)) {
	case TAF_VERSION_400:
		return V400_PARSE_SCHEMA;
	case TAF_VERSION_390:
		return V390_PARSE_SCHEMA;
	case TAF_VERSION_380:
		return V380_PARSE_SCHEMA;
	default:
		sc_fatal("parse_select_schema: invalid TAF file version\n");
		return NULL;
	}
}


/* The uncompressed TAF file from which we get all our data. */
static sc_tafref_t parse_taf = NULL;
static sc_int parse_tafline = 0;

/* The parse schema selected for this TAF file. */
static sc_parse_schema_t const *parse_schema = NULL;

/* Properties bundle and trace flag, set before parsing. */
static sc_prop_setref_t parse_bundle = NULL;
static sc_bool parse_trace = FALSE;

/*
 * Stack of property keys.  The stack is filled by parsing, and written
 * to the property store on parse terminals.
 */
static sc_vartype_t parse_vt_key[PARSE_MAX_DEPTH];
static sc_char parse_format[PARSE_MAX_DEPTH];
static sc_int parse_depth = 0;


/*
 * parse_push_key()
 * parse_pop_key()
 *
 * Push a key of the given type onto the property key stack, and pop a key
 * off on unwind.
 */
static void parse_push_key(sc_vartype_t vt_key, sc_char type) {
	if (parse_depth == PARSE_MAX_DEPTH)
		sc_fatal("parse_push_key: stack overrun\n");

	/* Push the key, and its associated type. */
	parse_vt_key[parse_depth] = vt_key;
	parse_format[parse_depth] = type;
	parse_depth++;
}

static void parse_pop_key(void) {
	/* Check the stack has something to pop, then pop it. */
	if (parse_depth == 0)
		sc_fatal("parse_pop_key: stack underrun\n");
	parse_depth--;
}


/*
 * parse_retrieve_stack()
 *
 * This is ugly.  The parse produces indexes before the things that they
 * index.  An expedient fix is to switch i-s keys before storing a property
 * value
 */
static void parse_retrieve_stack(sc_char format[], sc_vartype_t vt_key[], sc_int *depth) {
	sc_int index_;

	/* Switch index-string key pairs. */
	for (index_ = 0; index_ < parse_depth; index_++) {
		if (index_ < parse_depth - 1
		        && parse_format[index_] == PROP_KEY_INTEGER
		        && parse_format[index_ + 1] == PROP_KEY_STRING) {
			/* Swap format and key elements. */
			format[index_] = parse_format[index_ + 1];
			format[index_ + 1] = parse_format[index_];
			vt_key[index_] = parse_vt_key[index_ + 1];
			vt_key[index_ + 1] = parse_vt_key[index_];

			index_++;
		} else {
			/* Simple copy of format and key elements. */
			format[index_] = parse_format[index_];
			vt_key[index_] = parse_vt_key[index_];
		}
	}

	/* Return the parse depth. */
	*depth = parse_depth;
}


/*
 * parse_stack_backtrace()
 *
 * Dump the parse stack.  Used for diagnostics on finding what we think may
 * be a bad game.
 */
static void parse_stack_backtrace(void) {
	sc_vartype_t vt_key[PARSE_MAX_DEPTH];
	sc_char format[PARSE_MAX_DEPTH];
	sc_int depth, index_;

	parse_retrieve_stack(format, vt_key, &depth);

	sc_error("parse_stack_backtrace: version %s schema parsed to depth %ld\n",
	         (parse_schema == V400_PARSE_SCHEMA) ? "4.00" :
	         (parse_schema == V390_PARSE_SCHEMA) ? "3.90" :
	         (parse_schema == V380_PARSE_SCHEMA) ? "3.80" : "[Invalid]",
	         depth);

	sc_error("parse_stack_backtrace: parse stack backtrace follows...\n");
	for (index_ = 0; index_ < depth; index_++) {
		sc_char type;

		type = format[index_];
		if (type == PROP_KEY_INTEGER)
			sc_error("%2ld - [%c] %ld\n", index_, type, vt_key[index_].integer);
		else if (type == PROP_KEY_STRING)
			sc_error("%2ld - [%c] \"%s\"\n", index_, type, vt_key[index_].string);
		else
			sc_error("%2ld - [%c] %p\n", index_, type, vt_key[index_].voidp);
	}
}


/*
 * parse_put_property()
 * parse_get_property()
 *
 * Write or read a property based on the keys amassed so far.
 */
static void parse_put_property(sc_vartype_t vt_value, sc_char type) {
	sc_vartype_t vt_key[PARSE_MAX_DEPTH];
	sc_char format[PARSE_MAX_DEPTH + 4];
	sc_int depth;

	/* Retrieve the adjusted stack. */
	parse_retrieve_stack(format + 3, vt_key, &depth);

	/* Complete the format for the property put. */
	format[0] = type;
	format[1] = '-';
	format[2] = '>';
	format[depth + 3] = NUL;

	/* Store the property under the stacked keys. */
	assert(parse_bundle);
	prop_put(parse_bundle, format, vt_value, vt_key);
}

static sc_bool parse_get_property(sc_vartype_t *vt_rvalue, sc_char type) {
	sc_vartype_t vt_key[PARSE_MAX_DEPTH];
	sc_char format[PARSE_MAX_DEPTH + 4];
	sc_int depth;
	sc_bool status;

	/* Retrieve the adjusted stack. */
	parse_retrieve_stack(format + 3, vt_key, &depth);

	/* Complete the format for the property put. */
	format[0] = type;
	format[1] = '<';
	format[2] = '-';
	format[depth + 3] = NUL;

	/* Retrieve the property using the stacked keys. */
	assert(parse_bundle);
	status = prop_get(parse_bundle, format, vt_rvalue, vt_key);

	return status;
}


/*
 * parse_get_child_count()
 *
 * Convenience form of parse_get_property(), retrieve an integer property
 * indicating the child count of the effectively stacked node, or zero if
 * no such node exists.
 */
static sc_int parse_get_child_count(void) {
	sc_vartype_t vt_rvalue;

	if (!parse_get_property(&vt_rvalue, PROP_INTEGER))
		vt_rvalue.integer = 0;

	return vt_rvalue.integer;
}


/*
 * parse_get_integer_property()
 * parse_get_boolean_property()
 * parse_get_string_property()
 *
 * Convenience forms of parse_get_property(), retrieve directly, and report
 * a fatal error if the property does not exist.
 */
static sc_int parse_get_integer_property(void) {
	sc_vartype_t vt_rvalue;

	if (!parse_get_property(&vt_rvalue, PROP_INTEGER))
		sc_fatal("parse_get_integer_property: missing property\n");

	return vt_rvalue.integer;
}

static sc_bool parse_get_boolean_property(void) {
	sc_vartype_t vt_rvalue;

	if (!parse_get_property(&vt_rvalue, PROP_BOOLEAN))
		sc_fatal("parse_get_boolean_property: missing property\n");

	return vt_rvalue.boolean;
}

static const sc_char *parse_get_string_property(void) {
	sc_vartype_t vt_rvalue;

	if (!parse_get_property(&vt_rvalue, PROP_STRING))
		sc_fatal("parse_get_string_property: missing property\n");

	return vt_rvalue.string;
}


/* Pushback line, and pushback requested flag. */
static const sc_char *parse_pushback_line = NULL;
static sc_bool parse_use_pushback = FALSE;

/*
 * parse_get_taf_string()
 * parse_get_taf_integer()
 * parse_get_taf_boolean()
 * parse_taf_pushback()
 *
 * Wrapper round obtaining the next TAF file line, with variants to convert
 * the line content into an integer or boolean, and a function for effective
 * TAF line pushback.
 */
static const sc_char *parse_get_taf_string(CONTEXT) {
	const sc_char *line;

	/* If pushback requested, use that instead of reading. */
	if (parse_use_pushback) {
		/* Use the pushback line, and clear the request. */
		assert(parse_pushback_line);
		line = parse_pushback_line;
		parse_use_pushback = FALSE;
	} else {
		/* Get the next line, and complain if absent. */
		line = taf_next_line(parse_taf);
		if (!line) {
			sc_error("parse_get_taf_string:"
			         " out of TAF data at line %ld\n", parse_tafline);
			parse_stack_backtrace();
			LONG_JUMP0;
		}

		/* Note this line for possible pushback. */
		parse_pushback_line = line;
	}

	/* Print out the line we're parsing if tracing. */
	if (parse_trace)
		sc_trace("Parse: read in line %ld : %s\n", parse_tafline, line);

	parse_tafline++;
	return line;
}

static sc_int parse_get_taf_integer(CONTEXT) {
	const sc_char *line;
	sc_int integer;

	/* Get line, and scan for a single integer; return it. */
	R0FUNC0(parse_get_taf_string, line);
	if (sscanf(line, "%ld", &integer) != 1) {
		sc_error("parse_get_taf_integer:"
		         " invalid integer at line %ld\n", parse_tafline - 1);
		parse_stack_backtrace();
		LONG_JUMP0;
	}

	return integer;
}

static sc_bool parse_get_taf_boolean(CONTEXT) {
	const sc_char *line;
	sc_uint boolean;

	/*
	 * Get line, and scan for a single integer; check it's a valid-looking flag,
	 * and return it.
	 */
	R0FUNC0(parse_get_taf_string, line);
	if (sscanf(line, "%lu", &boolean) != 1) {
		sc_error("parse_get_taf_boolean:"
		         " invalid boolean at line %ld\n", parse_tafline - 1);
		parse_stack_backtrace();
		LONG_JUMP0;
	}
	if (boolean != 0 && boolean != 1) {
		sc_error("parse_get_taf_boolean:"
		         " warning: suspect boolean at line %ld\n", parse_tafline - 1);
	}

	return boolean != 0;
}

static void parse_taf_pushback(void) {
	if (parse_use_pushback || !parse_pushback_line)
		sc_fatal("parse_taf_pushback: too much pushback requested\n");

	/* Set pushback request, and decrement line counter. */
	parse_use_pushback = TRUE;
	parse_tafline--;

	/* Note pushback for tracing purposes. */
	if (parse_trace)
		sc_trace("Parse: push back at line %ld\n", parse_tafline);
}


/* Enumerations of parse types found in the parse schema. */
enum {
	PARSE_INTEGER = '#',
	PARSE_DEFAULT_ZERO = 'Z',
	PARSE_BOOLEAN = 'B',
	PARSE_DEFAULT_FALSE = 'F',
	PARSE_DEFAULT_TRUE = 'T',
	PARSE_STRING = '$',
	PARSE_DEFAULT_EMPTY = 'E',
	PARSE_MULTILINE = 'M',
	PARSE_VECTOR = 'V',
	PARSE_VECTOR_ALTERNATE = 'W',
	PARSE_ARRAY = '[',
	PARSE_EXPRESSION = '?',
	PARSE_EXPRESSION_NOT = '!',
	PARSE_GLOBAL_EXPRESSION = 'G',
	PARSE_CLASS = '<',
	PARSE_FIXUP = '|',
	PARSE_SPECIAL = '{',
	PARSE_IGNORE_INTEGER = 'i',
	PARSE_IGNORE_BOOLEAN = 'b',
	PARSE_IGNORE_STRING = 's'
};

/* Forward declarations of parse functions for recursion. */
static void parse_element(CONTEXT, const sc_char *element);
static void parse_class(CONTEXT, const sc_char *class_);
static void parse_descriptor(CONTEXT, const sc_char *descriptor);

/*
 * parse_array()
 *
 * Parse a descriptor [] array.
 */
static void parse_array(CONTEXT, const sc_char *array) {
	sc_int count, index_;
	sc_char element[PARSE_TEMP_LENGTH];

	if (parse_trace)
		sc_trace("Parse: entering array %s\n", array);

	/* Find the count of elements in the array, and the element itself. */
	if (sscanf(array, "[%ld]%[^ ]", &count, element) != 2)
		sc_fatal("parse_array: bad array, %s\n", array);

	/* Parse the element for array count iterations, each a key. */
	for (index_ = 0; index_ < count; index_++) {
		sc_vartype_t vt_key;

		vt_key.integer = index_;
		parse_push_key(vt_key, PROP_KEY_INTEGER);

		CALL1(parse_element, element);

		parse_pop_key();
	}

	if (parse_trace)
		sc_trace("Parse: leaving array %s\n", array);
}


/*
 * parse_vector_common()
 * parse_vector()
 * parse_vector_alternate()
 *
 * Parse a variable-length vector of properties.
 */
static void parse_vector_common(CONTEXT, const sc_char *vector, sc_int count) {
	sc_int index_;

	/* Parse the vector property count times, pushing a key on each. */
	for (index_ = 0; index_ < count; index_++) {
		sc_vartype_t vt_key;

		vt_key.integer = index_;
		parse_push_key(vt_key, PROP_KEY_INTEGER);

		CALL1(parse_element, vector + 1);

		parse_pop_key();
	}
}

static void parse_vector(CONTEXT, const sc_char *vector) {
	sc_int count;

	if (parse_trace)
		sc_trace("Parse: entering vector %s\n", vector);

	/* Find the count of elements in the vector, and parse. */
	FUNC0(parse_get_taf_integer, count);
	CALL2(parse_vector_common, vector, count);

	if (parse_trace)
		sc_trace("Parse: leaving vector %s\n", vector);
}

static void parse_vector_alternate(CONTEXT, const sc_char *vector) {
	sc_int count1;

	if (parse_trace)
		sc_trace("Parse: entering alternate vector %s\n", vector);

	/* Element count, this is a vector described by size - 1. */
	FUNC0(parse_get_taf_integer, count1);
	CALL2(parse_vector_common, vector, count1 + 1);

	if (parse_trace)
		sc_trace("Parse: leaving alternate vector %s\n", vector);
}


/*
 * parse_test_expression()
 * parse_expression()
 *
 * Parse a conditional field definition, with runtime test.
 */
static sc_bool parse_test_expression(const sc_char *test_expression) {
	sc_vartype_t vt_key;
	sc_char plhs[PARSE_TEMP_LENGTH];
	sc_int rhs;
	sc_bool retval = FALSE;

	/* Identify the type of expression to evaluate. */
	switch (test_expression[0]) {
	case PARSE_BOOLEAN:
		/* Read boolean property and return its value. */
		vt_key.string = test_expression + 1;
		parse_push_key(vt_key, PROP_KEY_STRING);
		retval = parse_get_boolean_property();
		parse_pop_key();
		break;

	case PARSE_INTEGER:
		/* Get the left and right sides of = comparison. */
		if (sscanf(test_expression, "#%[^=]=%ld", plhs, &rhs) != 2) {
			sc_fatal("parse_test_expression: bad = compare, %s\n",
			         test_expression + 1);
		}

		/* Read integer property and return comparison. */
		vt_key.string = plhs;
		parse_push_key(vt_key, PROP_KEY_STRING);
		retval = (parse_get_integer_property() == rhs);
		parse_pop_key();
		break;

	case PARSE_STRING:
		/* Read property and return TRUE if not an empty string. */
		vt_key.string = test_expression + 1;
		parse_push_key(vt_key, PROP_KEY_STRING);
		retval = !sc_strempty(parse_get_string_property());
		parse_pop_key();
		break;

	case PARSE_GLOBAL_EXPRESSION: {
		sc_vartype_t vt_gkey[2];

		/* Read the given Global boolean property and return it. */
		vt_gkey[0].string = "Globals";
		vt_gkey[1].string = test_expression + 1;
		retval = prop_get_boolean(parse_bundle, "B<-ss", vt_gkey);
		break;
	}

	default:
		sc_fatal("parse_test_expression:"
		         " bad expression, %s\n", test_expression + 1);
	}

	if (parse_trace)
		sc_trace("Parse: expression is %s\n", retval ? "true" : "false");

	return retval;
}

static void parse_expression(CONTEXT, const sc_char *expression) {
	sc_char test_expression[PARSE_TEMP_LENGTH];
	sc_bool is_present;

	if (parse_trace)
		sc_trace("Parse: entering expression %s\n", expression);

	/* Isolate the test part of the expression. */
	if (sscanf(expression, "?%[^:]", test_expression) != 1)
		sc_fatal("parse_expression: bad expression, %s\n", expression);

	/* Handle the remainder of the expression only if test passes. */
	is_present = (test_expression[0] == PARSE_EXPRESSION_NOT)
	             ? !parse_test_expression(test_expression + 1)
	             : parse_test_expression(test_expression);
	if (is_present) {
		sc_int next;

		/*
		 * Following the ':' may be a single element, or a comma-separated list.
		 */
		for (next = strlen(test_expression) + 2; expression[next] != NUL;) {
			sc_char element[PARSE_TEMP_LENGTH];

			/* Get the next individual element to parse. */
			if (sscanf(expression + next, "%[^,]", element) != 1)
				sc_fatal("parse_expression: bad list, %s\n", expression + next);

			/* Parse this isolated element. */
			CALL1(parse_element, element);

			/* Advance to the start of the next element. */
			next += strlen(element);
			next += strspn(expression + next, ",");
		}
	}

	if (parse_trace)
		sc_trace("Parse: leaving expression %s\n", expression);
}


/*
 * parse_read_multiline()
 *
 * Helper for parse_terminal(), reads in a multiline string.  The return
 * string is malloc'ed, and the caller needs to handle that.
 */
static sc_char *parse_read_multiline(CONTEXT) {
	const sc_byte *separator = NULL;
	const sc_char *line;
	sc_char *multiline;

	/* Select the appropriate multiline separator. */
	switch (taf_get_version(parse_taf)) {
	case TAF_VERSION_400:
		separator = V400_SEPARATOR;
		break;
	case TAF_VERSION_390:
		separator = V390_SEPARATOR;
		break;
	case TAF_VERSION_380:
		separator = V380_SEPARATOR;
		break;
	default:
		sc_fatal("parse_read_multiline: invalid TAF file version\n");
		break;
	}

	/* Take a simple copy of the first line. */
	R0FUNC0(parse_get_taf_string, line);
	multiline = (sc_char *)sc_malloc(strlen(line) + 1);
	strcpy(multiline, line);

	/* Now concatenate until separator found. */
	R0FUNC0(parse_get_taf_string, line);
	while (memcmp(line, separator, SEPARATOR_SIZE) != 0) {
		multiline = (sc_char *)sc_realloc(multiline,
		                                  strlen(multiline) + strlen(line) + 2);
		strcat(multiline, "\n");
		strcat(multiline, line);
		R0FUNC0(parse_get_taf_string, line);
	}

	return multiline;
}


/*
 * parse_terminal()
 *
 * Common handler for string, integer, boolean, and multiline parse terminals.
 */
static void parse_terminal(CONTEXT, const sc_char *terminal) {
	sc_vartype_t vt_key, vt_value;

	if (parse_trace)
		sc_trace("Parse: entering terminal %s\n", terminal);

	/* Push the key string. */
	vt_key.string = terminal + 1;
	parse_push_key(vt_key, PROP_KEY_STRING);

	/* Retrieve, or invent, then store the value. */
	switch (terminal[0]) {
	case PARSE_INTEGER:
		FUNC0(parse_get_taf_integer, vt_value.integer);
		parse_put_property(vt_value, PROP_INTEGER);
		break;
	case PARSE_DEFAULT_ZERO:
		vt_value.integer = 0;
		parse_put_property(vt_value, PROP_INTEGER);
		break;

	case PARSE_BOOLEAN:
		FUNC0(parse_get_taf_boolean, vt_value.boolean);
		parse_put_property(vt_value, PROP_BOOLEAN);
		break;
	case PARSE_DEFAULT_FALSE:
	case PARSE_DEFAULT_TRUE:
		vt_value.boolean = (terminal[0] == PARSE_DEFAULT_TRUE);
		parse_put_property(vt_value, PROP_BOOLEAN);
		break;

	case PARSE_STRING:
		FUNC0(parse_get_taf_string, vt_value.string);
		parse_put_property(vt_value, PROP_STRING);
		break;
	case PARSE_DEFAULT_EMPTY:
		vt_value.string = "";
		parse_put_property(vt_value, PROP_STRING);
		break;

	case PARSE_MULTILINE:
		/* Assign to and adopt mutable string rather than const string. */
		FUNC0(parse_read_multiline, vt_value.mutable_string);
		parse_put_property(vt_value, PROP_STRING);

		assert(parse_bundle);
		prop_adopt(parse_bundle, vt_value.mutable_string);
		break;

	case PARSE_IGNORE_INTEGER:
		CALL0(parse_get_taf_integer);
		break;
	case PARSE_IGNORE_BOOLEAN:
		CALL0(parse_get_taf_boolean);
		break;
	case PARSE_IGNORE_STRING:
		CALL0(parse_get_taf_string);
		break;

	default:
		sc_fatal("parse_terminal: bad type, %c\n", terminal[0]);
	}

	/* Pop terminal key. */
	parse_pop_key();

	if (parse_trace)
		sc_trace("Parse: leaving terminal %s\n", terminal);
}


/*
 * Resources table.  This table enables resource offsets to be calculated
 * for the various sound and graphic resources encountered on parsing
 * version 4.0 games.  It's unused if the version is not 4.0.
 */
struct sc_parse_resource_t {
	sc_char *name;
	sc_uint hash;
	sc_int length;
	sc_int offset;
};

enum { RESOURCE_GROW_INCREMENT = 32 };
static sc_int parse_resources_length = 0;
static sc_int parse_resources_size = 0;
static sc_parse_resource_t *parse_resources = NULL;


/*
 * parse_clear_v400_resources_table()
 *
 * Free and clear down the version 4.0 resources table.
 */
static void parse_clear_v400_resources_table(void) {
	/* Free allocated memory and return to initial values. */
	if (parse_resources) {
		sc_int index_;

		for (index_ = 0; index_ < parse_resources_length; index_++)
			sc_free(parse_resources[index_].name);

		sc_free(parse_resources);
		parse_resources = NULL;
	}
	parse_resources_length = 0;
	parse_resources_size = 0;
}


/*
 * parse_get_v400_resource_offset()
 *
 * Notes version 4.0 resource names encountered in the parse, and their
 * lengths, and builds up a list of resources with their data offsets.  The
 * function assumes that resources are appended to the TAF file in the
 * order in which they are encountered when reading through the TAF file.
 *
 * A warning -- this function may return a new length.  Resources that
 * have been seen once already have non-useful (though apparently non-zero)
 * lengths; this function needs to handle that.  The caller needs to compare
 * length with real_length to see if that happened.
 */
static sc_int parse_get_v400_resource_offset(const sc_char *name,
		sc_int length, sc_int *real_length) {
	sc_char *clean_name;
	sc_uint hash;
	sc_int index_, offset;

	/*
	 * Take a copy of the name, and remove any trailing "##" looping sound
	 * indicator flag.  Who thinks this junk up?
	 */
	clean_name = (sc_char *)sc_malloc(strlen(name) + 1);
	strcpy(clean_name, name);
	if (strcmp(clean_name + strlen(clean_name) - 2, "##") == 0)
		clean_name[strlen(clean_name) - 2] = NUL;

	/*
	 * Scan the current resources list for a matching name, and if the resource
	 * is already known, return its offset.  The hash check is an attempt to
	 * improve the search times, relative to using only string comparisons --
	 * the table's not fully hashed.  If found, we need to also pass back the
	 * corrected length.
	 */
	offset = -1;
	hash = sc_hash(clean_name);
	for (index_ = 0; index_ < parse_resources_length; index_++) {
		if (parse_resources[index_].hash == hash
		        && strcmp(parse_resources[index_].name, clean_name) == 0) {
			offset = parse_resources[index_].offset;
			break;
		}
	}
	if (offset != -1) {
		*real_length = parse_resources[index_].length;
		sc_free(clean_name);
		return offset;
	}

	/* Resize the resources table if required. */
	if (parse_resources_length == parse_resources_size) {
		parse_resources_size += RESOURCE_GROW_INCREMENT;
		parse_resources = (sc_parse_resource_t *)sc_realloc(parse_resources,
		                  parse_resources_size *
		                  sizeof(parse_resources[0]));
	}

	/*
	 * Calculate the offset.  For the first resource, it's zero; for others,
	 * it's one after the prior entry's offset and length.
	 */
	if (parse_resources_length == 0)
		offset = 0;
	else {
		offset = parse_resources[parse_resources_length - 1].offset
		         + parse_resources[parse_resources_length - 1].length + 1;
	}

	/* Add details to the table. */
	parse_resources[parse_resources_length].name = clean_name;
	parse_resources[parse_resources_length].hash = hash;
	parse_resources[parse_resources_length].offset = offset;
	parse_resources[parse_resources_length].length = length;
	parse_resources_length++;

	*real_length = length;
	return offset;
}


/*
 * parse_handle_v400_resources()
 *
 * Extra special handling for version 4.0 resources; extracts details of
 * the resource just parsed, and adds an offset property for each defined.
 *
 * A warning -- Adrift seems to use -ve numbers as lengths for resources
 * already parsed, where TAF files include the resource.  It's unclear
 * what the -ve values mean, so here we ignore them and work off the
 * resource file name given.  This means we have to look for length not
 * equal to zero, not just lengths greater than zero.
 *
 * TODO Work out what this means.  The -ve lengths look like a form of
 * 'resource number'; -(length+2) is tantalizingly close to the index into
 * our parse_resources table, but not always...
 */
static void parse_handle_v400_resources(sc_bool has_sound, sc_bool has_graphics) {
	sc_vartype_t vt_key, vt_value;
	const sc_char *file;
	sc_int length, offset;

	/*
	 * Retrieve the file and length for the sound just parsed.  If there's a
	 * file of non-zero length, rewrite its offset.
	 */
	if (has_sound) {
		/* Retrieve the file and length information. */
		vt_key.string = "SoundFile";
		parse_push_key(vt_key, PROP_KEY_STRING);
		file = parse_get_string_property();
		parse_pop_key();

		vt_key.string = "SoundLen";
		parse_push_key(vt_key, PROP_KEY_STRING);
		length = parse_get_integer_property();
		parse_pop_key();

		/*
		 * If defined and has a length, rewrite the offset, and also the length
		 * in case changed.
		 */
		if (!sc_strempty(file) && length != 0) {
			sc_int real_length;

			offset = parse_get_v400_resource_offset(file, length, &real_length);
			vt_key.string = "SoundOffset";
			parse_push_key(vt_key, PROP_KEY_STRING);

			vt_value.integer = offset;
			parse_put_property(vt_value, PROP_INTEGER);

			parse_pop_key();

			/* Rewrite length if changed. */
			if (real_length != length) {
				vt_key.string = "SoundLen";
				parse_push_key(vt_key, PROP_KEY_STRING);

				vt_value.integer = real_length;
				parse_put_property(vt_value, PROP_INTEGER);

				parse_pop_key();
			}
		}
	}

	/* Now do the same thing for graphics. */
	if (has_graphics) {
		/* Retrieve the file and length information. */
		vt_key.string = "GraphicFile";
		parse_push_key(vt_key, PROP_KEY_STRING);
		file = parse_get_string_property();
		parse_pop_key();

		vt_key.string = "GraphicLen";
		parse_push_key(vt_key, PROP_KEY_STRING);
		length = parse_get_integer_property();
		parse_pop_key();

		/*
		 * If defined and has a length, rewrite the offset, and also the length
		 * in case changed.
		 */
		if (!sc_strempty(file) && length != 0) {
			sc_int real_length;

			offset = parse_get_v400_resource_offset(file, length, &real_length);
			vt_key.string = "GraphicOffset";
			parse_push_key(vt_key, PROP_KEY_STRING);

			vt_value.integer = offset;
			parse_put_property(vt_value, PROP_INTEGER);

			parse_pop_key();

			/* Rewrite length if changed. */
			if (real_length != length) {
				vt_key.string = "GraphicLen";
				parse_push_key(vt_key, PROP_KEY_STRING);

				vt_value.integer = real_length;
				parse_put_property(vt_value, PROP_INTEGER);

				parse_pop_key();
			}
		}
	}
}


/*
 * parse_special()
 *
 * Handler for special items that can't be described accurately, and
 * therefore need careful treatment.
 */
static void parse_special(CONTEXT, const sc_char *special) {
	if (parse_trace)
		sc_trace("Parse: entering special %s\n", special);

	/* Special handling for version 4.0 resources. */
	if (strcmp(special, "{V400_RESOURCE}") == 0) {
		sc_vartype_t vt_key[2];
		sc_bool has_sound, has_graphics;

		/* Get sound and graphics global flags. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "Sound";
		has_sound = prop_get_boolean(parse_bundle, "B<-ss", vt_key);

		vt_key[1].string = "Graphics";
		has_graphics = prop_get_boolean(parse_bundle, "B<-ss", vt_key);

		/* Apply special handling to the resources. */
		parse_handle_v400_resources(has_sound, has_graphics);
	}

	/* Parse a version 4.0 optional set of room exit information. */
	else if (strcmp(special, "{V400_ROOM_EXIT:#Dest_#Var1_#Var2_#Var3}") == 0) {
		sc_int flag;

		/* Get next flag, and if true, pushback and parse. */
		FUNC0(parse_get_taf_integer, flag);
		if (flag != 0) {
			parse_taf_pushback();
			CALL1(parse_descriptor, "#Dest #Var1 #Var2 #Var3");
		}
	}

	/* Parse version 3.9 and version 3.8 optional room exit information. */
	else if (strcmp(special,
	                "{V390_V380_ROOM_EXIT:#Dest_#Var1_#Var2_ZVar3}") == 0) {
		sc_int flag;

		/* Get next flag, and if true, pushback and parse. */
		FUNC0(parse_get_taf_integer, flag);
		if (flag != 0) {
			parse_taf_pushback();
			CALL1(parse_descriptor, "#Dest #Var1 #Var2 ZVar3");
		}
	}

	/* Parse room lists, with optional extra room. */
	else if (strcmp(special, "{ROOM_LIST0}") == 0
	         || strcmp(special, "{ROOM_LIST1}") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int room_count, num_rooms, type, index_;

		/* Retrieve the room list type. */
		vt_key.string = "Type";
		parse_push_key(vt_key, PROP_KEY_STRING);
		type = parse_get_integer_property();
		parse_pop_key();

		/* Write remaining room list depending on the type. */
		switch (type) {
		case ROOMLIST_NO_ROOMS:
		case ROOMLIST_ALL_ROOMS:
		case ROOMLIST_NPC_PART:
			break;

		case ROOMLIST_ONE_ROOM:
			/* Store this room as the single list entry. */
			CALL1(parse_element, "#Room");
			break;

		case ROOMLIST_SOME_ROOMS:
			/* Get count of rooms defined, add one if necessary. */
			vt_key.string = "Rooms";
			room_count = prop_get_child_count(parse_bundle, "I<-s", &vt_key);

			if (strcmp(special, "{ROOM_LIST1}") == 0)
				num_rooms = room_count + 1;
			else
				num_rooms = room_count;

			/* Store an array of rooms flags for each room. */
			vt_key.string = "Rooms";
			parse_push_key(vt_key, PROP_KEY_STRING);
			for (index_ = 0; index_ < num_rooms; index_++) {
				sc_bool this_room;

				/* Get flag for this room. */
				FUNC0(parse_get_taf_boolean, this_room);

				/* Store flag directly. */
				vt_key.integer = index_;
				parse_push_key(vt_key, PROP_KEY_INTEGER);
				vt_value.boolean = this_room;
				parse_put_property(vt_value, PROP_BOOLEAN);
				parse_pop_key();
			}
			parse_pop_key();
			break;

		default:
			sc_fatal("parse_special: bad type, %ld\n", type);
		}
	}

	/* Parse Parent number iff this object is an NPC part. */
	else if (strcmp(special, "{OBJECT:#Parent}") == 0) {
		sc_vartype_t vt_key;
		sc_int type;

		/* Check object's Where room list Type for NPC part. */
		vt_key.string = "Where";
		parse_push_key(vt_key, PROP_KEY_STRING);
		vt_key.string = "Type";
		parse_push_key(vt_key, PROP_KEY_STRING);
		type = parse_get_integer_property();
		parse_pop_key();
		parse_pop_key();

		/* Get Parent if the object is part of an NPC. */
		if (type == ROOMLIST_NPC_PART) {
			CALL1(parse_element, "#Parent");
		}
	}

	/* Parse a list of rooms and times for a walk. */
	else if (strcmp(special, "{WALK:#Rooms_#Times}") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int num_stops, index_;

		/* Obtain the count of stops in this walk. */
		vt_key.string = "NumStops";
		parse_push_key(vt_key, PROP_KEY_STRING);
		num_stops = parse_get_integer_property();
		parse_pop_key();

		/* Look for a room and time for each stop. */
		for (index_ = 0; index_ < num_stops; index_++) {
			sc_int room, time;

			/* Parse and store Rooms[index_]. */
			vt_key.string = "Rooms";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_key.integer = index_;
			parse_push_key(vt_key, PROP_KEY_INTEGER);

			FUNC0(parse_get_taf_integer, room);

			vt_value.integer = room;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
			parse_pop_key();

			/* Parse and store Times[index_]. */
			vt_key.string = "Times";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_key.integer = index_;
			parse_push_key(vt_key, PROP_KEY_INTEGER);

			FUNC0(parse_get_taf_integer, time);

			vt_value.integer = time;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
			parse_pop_key();
		}
	}

	/* Parse a room group variable size boolean list. */
	else if (strcmp(special, "{ROOM_GROUP:[]BList}") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int num_rooms, index_, l2index_;
		sc_bool in_group;

		/* Get the count of rooms defined. */
		vt_key.string = "Rooms";
		num_rooms = prop_get_integer(parse_bundle, "I<-s", &vt_key);

		/* Read a boolean for each room. */
		l2index_ = 0;
		for (index_ = 0; index_ < num_rooms; index_++) {
			FUNC0(parse_get_taf_boolean, in_group);

			/* Store raw flag as List[index_]. */
			vt_key.string = "List";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_key.integer = index_;
			parse_push_key(vt_key, PROP_KEY_INTEGER);
			vt_value.boolean = in_group;
			parse_put_property(vt_value, PROP_BOOLEAN);

			parse_pop_key();
			parse_pop_key();

			/* Store in-group index'es as List2[0..n]. */
			if (in_group) {
				vt_key.string = "List2";
				parse_push_key(vt_key, PROP_KEY_STRING);
				vt_key.integer = l2index_;
				parse_push_key(vt_key, PROP_KEY_INTEGER);
				vt_value.integer = index_;
				parse_put_property(vt_value, PROP_INTEGER);

				parse_pop_key();
				parse_pop_key();

				l2index_++;
			}
		}
	}

	/* Error if no special handler available. */
	else {
		sc_fatal("parse_special: no handler for \"%s\"\n", special);
	}

	if (parse_trace)
		sc_trace("Parse: leaving special %s\n", special);
}


/*
 * parse_fixup_v390_v380_room_alt()
 *
 * Helper for parse_fixup_v390_v380_room_alts().  Handles creation of
 * version 4.0 room alts for version 3.9 and version 3.8 games.
 */
static void parse_fixup_v390_v380_room_alt(const sc_char *m1, sc_int type,
		const sc_char *resource1, const sc_char *m2, sc_int var2, const sc_char *resource2,
		sc_int hide_objects, const sc_char *changed, sc_int var3, sc_int display_room) {
	sc_vartype_t vt_key, vt_value, vt_gkey[2];
	sc_bool has_sound, has_graphics;
	sc_int alt_count;
	const sc_char *soundfile1, *graphicfile1;
	const sc_char *soundfile2, *graphicfile2;

	/*
	 * Initialize resource files to empty, for cases where no resource is copied
	 * over from the main room (NULL resource1/2).
	 */
	soundfile1 = "";
	graphicfile1 = "";
	soundfile2 = "";
	graphicfile2 = "";

	/* Get sound and graphics flags, always FALSE for version 3.8. */
	vt_gkey[0].string = "Globals";
	vt_gkey[1].string = "Sound";
	has_sound = prop_get_boolean(parse_bundle, "B<-ss", vt_gkey);

	vt_gkey[1].string = "Graphics";
	has_graphics = prop_get_boolean(parse_bundle, "B<-ss", vt_gkey);

	/* Get a count of alts so far defined for the room. */
	vt_key.string = "Alts";
	parse_push_key(vt_key, PROP_KEY_STRING);
	alt_count = parse_get_child_count();
	parse_pop_key();

	/*
	 * Lookup any resource details now, and save them.  Because this is not
	 * version 4.0, we can ignore lengths, and set them to zero when needed.
	 */
	if (has_sound || has_graphics) {
		if (resource1) {
			vt_key.string = resource1;
			parse_push_key(vt_key, PROP_KEY_STRING);
			if (has_sound) {
				vt_key.string = "SoundFile";
				parse_push_key(vt_key, PROP_KEY_STRING);
				soundfile1 = parse_get_string_property();
				parse_pop_key();
			}
			if (has_graphics) {
				vt_key.string = "GraphicFile";
				parse_push_key(vt_key, PROP_KEY_STRING);
				graphicfile1 = parse_get_string_property();
				parse_pop_key();
			}
			parse_pop_key();
		}

		if (resource2) {
			vt_key.string = resource2;
			parse_push_key(vt_key, PROP_KEY_STRING);
			if (has_sound) {
				vt_key.string = "SoundFile";
				parse_push_key(vt_key, PROP_KEY_STRING);
				soundfile2 = parse_get_string_property();
				parse_pop_key();
			}
			if (has_graphics) {
				vt_key.string = "GraphicFile";
				parse_push_key(vt_key, PROP_KEY_STRING);
				graphicfile2 = parse_get_string_property();
				parse_pop_key();
			}
			parse_pop_key();
		}
	}

	/*
	 * Create a room alt to match data passed in.  Start with the Alts string
	 * and the index to the alt being written.  To correctly emulate the parse,
	 * we also have to reverse the "Alts" and the index, as parse_put_property()
	 * will swap them.  Madness.
	 */
	vt_key.integer = alt_count;
	parse_push_key(vt_key, PROP_KEY_INTEGER);
	vt_key.string = "Alts";
	parse_push_key(vt_key, PROP_KEY_STRING);

	/* Write M1 and Type. */
	vt_key.string = "M1";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.string = m1;
	parse_put_property(vt_value, PROP_STRING);
	parse_pop_key();
	vt_key.string = "Type";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = type;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	/* If resources, add these as retrieved above. */
	if (has_sound || has_graphics) {
		vt_key.string = "Res1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		if (has_sound) {
			vt_key.string = "SoundFile";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = soundfile1;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();
			vt_key.string = "SoundLen";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = 0;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
		if (has_graphics) {
			vt_key.string = "GraphicFile";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = graphicfile1;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();
			vt_key.string = "GraphicLen";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = 0;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
		parse_pop_key();
	}

	/* Write M2 and Var2. */
	vt_key.string = "M2";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.string = m2;
	parse_put_property(vt_value, PROP_STRING);
	parse_pop_key();
	vt_key.string = "Var2";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = var2;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	/* If resources, again add these as retrieved above. */
	if (has_sound || has_graphics) {
		vt_key.string = "Res2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		if (has_sound) {
			vt_key.string = "SoundFile";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = soundfile2;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();
			vt_key.string = "SoundLen";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = 0;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
		if (has_graphics) {
			vt_key.string = "GraphicFile";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = graphicfile2;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();
			vt_key.string = "GraphicLen";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = 0;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
		parse_pop_key();
	}

	/* Finish off with the last four alt properties. */
	vt_key.string = "HideObjects";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = hide_objects;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();
	vt_key.string = "Changed";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.string = changed;
	parse_put_property(vt_value, PROP_STRING);
	parse_pop_key();
	vt_key.string = "Var3";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = var3;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();
	vt_key.string = "DisplayRoom";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = display_room;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	parse_pop_key();
	parse_pop_key();
}


/* Multiplier for combination AltDesc Type and HideObject values. */
enum { V390_V380_ALT_TYPEHIDE_MULT = 10 };

/*
 * parse_fixup_v390_v380_room_alts()
 *
 * Common helper function for parse_fixup_v390() and parse_fixup_v380(),
 * converts version 3.9 and version 3.8 fixed room description alts into
 * an equivalent array of version 4.0 style room alts.
 */
static void parse_fixup_v390_v380_room_alts(void) {
	sc_vartype_t vt_key;
	const sc_char *m1, *m2, *changed;
	sc_int type, var2, hide_objects, var3, display_room;

	/* Room alt invariants. */
	m2 = "";                      /* No else text */
	changed = "";                 /* No changed room name */

	/*
	 * Create a room alt to override all others, controlled by an object
	 * condition and with optional object hiding.
	 */
	type = 2;                     /* Object condition */
	display_room = 0;             /* Override all others */

	vt_key.string = "Obj";
	parse_push_key(vt_key, PROP_KEY_STRING);
	var3 = parse_get_integer_property();
	parse_pop_key();

	if (var3 > 0) {
		sc_int typehideobjects;

		vt_key.string = "AltDesc";
		parse_push_key(vt_key, PROP_KEY_STRING);
		m1 = parse_get_string_property();
		parse_pop_key();

		vt_key.string = "TypeHideObjects";
		parse_push_key(vt_key, PROP_KEY_STRING);
		typehideobjects = parse_get_integer_property();
		parse_pop_key();

		var2 = typehideobjects / V390_V380_ALT_TYPEHIDE_MULT;
		hide_objects = typehideobjects % V390_V380_ALT_TYPEHIDE_MULT;

		parse_fixup_v390_v380_room_alt(m1, type, "AltRes",
		                               m2, var2, NULL,
		                               hide_objects, changed, var3,
		                               display_room);
	}

	/*
	 * If a second task alternate description is defined, create a room alt to
	 * add after the main description, one that stops printing once done.
	 */
	type = 0;                     /* Task condition */
	display_room = 1;             /* Print after main and stop */

	vt_key.string = "Task2";
	parse_push_key(vt_key, PROP_KEY_STRING);
	var2 = parse_get_integer_property();
	parse_pop_key();

	if (var2 > 0) {
		vt_key.string = "AddDesc2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		m1 = parse_get_string_property();
		parse_pop_key();

		var3 = 0;
		hide_objects = 0;

		parse_fixup_v390_v380_room_alt(m1, type, "Task2Res",
		                               m2, var2, NULL,
		                               hide_objects, changed, var3,
		                               display_room);
	}

	/* Do the same for any first task additional description. */
	type = 0;                     /* Task condition */
	display_room = 1;             /* Print after main and stop */

	vt_key.string = "Task1";
	parse_push_key(vt_key, PROP_KEY_STRING);
	var2 = parse_get_integer_property();
	parse_pop_key();

	if (var2 > 0) {
		vt_key.string = "AddDesc1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		m1 = parse_get_string_property();
		parse_pop_key();

		var3 = 0;
		hide_objects = 0;

		parse_fixup_v390_v380_room_alt(m1, type, "Task1Res",
		                               m2, var2, NULL,
		                               hide_objects, changed, var3,
		                               display_room);
	}

	/*
	 * If still printing at this point, we need a catch-all room alt that will
	 * print.  So create one with an always true condition.
	 */
	type = 0;                     /* Task condition */
	display_room = 2;             /* Lowest priority output */

	vt_key.string = "LastDesc";
	parse_push_key(vt_key, PROP_KEY_STRING);
	m1 = parse_get_string_property();
	parse_pop_key();

	if (!sc_strempty(m1)) {
		var2 = 0;                 /* No task - always TRUE */
		var3 = 0;
		hide_objects = 0;

		parse_fixup_v390_v380_room_alt(m1, type, "LastRes",
		                               m2, var2, NULL,
		                               hide_objects, changed, var3,
		                               display_room);
	}
}


/*
 * parse_fixup_v390()
 *
 * Handler for fixup special items to help with conversions from TAF version
 * 3.9 format into version 4.0.
 */
static void parse_fixup_v390(CONTEXT, const sc_char *fixup) {
	if (parse_trace)
		sc_trace("Parse: entering version 3.9 fixup %s\n", fixup);

	/* Fixup a version 3.9 task action by incrementing Type > 4. */
	if (strcmp(fixup, "|V390_TASK_ACTION:Type>4?#Type++|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int type;

		/* Retrieve Type, and if > 4, increment. */
		vt_key.string = "Type";
		parse_push_key(vt_key, PROP_KEY_STRING);
		type = parse_get_integer_property();

		if (type > 4) {
			vt_value.integer = type + 1;
			parse_put_property(vt_value, PROP_INTEGER);
		}

		parse_pop_key();
	}

	/* Handle either Expr or Var5 for version 3.9 task actions. */
	else if (strcmp(fixup, "|V390_TASK_ACTION:$Expr_#Var5|") == 0) {
		sc_vartype_t vt_key;
		sc_int var2;

		/* Either Expr or Var5, depending on Var2. */
		vt_key.string = "Var2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		var2 = parse_get_integer_property();
		parse_pop_key();

		if (var2 == 5) {
			CALL1(parse_descriptor, "$Expr ZVar5");
		} else {
			CALL1(parse_descriptor, "EExpr #Var5");
		}
	}

	/*
	 * Exchange openable values 5 and 6, and write -1 key for openable objects.
	 */
	else if (strcmp(fixup, "|V390_OBJECT:_Openable_,Key|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int openable;

		/* Retrieve Openable, and if 5 or 6, exchange. */
		vt_key.string = "Openable";
		parse_push_key(vt_key, PROP_KEY_STRING);
		openable = parse_get_integer_property();

		if (openable == 5 || openable == 6) {
			vt_value.integer = (openable == 5) ? 6 : 5;
			parse_put_property(vt_value, PROP_INTEGER);
		}

		parse_pop_key();

		/* For openable objects, store a Key of -1. */
		if (openable == 5 || openable == 6) {
			vt_key.string = "Key";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = -1;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
	}

	/* Create a RestrMask that 'and's all the restrictions together. */
	else if (strcmp(fixup, "|V390_TASK:$RestrMask|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int restriction_count;

		/* Get a count of restrictions. */
		vt_key.string = "Restrictions";
		parse_push_key(vt_key, PROP_KEY_STRING);
		restriction_count = parse_get_child_count();
		parse_pop_key();

		/* Allocate and fill a new mask for these restrictions. */
		if (restriction_count > 0) {
			sc_char *restrmask;
			sc_int index_;

			restrmask = (sc_char *)sc_malloc(2 * restriction_count);
			strcpy(restrmask, "#");
			for (index_ = 1; index_ < restriction_count; index_++)
				strcat(restrmask, "A#");

			vt_key.string = "RestrMask";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = restrmask;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();

			prop_adopt(parse_bundle, restrmask);
		}
	}

	/*
	 * Increment var1 for variable restrictions to compensate for there being no
	 * referenced text comparison (no string variables).
	 */
	else if (strcmp(fixup, "|V390_TASK_RESTR:Var1>0?#Var1++|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int var1;

		/* Retrieve Var1, and if greater than zero, increment. */
		vt_key.string = "Var1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		var1 = parse_get_integer_property();

		if (var1 > 0) {
			vt_value.integer = var1 + 1;
			parse_put_property(vt_value, PROP_INTEGER);
		}

		parse_pop_key();
	}

	/* Convert version 3.9 fixed alts into a version 4.0 array. */
	else if (strcmp(fixup, "|V390_ROOM:_Alts_|") == 0) {
		parse_fixup_v390_v380_room_alts();
	}

	/* Error if no fixup special handler available. */
	else {
		sc_fatal("parse_fixup_v390: no handler for \"%s\"\n", fixup);
	}

	if (parse_trace)
		sc_trace("Parse: leaving version 3.9 fixup %s\n", fixup);
}


/*
 * Object surface and container masks for version 3.8 object fixup, container
 * capacity conversion factor and default object sizing, and the count of
 * task movements in a version 3.8 task.
 */
enum { V380_OBJ_IS_SURFACE = 2, V380_OBJ_IS_CONTAINER = 1 };
enum { V380_OBJ_CAPACITY_MULT = 10, V380_OBJ_DEFAULT_SIZE = 2 };
enum { V380_TASK_MOVEMENTS = 6 };

/*
 * parse_fixup_v380_action()
 *
 * Helper for parse_fixup_v380(), adds a task action.
 */
static void parse_fixup_v380_action(sc_int type, sc_int var_count,
		sc_int var1, sc_int var2, sc_int var3) {
	sc_vartype_t vt_key, vt_value;
	sc_int action_count;

	/* Get a count of actions so far defined for the task. */
	vt_key.string = "Actions";
	parse_push_key(vt_key, PROP_KEY_STRING);
	action_count = parse_get_child_count();
	parse_pop_key();

	/* Write actions key, reversed to emulate parse actions. */
	vt_key.integer = action_count;
	parse_push_key(vt_key, PROP_KEY_INTEGER);
	vt_key.string = "Actions";
	parse_push_key(vt_key, PROP_KEY_STRING);

	/* Write new action according to the given arguments. */
	vt_key.string = "Type";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = type;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	vt_key.string = "Var1";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = var1;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	if (var_count > 1) {
		vt_key.string = "Var2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		vt_value.integer = var2;
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	if (var_count > 2) {
		vt_key.string = "Var3";
		parse_push_key(vt_key, PROP_KEY_STRING);
		vt_value.integer = var3;
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	parse_pop_key();
	parse_pop_key();
}


/*
 * parse_fixup_v380_movement()
 *
 * Helper for parse_fixup_v380(), converts a task movement into an action.
 */
static void parse_fixup_v380_movement(sc_int mvar1, sc_int mvar2, sc_int mvar3) {
	sc_int var1;

	/* If nothing was selected to move, ignore the call. */
	if (mvar1 == 0)
		return;

	/*
	 * Accept only player moves into rooms.  Other combinations, such as move
	 * player to worn by player, are unlikely.  And move player to same room as
	 * player isn't useful.
	 */
	if (mvar1 == 1) {
		if (mvar3 == 0 && mvar2 >= 2)
			parse_fixup_v380_action(1, 3, 0, 0, mvar2 - 2);
		return;
	}

	/*
	 * Convert movement var1 into action var1.  Var1 is the dynamic object + 3,
	 * or 2 for referenced object, or 0 for all held.
	 */
	switch (mvar1) {
	case 2:
		var1 = 2;
		break;                    /* Referenced obj */
	case 3:
		var1 = 0;
		break;                    /* All held */
	default:
		var1 = mvar1 - 1;
		break;                    /* Dynamic obj */
	}

	/* Dissect the rest of the movement. */
	switch (mvar3) {
	case 0:                    /* To room */
		/*
		 * Convert movement var2 into action var2 and var3.  Var2 is 0 for move
		 * to room, 6 for move to player room.  Var3 is 0 for hidden, otherwise
		 * the room number plus one.
		 */
		if (mvar2 == 0)           /* Hidden */
			parse_fixup_v380_action(0, 3, var1, 0, 0);
		else if (mvar2 == 1)      /* Player room */
			parse_fixup_v380_action(0, 3, var1, 6, 0);
		else                      /* Specified room */
			parse_fixup_v380_action(0, 3, var1, 0, mvar2 - 1);
		break;

	case 1:                    /* To inside */
	case 2:                    /* To onto */
		/*
		 * Convert movement var2 and var3 into action var3 and var2, a simple
		 * conversion, but check that var2 is not 'not selected' first.
		 */
		if (mvar2 > 0)
			parse_fixup_v380_action(0, 3, var1, mvar3 + 1, mvar2 - 1);
		break;

	case 3:                    /* To held by */
	case 4:                    /* To worn by */
		/*
		 * Convert movement var2 and var3 into action var3 and var2, in this
		 * case a simple conversion, since version 4.0 task actions are close
		 * here.
		 */
		parse_fixup_v380_action(0, 3, var1, mvar3 + 1, mvar2);
		break;

	default:
		sc_fatal("parse_fixup_v380_movement: invalid mvar3, %ld\n", mvar3);
	}
}


/*
 * parse_fixup_v380_restr()
 *
 * Helper for parse_fixup_v380(), adds a task restriction.
 */
static void parse_fixup_v380_restr(sc_int type, sc_int var_count,
		sc_int var1, sc_int var2, sc_int var3, const sc_char *failmessage) {
	sc_vartype_t vt_key, vt_value;
	sc_int restriction_count;

	/* Get a count of restrictions so far defined for the task. */
	vt_key.string = "Restrictions";
	parse_push_key(vt_key, PROP_KEY_STRING);
	restriction_count = parse_get_child_count();
	parse_pop_key();

	/* Write restrictions key, reversed to emulate parse actions. */
	vt_key.integer = restriction_count;
	parse_push_key(vt_key, PROP_KEY_INTEGER);
	vt_key.string = "Restrictions";
	parse_push_key(vt_key, PROP_KEY_STRING);

	/* Write new restriction according to the given arguments. */
	vt_key.string = "Type";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = type;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	vt_key.string = "Var1";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.integer = var1;
	parse_put_property(vt_value, PROP_INTEGER);
	parse_pop_key();

	if (var_count > 1) {
		vt_key.string = "Var2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		vt_value.integer = var2;
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	if (var_count > 2) {
		vt_key.string = "Var3";
		parse_push_key(vt_key, PROP_KEY_STRING);
		vt_value.integer = var3;
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	vt_key.string = "FailMessage";
	parse_push_key(vt_key, PROP_KEY_STRING);
	vt_value.string = failmessage;
	parse_put_property(vt_value, PROP_STRING);
	parse_pop_key();

	parse_pop_key();
	parse_pop_key();
}


/*
 * parse_fixup_v380_obj_restr()
 * parse_fixup_v380_task_restr()
 * parse_fixup_v380_wear_restr()
 * parse_fixup_v380_npc_restr()
 * parse_fixup_v380_objroom_restr()
 * parse_fixup_v380_objstate_restr()
 *
 * Helper handlers for parse_fixup_v380(); create task restrictions.
 */
static void parse_fixup_v380_obj_restr(sc_bool holding, sc_int holdobj, const sc_char *failmessage) {
	/* Ignore if no object selected. */
	if (holdobj > 0) {
		sc_int var1, var2;

		/*
		 * Create version 4.0 task restriction to check for either the
		 * referenced object or a dynamic object being either held or in the
		 * same room (visible to player).
		 */
		var1 = (holdobj == 1) ? 2 : holdobj + 1;
		var2 = holding ? 1 : 3;
		parse_fixup_v380_restr(0, 3, var1, var2, 0, failmessage);
	}
}

static void parse_fixup_v380_task_restr(sc_bool tasknotdone, sc_int task, const sc_char *failmessage) {
	/* Ignore if no task selected. */
	if (task > 0) {
		sc_int var2;

		/* Create version 4.0 restriction to check task state. */
		var2 = tasknotdone ? 1 : 0;
		parse_fixup_v380_restr(2, 2, task, var2, 0, failmessage);
	}
}

static void parse_fixup_v380_wear_restr(sc_int wearobj, const sc_char *failmessage) {
	/* Ignore if no object selected. */
	if (wearobj > 0) {
		sc_vartype_t vt_key[3];
		sc_int object_count, object, dynamic, obj_index;

		/*
		 * Create version 4.0 restrictions for something or nothing worn by
		 * player.
		 */
		if (wearobj == 1) {
			parse_fixup_v380_restr(0, 3, 1, 2, 0, failmessage);
			return;
		} else if (wearobj == 2) {
			parse_fixup_v380_restr(0, 3, 0, 2, 0, failmessage);
			return;
		}

		/* Get the count of objects defined. */
		vt_key[0].string = "Objects";
		object_count = prop_get_child_count(parse_bundle, "I<-s", vt_key);

		/* Convert wearobj from worn index to object index. */
		wearobj -= 2;
		for (object = 0; object < object_count && wearobj > 0; object++) {
			sc_bool bstatic, wearable;

			vt_key[1].integer = object;
			vt_key[2].string = "Static";
			bstatic = prop_get_boolean(parse_bundle, "B<-sis", vt_key);
			if (!bstatic) {
				vt_key[2].string = "Wearable";
				wearable = prop_get_boolean(parse_bundle, "B<-sis", vt_key);
				if (wearable)
					wearobj--;
			}
		}
		obj_index = object - 1;

		/* Now convert wearobj from object index to dynamic index. */
		dynamic = 0;
		for (object = 0; object <= obj_index; object++) {
			sc_bool bstatic;

			vt_key[1].integer = object;
			vt_key[2].string = "Static";
			bstatic = prop_get_boolean(parse_bundle, "B<-sis", vt_key);
			if (!bstatic)
				dynamic++;
		}
		dynamic--;

		/* Create version 4.0 restriction for object worn by player. */
		parse_fixup_v380_restr(0, 3, dynamic + 3, 2, 0, failmessage);
	}
}

static void parse_fixup_v380_npc_restr(sc_bool notinsameroom, sc_int npc,
		const sc_char *failmessage) {
	/* Ignore if no NPC selected. */
	if (npc > 0) {
		sc_int var2;

		if (npc == 1) {
			/* Create restriction to look for alone, or not. */
			var2 = notinsameroom ? 3 : 2;
			parse_fixup_v380_restr(3, 3, 0, var2, 0, failmessage);
			return;
		}

		/* Create restriction to look for company. */
		var2 = notinsameroom ? 1 : 0;
		parse_fixup_v380_restr(3, 3, 0, var2, npc, failmessage);
	}
}

static void parse_fixup_v380_objroom_restr(sc_int obj, sc_int objroom, const sc_char *failmessage) {
	/* Ignore if no object selected. */
	if (obj > 0) {
		/* Create version 4.0 restriction to check object in room. */
		parse_fixup_v380_restr(0, 3, obj + 1, 0, objroom, failmessage);
	}
}

static void parse_fixup_v380_objstate_restr(sc_int obj, sc_int ivar1, sc_int ivar2,
		const sc_char *failmessage) {
	sc_vartype_t vt_key[3];
	sc_int object, dynamic, var2, var3;

	/* Initialize variables to avoid gcc warnings. */
	var2 = -1;
	var3 = -1;

	/* Ignore restrictions with no "type". */
	if (ivar1 == 0)
		return;

	/* Look for opened/closed restrictions, convert and return. */
	if (ivar1 == 3 || ivar1 == 4) {
		sc_int stateful;

		/* Convert obj from object to openable (stateful) index. */
		stateful = 0;
		for (object = 0; object <= obj - 1; object++) {
			sc_int openable;

			vt_key[0].string = "Objects";
			vt_key[1].integer = object;
			vt_key[2].string = "Openable";
			openable = prop_get_integer(parse_bundle, "I<-sis", vt_key);
			if (openable > 0)
				stateful++;
		}
		stateful--;

		/*
		 * Create a version 4.0 restriction that checks that an object's state
		 * is open (var2 = 0) or closed (var2 = 1).
		 */
		var2 = (ivar1 == 3) ? 0 : 1;
		parse_fixup_v380_restr(1, 2, stateful + 1, var2, 0, failmessage);
		return;
	}

	/* Convert obj from object to dynamic index. */
	dynamic = 0;
	for (object = 0; object <= obj - 1; object++) {
		sc_bool bstatic;

		vt_key[0].string = "Objects";
		vt_key[1].integer = object;
		vt_key[2].string = "Static";
		bstatic = prop_get_boolean(parse_bundle, "B<-sis", vt_key);
		if (!bstatic)
			dynamic++;
	}
	dynamic--;

	/* Create version 4.0 object location restrictions for the rest. */
	switch (ivar1) {
	case 1:
		var2 = 4;
		var3 = ivar2;
		break;                    /* Inside */
	case 2:
		var2 = 5;
		var3 = ivar2;
		break;                    /* On */
	case 5:
		var2 = 1;
		var3 = ivar2 + 1;
		break;                    /* Held by */
	case 6:
		var2 = 2;
		var3 = ivar2 + 1;
		break;                    /* Worn by */
	default:
		sc_fatal("parse_fixup_v380_objstate_restr: invalid ivar1, %ld\n", ivar1);
	}
	parse_fixup_v380_restr(0, 3, dynamic + 3, var2, var3, failmessage);
}


/*
 * parse_fixup_v380()
 *
 * Handler for fixup special items to help with conversions from TAF version
 * 3.8 format into version 4.0.
 */
static void parse_fixup_v380(const sc_char *fixup) {
	if (parse_trace)
		sc_trace("Parse: entering version 3.8 fixup %s\n", fixup);

	/* Convert container capacity attributes to version 4.0 values. */
	if (strcmp(fixup, "|V380_OBJECT:#Capacity*10+2|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int surfacecontainer;

		/* Get the object surface and container attributes. */
		vt_key.string = "SurfaceContainer";
		parse_push_key(vt_key, PROP_KEY_STRING);
		surfacecontainer = parse_get_integer_property();
		parse_pop_key();

		/* Convert capacity from version 3.8 format to version 4.0. */
		if (surfacecontainer == V380_OBJ_IS_CONTAINER) {
			sc_int capacity;

			vt_key.string = "Capacity";
			parse_push_key(vt_key, PROP_KEY_STRING);
			capacity = parse_get_integer_property();

			capacity = capacity * V380_OBJ_CAPACITY_MULT + V380_OBJ_DEFAULT_SIZE;

			vt_value.integer = capacity;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
	}

	/*
	 * Exchange openable values 5 and 6, watch for a possible 1 from a 3.8 game
	 * (interpret as 0), and write -1 key for openable objects.
	 */
	else if (strcmp(fixup, "|V380_OBJECT:_Openable_,Key|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int openable;

		/* Retrieve Openable, and if 5 or 6, exchange. */
		vt_key.string = "Openable";
		parse_push_key(vt_key, PROP_KEY_STRING);
		openable = parse_get_integer_property();

		if (openable == 5 || openable == 6) {
			vt_value.integer = (openable == 5) ? 6 : 5;
			parse_put_property(vt_value, PROP_INTEGER);
		}

		/* If the odd value of 1, rewrite as zero. */
		else if (openable == 1) {
			vt_value.integer = 0;
			parse_put_property(vt_value, PROP_INTEGER);
		}

		parse_pop_key();

		/* For openable objects, store a Key of -1. */
		if (openable == 5 || openable == 6) {
			vt_key.string = "Key";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.integer = -1;
			parse_put_property(vt_value, PROP_INTEGER);
			parse_pop_key();
		}
	}

	/* Create version 4.0 task actions from a version 3.8 task. */
	else if (strcmp(fixup, "|V380_TASK:_Actions_|") == 0) {
		sc_vartype_t vt_key;
		sc_int score;
		sc_bool killsplayer, wingame;
		sc_int movement;

		/* Retrieve the score change for the task. */
		vt_key.string = "Score";
		parse_push_key(vt_key, PROP_KEY_STRING);
		score = parse_get_integer_property();
		parse_pop_key();

		/* Create any appropriate score change action. */
		if (score != 0)
			parse_fixup_v380_action(4, 1, score, 0, 0);

		/* Get player death and game winning flags. */
		vt_key.string = "KillsPlayer";
		parse_push_key(vt_key, PROP_KEY_STRING);
		killsplayer = parse_get_boolean_property();
		parse_pop_key();
		vt_key.string = "WinGame";
		parse_push_key(vt_key, PROP_KEY_STRING);
		wingame = parse_get_boolean_property();
		parse_pop_key();

		/* Create any appropriate game ending actions. */
		if (killsplayer)
			parse_fixup_v380_action(6, 1, 2, 0, 0);
		if (wingame)
			parse_fixup_v380_action(6, 1, 0, 0, 0);

		/* Handle each defined movement for the task. */
		for (movement = 0; movement < V380_TASK_MOVEMENTS; movement++) {
			sc_int mvar1, mvar2, mvar3;

			vt_key.integer = movement;
			parse_push_key(vt_key, PROP_KEY_INTEGER);
			vt_key.string = "Movements";
			parse_push_key(vt_key, PROP_KEY_STRING);

			/* Retrieve the movement parameters. */
			vt_key.string = "Var1";
			parse_push_key(vt_key, PROP_KEY_STRING);
			mvar1 = parse_get_integer_property();
			parse_pop_key();
			vt_key.string = "Var2";
			parse_push_key(vt_key, PROP_KEY_STRING);
			mvar2 = parse_get_integer_property();
			parse_pop_key();
			vt_key.string = "Var3";
			parse_push_key(vt_key, PROP_KEY_STRING);
			mvar3 = parse_get_integer_property();
			parse_pop_key();

			parse_pop_key();
			parse_pop_key();

			/* Create the corresponding task action. */
			parse_fixup_v380_movement(mvar1, mvar2, mvar3);
		}
	}

	/* Create version 4.0 task restrictions from a version 3.8 task. */
	else if (strcmp(fixup, "|V380_TASK:_Restrictions_|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_bool holding, tasknotdone, notinsameroom;
		sc_int holdobj1, holdobj2, holdobj3, task;
		sc_int wearobj1, wearobj2, npc, obj1, obj1room, obj2;
		const sc_char *holdmsg, *taskmsg, *wearmsg, *companymsg;
		const sc_char *obj1msg;
		sc_int restriction_count;

		/* Create restrictions for objects not held or absent. */
		vt_key.string = "HoldingSameRoom";
		parse_push_key(vt_key, PROP_KEY_STRING);
		holding = parse_get_boolean_property();
		parse_pop_key();

		vt_key.string = "HoldObj1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		holdobj1 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "HoldObj2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		holdobj2 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "HoldObj3";
		parse_push_key(vt_key, PROP_KEY_STRING);
		holdobj3 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "HoldMsg";
		parse_push_key(vt_key, PROP_KEY_STRING);
		holdmsg = parse_get_string_property();
		parse_pop_key();

		parse_fixup_v380_obj_restr(holding, holdobj1, holdmsg);
		parse_fixup_v380_obj_restr(holding, holdobj2, holdmsg);
		parse_fixup_v380_obj_restr(holding, holdobj3, holdmsg);

		/* Create any task state restriction. */
		vt_key.string = "TaskNotDone";
		parse_push_key(vt_key, PROP_KEY_STRING);
		tasknotdone = parse_get_boolean_property();
		parse_pop_key();

		vt_key.string = "Task";
		parse_push_key(vt_key, PROP_KEY_STRING);
		task = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "TaskMsg";
		parse_push_key(vt_key, PROP_KEY_STRING);
		taskmsg = parse_get_string_property();
		parse_pop_key();

		parse_fixup_v380_task_restr(tasknotdone, task, taskmsg);

		/* Create any object not worn restrictions. */
		vt_key.string = "WearObj1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		wearobj1 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "WearObj2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		wearobj2 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "WearMsg";
		parse_push_key(vt_key, PROP_KEY_STRING);
		wearmsg = parse_get_string_property();
		parse_pop_key();

		parse_fixup_v380_wear_restr(wearobj1, wearmsg);
		parse_fixup_v380_wear_restr(wearobj2, wearmsg);

		/* Check for presence/absence of NPCs restriction. */
		vt_key.string = "NotInSameRoom";
		parse_push_key(vt_key, PROP_KEY_STRING);
		notinsameroom = parse_get_boolean_property();
		parse_pop_key();

		vt_key.string = "NPC";
		parse_push_key(vt_key, PROP_KEY_STRING);
		npc = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "CompanyMsg";
		parse_push_key(vt_key, PROP_KEY_STRING);
		companymsg = parse_get_string_property();
		parse_pop_key();

		parse_fixup_v380_npc_restr(notinsameroom, npc, companymsg);

		/* Create any object location restriction. */
		vt_key.string = "Obj1";
		parse_push_key(vt_key, PROP_KEY_STRING);
		obj1 = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "Obj1Room";
		parse_push_key(vt_key, PROP_KEY_STRING);
		obj1room = parse_get_integer_property();
		parse_pop_key();

		vt_key.string = "Obj1Msg";
		parse_push_key(vt_key, PROP_KEY_STRING);
		obj1msg = parse_get_string_property();
		parse_pop_key();

		parse_fixup_v380_objroom_restr(obj1, obj1room, obj1msg);

		/* And finally, any object state restriction. */
		vt_key.string = "Obj2";
		parse_push_key(vt_key, PROP_KEY_STRING);
		obj2 = parse_get_integer_property();
		parse_pop_key();

		if (obj2 > 0) {
			sc_int var1, var2;
			const sc_char *obj2msg;

			vt_key.string = "Obj2Var1";
			parse_push_key(vt_key, PROP_KEY_STRING);
			var1 = parse_get_integer_property();
			parse_pop_key();

			vt_key.string = "Obj2Var2";
			parse_push_key(vt_key, PROP_KEY_STRING);
			var2 = parse_get_integer_property();
			parse_pop_key();

			vt_key.string = "Obj2Msg";
			parse_push_key(vt_key, PROP_KEY_STRING);
			obj2msg = parse_get_string_property();
			parse_pop_key();

			parse_fixup_v380_objstate_restr(obj2, var1, var2, obj2msg);
		}

		/* Get a count of restrictions created. */
		vt_key.string = "Restrictions";
		parse_push_key(vt_key, PROP_KEY_STRING);
		restriction_count = parse_get_child_count();
		parse_pop_key();

		/* Allocate and fill a new mask for these restrictions. */
		if (restriction_count > 0) {
			sc_char *restrmask;
			sc_int index_;

			restrmask = (sc_char *)sc_malloc(2 * restriction_count);
			strcpy(restrmask, "#");
			for (index_ = 1; index_ < restriction_count; index_++)
				strcat(restrmask, "A#");

			vt_key.string = "RestrMask";
			parse_push_key(vt_key, PROP_KEY_STRING);
			vt_value.string = restrmask;
			parse_put_property(vt_value, PROP_STRING);
			parse_pop_key();

			prop_adopt(parse_bundle, restrmask);
		}
	}

	/*
	 * Adjust dynamic object initial positions and parents (where contained
	 * or on surfaces) into version 4.0 range.
	 */
	else if (strcmp(fixup, "|V380_OBJECT:_InitialPositions_|") == 0) {
		sc_vartype_t vt_key[3];
		sc_int object_count, object, *object_type;

		/* Get a count of objects. */
		vt_key[0].string = "Objects";
		object_count = prop_get_child_count(parse_bundle, "I<-s", vt_key);

		/* Build an array of object container/surface types. */
		object_type = (sc_int *)sc_malloc(object_count * sizeof(*object_type));
		for (object = 0; object < object_count; object++) {
			vt_key[1].integer = object;
			vt_key[2].string = "SurfaceContainer";
			object_type[object] = prop_get_integer(parse_bundle,
			                                       "I<-sis", vt_key);
		}

		/* Adjust each object's initial position if necessary. */
		for (object = 0; object < object_count; object++) {
			sc_vartype_t vt_value;
			sc_bool is_static;
			sc_int initialposition;

			/* Ignore static objects; we only want dynamic ones. */
			vt_key[1].integer = object;
			vt_key[2].string = "Static";
			is_static = prop_get_boolean(parse_bundle, "B<-sis", vt_key);
			if (is_static)
				continue;

			/* If initial position is above on/in, increment. */
			vt_key[1].integer = object;
			vt_key[2].string = "InitialPosition";
			initialposition = prop_get_integer(parse_bundle, "I<-sis", vt_key);
			if (initialposition > 2) {
				vt_value.integer = initialposition + 1;
				prop_put(parse_bundle, "I->sis", vt_value, vt_key);
			}

			/*
			 * If initial position is on or in, decide which, depending on the
			 * type of the parent.  From this, expand initial position into a
			 * version 4.0 value.
			 */
			if (initialposition == 2) {
				sc_int count, parent, index_;

				/* Get parent container/surface index. */
				vt_key[1].integer = object;
				vt_key[2].string = "Parent";
				count = prop_get_integer(parse_bundle, "I<-sis", vt_key);

				/* Convert container/surface index. */
				for (parent = 0; parent < object_count && count >= 0; parent++) {
					if (object_type[parent] == V380_OBJ_IS_CONTAINER
					        || object_type[parent] == V380_OBJ_IS_SURFACE)
						count--;
				}
				parent--;

				/* If parent is a surface, adjust position. */
				if (object_type[parent] == V380_OBJ_IS_SURFACE) {
					vt_key[2].string = "InitialPosition";
					vt_value.integer = initialposition + 1;
					prop_put(parse_bundle, "I->sis", vt_value, vt_key);
				}

				/*
				 * For both, adjust parent to be an object index for that type
				 * of object only.
				 */
				count = 0;
				for (index_ = 0; index_ < parent; index_++) {
					if (object_type[index_] == object_type[parent])
						count++;
				}
				vt_key[2].string = "Parent";
				vt_value.integer = count;
				prop_put(parse_bundle, "I->sis", vt_value, vt_key);
			}
		}

		/* Done with temporary array. */
		sc_free(object_type);
	}

	/* Convert carry limit into version 4.0-like size and weight limits. */
	else if (strcmp(fixup, "|V380_MaxSize_MaxWt_|") == 0) {
		sc_vartype_t vt_key, vt_value;
		sc_int maxcarried;

		vt_key.string = "MaxCarried";
		parse_push_key(vt_key, PROP_KEY_STRING);
		maxcarried = parse_get_integer_property();
		parse_pop_key();

		vt_value.integer = maxcarried * V380_OBJ_CAPACITY_MULT
		                   + V380_OBJ_DEFAULT_SIZE;

		vt_key.string = "MaxSize";
		parse_push_key(vt_key, PROP_KEY_STRING);
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();

		vt_key.string = "MaxWt";
		parse_push_key(vt_key, PROP_KEY_STRING);
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	/* Add up positive scoring tasks to arrive at max score. */
	else if (strcmp(fixup, "|V380_GLOBAL:_MaxScore_|") == 0) {
		sc_vartype_t vt_key[3], vt_value;
		sc_int task_count, maxscore, task;

		/* Get a count of tasks. */
		vt_key[0].string = "Tasks";
		task_count = prop_get_child_count(parse_bundle, "I<-s", vt_key);

		/* Sum positive scoring tasks. */
		maxscore = 0;
		for (task = 0; task < task_count; task++) {
			sc_int score;

			vt_key[1].integer = task;
			vt_key[2].string = "Score";
			score = prop_get_integer(parse_bundle, "I<-sis", vt_key);
			if (score > 0)
				maxscore += score;
		}

		/* Write MaxScore global property. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "MaxScore";
		vt_value.integer = maxscore;
		prop_put(parse_bundle, "I->ss", vt_value, vt_key);
	}

	/* Convert walk meetobject from dynamic index to object. */
	else if (strcmp(fixup, "|V380_WALK:_MeetObject_|") == 0) {
		sc_vartype_t vt_key, vt_value, vt_gkey[3];
		sc_int meetobject, count, object_count, object;

		vt_key.string = "MeetObject";
		parse_push_key(vt_key, PROP_KEY_STRING);
		meetobject = parse_get_integer_property();

		/* Get a count of objects. */
		vt_gkey[0].string = "Objects";
		object_count = prop_get_child_count(parse_bundle, "I<-s", vt_gkey);

		/* Convert dynamic index to object, and rewrite. */
		count = meetobject - 1;
		for (object = 0; object < object_count && count >= 0; object++) {
			sc_bool bstatic;

			vt_gkey[1].integer = object;
			vt_gkey[2].string = "Static";
			bstatic = prop_get_boolean(parse_bundle, "B<-sis", vt_gkey);
			if (!bstatic)
				count--;
		}
		object--;

		vt_value.integer = object;
		parse_put_property(vt_value, PROP_INTEGER);
		parse_pop_key();
	}

	/* Convert version 3.8 room data into a version 4.0 alts array. */
	else if (strcmp(fixup, "|V380_ROOM:_Alts_|") == 0) {
		parse_fixup_v390_v380_room_alts();
	}

	/* Error if no fixup special handler available. */
	else {
		sc_fatal("parse_fixup_v380: no handler for \"%s\"\n", fixup);
	}

	if (parse_trace)
		sc_trace("Parse: leaving version 3.8 fixup %s\n", fixup);
}


/*
 * parse_fixup()
 *
 * Handler for fixup special items to help with conversions from TAF version
 * 3.9 and version 3.8 formats into version 4.0.
 */
static void parse_fixup(CONTEXT, const sc_char *fixup) {
	/*
	 * Pick a fixup handler specific to the TAF version.  This helps keep
	 * fixup code separate, rather than glommed into one large function.
	 */
	switch (taf_get_version(parse_taf)) {
	case TAF_VERSION_400:
		sc_fatal("parse_fixup: unexpected call\n");
		break;
	case TAF_VERSION_390:
		CALL1(parse_fixup_v390, fixup);
		break;
	case TAF_VERSION_380:
		parse_fixup_v380(fixup);
		break;
	default:
		sc_fatal("parse_fixup: invalid TAF file version\n");
		break;
	}
}


/*
 * parse_element()
 *
 * Parse a class descriptor element.
 */
static void parse_element(CONTEXT, const sc_char *element) {
	if (parse_trace)
		sc_trace("Parse: entering element %s\n", element);

	/* Determine the element type from the first character. */
	switch (element[0]) {
	case PARSE_ARRAY:
		CALL1(parse_array, element);
		break;
	case PARSE_VECTOR:
		CALL1(parse_vector, element);
		break;
	case PARSE_VECTOR_ALTERNATE:
		CALL1(parse_vector_alternate, element);
		break;
	case PARSE_CLASS:
		CALL1(parse_class, element);
		break;
	case PARSE_EXPRESSION:
		CALL1(parse_expression, element);
		break;
	case PARSE_SPECIAL:
		CALL1(parse_special, element);
		break;
	case PARSE_FIXUP:
		CALL1(parse_fixup, element);
		break;

	case PARSE_INTEGER:
	case PARSE_DEFAULT_ZERO:
	case PARSE_BOOLEAN:
	case PARSE_DEFAULT_TRUE:
	case PARSE_DEFAULT_FALSE:
	case PARSE_STRING:
	case PARSE_DEFAULT_EMPTY:
	case PARSE_IGNORE_INTEGER:
	case PARSE_IGNORE_BOOLEAN:
	case PARSE_IGNORE_STRING:
	case PARSE_MULTILINE:
		CALL1(parse_terminal, element);
		break;
	default:
		sc_fatal("parse_element: bad type, %c\n", element[0]);
	}

	if (parse_trace)
		sc_trace("Parse: leaving element %s\n", element);
}


/*
 * parse_descriptor()
 *
 * Parse a class's properties descriptor list.
 */
static void parse_descriptor(CONTEXT, const sc_char *descriptor) {
	sc_int next;

	/* Find and parse each element in the descriptor. */
	for (next = 0; descriptor[next] != NUL;) {
		sc_char element[PARSE_TEMP_LENGTH];

		/* Isolate the next descriptor element. */
		if (sscanf(descriptor + next, "%[^ ]", element) != 1)
			sc_fatal("parse_element: no element, %s\n", descriptor + next);

		/* Parse this isolated element. */
		CALL1(parse_element, element);

		/* Advance over the element and any trailing whitespace. */
		next += strlen(element);
		next += strspn(descriptor + next, " ");
	}
}


/*
 * parse_class()
 *
 * Parse a class of properties.
 */
static void parse_class(CONTEXT, const sc_char *class_) {
	sc_char class_name[PARSE_TEMP_LENGTH];
	sc_int index_;
	sc_vartype_t vt_key;

	/* Isolate the class name. */
	if (sscanf(class_, "<%[^>]", class_name) != 1)
		sc_fatal("parse_class: error in class, %s\n", class_);
	if (parse_trace)
		sc_trace("Parse: entering class %s\n", class_name);

	/* Find the class in the parse schema, and fail if not found. */
	for (index_ = 0; parse_schema[index_].class_name; index_++) {
		if (strcmp(parse_schema[index_].class_name, class_name) == 0)
			break;
	}
	if (!parse_schema[index_].class_name)
		sc_fatal("parse_class: class not described, %s\n", class_name);

	/*
	 * Unless we are at the top level of the parse schema, push the class tag
	 * as a key.  The top level is "_GAME_", index_ 0, and isn't part of key
	 * formation.
	 */
	if (index_ > 0) {
		vt_key.string = class_ + strlen(class_name) + 2;
		parse_push_key(vt_key, PROP_KEY_STRING);
	}

	/* Parse each element in the descriptor. */
	CALL1(parse_descriptor, parse_schema[index_].descriptor);

	/* Pop a key if the class tag was pushed above. */
	if (index_ > 0)
		parse_pop_key();

	if (parse_trace)
		sc_trace("Parse: leaving class %s\n", class_name);
}


/*
 * parse_add_walkalerts()
 *
 * Add a list of all NPC walks started by each task.  This is post-processing
 * that occurs after the TAF file has been successfully parsed.
 */
static void parse_add_walkalerts(sc_prop_setref_t bundle) {
	sc_vartype_t vt_key[5];
	sc_int npcs_count, npc;

	/* Get the count of NPCs. */
	vt_key[0].string = "NPCs";
	npcs_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Set up each NPC. */
	for (npc = 0; npc < npcs_count; npc++) {
		sc_int walk_count, walk;

		/* Get NPC walk details. */
		vt_key[1].integer = npc;
		vt_key[2].string = "Walks";
		walk_count = prop_get_child_count(bundle, "I<-sis", vt_key);

		for (walk = 0; walk < walk_count; walk++) {
			sc_int starttask;

			/* Get start task of walk. */
			vt_key[3].integer = walk;
			vt_key[4].string = "StartTask";
			starttask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
			if (starttask >= 0) {
				sc_vartype_t vt_key2[4], vt_value;
				sc_int count;

				/* Count existing walkalerts for the task. */
				vt_key2[0].string = "Tasks";
				vt_key2[1].integer = starttask;
				vt_key2[2].string = "NPCWalkAlert";
				count = prop_get_child_count(bundle, "I<-sis", vt_key2);

				/* Add two more -- NPC and walk. */
				vt_key2[3].integer = count;
				vt_value.integer = npc;
				prop_put(bundle, "I->sisi", vt_value, vt_key2);
				vt_key2[3].integer = count + 1;
				vt_value.integer = walk;
				prop_put(bundle, "I->sisi", vt_value, vt_key2);
			}
		}
	}
}


/*
 * parse_add_movetimes()
 *
 * Add a list of move times to all NPC walks.  This is post-processing that
 * occurs after the TAF file has been successfully parsed.
 */
static void parse_add_movetimes(sc_prop_setref_t bundle) {
	sc_vartype_t vt_key[6];
	sc_int npcs_count, npc;

	/* Get the count of NPCs. */
	vt_key[0].string = "NPCs";
	npcs_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Set up each NPC. */
	for (npc = 0; npc < npcs_count; npc++) {
		sc_int walk_count, walk;

		/* Get NPC walk details. */
		vt_key[1].integer = npc;
		vt_key[2].string = "Walks";
		walk_count = prop_get_child_count(bundle, "I<-sis", vt_key);

		for (walk = 0; walk < walk_count; walk++) {
			sc_int waittimes;
			sc_int *movetimes, index_;
			sc_vartype_t vt_value;

			vt_key[3].integer = walk;
			vt_key[4].string = "Times";
			waittimes = prop_get_child_count(bundle, "I<-sisis", vt_key);

			movetimes = (sc_int *)sc_malloc((waittimes + 1) * sizeof(*movetimes));
			memset(movetimes, 0, (waittimes + 1) * sizeof(*movetimes));
			for (index_ = waittimes - 1; index_ >= 0; index_--) {
				vt_key[4].string = "Times";
				vt_key[5].integer = index_;
				movetimes[index_] = prop_get_integer(bundle, "I<-sisisi", vt_key)
				                    + movetimes[index_ + 1];
			}
			movetimes[waittimes] = -2;

			for (index_ = 0; index_ <= waittimes; index_++) {
				vt_key[4].string = "MoveTimes";
				vt_key[5].integer = index_;
				vt_value.integer = movetimes[index_];
				prop_put(bundle, "I->sisisi", vt_value, vt_key);
			}
			sc_free(movetimes);
		}
	}
}


/*
 * parse_add_alrs_index()
 *
 * Sort ALRs by original string length and store an indexer property, so
 * that ALR replacements look at longer strings before shorter ones.
 */
static void parse_add_alrs_index(sc_prop_setref_t bundle) {
	sc_vartype_t vt_key[3];
	sc_int alr_count, index_, alr;
	sc_int *alr_lengths, longest, shortest, length;

	/* Count ALRs, and set invariant part of properties key. */
	vt_key[0].string = "ALRs";
	alr_count = prop_get_child_count(bundle, "I<-s", vt_key);

	/*
	 * Set up an array of the lengths of ALR original strings, and while at it,
	 * get the shortest and longest defined.
	 */
	alr_lengths = (sc_int *)sc_malloc(alr_count * sizeof(*alr_lengths));
	shortest = INTEGER_MAX;
	longest = 0;
	for (index_ = 0; index_ < alr_count; index_++) {
		const sc_char *original;

		vt_key[1].integer = index_;
		vt_key[2].string = "Original";
		original = prop_get_string(bundle, "S<-sis", vt_key);
		length = strlen(original);

		alr_lengths[index_] = length;
		shortest = (length < shortest) ? length : shortest;
		longest = (length > longest) ? length : longest;
	}

	/*
	 * Now write a set of secondary properties that define the order of handling
	 * for ALRs.  Our friend qsort() can't help here as it doesn't define the
	 * final ordering of equal members, and we need here to retain file ordering
	 * for ALR originals of the same length.
	 */
	vt_key[0].string = "ALRs2";
	alr = 0;
	for (length = longest; length >= shortest; length--) {
		/* Find and add each ALR of this length. */
		for (index_ = 0; index_ < alr_count; index_++) {
			if (alr_lengths[index_] == length) {
				sc_vartype_t vt_value;

				vt_key[1].integer = alr++;
				vt_key[2].string = "ALRIndex";
				vt_value.integer = index_;
				prop_put(bundle, "I->sis", vt_value, vt_key);
			}
		}
	}
	assert(alr == alr_count);

	/* Done with ALR lengths array. */
	sc_free(alr_lengths);
}


/*
 * parse_add_resources_offset()
 *
 * Add the resources offset to the properties as an extra game property
 * for version 4.0 games.  For version 3.9 and version 3.8 games, write
 * zero; only version 4.0 games can embed their resources into the TAF file.
 */
static void parse_add_resources_offset(sc_prop_setref_t bundle, sc_tafref_t taf) {
	sc_vartype_t vt_key[2], vt_value;
	sc_bool embedded;
	sc_int offset;

	/*
	 * Get the resources offset from the TAF, or default to zero.  The resources
	 * offset is one byte after the end of game data.
	 */
	vt_key[0].string = "Globals";
	vt_key[1].string = "Embedded";
	embedded = prop_get_boolean(bundle, "B<-ss", vt_key);
	offset = embedded ? taf_get_game_data_length(taf) + 1 : 0;

	/* Add this offset to the properties. */
	vt_key[0].string = "ResourceOffset";
	vt_value.integer = offset;
	prop_put(bundle, "I->s", vt_value, vt_key);
}


/*
 * parse_add_version()
 *
 * Add the TAF version to the properties, both integer and character forms
 * for convenience.
 */
static void parse_add_version(sc_prop_setref_t bundle, sc_tafref_t taf) {
	sc_vartype_t vt_key, vt_value;

	/* Add the version integer to the properties. */
	vt_key.string = "Version";
	vt_value.integer = taf_get_version(taf);
	prop_put(bundle, "I->s", vt_value, &vt_key);

	/* Add the version string to the properties. */
	switch (taf_get_version(taf)) {
	case TAF_VERSION_400:
		vt_value.string = "4.00";
		break;
	case TAF_VERSION_390:
		vt_value.string = "3.90";
		break;
	case TAF_VERSION_380:
		vt_value.string = "3.80";
		break;
	default:
		sc_error("parse_add_version_string: invalid TAF file version\n");
		vt_value.string = "[Unknown version]";
		break;
	}
	vt_key.string = "VersionString";
	prop_put(bundle, "S->s", vt_value, &vt_key);
}


/*
 * parse_game()
 *
 * Parse a game into a set properties.  Return TRUE on success, FALSE if
 * it encountered an error reading the TAF file.
 */
sc_bool parse_game(sc_tafref_t taf, sc_prop_setref_t bundle) {
	assert(taf && bundle);
	Context context;

	/* Store the TAF to read from, and the bundle to store into. */
	parse_taf = taf;
	parse_bundle = bundle;
	parse_schema = parse_select_schema(parse_taf);
	parse_depth = 0;

	// Try parsing a complete game
	taf_first_line(parse_taf);
	parse_tafline = 0;
	parse_class(context, "<_GAME_>");

	if (context._break) {
		// Error with one of the TAF file lines
		parse_clear_v400_resources_table();
		parse_taf = NULL;
		parse_bundle = NULL;
		parse_schema = NULL;
		parse_depth = 0;
		return FALSE;
	}

	/* Free the accumulated version 4.0 resources details. */
	parse_clear_v400_resources_table();

	/* See if we reached the end of the TAF. */
	if (taf_more_lines(parse_taf))
		sc_error("parse_game: unexpected trailing data\n");

	/* Append post-processing walkalerts and move times. */
	parse_add_walkalerts(parse_bundle);
	parse_add_movetimes(parse_bundle);

	/* Append sorted ALR list and resources offset. */
	parse_add_alrs_index(parse_bundle);
	parse_add_resources_offset(parse_bundle, parse_taf);

	/* Add a note of the TAF file version. */
	parse_add_version(parse_bundle, parse_taf);

	/* Trim excess allocations from properties. */
	prop_solidify(parse_bundle);

	/* Return successfully. */
	parse_taf = NULL;
	parse_bundle = NULL;
	parse_schema = NULL;
	parse_depth = 0;
	return TRUE;
}


/*
 * parse_debug_trace()
 *
 * Set parse tracing on/off.
 */
void parse_debug_trace(sc_bool flag) {
	parse_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
