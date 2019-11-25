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

#include "glk/agt/agility.h"

namespace Glk {
namespace AGT {

/*
  This is a mishmash of utilities and preinitialized arrays,
  including the verblist, the metacommand token list,
  and the dictionary routines.
*/

/* ------------------------------------------------------------------- */
/*  Preinitialized data structures                                     */
/*    Most of the preinitialized data structures used by all of the    */
/*    AGT-related programs go here .                                   */
/* ------------------------------------------------------------------- */


/* ------------------------------------------------------------ */
/* The PC --> ASCII conversion table. This converts the 8th-bit */
/*   PC characters to their nearest ASCII equivalent.            */
/* ------------------------------------------------------------ */

const char trans_ibm[] =
    "CueaaaaceeeiiiAA"   /* 80 */
    "E@@ooouuyOUc$$pf"   /* 90 */
    "aiounNao?....!<>"   /* A0 */
    "###|++|+++|\\/++\\"   /* B0 */
    "\\+++-+||\\/+++=+="   /* C0 */
    "+=+++++++//@@@@@"   /* D0 */
    "abGpSsmtFTOd.fe^"   /* E0 */
    "=+><fj/=***/n2# ";  /* F0 */


/* ------------------------------------------------------------- */
/* Tables of built in properties and attributes */
/* ------------------------------------------------------------- */

#define rnc(p) {#p,offsetof(room_rec,p),offsetof(noun_rec,p), \
		offsetof(creat_rec,p)}
#define rn(p) {#p,offsetof(room_rec,p),offsetof(noun_rec,p),-1}
#define nc(p) {#p,-1,offsetof(noun_rec,p),offsetof(creat_rec,p)}
#define rc(p) {#p,offsetof(room_rec,p),-1,offsetof(creat_rec,p)}
#define r(p) {#p, offsetof(room_rec,p), -1, -1}
#define n(p) {#p, -1, offsetof(noun_rec,p),-1}
#define c(p) {#p, -1, -1, offsetof(creat_rec,p)}

const prop_struct proplist[NUM_PROP] = {
	/* The following are writable */
	rnc(oclass), rnc(points), r(light),
	n(num_shots), n(weight), n(size),
	c(counter), c(timecounter),
	/* The remaining properties are read-only */
	nc(location), rn(key), c(weapon), c(threshold), c(timethresh),
	nc(flagnum)
};

const prop_struct attrlist[NUM_ATTR] = {
	/* The following are writable */
	n(on), n(open), n(locked), n(movable),
	c(groupmemb), c(hostile),
	/* The remaining attributes are read-only */
	r(end), rn(win), r(killplayer), n(plural),
	n(pushable), n(pullable), n(turnable), n(playable), n(readable), n(closable),
	n(lockable), n(edible), n(wearable), n(drinkable), n(poisonous), n(light),
	n(shootable), nc(isglobal),
	/* This is writable again */
	rnc(seen),
	nc(proper) /* This is not writable */
};

#undef rnc
#undef rn
#undef rc
#undef cn
#undef r
#undef c
#undef n



/* ------------------------------------------------------------- */
/*  Tables of Opcodes                                            */
/*   These gives the names and argument types of all of the AGT  */
/*   opcodes.                                                    */
/* ------------------------------------------------------------- */


/* All of the following are undefined again just after the table */

#define n AGT_NUM
#define v AGT_VAR
#define r AGT_ROOM
#define i AGT_ITEM

#define o (AGT_ITEM|AGT_CREAT)  /* "object" */
#define l (r|o|AGT_NONE|AGT_SELF|AGT_WORN)   /* "location" */



/* opcode, argnum, arg1, arg2 */
#ifdef LOWMEM
#define a(s)  {"",0,0,0}
#define b(s,a1) {"",1,(a1),0}
#define c(s,a1,a2) {"",2,(a1),(a2)}
#else
#define a(s)  {#s,0,0,0}
#define b(s,a1) {#s,1,(a1),0}
#define c(s,a1,a2) {#s,2,(a1),(a2)}
#endif

const opdef cond_def[] = {
	b(AtLocation, r), b(AtLocationGT, n), b(AtLocationLT, n),
	a(SongPlaying), a(SoundIsOn), a(DirectionOK), b(DirectionIs, AGT_DIR),
	c(BetweenRooms, n, n), b(HasVisitedRoom, r),
	a(EnteredObject), b(TimeGT, n), b(TimeLT, n),
	a(FirstVisitToRoom),
	a(NewLife),
	a(IsCarryingSomething), a(IsCarryingNothing),
	a(IsWearingSomething),
	b(IsCarryingTreasure, n),
	a(IsWearingNothing),
	b(LoadWeightEquals, n), b(LoadWeightGT, n), b(LoadWeightLT, n),
	b(Present, o), b(IsWearing, o), b(IsCarrying, o),
	b(IsNowhere, o), b(IsSomewhere, o),
	b(InRoom, o), c(IsLocated, o, l), c(Together, o, o),
	b(IsON, o), b(IsOFF, o),
	b(IsGroupMember, AGT_CREAT),
	b(IsOpen, o), b(IsClosed, o), b(IsLocked, o), b(IsUnLocked, o),
	b(IsEdible, o), b(IsDrinkable, o), b(IsPoisonous, o),
	b(IsMovable, o),
	a(NOUNPresent), a(NOUNIsWearing), a(NOUNIsCarrying),
	a(NOUNIsNowhere), a(NOUNIsSomewhere),
	a(NOUNInRoom), b(NOUNIsLocated, l),
	a(NOUNIsOn), a(NOUNIsOff),
	a(NOUNIsOpen), a(NOUNIsClosed), a(NOUNIsLocked), a(NOUNIsUnLocked),
	a(NOUNIsEdible), a(NOUNIsDrinkable), a(NOUNIsPoisonous),
	a(NOUNIsMovable),
	b(NOUNpointsEquals, n), b(NOUNpointsGT, n), b(NOUNpointsLT, n),
	b(NOUNweightEquals, n), b(NOUNweightGT, n), b(NOUNweightLT, n),
	a(LightPresent), a(RoomNeedsLight),
	b(FlagON, AGT_FLAG), b(FlagOFF, AGT_FLAG),
	b(RoomFlagOn, AGT_ROOMFLAG), b(Room_PixHere, AGT_PIX),
	b(RoomFlagOff, AGT_ROOMFLAG),
	b(ScoreEquals, n), b(ScoreGT, n), b(ScoreLT, n),
	b(NumberEquals, n), b(NumberGT, n), b(NumberLT, n),
	a(AnswerIsCorrect), a(AnswerIsWrong),
	b(TurnsEquals, n), b(TurnsGT, n), b(TurnsLT, n),
	c(CounterEquals, AGT_CNT, n), c(CounterGT, AGT_CNT, n), c(CounterLT, AGT_CNT, n),
	c(VariableEquals, v | n, n), c(VariableGT, v | n, n), c(VariableLT, v | n, n),
	c(CompareVariables, v, v), c(VariableChance, v | n, n),
	a(NamePresent), b(NameIsNumber, o | AGT_NONE),  /* QQ:Not sure about these */
	b(NOUNIsNumber, o | AGT_NONE), b(ObjectIsNumber, o | AGT_NONE),
	b(SomethingInside, r | o | AGT_SELF),
	b(Chance, n),
	a(PromptForYES), a(PromptForNO),
	a(VerbIsDirection),
	a(NOUNIsCreature),
	a(NOUNIsMan), a(NOUNIsWoman), a(NOUNIsThing),
	a(OBJECTIsMan), a(OBJECTIsWoman), a(OBJECTIsThing),
	a(ObjectIsCreature),
	a(ObjectPresent),
	a(NOT), a(OR),
	a(BeforeCommand), a(AfterCommand),    /* 110,111 */
	b(HourEquals, n), b(HourGT, n), b(HourLT, n),
	b(MinuteEq, n), b(MinuteGT, n), b(MinuteLT, n),
	a(IsAM),

	a(OnDisambig),
	b(IsHostile, o), a(HostilePresent),
	a(NameWasPresent), a(OncePerTurn),
	c(IsClass, r | o, AGT_NONE | r | o),
	c(AttrOn, r | o, AGT_ATTR),
	a(NumericNOUN), a(NumericOBJECT),
	c(Equal, n, n), c(GT, n, n), c(LT, n, n), c(GE, n, n), c(LE, n, n),
	c(CaseCompareStrings, AGT_STR, AGT_STR), c(CaseStringBefore, AGT_STR, AGT_STR),
	c(CaseStringAfter, AGT_STR, AGT_STR),
	c(CompareStrings, AGT_STR, AGT_STR), c(StringBefore, AGT_STR, AGT_STR),
	c(StringAfter, AGT_STR, AGT_STR),
	c(StringIsAnswer, AGT_STR, AGT_QUEST),
	b(HasSeen, r | o),
	c(ObjFlagON, r | o, AGT_OBJFLAG),
	c(ObjFlagOFF, r | o, AGT_OBJFLAG),
	c(CanGo, r | o | AGT_SELF, AGT_DIR)
};


const opdef act_def[] = {
	b(GoToRoom, r), c(GoToRandomRoom, r, r),
	b(MakeVarRoomNum, v), b(MakeVarNounNum, v), b(MakeVarObjectNum, v),
	b(GoToVariableRoom, v | r), c(SendToVariableRoom, o, v | l),
	b(GetVariableIt, v | o), b(PrintVariableMessage, v | AGT_MSG),
	b(GetIt, o), b(WearIt, o), b(DropIt, o), b(RemoveIt, o),
	b(LoadFont, AGT_FONT), b(ShowPicture, AGT_PIC), c(ChangePicture, AGT_PIC, AGT_PIC),
	b(IfYShowPicture, AGT_PIC),
	b(ShowRoom_Pix, AGT_PIX), b(IfYShowRoom_Pix, AGT_PIX),
	b(PlaySong, AGT_SONG), c(PlayRandom, n, n), b(RepeatSong, AGT_SONG),
	a(EndRepeatSong), a(StopSong), a(SuspendSong), a(ResumeSong),
	b(ToggleMovable, i), c(ChangeDescr, r | o, AGT_MSG), c(ChangePoints, r | o, n),
	a(DestroyOBJECT), b(GetString, AGT_STR),
	b(GetVariable, v), b(SetVariableToTime, v), b(SetTimeToVariable, v | n),
	b(SetTime, n), b(AddToTime, n), b(SetDeltaTime, n),
	b(DoSubroutine, AGT_SUB), a(Return),
	a(GetNOUN), a(WearNOUN), a(DropNOUN), a(RemoveNOUN),
	a(DropEverything), a(RemoveEverything), a(KillPlayer),
	b(PutInCurrentRoom, o), c(SendToRoom, o, l),
	c(RePosition, o, l),
	a(PutNOUNInCurrentRoom), b(SendNOUNToRoom, l),
	b(SendAllToRoom, l), c(SendTreasuresToRoom, l, n),
	c(RelocateAll, l, l),
	b(Destroy, o), a(DestroyNOUN),
	c(SwapLocations, o, o), c(SendToItem, o, o), b(SendNOUNtoItem, o),
	b(AddToGroup, AGT_CREAT), b(RemoveFromGroup, AGT_CREAT), b(MoveTheGroup, l),
	a(RedirectTo),
	c(RandomMessage, AGT_MSG, AGT_MSG), b(ShowContents, r | o | AGT_SELF | AGT_WORN),
	b(OpenIt, i), b(CloseIt, i), b(LockIt, i), b(UnlockIt, i),
	a(OpenNOUN), a(CloseNOUN), a(LockNOUN), a(UnlockNOUN),
	a(ShowScore), b(PlusScore, n), b(MinusScore, n),
	a(ShowInventory), a(WaitForReturn), a(TimePasses),
	b(Delay, n),
	a(ClearScreen),
	b(DescribeThing, r | o), a(LookAtRoom),
	b(PrintMessage, AGT_MSG), a(BlankLine), c(Tone, n, n),
	c(GetNumberInput, n, n), b(AskQuestion, AGT_QUEST),
	c(ChangePassageway, AGT_DIR, AGT_EXIT),
	b(TurnFlagOn, AGT_FLAG), b(TurnFlagOff, AGT_FLAG), b(ToggleFlag, AGT_FLAG),
	b(TurnRoomFlagOn, AGT_ROOMFLAG), b(TurnRoomFlagOff, AGT_ROOMFLAG),
	b(ToggleRoomFlag, AGT_ROOMFLAG),
	b(TurnCounterOn, AGT_CNT), b(TurnCounterOff, AGT_CNT),
	c(SetVariableTo, v, n), c(AddToVariable, v | n, n), c(SubtractFromVariable, v | n, n),
	c(AddVariables, v, v), c(SubtractVariables, v, v),
	c(RandomVariable, v, n),
	b(NounToVariable, v), b(ObjectToVariable, v),
	b(Quote, AGT_MSG),
	b(TimePlus, n), b(TimeMinus, n), b(SetHour, n), b(SetMinute, n),
	b(TimePlusVariable, v | n), b(TimeMinusVariable, v | n),
	b(SetHourToVariable, v | n), b(SetMinutesToVariable, v | n),

	b(SubtractFromTime, n), b(SetDisambigPriority, n),
	b(SetVariableToDeltaTime, v), b(ChangeStatus, n),
	c(MultiplyVariable, v | n, n), c(DivideVariable, v | n, n),
	c(ComputeRemainder, v | n, n),
	a(WaitForKey),
	b(SetHE, o), b(SetSHE, o), b(SetIT, o), b(SetTHEY, o),
	b(PrintMessageNoNL, AGT_MSG),
	b(StandardMessage, AGT_ERR),
	b(FailMessage, AGT_MSG), b(FailStdMessage, AGT_ERR),
	c(ErrMessage, n, AGT_MSG),  c(ErrStdMessage, n, AGT_ERR),
	a(AND),
	c(SetClass, r | o, AGT_NONE | r | o),
	c(SetVariableToClass, v, r | o),
	b(PushStack, n), b(PopStack, v),
	a(AddStack), a(SubStack), a(MultStack), a(DivStack), a(ModStack),
	a(DupStack), a(DiscardStack),
	b(SetVariableToInput, v),
	c(TurnAttrOn, r | o, AGT_ATTR), c(TurnAttrOff, r | o, AGT_ATTR),
	c(PushProp, r | o, AGT_PROP), c(PopProp, r | o, AGT_PROP),
	b(Goto, n), b(OnFailGoto, n),
	b(EndDisambig, n),
	b(XRedirect, n),
	c(CopyString, AGT_STR, AGT_STR),
	b(UpcaseString, AGT_STR), b(DowncaseString, AGT_STR),
	c(TurnObjFlagON, r | o, AGT_OBJFLAG), c(TurnObjFlagOFF, r | o, AGT_OBJFLAG),
	c(ToggleObjFlag, r | o, AGT_OBJFLAG),
	c(PushObjProp, r | o, AGT_OBJPROP),
	c(PopObjProp, r | o, AGT_OBJPROP),
	c(MoveInDirection, o | AGT_SELF, AGT_DIR)
};

const opdef end_def[] = {
	a(WinGame), a(EndGame),
	a(QuitThisCMD), a(QuitAllCMDs), a(DoneWithTurn)
};

const opdef illegal_def = a(ILLEGAL);

#undef a
#undef b
#undef c

#undef n
#undef v
#undef r
#undef i
#undef o
#undef l




/* ------------------------------------------------------------- */
/*  Opcode Translation Tables                                    */
/*    These convert opcode numbers from the various AGT versions */
/*    to a uniform coding.                                       */
/* ------------------------------------------------------------- */

/*NOTE this is being changed so that rather than the second term
  is an absolute offset of the first term. Still applies to ranges
  up until next one. Also incorporates the +1000 correction
  into the correction set itself. (to avoid further problems
  when including more opcodes, e.g. AGT 1.83).
   The last table entry is now marked by a new value of -1.*/

/* Versions of the command set:
    v1.21 apparantly has a compatible command set w/ 1.7 (!)
      [except that their maxcmd is apparantly 22, not 30]
    1.0 doesn't; it seems to have an EOC code of 154, as opposed to
    165 or so.
    1.18 seems to be slightly different from 1.7, but seemingly only
       by one opcode.
    [And of course both ME and 1.8 have their own extended command sets]
*/

static const cmd_fix_rec FIX_ME[] =  /* No longer using this as baseline */
{	{0, 0},
	{110, 1000}, /* i.e. commands moved to start at opcode 1000 */
	{215, WIN_ACT},
	{220, -1}
};

static const cmd_fix_rec FIX_ME0[] =
    /* 169 */
{	{0, 0},
	{110, 1000},
	{136, 1028}, /* Skip ToggleMoveable and ChangeDescr */
	{156, 1049}, /* Skip RePosition */
	{212, WIN_ACT},
	{217, -1}
};

static const cmd_fix_rec FIX_ME0A[] = /* Pre-ME/1.0: */
    /* 169 */
{	{0, 0},
	{110, 1000},
	{130, 1021}, /* Skip PlayRandom */
	{135, 1028}, /* Skip ToggleMoveable and ChangeDescr */
	{155, 1049}, /* Skip RePosition */
	{211, WIN_ACT},
	{216, -1}
};

static const cmd_fix_rec FIX_ME15[] = {
	{0, 0},
	{110, 1000}, /* i.e. commands moved to start at opcode 1000 */
	{158, 1049},  /* Skip the one opcode added in 1.56: RePosition */
	{214, WIN_ACT},
	{219, -1}
};

static const cmd_fix_rec FIX_135[] = {
	{0, 0},
	{3, 12},
	{59, 71},
	{88, 106},
	{92, 1000},
	{105, 1039}, /* 149 */
	{114, 1049}, /* 159 */
	{157, 1095}, /* 205 */
	{167, WIN_ACT},
	{172, -1}
};

static const cmd_fix_rec FIX_118[] = {
	{0, 0},
	{3, 12},
	{59, 71},
	{88, 106},
	{92, 1000},
	{105, 1039}, /* 149 */
	{114, 1049}, /* 159 */
	{118, 1054}, /* Skip SendTreasuresToRoom */
	{156, 1095}, /* 205 */
	{166, WIN_ACT},
	{171, -1}
};


static const cmd_fix_rec FIX_182[] = {
	{0, 0},
	{3, 12},
	{53, 110}, /* Shift BeforeCmd and AfterCmd */
	{55, 62},
	{61, 71},
	{90, 106},
	{94, 1000},
	{107, 1039}, /* 149 */
	{116, 1049}, /* 159 */
	{143, 1105},  /* QUOTE-- need to move somewhere else */
	{144, 1076},
	{160, 1095}, /* 205 */
	{170, WIN_ACT},
	{175, -1}
};


static const cmd_fix_rec FIX_183[] = {
	{0, 0},
	{3, 12},
	{55, 110}, /* Shift BeforeCmd and AfterCmd */
	{57, 64},
	{61, 71},
	{90, 106},
	{94, 112},  /* Time condition tokens */
	{101, 1000},
	{114, 1039},
	{123, 1049},
	{158, 1105},  /* QUOTE-- need to move somewhere else */
	{159, 1084},
	{167, 1095},
	{169, 1106},    /* Time Action Tokens */
	{177, 1097},
	{185, WIN_ACT},
	{190, -1}
};

static const cmd_fix_rec FIX_10[] = /* This *seems* to work */
{	{0, 0},
	{3, 12},
	{59, 71},
	{80, 95},
	{84, 108},
	{86, 1000},
	{88, 1009},
	{92, 1039},
	{101, 1049},
	{105, 1054},
	{115, 1065},
	{142, 1095},
	{152, WIN_ACT},
	{157, -1}
};

static const cmd_fix_rec FIX_15[] =   /* This works */
{	{0, 0},
	{3, 12},    /* Skip 3-11 */
	{60, 70},   /* Skip 69 */
	/* {61,72}, */   /* Skip 71 -- WRONG! */
	{90, 106},  /* Skip 101-105 */
	{94, 1000},
	{107, 1039}, /* skip 1013-1038 */
	{116, 1049}, /* Skip 1048  */
	{172, WIN_ACT},
	{177, -1}
};

const fix_array FIX_LIST[] = /* An array of arrays, indexed by aver */
{
	FIX_135, /* Aver=0: unknown format, might as well assume Classic */
	FIX_10, FIX_118, FIX_135, FIX_135, FIX_135, FIX_182, FIX_183,
	FIX_15, FIX_15, FIX_15, FIX_ME0, FIX_ME0A, FIX_ME15, FIX_ME15, FIX_ME
};


/* ------------------------------------------------------------- */
/* Miscellaneous collections of strings                          */
/* ------------------------------------------------------------- */

const char *verstr[] = {"????", "SMALL", "BIG", "MASTER", "SOGGY"};
const char *averstr[] = {"????", "1.0", "1.18",
                         "1.2", "1.32/COS", "Classic",
                         "1.82", "1.83",
                         "1.5/H", "1.5/F", "1.6",
                         "ME/1.0b", "ME/1.0a",
                         "ME/1.5", "ME/1.55", "ME/1.6",
                         "Magx"
                        };

const char *portstr = PORTSTR;
const char *version_str = "version 1.1.1";

const char nonestr[5] = {4, 'n', 'o', 'n', 'e'};
static const char NONEstr[5] = {4, 'N', 'O', 'N', 'E'};


/* Names of exits */
const char *exitname[13] =
{"N", "S", "E", "W", "NE", "NW", "SE", "SW", "U", "D", "IN", "OUT", "SPC"};




/* ------------------------------------------------------------- */
/* Verblist is the  array of canonical forms of all the verbs    */
/* ------------------------------------------------------------- */
/* The following long string defines all the built in AGT verbs, in the
   following format:
  verb syn syn syn , prep prep ; next_verb ....
  except that if a verb takes no objects at all, it should be period
  terminated and if it is a metaverb it should be terminated by '!'. */
static const char verbdef[] =
    "north n. south s. east e. west w."
    "northeast ne. northwest nw. southeast se. southwest sw."
    "up u. down d."
    "enter in inside go&in go&into go&in&to get&in get&into get&in&to."
    "exit leave out go&out get&out get&out&of. special."
    "throw cast dump, at to in into across inside;"
    "open , with; close shut; lock, with; unlock, with;"
    "look l. examine x ex check inspect look&at look&in;"
    "change_locations change_location;"
    "read; eat; drink; score! attack kill fight hit, with;"
    "wait z. yell shout scream."
    "put place, in with inside into near behind over under on;"
    "quit q! tell talk talk&to talk&with, to about;"
    "inventory inv i. get take pick pick&up; ask, about for;"
    "turn, on off; push touch press, with; pull; play;"
    "list. show, to; drop;"
    "listexit listexits list_exits list&exits show&exits."
    "brief! verbose! save! restore!"
    "light; extinguish ext put&out; fire shoot, at with;"
    "help h. wear put&on; remove take&off;"
    "script script&on! unscript script&off! magic_word. view; after."
    "instructions ins!"   /* INSTRUCTIONS is "1.83 only" */
    /* The following are not defined in the original AGT */
    "again g. restart! oops; undo. notify!"
    "listexit_on listexit&on listexits&on!"
    "listexit_off listexit&off listexits&off!"
    "agildebug agtdebug! log! logoff log&off log&close! replay!"
    "replay_step replay&step! menu! replay_fast replay&fast."
    "sound sound_on sound&on! sound_off sound&off! introduction intro!"
    "dir_addr.";

/* 1.83: Removes listexit; adds instructions after remove. */

/* Then come the dummy verbs */
/* Dummy verb n ==> n-55     105,122
    Dummy_verb1...Dummy_Verb50        */

/* Possible extension to verb definitons (not implemented):
   If it _requires_ a prep, use : ?
   If it takes a prep and no dobj, use | ?
*/

/* These are alternative (that is, non-canonical) forms of verbs that
   were present in the oringal AGT interpreters.  They have the property
   that they have no effect if used in a dummy_verb declaration. */
/* Their dictionary indices are stored in old_agt_verb, which is
   initialized by reinit_dict. */
/* PICK, GO */
const char *const old_agt_verb_str[] = {
	"n", "s", "e", "w", "ne", "nw", "se", "sw", "u", "d", "in", "inside", "leave",
	"cast", "dump", "shut", "l", "ex", "inspect", "check", "kill", "fight", "hit",
	"shout", "scream", "place", "q", "talk", "i", "take", "touch", "ext",
	"shoot", "h", "ins", nullptr
};



/* ------------------------------------------------------------------- */
/* Dictionary primitives: the basic functions for manipulating the     */
/* dictionary data structures.                                         */
/* ------------------------------------------------------------------- */
#define HASHSIZE (1<<HASHBITS)
#define HASHMASK (HASHSIZE-1)

#ifdef DOHASH
static word DOSFARDATA hash[HASHSIZE];
#endif

static int hashfunc(const char *s) {
	unsigned long n, i;

	n = 0;
	for (; *s != 0; s++) {
		n += (n << 2) + (uchar) * s;
		i = n & ~HASHMASK;
		if (i)
			n = (n ^ (i >> HASHBITS))&HASHMASK;
	}
	return (n & HASHMASK);
}

static word search0_dict(const char *s) {
	int i;

#ifdef DOHASH
	for (i = hashfunc(s);
	        hash[i] != -1 && strcmp(s, dict[hash[i]]) != 0;
	        i = (i + 1)&HASHMASK);
	return hash[i];
#else
	for (i = 0; strcmp(s, dict[i]) != 0 && i < dp; i++);
	if (i < dp) return i;
	return -1;
#endif
}

word search_dict(const char *s)
/* This does a case-insensitive search */
{
	word w;
	char *t, *p;

	t = rstrdup(s);
	for (p = t; *p; p++) *p = tolower(*p);
	w = search0_dict(t);
	rfree(t);
	return w;
}

/* The basic routine to add s to the dictionary; this does no preprocessing
  of s; use add_dict for that */
static word add0_dict(const char *s) {
	int i;
	long newptr;
	char *newstr;

	i = search0_dict(s);
	if (i != -1) return i;
	/* Okay, it's not in the dictionary; need to add it. */
	/*  rprintf("Adding %s\n",s);*/

	dict = (char **)rrealloc(dict, sizeof(char *) * (dp + 1));
	newptr = dictstrptr + strlen(s) + 1;
	if (newptr > dictstrsize) { /* Enlarge dictstr */
		if (dictstrsize == 0) dictstrsize = DICT_INIT;
		while (newptr > dictstrsize)
			dictstrsize += DICT_GRAN;
		newstr = (char *)rrealloc(dictstr, dictstrsize);
		/* Now need to update all of our pointers */
		for (i = 0; i < dp; i++)
			dict[i] = (dict[i] - dictstr) + newstr;
		dictstr = newstr;
	}
	strcpy(dictstr + dictstrptr, s); /* Copy word into memory */
	dict[dp] = dictstr + dictstrptr;
	dictstrptr = newptr;

#ifdef DOHASH  /* Need to update the hash table */
	if (dp > HASHSIZE) fatal("Hash table overflow");
	for (i = hashfunc(s); hash[i] != -1; i = (i + 1)&HASHMASK);
	hash[i] = dp;
#endif
	return dp++;
}

#ifdef DOHASH

static void init_hash(void) {
	int i;

	for (i = 0; i < HASHSIZE; i++) hash[i] = -1;
}


/* This routine rebuilds the hash table from the dictionary. */
/* It's used by the AGX reading routines, since they save */
/* the dictionary but not the hash table */
static void rebuild_hash(void) {
	int i, j;

	if (dp > HASHSIZE) fatal("Hash table overflow");
	init_hash();

	for (i = 0; i < dp; i++) {
		for (j = hashfunc(dict[i]); hash[j] != -1; j = (j + 1)&HASHMASK);
		hash[j] = i;
	}
}
#endif


static void init0_dict(void)
/* This sets up the basic data structures associated with the dictionary */
/* (It's called by init_dict, which also adds the basic verbs) */
{
#ifdef DOHASH
	init_hash();
	hash[hashfunc("any")] = 0;
#endif

	dict = (char **)rmalloc(sizeof(char *));
	dictstr = (char *)rmalloc(DICT_GRAN);
	strcpy(dictstr, "any");
	dict[0] = dictstr;

	dictstrptr = 4; /* Point just after 'any' */
	dictstrsize = DICT_GRAN;
	dp = 1;
	syntbl = NULL;
	synptr = 0;
	syntbl_size = 0; /* Clear synonym table */
}




/* ------------------------------------------------------------------- */
/* Higher level dictionary routines: Things that load initial vocab,   */
/* and massage strings into the correct form for the dictionary        */
/* ------------------------------------------------------------------- */

static rbool no_syn;

/* This splits dict[w] into space-separated pieces and adds them to
   the dictionary and to a growing synonym list, which it marks the end of.
   It returns a pointer to the beginning of this list.
   If there are no spaces, it doesn't do anything and returns 0. */
slist add_multi_word(word w) {
	slist start_list;
	rbool end_found;
	char *curr;
	char *s, *t;

	for (s = dict[w]; *s != 0 && *s != ' '; s++);
	if (*s != ' ') return 0;

	start_list = synptr;
	curr = t = rstrdup(dict[w]);
	s = t + (s - dict[w]);

	addsyn(w); /* First entry is the 'word' to condense to */
	while (1) {
		end_found = (*s == 0);
		*s = 0;
		addsyn(add0_dict(curr)); /* Add to comb list */
		if (end_found) break;
		curr = ++s;
		while (*s != 0 && *s != ' ') s++;
	}
	addsyn(-1); /* Mark the end of the list */
	rfree(t);
	return start_list;
}


/* Check verb vp for multiwords and enter any found in the auxilary
   combination list */
static void verb_multiword(int vp) {
	int i;
	slist ptr;

	if (no_syn) return;
	for (i = auxsyn[vp]; syntbl[i] != 0; i++) {
		ptr = add_multi_word(syntbl[i]);
		if (ptr != 0) {
			num_auxcomb += 1;
			auxcomb = (slist *)rrealloc(auxcomb, num_auxcomb * sizeof(slist));
			auxcomb[num_auxcomb - 1] = ptr;
		}
	}
}


static void enter_verbs(int vp, const char *s)
/* Read definition string s, starting to make entries at verb # vp */
/* WARNING: This doesn't do any sort of checking; it assumes the input
   string is correctly formed. */
{
	const char *p; /* Points along string. */
	words curr;  /* word currently being read. */
	int n; /* length of curr */
	rbool have_multiword;

	n = 0;
	have_multiword = 0;
	auxsyn[vp] = synptr;
	for (p = s; *p != 0; p++)
		if (*p == ';' || *p == ',' || *p == '.' || *p == '!' || isspace(*p)) {
			if (n > 0) { /* word just ended: need to add it to dictionary etc */
				curr[n] = 0;
				n = 0;
				addsyn(add0_dict(curr)); /* Add to syn list or prep list, depending */
			}
			if (!isspace(*p))
				addsyn(-1); /* Mark the end of the list */
			if (*p == ';' || *p == '.' || *p == '!') {
				if (*p == ';') verbflag[vp] |= VERB_TAKEOBJ;
				if (*p == '!') verbflag[vp] |= VERB_META;
				if (have_multiword)
					verb_multiword(vp);
				have_multiword = 0;
				vp++;
				if (vp >= TOTAL_VERB) break;
				auxsyn[vp] = synptr; /* The following words will be the syn list */
			} else if (*p == ',')
				preplist[vp] = synptr; /* The following words will be the prep list */
		} else if (*p == '&') {
			curr[n++] = ' ';
			have_multiword = 1;
		} else curr[n++] = *p;
}




void init_dict(void) {
	dict = NULL;
	verblist = NULL;
	syntbl = NULL;
	no_syn = 0;
	auxsyn = NULL;
	preplist = NULL;
	verbflag = NULL;
	auxcomb = NULL;
	old_agt_verb = NULL;
	num_auxcomb = 0;
}

/* This is called by agttest.c */
void build_verblist(void) {
	int i;

	verblist = (words *)rmalloc(sizeof(words) * TOTAL_VERB);
	for (i = 0; i < TOTAL_VERB; i++)
		strncpy(verblist[i], dict[syntbl[auxsyn[i]]], sizeof(words));
#ifdef DUMP_VLIST
	{
		int j;
		rprintf("VERB LIST:\n");
		for (i = 0; i < TOTAL_VERB; i++) {
			rprintf("%2d %s:", i, verblist[i]);
			for (j = auxsyn[i]; syntbl[j] != 0; j++)
				rprintf(" %s", dict[syntbl[auxsyn[i]]]);
			rprintf(" ==> ");
			for (j = preplist[i]; syntbl[j] != 0; j++)
				rprintf(" %s", dict[ syntbl[preplist[i]]]);
			writeln("");
		}
	}
#endif
}



void set_verbflag(void) {
	verbflag[14] |= VERB_MULTI; /* throw */
	verbflag[29] |= VERB_MULTI; /* put */
	verbflag[33] |= VERB_MULTI; /* get */
	verbflag[41] |= VERB_MULTI; /* drop */
	verbflag[51] |= VERB_MULTI; /* wear */
	verbflag[52] |= VERB_MULTI; /* remove */
}


void reinit_dict(void)
/* reinit_dict initializes verblist and sets up aux_syn as well
   as loading the initial vocabulary into the dictionary. */
{
	char buff[16]; /* Needs to be big enough to hold dummy_verbNNN\0
            or subroutineNNN\0 */
	int i;

	no_syn = no_auxsyn;

	auxsyn = (slist *)rmalloc(sizeof(slist) * TOTAL_VERB);
	auxcomb = NULL;
	num_auxcomb = 0;
	preplist = (slist *)rmalloc(sizeof(slist) * TOTAL_VERB);
	verbflag = (uchar *)rmalloc(sizeof(uchar) * TOTAL_VERB);

	if (!agx_file)
		init0_dict();
#ifdef DOHASH
	else
		rebuild_hash();
#endif

	for (i = 0; i < TOTAL_VERB; i++)
		verbflag[i] = 0;

	auxsyn[0] = synptr;
	addsyn(-1);

	enter_verbs(1, verbdef);
	set_verbflag(); /* Do additional verbflag initialization */

	for (i = 0; i < DVERB; i++) {
		sprintf(buff, "dummy_verb%d", i + 1);
		auxsyn[i + BASE_VERB] = synptr;
		addsyn(add0_dict(buff));
		addsyn(-1);
	}
	for (i = 0; i < MAX_SUB; i++) {
		sprintf(buff, "subroutine%d", i + 1);
		auxsyn[i + BASE_VERB + DVERB] = synptr;
		addsyn(sub_name[i] = add0_dict(buff));
		addsyn(-1);
	}
	no_syn = 0; /* Return to usual state */
	verblist = NULL;

	/* Now initialize old_agt_verb array */
	for (i = 0; old_agt_verb_str[i] != NULL; i++);
	rfree(old_agt_verb);
	old_agt_verb = (word *)rmalloc(sizeof(word) * (i + 1));
	for (i = 0; old_agt_verb_str[i] != NULL; i++) {
		old_agt_verb[i] = search_dict(old_agt_verb_str[i]);
		assert(old_agt_verb[i] != -1);
	}
	old_agt_verb[i] = -1; /* Mark end of list */
}




void free_dict(void) {
	rfree(dict);
	rfree(verblist);
	rfree(syntbl);
	rfree(auxsyn);
	rfree(preplist);
	rfree(verbflag);
}

word add_dict(const char *str) {
	int i, j;
	char s[50];

	strncpy(s, str, 48);
	for (i = 0; s[i] != 0 && rspace(s[i]); i++);
	if (s[i] == 0) return 0; /* If it's all whitespace, ignore. */
	/* i now points at first non-whitespace character */
	/* Eliminate leading whitespace and lowercase the string. */
	for (j = 0; s[j + i] != 0; j++) s[j] = tolower(s[j + i]);
	s[j] = 0;
	/* Now eliminate trailing whitespace (j points to end of string) */
	for (j--; rspace(s[j]) && j > 0; j--);
	s[j + 1] = 0;
	/* Okay, now make sure it isn't 'none' */
	if (strcmp(s, "none") == 0) return 0;
	/* Finally, add it to the dictionary if it isn't already there */
	return add0_dict(s);
}

/* Adds w to dynamically grown synonym list */
/* If no_syn is set, then *don't* add a synonym: return immediatly */
/*  (This is done by agt2agx to avoid creating the auxsyn lists,  */
/*   since those should be created when the interpreter loads the */
/*   game file and not before) */
void addsyn(word w) {
	if (no_syn) return;
	if (w == 0) return;
	if (w == -1) w = 0;
	if (synptr >= syntbl_size) {
		syntbl_size += SYN_GRAIN;
		if (syntbl_size > 0x7FFF)
			fatal("Too many synonyms.");
		syntbl = (word *)rrealloc(syntbl, ((long)syntbl_size) * sizeof(word));
	}
	syntbl[synptr++] = w;
}


/* Returns the given dictionary word with some checking for -1 */
const char *gdict(word w) {
	assert(w >= -1 && w < dp);
	if (w == -1) return "___"; /* NONE */
	return dict[w];
}





/* ------------------------------------------------------------------- */
/* General utilities linking objects to their names                   */
/* ------------------------------------------------------------------- */

/* Search auxsyn for verb: that is, check built in synonyms */
int verb_builtin(word w) {
	int i, j;

	for (i = 1; i < TOTAL_VERB; i++)
		for (j = auxsyn[i]; syntbl[j] != 0; j++)
			if (syntbl[j] == w) return i;

	/* Failed to find a match */
	return 0;
}

int verb_authorsyn(word w) {
	int i, j;

	/* Check game-specific synonyms first */
	/* Scan in reverse so later synonyms will override earlier ones */
	for (i = TOTAL_VERB - 1; i > 0; i--)
		for (j = synlist[i]; syntbl[j] != 0; j++)
			if (w == syntbl[j]) return i;
	return 0;
}


int verb_code(word w)
/* Given a word w, searches auxsyn and returns the verb id */
{
	int canon, tmp;

	/* Expand author-defined synonyms */
	tmp = verb_authorsyn(w);
	if (tmp != 0) return tmp;

	/* Expand built-in synonyms */
	canon = verb_builtin(w);
	if (canon != 0) {
		/* Allow built-in verbs to be overridden */
		tmp = verb_authorsyn(syntbl[auxsyn[canon]]);
		if (tmp != 0) return tmp;
	}

	return canon; /* No new synonyms; return canonical match if it exists */
}


/* This is a faster version of the above  for use in the special case of
   command headers where the verb word is much more restricted; it should
   be the first auxsyn entry and it should never by a synlist entry. */
static int cmdverb_code(word w) {
	int i, j;

	for (i = 0; i < TOTAL_VERB; i++)
		if (syntbl[auxsyn[i]] == w) return i;
	/* Hmm... that failed. Search the rest of the auxsyns in case the
	   order of auxsyns has changed or something */
	agtwarn("Header verb not in canonical form.", 1);
	for (i = 1; i < TOTAL_VERB; i++)
		for (j = auxsyn[i]; syntbl[j] != 0; j++)
			if (syntbl[j] == w) return i;
	agtwarn("Header verb not in internal list.", 1);
	return verb_code(w);
}

char *objname(int i) { /* returns malloc'd name string of object i */
	char *s;

	if (i < 0)
		return rstrdup(dict[-i]);
	if (i == 0)
		return rstrdup("....");
	if (i == 1) return rstrdup("*Self*");
	if (i == 1000) return rstrdup("*Worn*");
	if (i >= first_room && i <= maxroom)
		return rstrdup(room[i - first_room].name);
	if ((i >= first_noun && i <= maxnoun) || (i >= first_creat && i <= maxcreat)) {
		word adjw, nounw;
		if (i >= first_noun && i <= maxnoun) {
			adjw = noun[i - first_noun].adj;
			nounw = noun[i - first_noun].name;
		} else {
			adjw = creature[i - first_creat].adj;
			nounw = creature[i - first_creat].name;
		}
		if (adjw == 0 || !strcmp(dict[adjw], "no_adjective"))
			return rstrdup(dict[nounw]);
		return concdup(dict[adjw], dict[nounw]);
	}
	/* At this point we can't get a name: return ILLn. */
	s = (char *)rmalloc(3 + 1 + (5 * sizeof(int)) / 2 + 1);
	/* Make sure we have enough space in case i is big */
	sprintf(s, "ILL%d", i);
	return s;
}



/* ------------------------------------------------------------------- */
/*  Routines to sort the command array and construct verbptr           */
/* ------------------------------------------------------------------- */

#define SORT_META

#ifdef SORT_META

#define ch1 ((const cmd_rec*)cmd1)
#define ch2 ((const cmd_rec*)cmd2)

/* See notes below before trying to decipher this routine;
   during the sort, many of the fields are being used for nonstandard
   purposes */



#define s_verb(cmd) ( (cmd)->actor<0 ? (cmd)->data[0] : (cmd)->verbcmd)

static int cmp_cmd(const void *cmd1, const void *cmd2) {
	word v1, v2;

	/* We are sorting on command[].verbcmd, but if one of the headers
	   is really the object of a redirect command then we need to use
	   its parent's verbcmd */
	/* For commands with actors, we need to avoid sorting them at all. */
	v1 = s_verb(ch1);
	v2 = s_verb(ch2);

	if (v1 < v2) return -1;
	if (v1 > v2) return +1;

	/* v1==v2, so leave them in the same order as before */
	/* We have to take absolute values here because we are using negatives
	   to indicate redirection objects */
	if (ABS(ch1->actor) < ABS(ch2->actor))
		return -1;
	else if (ABS(ch1->actor) == ABS(ch2->actor))
		return 0;
	else return 1;
	/* Equality should be impossible */
}

#undef ch1
#undef ch2

/* This sets things up for qsort */
/* We need a sort that is
   i) Stable and
   ii) Keeps "redirection headers" attached to the correct command */
/* We steal the field actor for this purpose */
/*   actor will equal the index of the header in the original list. */
/*   (or negative the header if the command is a redirection) */
/* For redirected commands, we steal the data pointer since it shouldn't
    be being used anyhow. */
/* In a field pointed to by data we store the verb word */
/* NOTE: this routine requires that the data type of *data (namely
  integer) is big enough to hold a value of type word. */

static void rsort(void) {
	long i;
	integer *save_actor;
	word *save_verb;

	save_actor = (integer *)rmalloc(last_cmd * sizeof(integer));
	save_verb = (word *)rmalloc(last_cmd * sizeof(word));

	/* The following loop does three things:
	   i) Copies command[].actor to save_actor[]
	   ii) Sets command[].actor to the commands index in the array
	   iii) For actor commands, sets the verb to .... after saving it
	          in save_verb.
	   iv) For redirection commands, stores the verb of the owning
	     header in a block pointed to by data */

	for (i = 0; i < last_cmd; i++) { /* Copy actor to save_actor */
		save_verb[i] = command[i].verbcmd;
		if (command[i].actor > 1) /* i.e. there _is_ an actor */
			command[i].verbcmd = syntbl[auxsyn[DIR_ADDR_CODE]];
		save_actor[i] = command[i].actor;
		command[i].actor = i;
		if (save_actor[i] < 0) { /* Redirected command */
			int j;

			command[i].actor = -i;
			rfree(command[i].data); /* data should be NULL, anyhow */
			command[i].data = (integer *)rmalloc(sizeof(integer));
			for (j = i; j > 0 && save_actor[j] < 0; j--);
			if (save_actor[j] > 0)
				command[i].data[0] = command[j].verbcmd;
			else {
				command[i].data[0] = 0;
				agtwarn("First command header is REDIRECT object!", 0);
			}
		}
	}

	/* Now do the sort... */
	qsort(command, last_cmd, sizeof(cmd_rec), cmp_cmd);

#if 0  /* This is code to test the integrity of the sort */
	for (i = 0; i < last_command; i++)
		if (command[i].actor < 0)
			assert(i == 0 || command[i].data[0] == command[i - 1].verbcmd);
#endif

	/* Finally, restore everything to normal */
	for (i = 0; i < last_cmd; i++) { /* Restore actor */
		command[i].verbcmd = save_verb[ABS(command[i].actor)];
		command[i].actor = save_actor[ABS(command[i].actor)];
		if (command[i].actor < 0) {
			rfree(command[i].data);  /* Sets it to NULL automatically */
			command[i].cmdsize = 0;
		}
	}
	rfree(save_actor);
	rfree(save_verb);
}

#endif

void sort_cmd(void) {
	int i;
	word curr_vb;
	word all_word, global_word;

	verbptr = (short *)rmalloc(sizeof(short) * TOTAL_VERB);
	verbend = (short *)rmalloc(sizeof(short) * TOTAL_VERB);

	if (mars_fix) {  /* Don't bother if mars scanning is active */
		for (i = 0; i < TOTAL_VERB; i++) {
			verbptr[i] = 0;       /* That is, scan the whole space for all verbs */
			verbend[i] = last_cmd;
		}
		return;
	}

#ifdef SORT_META
	if (!agx_file && aver >= AGX00) rsort();
#endif


	if (no_auxsyn) return; /* Used by agt2agx */

	for (i = 0; i < TOTAL_VERB; i++) {
		verbptr[i] = last_cmd;
		verbend[i] = 0;
	}

	all_word = search_dict("all");
	if (all_word == 0) all_word = -1; /* This means none of the metacommands
                   used ALL, so prevent ANY matches */
	global_word = search_dict("global_scope");
	if (global_word == 0) global_word = -1; /* Ditto */


	for (i = 0; i < last_cmd; i++) {
		if (command[i].actor < 0) continue; /* Redirection */
		if (command[i].nouncmd == all_word)
			/* Detect multinoun accepting verbs by ALL */
			verbflag[cmdverb_code(command[i].verbcmd)] |= VERB_MULTI;
		if (command[i].actor > 1)
			curr_vb = DIR_ADDR_CODE;
		else
			curr_vb = cmdverb_code(command[i].verbcmd);
		if (i < verbptr[curr_vb]) verbptr[curr_vb] = i;
		if (i > verbend[curr_vb]) verbend[curr_vb] = i;
	}

	for (i = 0; i < TOTAL_VERB; i++)
		if (verbptr[i] == last_cmd) /* No occurences of this verb */
			verbend[i] = last_cmd;
		else verbend[i]++; /* Point *after* last occurance */

	for (i = 0; i < TOTAL_VERB; i++) {
		int j;

		j = synlist[i];
		if (syntbl[j] == 0) continue;
		while (syntbl[j] != 0) j++;
		j--;
		if (syntbl[j] == global_word) { /* Ends with global_scope */
			verbflag[i] |= VERB_GLOBAL;
			syntbl[j] = 0;
		}
	}
}




/* ------------------------------------------------------------------- */
/*  Functions for getting opcode information                           */
/* ------------------------------------------------------------------- */


/* Returns the opdef structure associated with an opcode */
const opdef *get_opdef(integer op) {
	op = op % 2048; /* Strip operand information */
	if (op < 0 || (op > MAX_COND && op < START_ACT) || (op > PREWIN_ACT && op < WIN_ACT)
	        || (op > MAX_ACT)) {
		return &illegal_def;
	}
	if (op >= 2000)
		return &end_def[op - 2000];
	if (op >= 1000)
		return &act_def[op - 1000];
	return &cond_def[op];
}



/* ------------------------------------------------------------------- */
/*  Functions for processing strings                                   */
/* ------------------------------------------------------------------- */

long new_str(char *buff, int max_leng, rbool pasc)
/* Stores the (up to leng) characters of a string
  into our master string space (enlarging it if neccessary)
  and returns the offset into the array.
  pasc=1 ==> pascal-style string
  pasc=0 ==> C-style string; ignore max_leng and NONE strings
  */
{
	int leng, i;
	long p;

	if (pasc) {
		leng = buff[0];
		if (leng > max_leng) leng = max_leng;
	} else
		leng = strlen(buff);

	if (ss_size < ss_end + leng + 1) {
		while (ss_size < ss_end + leng + 1) ss_size += SS_GRAIN;
		static_str = (char *)rrealloc(static_str, sizeof(char) * ss_size);
	}

	if (pasc)
		if (memcmp(buff, nonestr, 5) == 0 || memcmp(buff, NONEstr, 5) == 0) {
			/* "none" --> empty string */
			if (ss_end != 0) return (ss_end - 1); /* Points to last \0 */
			else { /* Very first string */
				static_str[0] = 0;
				ss_end = 1;
				return 0;
			}
		}

	p = ss_end; /* Remember begining of string */
	for (i = 0; i < leng;)
		static_str[ss_end++] = fixchar[(uchar)buff[pasc + (i++)]];
	static_str[ss_end++] = 0;

	return p;
}



/* ------------------------------------------------------------------- */
/* Functions for reading in descriptions                               */
/* ------------------------------------------------------------------- */


descr_line *read_descr(long start, long size) {
	if (agx_file)
		return agx_read_descr(start, size);
	else
		return agt_read_descr(start, size);
}

void free_descr(descr_line *txt) {
	if (txt == NULL) return;
	if (mem_descr == NULL)
		rfree(txt[0]);  /* First free the string block containing the text...*/
	rfree(txt);    /* ... then the array of pointers to it */
}



/* ------------------------------------------------------------------- */
/*  ObjFlag and ObjProp routines                                       */
/* ------------------------------------------------------------------- */

long objextsize(char op) {

	/* op=0 for flags, =1 for props */
	if (op == 0)
		return num_rflags * rangefix(maxroom - first_room + 1)
		       + num_nflags * rangefix(maxnoun - first_noun + 1)
		       + num_cflags * rangefix(maxcreat - first_creat + 1);
	else
		return num_rprops * rangefix(maxroom - first_room + 1)
		       + num_nprops * rangefix(maxnoun - first_noun + 1)
		       + num_cprops * rangefix(maxcreat - first_creat + 1);
}

long lookup_objflag(int id, int t, char *ofs) {
	if (id < 0 || id >= oflag_cnt) return -1;
	switch (t) {
	case 0:
		*ofs = attrtable[id].rbit;
		return attrtable[id].r;
	case 1:
		*ofs = attrtable[id].nbit;
		return attrtable[id].n;
	case 2:
		*ofs = attrtable[id].cbit;
		return attrtable[id].c;
	default:
		rprintf("INT ERROR: Invalid object type.\n");
		return -1;
	}
}

long lookup_objprop(int id, int t) {
	if (id < 0 || id >= oprop_cnt) return -1;
	switch (t) {
	case 0:
		return proptable[id].r;
	case 1:
		return proptable[id].n;
	case 2:
		return proptable[id].c;
	default:
		rprintf("INT ERROR: Invalid object type.\n");
		return -1;
	}
}

int num_oattrs(int t, rbool isflag) {
	switch (t) {
	case 0:
		return isflag ? num_rflags : num_rprops;
	case 1:
		return isflag ? num_nflags : num_nprops;
	case 2:
		return isflag ? num_cflags : num_cprops;
	default:
		rprintf("INT ERROR: Invalid object type.\n");
		return 0;
	}
}

rbool op_simpflag(uchar *pf, char ofs, int op)
/* op: 0=clear, 1=set, 2=nop, 3=toggle    two bits: <ab> */
{
	unsigned char mask, amask, bmask;

	mask = 1 << ofs;
	amask = ~mask | ((op >> 1) << ofs);
	bmask = (op & 1) << ofs;

	*pf = (*pf & amask)^bmask;

	return (*pf & mask) != 0;
}

static long calcindex(integer obj, integer objbase, int ocnt, int base) {
	int rval;

	if (base == -1) rval = -1;
	else rval = (obj - objbase) * ocnt + base;
	/* rprintf("INDEX %d + %d::%d ==> %d\n",base,obj,ocnt,rval); */
	return rval;
}


rbool have_objattr(rbool prop, integer obj, int id) {
	int t;
	char ofs;

	if (troom(obj)) t = 0;
	else if (tnoun(obj)) t = 1;
	else if (tcreat(obj)) t = 2;
	else return 0;
	if (prop)
		return (lookup_objprop(id, t) >= 0);
	else
		return (lookup_objflag(id, t, &ofs) >= 0);
}



rbool op_objflag(int op, integer obj, int id) {
	/* op: 0=clear, 1=set, 2=nop, 3=toggle    two bits: <ab> */
	/*  <flagbit>= (<flagbit>&<a>)^<b> ) */
	int index;
	int t, firstobj;
	char ofs;

	if (troom(obj)) {
		t = 0;
		firstobj = first_room;
	} else if (tnoun(obj)) {
		t = 1;
		firstobj = first_noun;
	} else if (tcreat(obj)) {
		t = 2;
		firstobj = first_creat;
	} else return 0;

	index = calcindex(obj, firstobj, num_oattrs(t, 1), lookup_objflag(id, t, &ofs));
	if (index == -1) return 0;

	return op_simpflag(&objflag[index], ofs, op);
}

long op_objprop(int op, int obj, int id, long val) {
	/* op: 2=get, 1=set */
	int index, t, firstobj;

	if (troom(obj)) {
		t = 0;
		firstobj = first_room;
	} else if (tnoun(obj)) {
		t = 1;
		firstobj = first_noun;
	} else if (tcreat(obj)) {
		t = 2;
		firstobj = first_creat;
	} else return 0;

	index = calcindex(obj, firstobj, num_oattrs(t, 0), lookup_objprop(id, t));
	if (index == -1) return 0;

	if (op == 2) return objprop[index];
	else objprop[index] = val;
	return val;
}

const char *get_objattr_str(int dtype, int id, long val) {
	int max_val;

	if (dtype == AGT_OBJPROP) {
		if (!proptable || !propstr || id < 0 || id >= oprop_cnt) return "";
		max_val = proptable[id].str_cnt;
		if (val < 0) val = 0;
		if (val >= max_val) val = max_val - 1;
		if (max_val > 0)
			return propstr[ proptable[id].str_list + val ];
		return "";
	} else if (dtype == AGT_VAR) {
		if (!vartable || !propstr || id < 0 || id > VAR_NUM) return "";
		max_val = vartable[id].str_cnt;
		if (val < 0) val = 0;
		if (val >= max_val) val = max_val - 1;
		if (max_val > 0)
			return propstr[ vartable[id].str_list + val ];
		return "";
	} else if (dtype == AGT_OBJFLAG) {
		if (attrtable && id >= 0 && id < oflag_cnt)
			return (val ? attrtable[id].ystr : attrtable[id].nstr);
		else
			return (val ? "yes" : "no");
	} else if (dtype == AGT_FLAG) {
		/* This uses yes/no as defaults, not on/off */
		if (flagtable && id >= 0 && id <= FLAG_NUM)
			return val ? flagtable[id].ystr : flagtable[id].nstr;
		else
			return val ? "on" : "off";
	} else
		rprintf("INTERNAL ERROR: Invalid data type for get_objattr_str().");
	return "";
}

/* ------------------------------------------------------------------- */
/* Warning and error functions                                         */
/* ------------------------------------------------------------------- */

void agtwarn(const char *s, int elev) {
	if (ERR_LEVEL >= elev)
		rprintf("Warning: %s\n", s);
}

void agtnwarn(const char *s, int n, int elev) {
	if (ERR_LEVEL >= elev)
		rprintf("Warning: %s%d.\n", s, n);
}

void fatal(const char *s) {
	error("Fatal error: %s", s);
}

void init_flags(void) {
	rm_trap = 1;
	DIAG = def_DIAG;
	interp_arg = def_interp_arg;
	debug_da1 = def_debug_da1;
	RAW_CMD_OUT = def_RAW_CMD_OUT;
	ERR_LEVEL = def_ERR_LEVEL;
	irun_mode = 0;
	fix_ascii_flag = fix_ascii;
	descr_maxmem = DESCR_BUFFSIZE;
	bold_mode = 0;
	dbg_nomsg = 0; /* Print out MSG arguments to metacommands */
	debug_mode = 0;
	dbgflagptr = NULL;
	dbgvarptr = NULL;
	dbgcntptr = NULL;
	no_auxsyn = 0;
	text_file = 0;
#ifdef PATH_SEP
	gamepath = NULL;
#endif
	BATCH_MODE = make_test = 0;
	font_status = 0;
#ifdef OPEN_AS_TEXT
	open_as_binary = 0;
#endif
}

} // End of namespace AGT
} // End of namespace Glk
