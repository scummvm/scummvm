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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/define.h"
#include "watchmaker/extraLS.h"
#include "watchmaker/game.h"
#include "watchmaker/globvar.h"
#include "watchmaker/init/nl_parse.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/types.h"

namespace Watchmaker {

char *TextBucket, cr = 0;                    //da rimpiazzare con la areatext allocata da fab
static char *CurText;
int SentenceNum = 1, SysSentNum = 1, TooltipSentNum = 1, ObjNameNum = 1, ExtraLSNum = 1;
int DlgItemNum = 1;

uint16 Credits_numRoles = 0;
uint16 Credits_numNames = 0;

#define DEFAULT_DEFINE_INPUT "src/define.h"

#define MAXFLAGS 360
#define MAXTOKENLEN 30

#define ANIM_MARKER   'a'
#define CREDIT_MARKER 'c'
#define DLG_MARKER    'd'
#define DIARY_MARKER  'e'
#define ATFDO_MARKER  'f'
#define ACT_MARKER    'h'
#define INV_MARKER    'i'
#define PDALOG_MARKER 'l'
#define MENU_MARKER   'm'
#define MUSIC_MARKER  'n'
#define OBJ_MARKER    'o'
#define ROOM_MARKER   'r'
#define DESC_MARKER   's'
#define ENV_MARKER    'v'
#define SOUND_MARKER  'w'

#define TOKEN_NOT_FOUND 0xffff

typedef struct {
	char  token[MAXTOKENLEN];
	unsigned int value;
} TOKEN;                                // 32 bytes

int SaveTextTable(const char *name);

#define AddSound2Room(inx,val)  \
	{   \
		\
		unsigned short j=0,err=0,*tab=_init.Room[inx].sounds.rawArray(); \
		while(*tab!=0)  \
		{   \
			\
			tab++;         \
			if(j++>=MAX_SOUNDS_IN_ROOM)   \
				err=1;       \
		}         \
		if(err)         \
			_parser->ParseError("Too many Sounds in room %s max is %d",_init.Room[inx].name,MAX_SOUNDS_IN_ROOM); \
		else         \
			*tab=val;        \
	}

#define AddAnim2Room(room,inx)  \
	{   \
		\
		room->anims[animcnt++]=inx;     \
		if(animcnt>=MAX_ANIMS_IN_ROOM)     \
			return _parser->ParseError("Too many Anims in room %s max is %d",s,MAX_ANIMS_IN_ROOM); \
	}

#define AddAct2Room(room,inx)   \
	{   \
		\
		room->actions[actcnt++]=inx;     \
		if(actcnt>=MAX_ACTIONS_IN_ROOM)     \
			return _parser->ParseError("Too many Acts in room %s max is %d",s,MAX_ACTIONS_IN_ROOM); \
	}

#define AddObject2Room(room,inx)    \
	{   \
		\
		room->objects[objcnt++]=inx;     \
		if(objcnt>=MAX_OBJS_IN_ROOM)     \
			return _parser->ParseError("Too many Objects in room %s max is %d",s,MAX_OBJS_IN_ROOM); \
	}

TOKEN *FlagToken = NULL;
TOKEN *RoomToken = NULL;
TOKEN *ObjToken = NULL;
TOKEN *AnimToken = NULL;
TOKEN *InvToken = NULL;
TOKEN *WaveToken = NULL;
TOKEN *DoToken = NULL;
TOKEN *DlgToken = NULL;
TOKEN *ActToken = NULL;
TOKEN *MenuToken = NULL;
TOKEN *DiaryToken = NULL;
TOKEN *EnvToken = NULL;
TOKEN *PDALogToken = NULL;
TOKEN *MusicToken = NULL;

const char *ShortName[] = {
	"???",
	"DAR",
	"VIC",
	"CUO",
	"DOM",
	"GIA",
	"CUS",
	"SER",
	"SUP",
	"MOG",
	"MOG",
	"CAC",
	"VEC",
	"CHI",
	"TRA",
	"ORO",
	"KRE",
	"DUK",
	"COR",
	"VAL",
	"NOT",
	"MOO",
	"DAR",
	"CAC",
	"MOO",
	"BOT",
	"***"
};
char csn = 0;


/* -----------------16/03/98 18.14-------------------
 *                  FreeDefineTable
 * --------------------------------------------------*/
void FreeDefineTable(void) {
	free(FlagToken);
	free(RoomToken);
	free(ObjToken);
	free(AnimToken);
	free(InvToken);
	free(WaveToken);
	free(DoToken);
	free(ActToken);
	free(MenuToken);
	free(DlgToken);
	free(DiaryToken);
	free(EnvToken);
	free(PDALogToken);
	free(MusicToken);

	FlagToken = RoomToken = DiaryToken = ObjToken = ActToken = MenuToken = PDALogToken = MusicToken = NULL;
	AnimToken = InvToken = WaveToken = DoToken = DlgToken = EnvToken = NULL;
}

/* -----------------16/03/98 18.14-------------------
 *                  BuildDefineTable
 * --------------------------------------------------*/
int BuildDefineTable(const char *name) {
	auto parser = NLParser::open(DEFAULT_DEFINE_INPUT);
	if (!parser) {
		parser = NLParser::open(name);
		if (!parser) {
			error("Error Opening File NL %s", name);
		}
	}
	parser->IfParseErrorDo(FreeDefineTable);

	char str[MAXTOKENLEN];
	int res, done = FALSE;
	TOKEN *token = NULL;
	int cur_dlg, cur_flag, cur_room, cur_obj, cur_anim, cur_inv, cur_wave, cur_do, cur_act, cur_menu, cur_diary, cur_env, cur_pdalog, cur_music;

	CurText = TextBucket = t3dCalloc<char>(TEXT_BUCKET_SIZE);
	if (TextBucket == nullptr)
		return parser->ParseError("Error Allocating TextBucket");

	FlagToken = t3dCalloc<TOKEN>(MAXFLAGS);
	RoomToken = t3dCalloc<TOKEN>(MAX_ROOMS);
	ObjToken  = t3dCalloc<TOKEN>(MAX_OBJS);
	AnimToken = t3dCalloc<TOKEN>(MAX_ANIMS);
	InvToken  = t3dCalloc<TOKEN>(MAX_ICONS);
	WaveToken = t3dCalloc<TOKEN>(MAX_SOUNDS);
	DoToken = t3dCalloc<TOKEN>(MAX_ATF_DO);
	ActToken = t3dCalloc<TOKEN>(MAX_ACTIONS);
	MenuToken = t3dCalloc<TOKEN>(MAX_DLG_MENUS);
	DiaryToken = t3dCalloc<TOKEN>(MAX_DIARIES);
	DlgToken = t3dCalloc<TOKEN>(MAX_DIALOGS);
	EnvToken = t3dCalloc<TOKEN>(MAX_ENVIRONMENTS);
	PDALogToken = t3dCalloc<TOKEN>(MAX_PDALOGS);
	MusicToken = t3dCalloc<TOKEN>(MAX_MUSICS);

	if (!DlgToken || !MenuToken || !ActToken || !RoomToken || !ObjToken || !AnimToken || !InvToken || !DiaryToken || !WaveToken || !DoToken || !EnvToken || !PDALogToken || !MusicToken)
		return parser->ParseError("Error Allocating Define Tables");

	cur_dlg = cur_act = cur_menu = cur_flag = cur_room = cur_obj = cur_anim = cur_inv = cur_wave = cur_do = cur_diary = cur_env = cur_pdalog = cur_music = 0;

	while (!done) {
		res = parser->ReadArgument(str);
		warning("Cur res: %d Cur str: '%s'", res, str);
		if (res == EOF_PARSED)break;
		if (scumm_stricmp("#define", str))
			return parser->ParseError("Match %s instead of %s", str, "#define");

		if (parser->ReadArgument(str) < 0)
			return parser->ParseError("Error reading a string in file %s at line %d", name, parser->getCurLine());
		warning("Argument: '%s'", str);
		switch (str[0]) {
		case ROOM_MARKER:
			token = &RoomToken[cur_room++];
			if (cur_room >= MAX_ROOMS)return parser->ParseError("Too many Rooms defined (MAX is %d)", MAX_ROOMS);
			break;
		case OBJ_MARKER:
			token = &ObjToken[cur_obj++];
			if (cur_obj >= MAX_OBJS)return parser->ParseError("Too many Objects defined (MAX is %d)", MAX_OBJS);
			break;
		case INV_MARKER:
			token = &InvToken[cur_inv++];
			if (cur_inv >= MAX_ICONS)return parser->ParseError("Too many Icons defined (MAX is %d)", MAX_ICONS);
			break;
		case ANIM_MARKER:
			token = &AnimToken[cur_anim++];
			if (cur_anim >= MAX_ANIMS)return parser->ParseError("Too many Anims defined (MAX is %d)", MAX_ANIMS);
			break;
		case SOUND_MARKER:
			token = &WaveToken[cur_wave++];
			if (cur_wave >= MAX_SOUNDS)return parser->ParseError("Too many Samples defined (MAX is %d)", MAX_SOUNDS);
			break;
		case ATFDO_MARKER:
			token = &DoToken[cur_do++];
			if (cur_do >= MAX_ATF_DO)return parser->ParseError("Too many AtfDo defined (MAX is %d)", MAX_ATF_DO);
			break;
		case ACT_MARKER:
			token = &ActToken[cur_act++];
			if (cur_act >= MAX_ACTIONS)return parser->ParseError("Too many Action defined (MAX is %d)", MAX_ACTIONS);
			break;
		case MENU_MARKER:
			token = &MenuToken[cur_menu++];
			if (cur_menu >= MAX_DLG_MENUS)return parser->ParseError("Too many Menus defined (MAX is %d)", MAX_DLG_MENUS);
			break;
		case DIARY_MARKER:
			token = &DiaryToken[cur_diary++];
			if (cur_diary >= MAX_DIARIES)return parser->ParseError("Too many Diaries defined (MAX is %d)", MAX_DIARIES);
			break;
		case DLG_MARKER:
			token = &DlgToken[cur_dlg++];
			if (cur_dlg >= MAX_DIALOGS)return parser->ParseError("Too many Dialogs defined (MAX is %d) ", MAX_DIALOGS);
			break;
		case ENV_MARKER:
			token = &EnvToken[cur_env++];
			if (cur_env >= MAX_ENVIRONMENTS)return parser->ParseError("Too many Environments defined (MAX is %d) ", MAX_ENVIRONMENTS);
			break;
		case PDALOG_MARKER:
			token = &PDALogToken[cur_pdalog++];
			if (cur_pdalog >= MAX_PDALOGS)return parser->ParseError("Too many PDALogs defined (MAX is %d) ", MAX_PDALOGS);
			break;
		case MUSIC_MARKER:
			token = &MusicToken[cur_music++];
			if (cur_music >= MAX_MUSICS)return parser->ParseError("Too many Musics defined (MAX is %d) ", MAX_MUSICS);
			break;

		default:
			if (cur_flag >= MAXFLAGS) {         //ne legge solo un po'
				parser->ReadNumber();
				token = NULL;
				break;
			}
			token = &FlagToken[cur_flag++];
			break;
		}
		if (token) {
			strcpy(token->token, str);
			token->value = parser->ReadNumber();
		}
	}
	parser.reset();
	return 1;
}

/*PELS: end DEFINE ***********************************/

#undef Match
#define Match(t) {if((_parser->ReadArgument(str)<0) || scumm_stricmp(str,t))return _parser->ParseError("Syntax Error! can't match '%s' but found '%s'",t,str);}

static const char *InitFile[] = {
	"Init.nl",
	"InitLev1.nl",
	"InitLev2.nl",
	"InitLev3.nl",
	"InitLev4.nl",
	"InitAnim.nl",
	"InitAnim1.nl",
	"InitAnim2.nl",
	"InitAnim3.nl",
	"InitAnim4.nl",
	"InitAnimRtv.nl",
	"InitBkg.nl",
	"InitText.nl",
	"InitIcon.nl",
	"InitDlg.nl",
	"InitRtv1.nl",
	"InitRtv2.nl",
	"InitRtv3.nl",
	"InitRtv4.nl",
	"InitSound.nl",
	"InitT2D.nl",
	"InitDiary.nl",
	"Credits.nl",
	NULL
};

class StructureInitializer {
	Common::SharedPtr<NLParser> _parser;
	Common::String _filename;
	Init &_init;
public:
	StructureInitializer(Common::SharedPtr<NLParser> &parser, const Common::String &filename, Init &init) : _parser(parser), _filename(filename), _init(init) {
	}
	static Common::SharedPtr<StructureInitializer> open(Common::String &path, Init &init) {
		// TODO: Hook up the table-freeing
		auto parser = NLParser::open(path);
		if (!parser) {
			return nullptr;
		}
		warning("Casting away constness");
		if (parser->SearchArgument((char *)path.c_str(), "NL.", NULL) < 0)
			parser->ParseError("%s is not a NL file", path.c_str());
		return Common::SharedPtr<StructureInitializer>(new StructureInitializer(parser, path, init));
	}

