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

#include "gargoyle/scott/scott.h"

namespace Gargoyle {
namespace Scott {

/*
glkunix_argumentlist_t glkunix_arguments[] =
{
	{ "-y",		glkunix_arg_NoValue,		"-y		Generate 'You are', 'You are carrying' type messages for games that use these instead (eg Robin Of Sherwood)" },
	{ "-i",		glkunix_arg_NoValue,		"-i		Generate 'I am' type messages (default)" },
	{ "-d",		glkunix_arg_NoValue,		"-d		Debugging info on load " },
	{ "-s",		glkunix_arg_NoValue,		"-s		Generate authentic Scott Adams driver light messages rather than other driver style ones (Light goes out in n turns..)" },
	{ "-t",		glkunix_arg_NoValue,		"-t		Generate TRS80 style display (terminal width is 64 characters; a line <-----------------> is displayed after the top stuff; objects have periods after them instead of hyphens" },
	{ "-p",		glkunix_arg_NoValue,		"-p		Use for prehistoric databases which don't use bit 16" },
	{ "-w",		glkunix_arg_NoValue,		"-w		Disable upper window" },
	{ "",		glkunix_arg_ValueFollows,	"filename	file to load" },

	{ nullptr, glkunix_arg_End, nullptr }
};
*/

Scott::Scott(OSystem *syst, const GargoyleGameDescription *gameDesc) : Glk(syst, gameDesc),
		Items(nullptr), Rooms(nullptr), Verbs(nullptr), Nouns(nullptr), Messages(nullptr),
		Actions(nullptr), CurrentCounter(0), SavedRoom(0), Options(0), Width(0), TopHeight(0),
		split_screen(true), Bottom(0), Top(0), BitFlags(0) {
	Common::fill(&NounText[0], &NounText[16], '\0');
	Common::fill(&Counters[0], &Counters[16], 0);
	Common::fill(&RoomSaved[0], &RoomSaved[16], 0);
}

void Scott::runGame(Common::SeekableReadStream *gameFile) {
	int vb, no;
	initialize();

	Bottom = glk_window_open(0, 0, 0, wintype_TextBuffer, 1);
	if (Bottom == nullptr)
		glk_exit();
	glk_set_window(Bottom);

	if (Options & TRS80_STYLE) {
		Width = 64;
		TopHeight = 11;
	} else {
		Width = 80;
		TopHeight = 10;
	}

	if (split_screen) {
		Top = glk_window_open(Bottom, winmethod_Above | winmethod_Fixed, TopHeight, wintype_TextGrid, 0);
		if (Top == nullptr) {
			split_screen = 0;
			Top = Bottom;
		}
	} else {
		Top = Bottom;
	}

	Output("\
Scott Free, A Scott Adams game driver in C.\n\
Release 1.14, (c) 1993,1994,1995 Swansea University Computer Society.\n\
Distributed under the GNU software license\n\n");
	LoadDatabase(gameFile, (Options & DEBUGGING) ? 1 : 0);

	while (!shouldQuit()) {
		glk_tick();

		PerformActions(0, 0);

		Look();

		if (GetInput(&vb, &no) == -1)
			continue;
		switch (PerformActions(vb, no)) {
		case -1:
			Output("I don't understand your command. ");
			break;
		case -2:
			Output("I can't do that yet. ");
			break;
		default:
			break;
		}

		/* Brian Howarth games seem to use -1 for forever */
		if (Items[LIGHT_SOURCE].Location/*==-1*/ != DESTROYED && GameHeader.LightTime != -1) {
			GameHeader.LightTime--;
			if (GameHeader.LightTime < 1) {
				BitFlags |= (1 << LIGHTOUTBIT);
				if (Items[LIGHT_SOURCE].Location == CARRIED ||
					Items[LIGHT_SOURCE].Location == MyLoc) {
					if (Options&SCOTTLIGHT)
						Output("Light has run out! ");
					else
						Output("Your light has run out. ");
				}
				if (Options&PREHISTORIC_LAMP)
					Items[LIGHT_SOURCE].Location = DESTROYED;
			} else if (GameHeader.LightTime < 25) {
				if (Items[LIGHT_SOURCE].Location == CARRIED ||
					Items[LIGHT_SOURCE].Location == MyLoc) {

					if (Options&SCOTTLIGHT) {
						Output("Light runs out in ");
						OutputNumber(GameHeader.LightTime);
						Output(" turns. ");
					} else {
						if (GameHeader.LightTime % 5 == 0)
							Output("Your light is growing dim. ");
					}
				}
			}
		}
	}
}

void Scott::initialize() {
	/*
	int argc = data->argc;
	char **argv = data->argv;

	if (argc < 1)
		return 0;

	while (argv[1])
	{
		if (*argv[1] != '-')
			break;
		switch (argv[1][1])
		{
		case 'y':
			Options |= YOUARE;
			break;
		case 'i':
			Options &= ~YOUARE;
			break;
		case 'd':
			Options |= DEBUGGING;
			break;
		case 's':
			Options |= SCOTTLIGHT;
			break;
		case 't':
			Options |= TRS80_STYLE;
			break;
		case 'p':
			Options |= PREHISTORIC_LAMP;
			break;
		case 'w':
			split_screen = 0;
			break;
		}
		argv++;
		argc--;
	}

	if (argc == 2)
	{
		game_file = argv[1];
#ifdef GARGLK
		const char *s;
		if ((s = strrchr(game_file, '/')) != nullptr || (s = strrchr(game_file, '\\')) != nullptr)
		{
			garglk_set_story_name(s + 1);
		}
		else
		{
			garglk_set_story_name(game_file);
		}
#endif
	}
	*/
}

void Scott::Display(winid_t w, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	Common::String msg = Common::String::vformat(fmt, ap);
	va_end(ap);

	glk_put_string_stream(glk_window_get_stream(w), msg.c_str());
}

void Scott::Delay(int seconds) {
	event_t ev;

	if (!glk_gestalt(gestalt_Timer, 0))
		return;

	glk_request_timer_events(1000 * seconds);

	do
	{
		glk_select(&ev);
	} while (ev.type != evtype_Timer);

	glk_request_timer_events(0);
}

void Scott::Fatal(const char *x) {
	error(x);
}

void Scott::ClearScreen(void) {
	glk_window_clear(Bottom);
}

void *Scott::MemAlloc(int size) {
	void *t = (void *)malloc(size);
	if (t == nullptr)
		Fatal("Out of memory");
	return t;
}

bool Scott::RandomPercent(uint n) {
	return _random.getRandomNumber(99) < n;
}

int Scott::CountCarried(void) {
	int ct = 0;
	int n = 0;
	while (ct <= GameHeader.NumItems) {
		if (Items[ct].Location == CARRIED)
			n++;
		ct++;
	}
	return n;
}

const char *Scott::MapSynonym(const char *word) {
	int n = 1;
	const char *tp;
	static char lastword[16];	/* Last non synonym */
	while (n <= GameHeader.NumWords) {
		tp = Nouns[n];
		if (*tp == '*')
			tp++;
		else
			strcpy(lastword, tp);
		if (xstrncasecmp(word, tp, GameHeader.WordLength) == 0)
			return lastword;
		n++;
	}
	return nullptr;
}

int Scott::MatchUpItem(const char *text, int loc) {
	const char *word = MapSynonym(text);
	int ct = 0;

	if (word == nullptr)
		word = text;

	while (ct <= GameHeader.NumItems) {
		if (Items[ct].AutoGet && Items[ct].Location == loc &&
			xstrncasecmp(Items[ct].AutoGet, word, GameHeader.WordLength) == 0)
			return ct;
		ct++;
	}

	return -1;
}

char *Scott::ReadString(Common::SeekableReadStream *f)
{
	char tmp[1024];
	char *t;
	int c, nc;
	int ct = 0;
	do {
		c = f->readByte();
	} while (f->pos() < f->size() && Common::isSpace(c));
	if (c != '"') {
		Fatal("Initial quote expected");
	}

	do {
		c = f->readByte();
		if (c == EOF)
			Fatal("EOF in string");
		if (c == '"')
		{
			nc = f->readByte();
			if (nc != '"') {
				f->seek(-1, SEEK_CUR);
				break;
			}
		}
		if (c == '`')
			c = '"'; /* pdd */

					 /* Ensure a valid Glk newline is sent. */
		if (c == '\n')
			tmp[ct++] = 10;
		/* Special case: assume CR is part of CRLF in a
		* DOS-formatted file, and ignore it.
		*/
		else if (c == 13)
			;
		/* Pass only ASCII to Glk; the other reasonable option
		* would be to pass Latin-1, but it's probably safe to
		* assume that Scott Adams games are ASCII only.
		*/
		else if ((c >= 32 && c <= 126))
			tmp[ct++] = c;
		else
			tmp[ct++] = '?';
	} while (1);

	tmp[ct] = 0;
	t = (char *)MemAlloc(ct + 1);
	memcpy(t, tmp, ct + 1);
	return t;
}

void Scott::LoadDatabase(Common::SeekableReadStream *f, int loud) {
	int unused, ni, na, nw, nr, mc, pr, tr, wl, lt, mn, trm;
	int ct;
	int lo;
	Action *ap;
	Room *rp;
	Item *ip;
	/* Load the header */

	readInts(f, 12, &unused, &ni, &na, &nw, &nr, &mc, &pr, &tr, &wl, &lt, &mn, &trm);

	GameHeader.NumItems = ni;
	Items = (Item *)MemAlloc(sizeof(Item)*(ni + 1));
	GameHeader.NumActions = na;
	Actions = (Action *)MemAlloc(sizeof(Action)*(na + 1));
	GameHeader.NumWords = nw;
	GameHeader.WordLength = wl;
	Verbs = (const char **)MemAlloc(sizeof(char *)*(nw + 1));
	Nouns = (const char **)MemAlloc(sizeof(char *)*(nw + 1));
	GameHeader.NumRooms = nr;
	Rooms = (Room *)MemAlloc(sizeof(Room)*(nr + 1));
	GameHeader.MaxCarry = mc;
	GameHeader.PlayerRoom = pr;
	GameHeader.Treasures = tr;
	GameHeader.LightTime = lt;
	LightRefill = lt;
	GameHeader.NumMessages = mn;
	Messages = (const char **)MemAlloc(sizeof(char *)*(mn + 1));
	GameHeader.TreasureRoom = trm;

	/* Load the actions */

	ct = 0;
	ap = Actions;
	if (loud)
		debug("Reading %d actions.", na);
	while (ct < na + 1) {
		readInts(f, 8,
			&ap->Vocab,
			&ap->Condition[0],
			&ap->Condition[1],
			&ap->Condition[2],
			&ap->Condition[3],
			&ap->Condition[4],
			&ap->action[0],
			&ap->action[1]);
		ap++;
		ct++;
	}

	ct = 0;
	if (loud)
		debug("Reading %d word pairs.", nw);
	while (ct<nw + 1) {
		Verbs[ct] = ReadString(f);
		Nouns[ct] = ReadString(f);
		ct++;
	}
	ct = 0;
	rp = Rooms;
	if (loud)
		debug("Reading %d rooms.", nr);
	while (ct<nr + 1) {
		readInts(f, 6,
				&rp->Exits[0], &rp->Exits[1], &rp->Exits[2],
				&rp->Exits[3], &rp->Exits[4], &rp->Exits[5]);
	
		rp->Text = ReadString(f);
		ct++;
		rp++;
	}

	ct = 0;
	if (loud)
		debug("Reading %d messages.", mn);
	while (ct<mn + 1) {
		Messages[ct] = ReadString(f);
		ct++;
	}

	ct = 0;
	if (loud)
		debug("Reading %d items.", ni);
	ip = Items;
	while (ct < ni + 1) {
		ip->Text = ReadString(f);
		ip->AutoGet = strchr(ip->Text, '/');
		/* Some games use // to mean no auto get/drop word! */
		if (ip->AutoGet && strcmp(ip->AutoGet, "//") && strcmp(ip->AutoGet, "/*")) {
			char *t;
			*ip->AutoGet++ = 0;
			t = strchr(ip->AutoGet, '/');
			if (t != nullptr)
				*t = 0;
		}

		readInts(f, 1, &lo);
		ip->Location = (unsigned char)lo;
		ip->InitialLoc = ip->Location;
		ip++;
		ct++;
	}
	ct = 0;
	/* Discard Comment Strings */
	while (ct<na + 1) {
		free(ReadString(f));
		ct++;
	}

	readInts(f, 1, &ct);
	if (loud)
		debug("Version %d.%02d of Adventure ", ct / 100, ct % 100);
	readInts(f, 1, &ct);

	if (loud)
		debug("%d.\nLoad Complete.\n", ct);
}

void Scott::Output(const char *a) {
	Display(Bottom, "%s", a);
}

void Scott::OutputNumber(int a) {
	Display(Bottom, "%d", a);
}

void Scott::Look(void) {
	static char *ExitNames[6] = { "North", "South", "East", "West", "Up", "Down" };
	Room *r;
	int ct, f;
	int pos;

	if (split_screen)
		glk_window_clear(Top);

	if ((BitFlags&(1 << DARKBIT)) && Items[LIGHT_SOURCE].Location != CARRIED
		&& Items[LIGHT_SOURCE].Location != MyLoc) {
		if (Options&YOUARE)
			Display(Top, "You can't see. It is too dark!\n");
		else
			Display(Top, "I can't see. It is too dark!\n");
		if (Options & TRS80_STYLE)
			Display(Top, TRS80_LINE);
		return;
	}
	r = &Rooms[MyLoc];
	if (*r->Text == '*')
		Display(Top, "%s\n", r->Text + 1);
	else {
		if (Options&YOUARE)
			Display(Top, "You are in a %s\n", r->Text);
		else
			Display(Top, "I'm in a %s\n", r->Text);
	}

	ct = 0;
	f = 0;
	Display(Top, "\nObvious exits: ");
	while (ct<6) {
		if (r->Exits[ct] != 0)
		{
			if (f == 0)
				f = 1;
			else
				Display(Top, ", ");
			Display(Top, "%s", ExitNames[ct]);
		}
		ct++;
	}

	if (f == 0)
		Display(Top, "none");
	Display(Top, ".\n");
	ct = 0;
	f = 0;
	pos = 0;
	while (ct <= GameHeader.NumItems) {
		if (Items[ct].Location == MyLoc) {
			if (f == 0) {
				if (Options & YOUARE) {
					Display(Top, "\nYou can also see: ");
					pos = 18;
				} else {
					Display(Top, "\nI can also see: ");
					pos = 16;
				}
				f++;
			} else if (!(Options & TRS80_STYLE)) {
				Display(Top, " - ");
				pos += 3;
			}
			if (pos + (int)strlen(Items[ct].Text) > (Width - 10)) {
				pos = 0;
				Display(Top, "\n");
			}
			Display(Top, "%s", Items[ct].Text);
			pos += strlen(Items[ct].Text);
			if (Options & TRS80_STYLE) {
				Display(Top, ". ");
				pos += 2;
			}
		}
		ct++;
	}

	Display(Top, "\n");
	if (Options & TRS80_STYLE)
		Display(Top, TRS80_LINE);
}

int Scott::WhichWord(const char *word, const char **list) {
	int n = 1;
	int ne = 1;
	const char *tp;
	while (ne <= GameHeader.NumWords) {
		tp = list[ne];
		if (*tp == '*')
			tp++;
		else
			n = ne;
		if (xstrncasecmp(word, tp, GameHeader.WordLength) == 0)
			return n;
		ne++;
	}
	return -1;
}

void Scott::LineInput(char *buf, size_t n) {
	event_t ev;

	glk_request_line_event(Bottom, buf, n - 1, 0);

	do {
		glk_select(&ev);

		if (ev.type == evtype_LineInput)
			break;
		else if (ev.type == evtype_Arrange && split_screen)
			Look();
	} while (ev.type != evtype_Quit);

	buf[ev.val1] = 0;
}

void Scott::SaveGame(void) {
	strid_t file;
	frefid_t ref;
	int ct;
	Common::String msg;

	ref = glk_fileref_create_by_prompt(fileusage_TextMode | fileusage_SavedGame, filemode_Write, 0);
	if (ref == nullptr) return;

	file = glk_stream_open_file(ref, filemode_Write, 0);
	glk_fileref_destroy(ref);
	if (file == nullptr) return;

	for (ct = 0; ct < 16; ct++) {
		msg = Common::String::format("%d %d\n", Counters[ct], RoomSaved[ct]);
		glk_put_string_stream(file, msg.c_str());
	}

	msg = Common::String::format("%lu %d %hd %d %d %hd\n",
		BitFlags, (BitFlags&(1 << DARKBIT)) ? 1 : 0,
		MyLoc, CurrentCounter, SavedRoom, GameHeader.LightTime);
	glk_put_string_stream(file, msg.c_str());

	for (ct = 0; ct <= GameHeader.NumItems; ct++) {
		msg = Common::String::format("%hd\n", (short)Items[ct].Location);
		glk_put_string_stream(file, msg.c_str());
	}

	glk_stream_close(file, nullptr);
	Output("Saved.\n");
}

void Scott::LoadGame(void) {
	strid_t file;
	frefid_t ref;
	char buf[128];
	int ct = 0;
	short lo;
	short DarkFlag;

	ref = glk_fileref_create_by_prompt(fileusage_TextMode | fileusage_SavedGame, filemode_Read, 0);
	if (ref == nullptr) return;

	file = glk_stream_open_file(ref, filemode_Read, 0);
	glk_fileref_destroy(ref);
	if (file == nullptr) return;

	for (ct = 0; ct<16; ct++) {
		glk_get_line_stream(file, buf, sizeof buf);
		sscanf(buf, "%d %d", &Counters[ct], &RoomSaved[ct]);
	}

	glk_get_line_stream(file, buf, sizeof buf);
	sscanf(buf, "%ld %hd %d %d %d %d\n",
		&BitFlags, &DarkFlag, &MyLoc, &CurrentCounter, &SavedRoom,
		&GameHeader.LightTime);

	/* Backward compatibility */
	if (DarkFlag)
		BitFlags |= (1 << 15);
	for (ct = 0; ct <= GameHeader.NumItems; ct++) {
		glk_get_line_stream(file, buf, sizeof buf);
		sscanf(buf, "%hd\n", &lo);
		Items[ct].Location = (unsigned char)lo;
	}
}

int Scott::GetInput(int *vb, int *no) {
	char buf[256];
	char verb[10], noun[10];
	int vc, nc;
	int num;

	do {
		do {
			Output("\nTell me what to do ? ");
			LineInput(buf, sizeof buf);
			num = sscanf(buf, "%9s %9s", verb, noun);
		} while (num == 0 || *buf == '\n');
		
		if (xstrcasecmp(verb, "restore") == 0) {
			LoadGame();
			return -1;
		}
		if (num == 1)
			*noun = 0;
		if (*noun == 0 && strlen(verb) == 1) {
			switch (Common::isUpper((unsigned char)*verb) ? tolower((unsigned char)*verb) : *verb) {
			case 'n':strcpy(verb, "NORTH"); break;
			case 'e':strcpy(verb, "EAST"); break;
			case 's':strcpy(verb, "SOUTH"); break;
			case 'w':strcpy(verb, "WEST"); break;
			case 'u':strcpy(verb, "UP"); break;
			case 'd':strcpy(verb, "DOWN"); break;
				/* Brian Howarth interpreter also supports this */
			case 'i':strcpy(verb, "INVENTORY"); break;
			}
		}
		nc = WhichWord(verb, Nouns);
		/* The Scott Adams system has a hack to avoid typing 'go' */
		if (nc >= 1 && nc <= 6) {
			vc = 1;
		} else {
			vc = WhichWord(verb, Verbs);
			nc = WhichWord(noun, Nouns);
		}
		*vb = vc;
		*no = nc;
		if (vc == -1) {
			Output("You use word(s) I don't know! ");
		}
	} while (vc == -1);

	strcpy(NounText, noun);	/* Needed by GET/DROP hack */
	return 0;
}

int Scott::PerformLine(int ct) {
	int continuation = 0;
	int param[5], pptr = 0;
	int act[4];
	int cc = 0;

	while (cc<5) {
		int cv, dv;
		cv = Actions[ct].Condition[cc];
		dv = cv / 20;
		cv %= 20;
		switch (cv) {
		case 0:
			param[pptr++] = dv;
			break;
		case 1:
			if (Items[dv].Location != CARRIED)
				return 0;
			break;
		case 2:
			if (Items[dv].Location != MyLoc)
				return 0;
			break;
		case 3:
			if (Items[dv].Location != CARRIED&&
				Items[dv].Location != MyLoc)
				return 0;
			break;
		case 4:
			if (MyLoc != dv)
				return 0;
			break;
		case 5:
			if (Items[dv].Location == MyLoc)
				return 0;
			break;
		case 6:
			if (Items[dv].Location == CARRIED)
				return 0;
			break;
		case 7:
			if (MyLoc == dv)
				return 0;
			break;
		case 8:
			if ((BitFlags&(1 << dv)) == 0)
				return 0;
			break;
		case 9:
			if (BitFlags&(1 << dv))
				return 0;
			break;
		case 10:
			if (CountCarried() == 0)
				return 0;
			break;
		case 11:
			if (CountCarried())
				return 0;
			break;
		case 12:
			if (Items[dv].Location == CARRIED || Items[dv].Location == MyLoc)
				return 0;
			break;
		case 13:
			if (Items[dv].Location == 0)
				return 0;
			break;
		case 14:
			if (Items[dv].Location)
				return 0;
			break;
		case 15:
			if (CurrentCounter>dv)
				return 0;
			break;
		case 16:
			if (CurrentCounter <= dv)
				return 0;
			break;
		case 17:
			if (Items[dv].Location != Items[dv].InitialLoc)
				return 0;
			break;
		case 18:
			if (Items[dv].Location == Items[dv].InitialLoc)
				return 0;
			break;
		case 19:/* Only seen in Brian Howarth games so far */
			if (CurrentCounter != dv)
				return 0;
			break;
		}
		cc++;
	}
	/* Actions */
	act[0] = Actions[ct].action[0];
	act[2] = Actions[ct].action[1];
	act[1] = act[0] % 150;
	act[3] = act[2] % 150;
	act[0] /= 150;
	act[2] /= 150;
	cc = 0;
	pptr = 0;
	while (cc<4)
	{
		if (act[cc] >= 1 && act[cc] < 52) {
			Output(Messages[act[cc]]);
			Output("\n");
		} else if (act[cc] > 101) {
			Output(Messages[act[cc] - 50]);
			Output("\n");
		}
		else {
			switch (act[cc]) {
			case 0:/* NOP */
				break;
			case 52:
				if (CountCarried() == GameHeader.MaxCarry)
				{
					if (Options&YOUARE)
						Output("You are carrying too much. ");
					else
						Output("I've too much to carry! ");
					break;
				}
				Items[param[pptr++]].Location = CARRIED;
				break;
			case 53:
				Items[param[pptr++]].Location = MyLoc;
				break;
			case 54:
				MyLoc = param[pptr++];
				break;
			case 55:
				Items[param[pptr++]].Location = 0;
				break;
			case 56:
				BitFlags |= 1 << DARKBIT;
				break;
			case 57:
				BitFlags &= ~(1 << DARKBIT);
				break;
			case 58:
				BitFlags |= (1 << param[pptr++]);
				break;
			case 59:
				Items[param[pptr++]].Location = 0;
				break;
			case 60:
				BitFlags &= ~(1 << param[pptr++]);
				break;
			case 61:
				if (Options&YOUARE)
					Output("You are dead.\n");
				else
					Output("I am dead.\n");
				BitFlags &= ~(1 << DARKBIT);
				MyLoc = GameHeader.NumRooms;/* It seems to be what the code says! */
				break;
			case 62:
			{
				/* Bug fix for some systems - before it could get parameters wrong */
				int i = param[pptr++];
				Items[i].Location = param[pptr++];
				break;
			}
			case 63:
				doneit:				Output("The game is now over.\n");
									glk_exit();
			case 64:
				break;
			case 65:
			{
				int i = 0;
				int n = 0;
				while (i <= GameHeader.NumItems)
				{
					if (Items[i].Location == GameHeader.TreasureRoom &&
						*Items[i].Text == '*')
						n++;
					i++;
				}
				if (Options&YOUARE)
					Output("You have stored ");
				else
					Output("I've stored ");
				OutputNumber(n);
				Output(" treasures.  On a scale of 0 to 100, that rates ");
				OutputNumber((n * 100) / GameHeader.Treasures);
				Output(".\n");
				if (n == GameHeader.Treasures)
				{
					Output("Well done.\n");
					goto doneit;
				}
				break;
			}
			case 66:
			{
				int i = 0;
				int f = 0;
				if (Options&YOUARE)
					Output("You are carrying:\n");
				else
					Output("I'm carrying:\n");
				while (i <= GameHeader.NumItems)
				{
					if (Items[i].Location == CARRIED)
					{
						if (f == 1)
						{
							if (Options & TRS80_STYLE)
								Output(". ");
							else
								Output(" - ");
						}
						f = 1;
						Output(Items[i].Text);
					}
					i++;
				}
				if (f == 0)
					Output("Nothing");
				Output(".\n");
				break;
			}
			case 67:
				BitFlags |= (1 << 0);
				break;
			case 68:
				BitFlags &= ~(1 << 0);
				break;
			case 69:
				GameHeader.LightTime = LightRefill;
				Items[LIGHT_SOURCE].Location = CARRIED;
				BitFlags &= ~(1 << LIGHTOUTBIT);
				break;
			case 70:
				ClearScreen(); /* pdd. */
				break;
			case 71:
				SaveGame();
				break;
			case 72:
			{
				int i1 = param[pptr++];
				int i2 = param[pptr++];
				int t = Items[i1].Location;
				Items[i1].Location = Items[i2].Location;
				Items[i2].Location = t;
				break;
			}
			case 73:
				continuation = 1;
				break;
			case 74:
				Items[param[pptr++]].Location = CARRIED;
				break;
			case 75:
			{
				int i1, i2;
				i1 = param[pptr++];
				i2 = param[pptr++];
				Items[i1].Location = Items[i2].Location;
				break;
			}
			case 76:	/* Looking at adventure .. */
				break;
			case 77:
				if (CurrentCounter >= 0)
					CurrentCounter--;
				break;
			case 78:
				OutputNumber(CurrentCounter);
				break;
			case 79:
				CurrentCounter = param[pptr++];
				break;
			case 80:
			{
				int t = MyLoc;
				MyLoc = SavedRoom;
				SavedRoom = t;
				break;
			}
			case 81:
			{
				/* This is somewhat guessed. Claymorgue always
				seems to do select counter n, thing, select counter n,
				but uses one value that always seems to exist. Trying
				a few options I found this gave sane results on ageing */
				int t = param[pptr++];
				int c1 = CurrentCounter;
				CurrentCounter = Counters[t];
				Counters[t] = c1;
				break;
			}
			case 82:
				CurrentCounter += param[pptr++];
				break;
			case 83:
				CurrentCounter -= param[pptr++];
				if (CurrentCounter < -1)
					CurrentCounter = -1;
				/* Note: This seems to be needed. I don't yet
				know if there is a maximum value to limit too */
				break;
			case 84:
				Output(NounText);
				break;
			case 85:
				Output(NounText);
				Output("\n");
				break;
			case 86:
				Output("\n");
				break;
			case 87:
			{
				/* Changed this to swap location<->roomflag[x]
				not roomflag 0 and x */
				int p = param[pptr++];
				int sr = MyLoc;
				MyLoc = RoomSaved[p];
				RoomSaved[p] = sr;
				break;
			}
			case 88:
				Delay(2);
				break;
			case 89:
				pptr++;
				/* SAGA draw picture n */
				/* Spectrum Seas of Blood - start combat ? */
				/* Poking this into older spectrum games causes a crash */
				break;
			default:
				error("Unknown action %d [Param begins %d %d]\n",
					act[cc], param[pptr], param[pptr + 1]);
				break;
			}
		}

		cc++;
	}

	return 1 + continuation;
}

int Scott::PerformActions(int vb, int no) {
	static int disable_sysfunc = 0;	/* Recursion lock */
	int d = BitFlags&(1 << DARKBIT);

	int ct = 0;
	int fl;
	int doagain = 0;
	if (vb == 1 && no == -1) {
		Output("Give me a direction too.");
		return 0;
	}
	if (vb == 1 && no >= 1 && no <= 6) {
		int nl;
		if (Items[LIGHT_SOURCE].Location == MyLoc ||
			Items[LIGHT_SOURCE].Location == CARRIED)
			d = 0;
		if (d)
			Output("Dangerous to move in the dark! ");
		nl = Rooms[MyLoc].Exits[no - 1];
		if (nl != 0) {
			MyLoc = nl;
			return 0;
		}
		if (d) {
			if (Options&YOUARE)
				Output("You fell down and broke your neck. ");
			else
				Output("I fell down and broke my neck. ");
			glk_exit();
		}
		if (Options&YOUARE)
			Output("You can't go in that direction. ");
		else
			Output("I can't go in that direction. ");
		return 0;
	}

	fl = -1;
	while (ct <= GameHeader.NumActions) {
		int vv, nv;
		vv = Actions[ct].Vocab;
		/* Think this is now right. If a line we run has an action73
		run all following lines with vocab of 0,0 */
		if (vb != 0 && (doagain&&vv != 0))
			break;
		/* Oops.. added this minor cockup fix 1.11 */
		if (vb != 0 && !doagain && fl == 0)
			break;
		nv = vv % 150;
		vv /= 150;
		if ((vv == vb) || (doagain&&Actions[ct].Vocab == 0)) {
			if ((vv == 0 && RandomPercent(nv)) || doagain ||
					(vv != 0 && (nv == no || nv == 0))) {
				int f2;
				if (fl == -1)
					fl = -2;
				if ((f2 = PerformLine(ct)) > 0) {
					/* ahah finally figured it out ! */
					fl = 0;
					if (f2 == 2)
						doagain = 1;
					if (vb != 0 && doagain == 0)
						return 0;
				}
			}
		}
		ct++;

		/* Previously this did not check ct against
		* GameHeader.NumActions and would read past the end of
		* Actions.  I don't know what should happen on the last
		* action, but doing nothing is better than reading one
		* past the end.
		* --Chris
		*/
		if (ct <= GameHeader.NumActions && Actions[ct].Vocab != 0)
			doagain = 0;
	}
	if (fl != 0 && disable_sysfunc == 0) {
		int item;
		if (Items[LIGHT_SOURCE].Location == MyLoc ||
			Items[LIGHT_SOURCE].Location == CARRIED)
			d = 0;
		if (vb == 10 || vb == 18) {
			/* Yes they really _are_ hardcoded values */
			if (vb == 10) {
				if (xstrcasecmp(NounText, "ALL") == 0) {
					int i = 0;
					int f = 0;

					if (d) {
						Output("It is dark.\n");
						return 0;
					}
					while (i <= GameHeader.NumItems) {
						if (Items[i].Location == MyLoc && Items[i].AutoGet != nullptr && Items[i].AutoGet[0] != '*') {
							no = WhichWord(Items[i].AutoGet, Nouns);
							disable_sysfunc = 1;	/* Don't recurse into auto get ! */
							PerformActions(vb, no);	/* Recursively check each items table code */
							disable_sysfunc = 0;
							if (CountCarried() == GameHeader.MaxCarry) {
								if (Options&YOUARE)
									Output("You are carrying too much. ");
								else
									Output("I've too much to carry. ");
								return 0;
							}
							Items[i].Location = CARRIED;
							Output(Items[i].Text);
							Output(": O.K.\n");
							f = 1;
						}
						i++;
					}
					if (f == 0)
						Output("Nothing taken.");
					return 0;
				}
				if (no == -1)
				{
					Output("What ? ");
					return 0;
				}
				if (CountCarried() == GameHeader.MaxCarry)
				{
					if (Options&YOUARE)
						Output("You are carrying too much. ");
					else
						Output("I've too much to carry. ");
					return 0;
				}
				item = MatchUpItem(NounText, MyLoc);
				if (item == -1)
				{
					if (Options&YOUARE)
						Output("It is beyond your power to do that. ");
					else
						Output("It's beyond my power to do that. ");
					return 0;
				}
				Items[item].Location = CARRIED;
				Output("O.K. ");
				return 0;
			}
			if (vb == 18) {
				if (xstrcasecmp(NounText, "ALL") == 0) {
					int i = 0;
					int f = 0;
					while (i <= GameHeader.NumItems) {
						if (Items[i].Location == CARRIED && Items[i].AutoGet && Items[i].AutoGet[0] != '*') {
							no = WhichWord(Items[i].AutoGet, Nouns);
							disable_sysfunc = 1;
							PerformActions(vb, no);
							disable_sysfunc = 0;
							Items[i].Location = MyLoc;
							Output(Items[i].Text);
							Output(": O.K.\n");
							f = 1;
						}
						i++;
					}
					if (f == 0)
						Output("Nothing dropped.\n");
					return 0;
				}
				if (no == -1) {
					Output("What ? ");
					return 0;
				}
				item = MatchUpItem(NounText, CARRIED);
				if (item == -1) {
					if (Options&YOUARE)
						Output("It's beyond your power to do that.\n");
					else
						Output("It's beyond my power to do that.\n");
					return 0;
				}
				Items[item].Location = MyLoc;
				Output("O.K. ");
				return 0;
			}
		}
	}

	return fl;
}

int Scott::xstrcasecmp(const char *s1, const char *s2) {
	const unsigned char
		*us1 = (const unsigned char *)s1,
		*us2 = (const unsigned char *)s2;

	while (tolower(*us1) == tolower(*us2++))
		if (*us1++ == '\0')
			return (0);
	return (tolower(*us1) - tolower(*--us2));
}

int Scott::xstrncasecmp(const char *s1, const char *s2, size_t n) {
	if (n != 0) {
		const unsigned char
			*us1 = (const unsigned char *)s1,
			*us2 = (const unsigned char *)s2;

		do {
			if (tolower(*us1) != tolower(*us2++))
				return (tolower(*us1) - tolower(*--us2));
			if (*us1++ == '\0')
				break;
		} while (--n != 0);
	}

	return 0;
}

void Scott::readInts(Common::SeekableReadStream *f, size_t count, ...) {
	va_list va;
	va_start(va, count);
	unsigned char c = '\0';

	for (size_t idx = 0; idx < count; ++idx) {
		if (idx > 0) {
			while (f->pos() < f->size() && Common::isSpace(c))
				c = f->readByte();
		} else {
			c = f->readByte();
		}

		// Get the next value
		int *val = (int *)va_arg(va, int);
		*val = 0;
		while (Common::isDigit(c)) {
			*val = (*val * 10) + (c - '0');
			c = f->readByte();
		}
	}

	va_end(va);
}

} // End of namespace Scott
} // End of namespace Gargoyle