	int parseLoop() {
		int res = 1;
		while (true) {
			char  str[J_MAXSTRLEN];
			if (_parser->ReadArgument(str) < 0) {
				if (!scumm_stricmp(str, "end."))break;
				else if ((toupper(str[0]) == 'E') && (toupper(str[1]) == 'N') &&
				         (toupper(str[2]) == 'D') && (toupper(str[3]) == '.'))
					break;
				else
					return _parser->ParseError("Error Reading File %s -%s-", _filename.c_str(), str);
			}
			if (!scumm_stricmp(str, "end."))break;
			if ((toupper(str[0]) == 'E') && (toupper(str[1]) == 'N') &&
			        (toupper(str[2]) == 'D') && (toupper(str[3]) == '.'))
				break;

			res = 0;
			switch (str[0]) {
			case OBJ_MARKER:
				res = ParseObject(str);
				break;
			case ROOM_MARKER:
				res = ParseRoom(str);
				break;
			case ANIM_MARKER:
				res = ParseAnim(str);
				break;
			case INV_MARKER:
				res = ParseInv(str);
				break;
			case SOUND_MARKER:
				res = ParseSound(str);
				break;
			case DLG_MARKER:
				res = ParseDialog(str);
				break;
			case DIARY_MARKER:
				res = ParseDiary(str);
				break;
			case MENU_MARKER:
				res = ParseMenu(str);
				break;
			case PDALOG_MARKER:
				res = ParsePDALog(str);
				break;
			case MUSIC_MARKER:
				res = ParseMusic(str);
				break;
			case CREDIT_MARKER:
				res = ParseCredits(str);
				break;
			}
			if (!res)
				error("Some parse error occurred");
		}
		return 1;
	}

	/* -----------------16/03/98 18.13-------------------
	 *                  GetTokenValue
	 * --------------------------------------------------*/
	unsigned int GetTokenValue(TOKEN *where, char *what) {
		if (!scumm_stricmp("none", what))return 0;
		while (where->token[0] != 0) {
			if (!strcmp(where->token, what))
				return where->value;
			where++;
		}
		_parser->ParseError("Can't find token %s", what);
		return TOKEN_NOT_FOUND;
	}

	/* -----------------16/03/98 18.13-------------------
	 *              GetTokenValueDefault
	 * --------------------------------------------------*/
	unsigned short GetTokenValueDefault(TOKEN *where, char *what) {
		if (!scumm_stricmp("none", what))return 0;
		while (where->token[0] != 0) {
			if (!strcmp(where->token, what))
				return where->value;
			where++;
		}
		return TOKEN_NOT_FOUND;
	}

	/* -----------------16/03/98 17.48-------------------
	 *                  ReadSentence
	 * --------------------------------------------------*/
	int ReadSentence(void) {
		int a;
		if ((a = _parser->ReadArgument(CurText)) < 0)
			_parser->ParseError("Error Reading a string in ReadSentence");
		Sentence[SentenceNum++] = CurText;
		switch (SentenceNum - 1) {
		case 2:
		case 3:
		case 6:
		case 8:
		case 10:
		case 12:
		case 13:
		case 15:
			csn = ocVECCHIO;
			break;
		case 4:
		case 5:
		case 7:
		case 9:
		case 11:
			csn = ocCHIRURGO;
			break;
		case 14:
			csn = ocTRADUTTORE;
			break;
		case 1:
			csn = ocOROLOGIAIO;
			break;
		}
		debug("%s s%04d %s\n", ShortName[csn], SentenceNum - 1, CurText);
		CurText += a;
		return (SentenceNum - 1);
	}

	/* -----------------16/03/98 17.48-------------------
	 *                  ReadSysSent
	 * --------------------------------------------------*/
	int ReadSysSent(void) {
		int a;
		if ((a = _parser->ReadArgument(CurText)) < 0)
			_parser->ParseError("Error Reading a string in ReadSysSent");
		SysSent[SysSentNum++] = CurText;
		csn = ocNOTRANS;
		debug("%s y%04d %s\n", ShortName[csn], SysSentNum - 1, CurText);
		CurText += a;
		return (SysSentNum - 1);
	}

	/* -----------------19/01/99 10.34-------------------
	 *                  ReadTooltipSent
	 * --------------------------------------------------*/
	int ReadTooltipSent(void) {
		int a;
		if ((a = _parser->ReadArgument(CurText)) < 0)
			_parser->ParseError("Error Reading a string in ReadTooltipSent");
		TooltipSent[TooltipSentNum++] = CurText;
		csn = ocNOTRANS;
		debug("%s t%04d %s\n", ShortName[csn], TooltipSentNum - 1, CurText);
		CurText += a;
		return (TooltipSentNum - 1);
	}


	/* -----------------21/03/01 09.40-------------------
	 *                  ReadExtraLS
	 * --------------------------------------------------*/
	int ReadExtraLS(void) {
		int a;
		if ((a = _parser->ReadArgument(CurText)) < 0)
			_parser->ParseError("Error Reading a string in ReadExtraLS");
		ExtraLS[ExtraLSNum++] = CurText;
		csn = ocNOTRANS;
		debug("%s e%04d %s\n", ShortName[csn], ExtraLSNum - 1, CurText);
		CurText += a;
		return (ExtraLSNum - 1);
	}

	/* -----------------16/03/98 17.47-------------------
	 *                  ReadObjName
	 * --------------------------------------------------*/
	int ReadObjName(void) {
		int a;
		if ((a = _parser->ReadArgument(CurText)) < 0)
			_parser->ParseError("Error Reading a string in ReadObjName");
		ObjName[ObjNameNum++] = CurText;
		csn = ocNOTRANS;
		debug("%s n%04d %s\n", ShortName[csn], ObjNameNum - 1, CurText);
		CurText += a;
		return (ObjNameNum - 1);
	}

	/* -----------------16/03/98 17.47-------------------
	 *                  ReadIndex
	 * --------------------------------------------------*/
	int ReadIndex() {
		char str[J_MAXSTRLEN];
		int inx;
		if (_parser->ReadArgument(str) <= 0)
			_parser->ParseError("Error Reading a string in ReadIndex");
		if ((inx = GetTokenValueDefault(WaveToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(ObjToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(DoToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(RoomToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(AnimToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(InvToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(MenuToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(DiaryToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(DlgToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(FlagToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(EnvToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(PDALogToken, str)) != TOKEN_NOT_FOUND)
			return inx;
		if ((inx = GetTokenValueDefault(MusicToken, str)) != TOKEN_NOT_FOUND)
			return inx;

		return atoi(str);
	}

	/* -----------------16/03/98 17.47-------------------
	 *                  ReadFlags
	 * --------------------------------------------------*/
	int ReadFlags() {
		int val = 0;
		char str[J_MAXSTRLEN];
		while (_parser->ReadArgumentEOL(str) > 0) {
			val |= GetTokenValue(FlagToken, str);
		}
		return val;
	}

	float ReadFloat(void) {
		char str[J_MAXSTRLEN];
		if (_parser->ReadArgument(str) < 0)
			return 0;
		return (float)atof(str);
	}


	/* -----------------16/03/98 17.47-------------------
	 *                  AssignSound
	 * --------------------------------------------------*/
	void AssignSound(int val) {
		int inx;
		char str[J_MAXSTRLEN];
		while (_parser->ReadArgumentEOL(str) > 0) {
			if ((inx = GetTokenValue(RoomToken, str)) != TOKEN_NOT_FOUND)
				AddSound2Room(inx, val);
		}
	}


	int ParseInv(char *s);
	int ParseRoom(char *s);
	int ParseAnim(char *s);
	int ParseObject(char *s);
	int ParseSound(char *s);
	int ParseItem(int d);
	int ParseDialog(char *s);
	int ParseDiary(char *s);
	int ParseMenu(char *s);
	int ParsePDALog(char *s);
	int ParseMusic(char *s);
	int ParseCredits(char *s);
};

int InitStructures(WGame &game) {
	const char **filelist = InitFile;
	//
	int  res = TRUE;
	if (!BuildDefineTable(game.workDirs._define.c_str()))
		return 0;

	SaveTextTable("SentUK.txt");

	for (; *filelist != NULL; filelist++) {
		Common::String path = game.workDirs._initDir + *filelist;
		auto initializer = StructureInitializer::open(path, game.init);
		if (!initializer)
			error("Error Opening File NL %s", path.c_str());

		initializer->parseLoop();

		initializer.reset();
	}

	FreeDefineTable();
	if ((CurText - TextBucket) > TEXT_BUCKET_SIZE)
		error("TextBucket OverFlow in %s by %d", *filelist, (CurText - TextBucket));

	{
		debug("\n// ROOMS DESCRIPTIONS\n");
		for (int i = 0; i < MAX_ROOMS; i++)
			debug("*** d%04d %s\n", i, game.init.Room[i].desc);
	}

//	tapullo perchè mancava il nome della porta in inittext
	game.init.Obj[o2MBp2ME].name = game.init.Obj[o2MFp2MB].name;
	game.init.Obj[0] = SObject();

	return 1;
}


/* -----------------24/08/00 9.31--------------------
 *                  LoadExternalText
 * --------------------------------------------------*/
int LoadExternalText(Init *init, char *et) {
	char line[1000];
	int len, num;

	if (!et) return false;
	if (et[0] == '\0') return true;

	auto stream = openFile(et);
	if (!stream)
		return false;

	CurText = TextBucket;
	memset(TextBucket, 0, TEXT_BUCKET_SIZE);
	SentenceNum = SysSentNum = TooltipSentNum = ObjNameNum = ExtraLSNum = 1;

	while (stream->readLine(line, 1000) != nullptr) {
		if ((line[0] == '/') && (line[1] == '/')) continue;

		if ((len = strlen(line)) > 260)
			error("ExternalText: line too long! curlen %d (MAX 250)\n%s", len - 10, line);

		if (len < 2) continue;

		if (sscanf(&line[5], "%d", &num) < 1)
			error("ExternalText: sentence number not found in line:\n%s", line);

		switch (line[4]) {
		case 's':
			Sentence[num] = CurText;
			SentenceNum ++;
			break;
		case 'y':
			SysSent[num] = CurText;
			SysSentNum ++;
			break;
		case 't':
			TooltipSent[num] = CurText;
			TooltipSentNum ++;
			break;
		case 'e':
			ExtraLS[num] = CurText;
			ExtraLSNum ++;
			break;
		case 'n':
			ObjName[num] = CurText;
			ObjNameNum ++;
			break;
		case 'd':
			break;
		default:
			error("ExternalText: unknown paramenters in line:\n%s", line);
		}

		if ((len - 10 - 1) > 0) {
			memcpy(CurText, &line[10], len - 10 - 1);

			switch (line[4]) {
			case 'd':
				strcpy(init->Room[num].desc, CurText);
				break;
			}

			CurText += (len - 10);
		} else {
			switch (line[4]) {
			case 'd':
				strcpy(init->Room[num].desc, "");
				break;
			}

			CurText ++;
		}
	}

	return true;
}

/* -----------------16/03/98 17.46-------------------
 *                  ParseObject
 * --------------------------------------------------*/
int StructureInitializer::ParseObject(char *s) {
	int inx = GetTokenValue(ObjToken, s);
	int done = FALSE, arg, a, len;
	struct SObject *obj;
	char str[J_MAXSTRLEN];
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Object %s doesn't exist!", s);
	obj = &_init.Obj[inx];
	obj->room = cr;
	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}",
		                                   "name:", "examine:", "examined:", "examinev:", "action:", "actiond:", "actionv:",
		                                   "anim:", "animd:", "animv:", "goroom:", "godlg:", "ninv:", "flags:", "pos:", "meshlink:",
		                                   "text:", "log:", "sys:", "tooltip:", "anim2:", "anim2d:", "anim2v:", "log2:", "extrals:",
		                                   NULL)) < 0)return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			debug("\n// %s\n", s);
			obj->name = ReadObjName();
			break;
		case 2:
			csn = ocBOTH;
			obj->examine[DARRELL] = ReadSentence();
			obj->examine[VICTORIA] = obj->examine[DARRELL];
			break;
		case 3:
			csn = ocDARRELL;
			obj->examine[DARRELL] = ReadSentence();
			break;
		case 4:
			csn = ocVICTORIA;
			obj->examine[VICTORIA] = ReadSentence();
			break;
		case 5:
			csn = ocBOTH;
			obj->action[DARRELL] = ReadSentence();
			obj->action[VICTORIA] = obj->action[DARRELL];
			break;
		case 6:
			csn = ocDARRELL;
			obj->action[DARRELL] = ReadSentence();
			break;
		case 7:
			csn = ocVICTORIA;
			obj->action[VICTORIA] = ReadSentence();
			break;
		case 8:
			_parser->ReadArgument(str);
			obj->anim[DARRELL] = GetTokenValue(AnimToken, str);
			obj->anim[VICTORIA] = obj->anim[DARRELL];
			break;
		case 9:
			_parser->ReadArgument(str);
			obj->anim[DARRELL] = GetTokenValue(AnimToken, str);
			break;
		case 10:
			_parser->ReadArgument(str);
			obj->anim[VICTORIA] = GetTokenValue(AnimToken, str);
			break;
		case 11:
			_parser->ReadArgument(str);
			obj->goroom = (uint8)GetTokenValue(RoomToken, str);
			break;
		case 12:
			_parser->ReadArgument(str);
			obj->goroom = (uint8)GetTokenValue(DlgToken, str);
			break;
		case 13:
			_parser->ReadArgument(str);
			obj->ninv = (uint8)GetTokenValue(InvToken, str);
			break;
		case 14:
			obj->flags = ReadFlags();
			break;
		case 15:
			obj->pos = _parser->ReadNumber();
			break;
		case 16:
			for (a = 0; a < MAX_OBJ_MESHLINKS; a++) if (obj->meshLinkIsEmpty(a)) break;
			if (a < MAX_OBJ_MESHLINKS)
				if ((len = _parser->ReadArgument(str)) < 0 || len > MAX_MESHLINK_SIZE)return _parser->ParseError("Error reading meshlink %s in %s", str, s);
				else obj->setMeshLink(a, str);
			else _parser->ParseError("Too many Meshlink in Obj %s %d max is %d", s, a, MAX_OBJ_MESHLINKS);
			break;
		case 17:
			for (a = 0; a < MAX_OBJ_USER_SENTS; a++) if (!obj->text[a]) break;
			csn = ocBOTH;
			if (a < MAX_OBJ_USER_SENTS) obj->text[a] = ReadSentence();
			else _parser->ParseError("Too many UserText in Obj %s %d max is %d", s, a, MAX_OBJ_USER_SENTS);
			break;
		case 18:
			_parser->ReadArgument(str);
			debug("\n// %s\n", str);
			break;
		case 19:
			ReadSysSent();
			break;
		case 20:
			ReadTooltipSent();
			break;
		case 21:
			_parser->ReadArgument(str);
			obj->anim2[DARRELL] = GetTokenValue(AnimToken, str);
			obj->anim2[VICTORIA] = obj->anim2[DARRELL];
			break;
		case 22:
			_parser->ReadArgument(str);
			obj->anim2[DARRELL] = GetTokenValue(AnimToken, str);
			break;
		case 23:
			_parser->ReadArgument(str);
			obj->anim2[VICTORIA] = GetTokenValue(AnimToken, str);
			break;
		case 24:
			_parser->ReadArgument(str);
			debug("// %s\n", str);
			break;
		case 25:
			ReadExtraLS();
			break;
		}
	}
	return TRUE;
}

/* -----------------16/03/98 17.50-------------------
 *                      ParseRoom
 * --------------------------------------------------*/
int StructureInitializer::ParseRoom(char *s) {
	int inx = GetTokenValue(RoomToken, s);
	int a, done = FALSE, res = TRUE, objcnt = 0, animcnt = 0, actcnt = 0;
	char str[J_MAXSTRLEN];
	struct SRoom *room;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Room %s doesn't exist!", s);
	room = &_init.Room[inx];

	a = _parser->ReadArgument(str);
	if (a < 0 || a > sizeof(room->name))return _parser->ParseError("Error reading basename in room %s", s);
	strcpy((char *)room->name, str);

	cr = inx;
	Match("{");
	while (!done) {
		if (_parser->ReadArgument(str) < 0)
			return _parser->ParseError("Error Reading room %s at line %d", s, _parser->getCurLine());
		if (!scumm_stricmp(str, "}"))break;
		switch (str[0]) {
		case OBJ_MARKER:
			inx = GetTokenValue(ObjToken, str);
			if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Can't process %s", str);
			AddObject2Room(room, inx);
			res = ParseObject(str);
			break;
		case ANIM_MARKER:
			inx = GetTokenValue(AnimToken, str);
			if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Can't process %s", str);
			AddAnim2Room(room, inx);
			//res=ParseAnim(str);
			break;
		case ACT_MARKER:
			inx = GetTokenValue(ActToken, str);
			if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Can't process %s", str);
			AddAct2Room(room, inx);
			break;
		case ENV_MARKER:
			inx = GetTokenValue(EnvToken, str);
			if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Can't process %s", str);
			room->env = inx;
			break;
		case DESC_MARKER:
			Match("{");
			_parser->ReadArgument(str);
			strncpy(room->desc, str, 62);
			Match("}");
			break;
		default:
			return _parser->ParseError("Can't process word %s", str);
		}
		if (!res)
			return _parser->ParseError("Error Parsing room %s", s);
	}
	cr = 0;

	return TRUE;
}

/* -----------------16/03/98 18.01-------------------
 *                  ParseAnim
 * --------------------------------------------------*/
int StructureInitializer::ParseAnim(char *s) {
	int inx = GetTokenValue(AnimToken, s);
	int done = FALSE, arg, a, j, len;
	struct SAnim *anim;
	char str[J_MAXSTRLEN];
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Anim %s doesn't exist!", s);
	anim = &_init.Anim[inx];
	/*  a=_parser->ReadArgument(str);
	    if(a<0 || a>sizeof(anim->name))return _parser->ParseError("Error reading name in anim %s",s);
	    strcpy(anim->name,str);
	*/  a = 0;

	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "flags:", "name:", "objlink:", "meshlink:", "roomname:", "portallink:", "pos:",
		                                   "camera:", "obj:", "atframe:", NULL)) < 0)return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			anim->flags = ReadFlags();
			break;
		case 2:
			for (a = 0; a < MAX_SUBANIMS; a++) if (anim->meshLinkIsEmpty(a)) break;
			if ((a < MAX_SUBANIMS) && (--a >= 0))
				if ((len = _parser->ReadArgument(str)) < 0 || len > sizeof(anim->name[a])) return _parser->ParseError("Error reading animname %s in %s", str, s);
				else strcpy((char*)anim->name[a].rawArray(), str);
			else return _parser->ParseError("Too many subanim in Anim %s sub %d max is %d", str, a, MAX_SUBANIMS);
			break;
		case 3:
			for (a = 0; a < MAX_SUBANIMS; a++) if (anim->meshLinkIsEmpty(a)) break;
			if ((a < MAX_SUBANIMS) && (len = _parser->ReadArgument(str)))
				anim->setMeshLink(a, _init.Obj[GetTokenValue(ObjToken, str)].getMeshLink(0));
			else return _parser->ParseError("Too many Objlinks in Anim %s sub %d max is %d", str, a, MAX_SUBANIMS);
			break;
		case 4:
			for (a = 0; a < MAX_SUBANIMS; a++) if (anim->meshLinkIsEmpty(a)) break;
			if (a < MAX_SUBANIMS)
				if ((len = _parser->ReadArgument(str)) < 0 || len > MAX_MESHLINK_SIZE) return _parser->ParseError("Error reading meshlink %s in %s", str, s);
				else anim->setMeshLink(a, str);
			else return _parser->ParseError("Too many Meshlinks in Anim %s sub %d max is %d", str, a, MAX_SUBANIMS);
			break;
		case 5:
			if ((len = _parser->ReadArgument(str)) < 0 || len > sizeof(anim->RoomName)) return _parser->ParseError("Error reading RoomName %s in %s", str, s);
			else strcpy((char*)anim->RoomName.rawArray(), str);
			break;
		case 6:
			if ((len = _parser->ReadArgument(str)) < 0 || len > sizeof(anim->RoomName)) return _parser->ParseError("Error reading PortalLink %s in %s", str, s);
			else strcpy((char*)anim->RoomName.rawArray(), str);
			break;
		case 7:
			anim->pos = _parser->ReadNumber();
			break;
		case 8:
			anim->cam = _parser->ReadNumber();
			break;
		case 9:
			_parser->ReadArgument(str);
			anim->obj = GetTokenValue(ObjToken, str);
			break;
		case 10:
			while (_parser->ReadArgumentEOL(str) > 0) {
				j = 0;
				while (anim->atframe[j].type != 0)
					if (j++ >= MAX_ATFRAMES) return _parser->ParseError("Too many ATFrame in Anim %s max is %d", anim->name[a].rawArray(), MAX_ATFRAMES);
				anim->atframe[j].nframe = atoi(str);
				if ((anim->atframe[j].nframe > 1) && (anim->atframe[j].nframe < 9000))
					anim->atframe[j].nframe *= 3;
				anim->atframe[j].anim = _parser->ReadNumber();
				_parser->ReadArgument(str);
				anim->atframe[j].type = (uint8)GetTokenValue(FlagToken, str);
				anim->atframe[j].index = ReadIndex();
			}
			break;
		}
	}

//	for(a=0;a<MAX_SUBANIMS;a++)
//		DebugFile("Anim |%s| |%s| |%s|",s,anim->meshlink[a],anim->name[a]);

	return TRUE;
}

/* -----------------16/03/98 18.06-------------------
 *                  ParseInv
 * --------------------------------------------------*/
int StructureInitializer::ParseInv(char *s) {
	int inx = GetTokenValue(InvToken, s);
	int done = FALSE, arg, a, len;
	char str[J_MAXSTRLEN];
	struct SInvObject *inv;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Inv %s doesn't exist!", s);
	inv = &_init.InvObj[inx];
	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "name:", "examine:", "examined:", "examinev:", "action:", "actiond:", "actionv:",
		                                   "meshlink:", "flags:", "text:", "log:", "uwobj:", "anim:", "animd:", "animv:", "anim2:", "anim2d:", "anim2v:",
		                                   NULL)) < 0)return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			debug("\n// %s\n", s);
			inv->name = ReadObjName();
			break;
		case 2:
			csn = ocBOTH;
			inv->examine[DARRELL] = ReadSentence();
			inv->examine[VICTORIA] = inv->examine[DARRELL];
			break;
		case 3:
			csn = ocDARRELL;
			inv->examine[DARRELL] = ReadSentence();
			break;
		case 4:
			csn = ocVICTORIA;
			inv->examine[VICTORIA] = ReadSentence();
			break;
		case 5:
			csn = ocBOTH;
			inv->action[DARRELL] = ReadSentence();
			inv->action[VICTORIA] = inv->action[DARRELL];
			break;
		case 6:
			csn = ocDARRELL;
			inv->action[DARRELL] = ReadSentence();
			break;
		case 7:
			csn = ocVICTORIA;
			inv->action[VICTORIA] = ReadSentence();
			break;
		case 8:
			if ((len = _parser->ReadArgument(str)) < 0 || len > sizeof(inv->meshlink)) return _parser->ParseError("Error reading meshlink %s in %s", str, s);
			else strcpy((char*)inv->meshlink.rawArray(), str);
			break;
		case 9:
			inv->flags = ReadFlags();
			break;
		case 10:
			for (a = 0; a < MAX_ICON_USER_SENTS; a++) if (!inv->text[a]) break;
			csn = ocBOTH;
			if (a < MAX_ICON_USER_SENTS) inv->text[a] = ReadSentence();
			else _parser->ParseError("Too many UserText in Inv %s %d max is %d", str, a, MAX_ICON_USER_SENTS);
			break;
		case 11:
			_parser->ReadArgument(str);
			debug("\n// %s\n", str);
			break;
		case 12:
			_parser->ReadArgument(str);
			inv->uwobj = GetTokenValue(ObjToken, str);
			break;
		case 13:
			_parser->ReadArgument(str);
			inv->anim[DARRELL] = GetTokenValue(AnimToken, str);
			inv->anim[VICTORIA] = inv->anim[DARRELL];
			break;
		case 14:
			_parser->ReadArgument(str);
			inv->anim[DARRELL] = GetTokenValue(AnimToken, str);
			break;
		case 15:
			_parser->ReadArgument(str);
			inv->anim[VICTORIA] = GetTokenValue(AnimToken, str);
			break;
		case 16:
			_parser->ReadArgument(str);
			inv->anim2[DARRELL] = GetTokenValue(AnimToken, str);
			inv->anim2[VICTORIA] = inv->anim2[DARRELL];
			break;
		case 17:
			_parser->ReadArgument(str);
			inv->anim2[DARRELL] = GetTokenValue(AnimToken, str);
			break;
		case 18:
			_parser->ReadArgument(str);
			inv->anim2[VICTORIA] = GetTokenValue(AnimToken, str);
			break;
		}
	}
	return TRUE;
}

/* -----------------16/03/98 18.11-------------------
 *                  ParseSound
 * --------------------------------------------------*/
int StructureInitializer::ParseSound(char *s) {
	int inx = GetTokenValue(WaveToken, s);
	int done = FALSE, arg, len, a;
	char str[J_MAXSTRLEN];
	struct SSound *sound;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Sound %s doesn't exist!", s);
	sound = &_init.Sound[inx];
	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "name:", "flags:", "room:", "meshlink:", "cone:", "dist:", "angle:",
		                                   NULL)) < 0)return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			len = _parser->ReadArgument(str);
			if (len < 0 || len > sizeof(sound->name))return _parser->ParseError("Error reading sound 'filename' %s in %s", str, s);
			strcpy(sound->name, str);
			break;
		case 2:
			sound->flags = ReadFlags();
			break;
		case 3:
			AssignSound(inx);
			break;
		case 4:
			for (a = 0; a < MAX_SOUND_MESHLINKS; a++) if (sound->meshlink[a][0] == 0) break;
			if (a < MAX_SOUND_MESHLINKS)
				if ((len = _parser->ReadArgument(str)) < 0 || len > sizeof(sound->meshlink[a]))return _parser->ParseError("Error reading meshlink %s in %s", str, s);
				else strcpy((char*)sound->meshlink[a].rawArray(), str);
			else _parser->ParseError("Too many Meshlink in Sound %s %d max is %d", s, a, MAX_SOUND_MESHLINKS);
			break;
		case 5:
			sound->ConeInside = _parser->ReadNumber();
			sound->ConeOutside = _parser->ReadNumber();
			sound->ConeOutsideVolume = _parser->ReadNumber();
			break;
		case 6:
			sound->MinDist = ReadFloat();
			sound->MaxDist = ReadFloat();
			break;
		case 7:
			sound->Angle = _parser->ReadNumber();
			break;
		}
	}
	return TRUE;
}

/* -----------------01/06/98 10.02-------------------
 *                  ParseItem
 * --------------------------------------------------*/
int StructureInitializer::ParseItem(int d) {
	char str[J_MAXSTRLEN];
	int done = FALSE, arg, a, cp = ocBOTH, i1, i2;
	struct SItemCommand *ic[MAX_PLAYERS];
	unsigned short com, p1, p2;

	_parser->ReadArgument(str);
	debug("\n// %s\n", str);
	a = GetTokenValue(MenuToken, str);
	if (a >= MAX_DLG_MENUS)
		return _parser->ParseError("Troppi Menu nel Dialogo %d MAX = %d", d, MAX_DLG_MENUS);

	_init.Dialog[d].ItemIndex[a] = DlgItemNum;
	DlgItemNum ++;
	if (DlgItemNum >= MAX_DLG_ITEMS)
		return _parser->ParseError("Troppi DlgItem nel Dialogo %d MAX = %d", d, MAX_DLG_ITEMS);

	ic[0] = (struct SItemCommand *)&_init.DlgItem[DlgItemNum - 1].item[0];
	ic[1] = (struct SItemCommand *)&_init.DlgItem[DlgItemNum - 1].item[1];

	for (i1 = 0; i1 < MAX_IC_PER_DLG_ITEM; i1++) if (!ic[DARRELL][i1].com) break;
	for (i2 = 0; i2 < MAX_IC_PER_DLG_ITEM; i2++) if (!ic[VICTORIA][i2].com) break;

	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "setplayer:", "anim:", "setcamera:", "movecamerato:", "settarget:",
		                                   "setchar:", "walkchar:", "runchar:", "backchar:", "hidechar:", "unhidechar:", "changeroom:",
		                                   "expression:", "changeplayer:", "debug:", "item:", "setflags:", "clrflags:", "atframe:", "nextdlg:", "setchar2:",
		                                   "introt1:", "introt2:",
		                                   "tanim:", "tanim2:", "twalkchar:", "trunchar:", "tbackchar:", "twaitcamera:", "twait:", "tfadout:", NULL)) < 0)
			return _parser->ParseError("ParseItem %d %d error %s", d, a, str);
		com = arg;
		p1 = 0;
		p2 = 0;
		switch (arg) {
		case IC_NULL:
			done = TRUE;
			break;
		case IC_SET_PLAYER:
			_parser->ReadArgument(str);
			cp = GetTokenValue(ObjToken, str);
			break;
		case IC_TIME_ANIM2:
		case IC_ANIM:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(AnimToken, str);
			break;
		case IC_SET_CAMERA:
		case IC_TIME_WAIT_CAMERA:
		case IC_MOVE_CAMERA_TO:
			p1 = _parser->ReadNumber();
			break;
		case IC_CHANGE_PLAYER:
		case IC_HIDE_CHAR:
		case IC_UNHIDE_CHAR:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(ObjToken, str);
			break;
		case IC_CHANGE_ROOM:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(RoomToken, str);
			break;
		case IC_SET_TARGET:
		case IC_SET_CHAR2:
		case IC_SET_CHAR:
		case IC_WALK_CHAR:
		case IC_RUN_CHAR:
		case IC_BACK_CHAR:
		case IC_TIME_WALK_CHAR:
		case IC_TIME_RUN_CHAR:
		case IC_TIME_BACK_CHAR:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(ObjToken, str);
			p2 = _parser->ReadNumber();
			break;
		case IC_TIME_ANIM:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(AnimToken, str);
			csn = _init.Anim[p1].obj;
			if (csn > ocBOTH) csn = oNULL;
			p2 = ReadSentence();
			if (strlen(Sentence[SentenceNum - 1]) < 2) {
				p2 = 0;
				SentenceNum--;
			}
			break;
		case IC_TIME_FADOUT:
		case IC_TIME_WAIT:
			p1 = (unsigned short)(ReadFloat() * FRAME_PER_SECOND);
			break;
		case IC_EXPRESSION:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(ObjToken, str);
			p2 = ReadFlags();
			break;
		case IC_DEBUG:
			csn = ocNOTRANS;
			p1 = ReadSentence();
			break;
		case IC_ITEM:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(MenuToken, str);
			p2 = _parser->ReadNumber();
			break;
		case IC_SET_FLAGS:
		case IC_CLR_FLAGS:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(DlgToken, str);
			p2 = ReadFlags();
			break;
		case IC_NEXT_DLG:
			_parser->ReadArgument(str);
			p1 = GetTokenValue(DlgToken, str);
			break;
		case IC_ATFRAME:
			_parser->ReadArgument(str);
			p1 = (uint8)GetTokenValue(FlagToken, str);
			p2 = ReadIndex();
			break;
		case IC_INTRO_TEXT1:
		case IC_INTRO_TEXT2:
			p1 = _parser->ReadNumber();
			p2 = ReadSysSent();
			if (strlen(SysSent[SysSentNum - 1]) < 2) {
				p2 = 0;
				SysSentNum--;
			}
			break;
		}
		if ((i1 >= MAX_IC_PER_DLG_ITEM) || (i2 >= MAX_IC_PER_DLG_ITEM))
			return _parser->ParseError("Troppi ItemCommands nel Dialogo %d item %d,%d MAX = %d", d, i1, i2, MAX_IC_PER_DLG_ITEM);
		if (cp == ocBOTH) {
			ic[DARRELL][i1].com = ic[VICTORIA][i2].com = (uint8)com;
			ic[DARRELL][i1].param1 = ic[VICTORIA][i2].param1 = p1;
			ic[DARRELL][i1++].param2 = ic[VICTORIA][i2++].param2 = p2;
		} else if (cp == ocVICTORIA) {
			ic[VICTORIA][i2].com = (uint8)com;
			ic[VICTORIA][i2].param1 = p1;
			ic[VICTORIA][i2++].param2 = p2;
		} else {
			ic[DARRELL][i1].com = (uint8)com;
			ic[DARRELL][i1].param1 = p1;
			ic[DARRELL][i1++].param2 = p2;
		}
	}
	return TRUE;
}

/* -----------------01/06/98 9.55--------------------
 *                  ParseDialog
 * --------------------------------------------------*/
int StructureInitializer::ParseDialog(char *s) {
	int inx = GetTokenValue(DlgToken, s);
	int arg, done = FALSE, a, alt;
	char str[J_MAXSTRLEN];
	struct SDialog *d;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Dialog %s doesn't exist!", s);
	d = &_init.Dialog[inx];
	debug("\n// %s\n", s);
	Match("{");

	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "flags:", "item:", "alt1pos:", "alt2pos:", "alt3pos:",
		                                   "alt1cam:", "alt2cam:", "alt3cam:", "alt1an:", "alt2an:", "alt3an:", "obj:", "log:",
		                                   NULL)) < 0) return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			d->flags = ReadFlags();
			break;
		case 2:
			if (!ParseItem(inx)) return FALSE;
			break;

		case 3:
		case 4:
		case 5:
			d->AltPosSco[arg - 3] = _parser->ReadNumber();
			break;
		case 6:
		case 7:
		case 8:
			d->AltCamSco[arg - 6] = _parser->ReadNumber();
			break;
		case 9:
		case 10:
		case 11:
			for (a = 0, alt = arg - 9; a < MAX_ALT_ANIMS; a++)
				if (d->AltAnims[alt][a][0] == aNULL)
					break;
			if (a >= MAX_ALT_ANIMS)
				return _parser->ParseError("Too many Alternate Anims defined in Dialog %d max is %d", inx, MAX_ALT_ANIMS);
			_parser->ReadArgument(str);
			d->AltAnims[alt][a][0] = GetTokenValue(AnimToken, str);
			_parser->ReadArgument(str);
			d->AltAnims[alt][a][1] = GetTokenValue(AnimToken, str);
			break;
		case 12:
			_parser->ReadArgument(str);
			d->obj = GetTokenValue(ObjToken, str);
			break;
		case 13:
			_parser->ReadArgument(str);
			debug("\n// %s\n", str);
			break;
		}
	}
	return TRUE;
}

/* -----------------02/06/98 16.15-------------------
 *                  ParseDiary
 * --------------------------------------------------*/
int StructureInitializer::ParseDiary(char *s) {
	int inx = GetTokenValue(DiaryToken, s);
	int done = FALSE, arg, i, j;
	struct SDiary *e;
	char str[J_MAXSTRLEN];

	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Diary %s doesn't exist!", s);

	e = &_init.Diary[inx];
	e->startt = _parser->ReadNumber();
	e->endt = _parser->ReadNumber();

	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "room:", "obj:", "rand:", "end_hideobj:", NULL)) < 0) return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			_parser->ReadArgument(str);
			e->room = GetTokenValue(RoomToken, str);
			break;
		case 2:
			_parser->ReadArgument(str);
			e->obj = GetTokenValue(ObjToken, str);
			break;
		case 3:
			i = 0;
			while (e->item[i].anim[0] != aNULL)
				if (i++ >= MAX_DIARY_ITEMS)
					return _parser->ParseError("Too many DiaryItem in Diary %d max is %d", inx, MAX_DIARY_ITEMS);
			e->item[i].rand = _parser->ReadNumber();
			e->item[i].loop = _parser->ReadNumber();
			e->item[i].bnd = _parser->ReadNumber();

			while (_parser->ReadArgumentEOL(str) > 0) {
				j = 0;
				while (e->item[i].anim[j] != aNULL)
					if (j++ >= MAX_ANIMS_PER_DIARY_ITEM)
						return _parser->ParseError("Too many Anims per DiaryItem in Diary %d max is %d", inx, MAX_ANIMS_PER_DIARY_ITEM);
				e->item[i].anim[j] = GetTokenValue(AnimToken, str);
			}
			break;
		case 4:
			_parser->ReadArgument(str);
			e->end_hideobj = GetTokenValue(ObjToken, str);
			break;
		}
	}
	return TRUE;
}

/* -----------------04/06/98 10.04--------------------
 *                  ParseMenu
 * --------------------------------------------------*/
int StructureInitializer::ParseMenu(char *s) {
	int inx = GetTokenValue(MenuToken, s);
	char str[J_MAXSTRLEN];
	struct SDlgMenu *m;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Menu %s doesn't exist!", s);
	m = &_init.DlgMenu[inx];

	csn = ocNOTRANS;
	m->titolo = ReadSentence();
	_parser->ReadArgument(str);
	m->parent = (uint8)GetTokenValue(MenuToken, str);
	m->on = _parser->ReadNumber();

	return TRUE;
}

/* -----------------26/04/00 16.36-------------------
 *                  ParsePDALog
 * --------------------------------------------------*/
int StructureInitializer::ParsePDALog(char *s) {
	int done = FALSE, arg, a;
	int inx = GetTokenValue(PDALogToken, s);
	char str[J_MAXSTRLEN];
	struct SPDALog *l;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("PDALog %s doesn't exist!", s);
	l = &_init.PDALog[inx];


	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "flags:", "time:", "menu_appartenenza:", "menu_creato:", "text:", NULL)) < 0) return _parser->ParseError("Keyword %s Unknown in %s", str, s);

		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			l->flags = ReadFlags();
			break;
		case 2:
			l->time = _parser->ReadNumber();
			break;
		case 3:
			l->menu_appartenenza = _parser->ReadNumber();
			break;
		case 4:
			l->menu_creato = _parser->ReadNumber();
			break;
		case 5:
			csn = ocNOTRANS;
			for (a = 0; a < MAX_PDA_INFO; a++)
				if (l->text[a] == 0)
					break;
			if (a >= MAX_PDA_INFO) return _parser->ParseError("Too many Info in PDALog item %s: cur %d (MAX %d)", s, a, MAX_PDA_INFO);
			l->text[a] = ReadSentence();
			break;
		}
	}

	return TRUE;
}

/* -----------------23/06/00 16.46-------------------
 *                  ParseMusic
 * --------------------------------------------------*/
int StructureInitializer::ParseMusic(char *s) {
	int done = FALSE, arg;
	int inx = GetTokenValue(MusicToken, s), len;
	char str[J_MAXSTRLEN];
	struct SMusic *n;
	if (inx == TOKEN_NOT_FOUND)return _parser->ParseError("Music %s doesn't exist!", s);
	n = &_init.Music[inx];

	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}", "sub0:", "sub1:", "sub2:", "sub3:", "sub4:", "sub5:", "sub6:", "sub7:", "sub8:", "sub9:", "room:",
		                                   NULL)) < 0) return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
			len = _parser->ReadArgument(str);
			if (len < 0 || len > sizeof(n->name[arg - 1]))return _parser->ParseError("Error reading music 'filename' %s in %s", str, s);
			strcpy(n->name[arg - 1], str);
			n->vol[arg - 1] = _parser->ReadNumber();
			break;
		case 11:
			_parser->ReadArgument(str);
			_init.Room[GetTokenValue(RoomToken, str)].music = inx;
			break;
		}
	}

	return TRUE;
}

/* -----------------22/11/00 15.16-------------------
 *                  ParseCredits
 * --------------------------------------------------*/
int StructureInitializer::ParseCredits(char *s) {
	char str[J_MAXSTRLEN];
	int done, arg, len;
	uint8 curflags;

	curflags = 0;
	done = FALSE;
	Match("{");
	while (!done) {
		if ((arg = _parser->SearchArgument(str, "}",
		                                   "role:", "name:", "flags:",
		                                   NULL)) < 0)return _parser->ParseError("Keyword %s Unknown in %s", str, s);
		switch (arg) {
		case 0:
			done = TRUE;
			break;
		case 1:
			_init._creditsRoles = SerializableDynamicArray<SCreditsRole>((Credits_numRoles + 1));

			len = _parser->ReadArgument(str);
			if (len) {
				if (len >= 48) return _parser->ParseError("Credits role string too long (max is 47)");
				strcpy(_init._creditsRoles[Credits_numRoles].role, str);
			} else
				strcpy(_init._creditsRoles[Credits_numRoles].role, "");
			_init._creditsRoles[Credits_numRoles].flags = curflags;

			Credits_numRoles ++;

			if (!(curflags & CF_STATIC))    curflags = CF_NULL;
			break;
		case 2:
			_init._creditsNames = SerializableDynamicArray<SCreditsName>((Credits_numNames + 1));

			len = _parser->ReadArgument(str);
			if (len) {
				if (len >= 64) return _parser->ParseError("Credits name string too long (max is 31)");
				strcpy(_init._creditsNames[Credits_numNames].name, str);
			} else
				strcpy(_init._creditsNames[Credits_numNames].name, "");
			_init._creditsNames[Credits_numNames].role = Credits_numRoles - 1;
			_init._creditsNames[Credits_numNames].flags = curflags;

			Credits_numNames ++;

			if (!(curflags & CF_STATIC))    curflags = CF_NULL;
			break;
		case 3:
			curflags = (uint8)ReadFlags();
			break;
		}
	}

	return TRUE;
}

int SaveTextTable(const char *name) {
	warning("TODO: Implement SaveTextTable(%s)", name);
	return 0;
}


} // End of namespace Watchmaker
