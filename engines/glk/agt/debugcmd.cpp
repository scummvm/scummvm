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
#include "glk/agt/interp.h"
#include "glk/agt/exec.h"

namespace Glk {
namespace AGT {

static void d_moveobj(int obj, int dest)
/* 1=the player, -1=unknown: ask */
{
	if (obj == -1) {
		writestr("Which object? ");
		obj = read_number();
		if (obj != 1 && !tnoun(obj) && !tcreat(obj)) {
			writeln("Invalid object");
			return;
		}
	}
	if (dest == -1) {
		writestr("To where? ");
		dest = read_number();
		if (dest != 1 && dest != 0 && !tnoun(dest) && !tcreat(dest) && !troom(dest)) {
			writeln("Invalid object");
			return;
		}
	}
	if (obj != 1)
		it_move(obj, dest);
	else  {
		if (!troom(dest)) {
			writeln("Player can only be moved to a room");
			return;
		}
		goto_room(dest - first_room);
	}
}

static int print_objid(int obj) {
	char buff[10];
	char *s;
	int n;

	sprintf(buff, "%4d: ", obj);
	writestr(buff);
	s = objname(obj);
	for (n = 0; s[n] != 0; n++)
		if (s[n] <= 8 || (uchar)s[n] == 0xFF) s[n] = ' '; /* Strip out format codes */
	writestr(s);
	n = strlen(s);
	rfree(s);
	return n;
}

static void d_listroom() {
	int i;

	writeln("       ROOM");
	writeln("      ------");
	writeln("");
	for (i = 0; i <= maxroom - first_room; i++) {
		print_objid(i + first_room);
		writeln("");
	}
}

#define SEPLENG 27  /* Width between beginning of object column and
               location column */

static void d_listnoun() {
	int i;
	int len;

	writestr("      NOUN ");
	padout(SEPLENG - 6);
	writeln("     LOCATION ");
	writestr("     ------");
	padout(SEPLENG - 6);
	writeln("    ----------");
	writeln("");
	len = SEPLENG - print_objid(1);
	padout(len);
	writestr("[");
	print_objid(loc);
	writeln("]");

	nounloop(i) {
		len = print_objid(i + first_noun);
		len = SEPLENG - len;
		if (len > 0) padout(len);
		writestr("[");
		print_objid(noun[i].location);
		writeln("]");
	}
}

static void d_listcreat() {
	int i;
	int len;

	writestr("        CREATURE ");
	padout(SEPLENG - 11);
	writeln("     LOCATION ");
	writestr("       ----------");
	padout(SEPLENG - 11);
	writeln("    ----------");
	writeln("");

	creatloop(i) {
		len = print_objid(i + first_creat);
		len = SEPLENG - len;
		if (len > 0) padout(len);
		writestr("    [");
		print_objid(creature[i].location);
		writeln("]");
	}
}

static void writetbl(const char *s, int width)
/* This writes out s and then prints out any additional spaces needed
   to make the output string *width* wide. */
{
	writestr(s);
	width = width - strlen(s);
	if (width > 0) padout(width);
}

static void var_edit(int vtype)
/* vtype=0 for variable, 1 for counter, 2 for flag */
{
	long n;
	int i;
	int imax;
	char sbuff[30];

	switch (vtype) {
	case 0:
		imax = VAR_NUM;
		break;
	case 1:
		imax = CNT_NUM;
		break;
	case 2:
		imax = FLAG_NUM;
		break;
	default:
		writeln("INTERNAL ERROR: Invalid vtype.");
		return;
	}

	for (;;) {
		agt_clrscr();
		writeln("");
		switch (vtype) {
		case 0:
			writeln("Variables");
			break;
		case 1:
			writeln("Counters (-1 means the counter is off)");
			break;
		case 2:
			writeln("Flags ( f=false [OFF]  and t=true [ON] )");
			break;
		default:
			break;
		}
		writeln("");
		for (i = 0; i <= imax; i++) {
			switch (vtype) {
			case 0:
				sprintf(sbuff, "[Var%3d]=%4ld", i, (long)agt_var[i]);
				break;
			case 1:
				sprintf(sbuff, "[Cnt%3d]=%4ld", i, (long)agt_counter[i]);
				break;
			case 2:
				sprintf(sbuff, "%3d%c", i, flag[i] ? 't' : 'f');
				break;
			default:
				break;
			}
			writetbl(sbuff, (vtype == 2) ? 5 : 20);
		}
		writeln("");
		writeln("");
		for (;;) {
			switch (vtype) {
			case 0:
				writestr("Variable to change");
				break;
			case 1:
				writestr("Counter to change");
				break;
			case 2:
				writestr("Flag to toggle");
				break;
			default:
				break;
			}
			writestr(" (-1 to quit): ");
			i = read_number();
			if (i < 0) return;
			if (i <= imax) {
				if (vtype != 2) {
					if (vtype == 0)
						sprintf(sbuff, "[Var%d]=%ld", i, (long)agt_var[i]);
					else sprintf(sbuff, "[Cnt%d]=%ld (-1 means it's off)",
						             i, (long)agt_counter[i]);
					writestr(sbuff);
					writestr("; new value = ");
					n = read_number();
					if (vtype == 0)
						agt_var[i] = n;
					else if (n < -1 || n > (((long)1) << 15) - 1)
						writeln("Invalid value for a counter.");
					else agt_counter[i] = n;
				} else flag[i] = !flag[i];
				break;
			} else
				writeln("Invalid index.");
		}
	}
}

/* Routines to edit user strings */
static void edit_str() {
	int i, j;
	char buff[10];
	char *tmpstr;

	if (MAX_USTR == 0 || userstr == NULL) {
		writeln("This game doesn't contain any user strings");
		return;
	}
	for (;;) {
		agt_clrscr();
		writeln("User Definable Strings");
		writeln("");
		for (i = 0; i < MAX_USTR; i++) {
			sprintf(buff, "%2d:", i + 1);
			writestr(buff);
			writeln(userstr[i]);
		}
		writestr(" (0 to quit): ");
		i = read_number();
		if (i == 0) return;
		if (i > 0 && i <= MAX_USTR) {
			writeln("Enter new string:");
			tmpstr = agt_readline(3);
			j = strlen(tmpstr) - 1;
			if (j > 0 && tmpstr[j] == '\n') tmpstr[j] = 0;
			strncpy(userstr[i - 1], tmpstr, 80);
		} else writeln("Invalid string number");
	}
}

static uchar attrcol;  /* Determines which column the attribute is put in */
static uchar attrwidth; /* Number of attribute columns */

static void next_col() {
	if (++attrcol == attrwidth) {
		writeln("");
		attrcol = 0;
	} else
		padout(10);
}

static void writeattr(const char *attrname, rbool attrval) {
	writestr(attrname);
	padout(15 - strlen(attrname));
	if (attrval) writestr("yes");
	else writestr("no ");
	next_col();
}

static void writegender(const char *gendername, uchar genderval) {
	writestr(gendername);
	padout(15 - strlen(gendername) - 3);
	switch (genderval) {
	case 2:
		writestr("Male  ");
		break;
	case 1:
		writestr("Female");
		break;
	case 0:
		writestr("Thing");
		break;
	default:
		break;
	}
	next_col();
}

static void writeprop(const char *propname, int obj) {
	writestr(propname);
	writestr(" [");
	print_objid(obj);
	writeln("]");
}

static int writedir(int index, int dir, int obj) {
	char sbuff[40];

	sprintf(sbuff, "%2d.%-2s %d", index, exitname[dir], obj);
	writestr(sbuff);
	return strlen(sbuff);
}

void writenum(const char *propname, int n) {
	char sbuff[20];

	writestr(propname);
	sprintf(sbuff, "%4d", n);
	writeln(sbuff);
}

static void writeflags(const char *flagname, int32 flags) {
	int i;
	char sbuff[5];

	writestr(flagname);
	for (i = 0; i < 32; i++) {
		if (flags & 1) {
			sprintf(sbuff, "%2d ", i);
			writestr(sbuff);
		} else
			writestr("   ");
		flags >>= 1;
		if (i % 12 == 11) {
			writeln("");
			padout(strlen(flagname));
		}
	}
	writeln("");
}

static void readflags(int32 *flags) {
	long n;

	writestr("Room flag to toggle (0-31)? ");
	n = read_number();
	if (n <= 31 && n >= 0)
		*flags ^= (((long)1) << n);
}

static long readval(const char *prompt, int type) {
	long val;

	for (;;) {
		writestr(prompt);
		writestr(" ");
		val = read_number();
		if (argvalid(type, val)) return val;
		writeln("Invalid value.");
	}
}

static uchar readgender() {
	char c;

	writestr("Gender (M/F/N): ");
	for (;;) {
		c = tolower(agt_getchar());
		switch (c) {
		case 'm':
			return 2;
		case 'w':
		case 'f':
			return 1;
		case 'n':
		case 't':
			return 0;
		default: ;/* Do nothing */
		}
	}
}

static void edit_objattr(int obj) {
	int i, k, kprop, n;
	long v;

	for (;;) {
		k = 1;
		agt_clrscr();
		print_objid(obj);
		writeln("");
		if (oflag_cnt > 0) {
			writeln("ObjFlags:");
			for (i = 0; i < oflag_cnt; i++)
				if (have_objattr(0, obj, i)) {
					v = op_objflag(2, obj, i);
					rprintf("%2d. ObjProp%2d:%c %-40s\n", k++, i, (v ? '+' : '-'),
					        get_objattr_str(AGT_OBJFLAG, i, v));
				}
			writeln("");
		}
		kprop = k;
		if (oprop_cnt > 0) {
			writeln("ObjProps:");
			for (i = 0; i < oprop_cnt; i++)
				if (have_objattr(1, obj, i)) {
					v = op_objprop(2, obj, i, 0);
					rprintf("%2d. ObjFlag%2d: [%3ld] %-40s\n", k++, i, v,
					        get_objattr_str(AGT_OBJPROP, i, v));
				}
			writeln("");
		}
		writestr("Field to change (0 to return to main view)? ");
		n = read_number();
		if (n == 0) return;
		if (n < 1 || n >= k) continue;
		k = 0;
		if (n < kprop) { /* Attribute */
			for (i = 0; i < oflag_cnt; i++)
				if (have_objattr(0, obj, i))
					if (n == ++k) break;
			if (n == k && have_objattr(0, obj, i))
				op_objflag(3, obj, i); /* Toggle it */
		} else { /* Property */
			for (i = 0; i < oprop_cnt; i++)
				if (have_objattr(1, obj, i))
					if (n == ++k) break;
			if (n == k && have_objattr(1, obj, i))
				op_objprop(1, obj, i, readval("New value:", AGT_NUM));
		}
	}
}

static void room_edit(int i) {
	int n, j;

	for (;;) {
		agt_clrscr();
		writestr("ROOM ");
		print_objid(i + first_room);
		writeln("");
		writeln("");
		attrcol = 0;
		attrwidth = 2;
		writeattr("1.*WinGame:", room[i].win);
		writeattr("4. Seen:", room[i].seen);
		writeattr("2.*EndGame:", room[i].end);
		writeattr("5. LockedDoor:", room[i].locked_door);
		writeattr("3.*Die:", room[i].killplayer);
		writeln("");
		writeln("");
		writeprop("6.*Key    =", room[i].key);
		writeprop("7. Light  =", room[i].light);
		writenum("8. Points =", room[i].points);
		writeprop("9. Class = ", room[i].oclass);
		writeln("");
		writeln("EXITS:");
		for (j = 0; j < 12; j++) {
			n = writedir(j + 10, j, room[i].path[j]);
			if (j % 4 == 3) writeln("");
			else padout(15 - n);
		}
		writeprop("22. SPECIAL:", room[i].path[12]);
		writeflags("23. Room Flags:", room[i].flag_noun_bits);
		writeln("24. Object properties and attributes.");
		writeln("");
		writeln("(Fields marked with an * are not saved or restored.)");
		/* writeln(""); */
		writestr("Field to change (0 to exit)? ");
		n = read_number();
		if (n == 0) return;
		switch (n) {
		case 1:
			room[i].win = !room[i].win;
			break;
		case 2:
			room[i].end = !room[i].end;
			break;
		case 3:
			room[i].killplayer = !room[i].killplayer;
			break;
		case 4:
			room[i].seen = !room[i].seen;
			break;
		case 5:
			room[i].locked_door = !room[i].locked_door;
			break;
		case 6:
			room[i].key = readval("Key = ", AGT_ITEM | AGT_NONE);
			break;
		case 7:
			room[i].light = readval("Light = ", AGT_ITEM | AGT_NONE | AGT_SELF);
			break;
		case 8:
			room[i].points = readval("Points = ", AGT_NUM);
			break;
		case 9:
			room[i].oclass = readval("Class = ", AGT_ROOM | AGT_NONE);
			break;
		case 22:
			room[i].path[12] = readval("SPECIAL: ", AGT_NUM);
			break;
		case 23:
			readflags(&room[i].flag_noun_bits);
			break;
		case 24:
			edit_objattr(i + first_room);
			break;
		default:
			if (n >= 10 && n < 22) { /* Direction */
				room[i].path[n - 10] = readval(exitname[n - 10], AGT_NUM);
			} else writeln("Invalid field");
		}
	}
}

#define tog(x) {x=!x;break;}

static void noun_edit(int i) {
	int n;

	for (;;) {
		agt_clrscr();
		/* writeln("");*/
		writestr("NOUN       ");
		print_objid(i + first_noun);
		/*    writeln("");*/
		/* writeln("");*/
		writeprop("      Location=", noun[i].location);
		writeln("");
		attrcol = 0;
		attrwidth = 3;
		writeattr(" 1.*Pushable:", noun[i].pushable);
		writeattr(" 8.*Lockable:", noun[i].lockable);
		writeattr("15.*Drinkable:", noun[i].drinkable);
		writeattr(" 2.*Pullable:", noun[i].pullable);
		writeattr(" 9.*Light:", noun[i].light);
		writeattr("16.*Poisonous:", noun[i].poisonous);
		writeattr(" 3.*Turnable:", noun[i].turnable);
		writeattr("10.*Plural:", noun[i].plural);
		writeattr("17. Open:", noun[i].open);
		writeattr(" 4.*Playable:", noun[i].playable);
		writeattr("11. Movable:", noun[i].movable);
		writeattr("18. Locked:", noun[i].locked);
		writeattr(" 5.*Readable:", noun[i].readable);
		writeattr("12.*Shootable:", noun[i].shootable);
		writeattr("19.*Win Game:", noun[i].win);
		writeattr(" 6.*Wearable:", noun[i].wearable);
		writeattr("13. On:", noun[i].on);
		writeattr("20.*Global:", noun[i].isglobal);
		writeattr(" 7.*Closable:", noun[i].closable);
		writeattr("14.*Edible:", noun[i].edible);
		writeattr("21.*Proper:", noun[i].proper);

		writeln("");
		writenum("22. Shots  =", noun[i].num_shots);
		writenum("23. Points =", noun[i].points);
		writenum("24. Weight =", noun[i].weight);
		writenum("25. Size   =", noun[i].size);
		writeprop("26.*Key    =", noun[i].key);
		writeprop("27. Class  =", noun[i].oclass);
		writenum("28. Flag   =", noun[i].flagnum);
		writeln("");
		/* writeln(""); */
		writeln("29. Object properties and attributes.");
		writeln("");
		writeln("(Fields marked with an * are not saved or restored.)");
		writestr("Field to change (0 to exit)? ");
		n = read_number();
		if (n == 0) return;
		switch (n) {
		case 1:
			tog(noun[i].pushable); /* tog() macro includes break */
		case 2:
			tog(noun[i].pullable);
		case 3:
			tog(noun[i].turnable);
		case 4:
			tog(noun[i].playable);
		case 5:
			tog(noun[i].readable);
		case 6:
			tog(noun[i].wearable);
		case 7:
			tog(noun[i].closable);
		case 8:
			tog(noun[i].lockable);
		case 9:
			tog(noun[i].light);
		case 10:
			tog(noun[i].plural);
		case 11:
			tog(noun[i].movable);
		case 12:
			tog(noun[i].shootable);
		case 13:
			tog(noun[i].on);
		case 14:
			tog(noun[i].edible);
		case 15:
			tog(noun[i].drinkable);
		case 16:
			tog(noun[i].poisonous);
		case 17:
			tog(noun[i].open);
		case 18:
			tog(noun[i].locked);
		case 19:
			tog(noun[i].win);
		case 20:
			tog(noun[i].isglobal);
		case 21:
			tog(noun[i].proper);

		case 22:
			noun[i].num_shots = readval("Shots =", AGT_NUM);
			break;
		case 23:
			noun[i].points = readval("Points =", AGT_NUM);
			break;
		case 24:
			noun[i].weight = readval("Weight =", AGT_NUM);
			break;
		case 25:
			noun[i].size = readval("Size =", AGT_NUM);
			break;
		case 26:
			noun[i].key = readval("Key =", AGT_ITEM | AGT_NONE);
			break;
		case 27:
			noun[i].oclass = readval("Class =", AGT_ITEM | AGT_NONE);
			break;
		case 28:
			noun[i].flagnum = readval("Flag Number=", AGT_ROOMFLAG);
			break;
		case 29:
			edit_objattr(i + first_noun);
			break;
		default:
			writeln("Invalid field");
		}
	}
}

static void creat_edit(int i) {
	int n;

	for (;;) {
		agt_clrscr();
		writestr("CREATURE    ");
		print_objid(i + first_creat);
		writeln("");
		writeln("");
		writeprop("Location =", creature[i].location);
		writeln("");
		attrcol = 0;
		attrwidth = 2;
		writeattr(" 1. Hostile:", creature[i].hostile);
		writeattr(" 4. Global:", creature[i].isglobal);
		writeattr(" 2. Grp member:", creature[i].groupmemb);
		writeattr(" 5.*Proper:", creature[i].proper);
		writegender(" 3.*Gender:", creature[i].gender);
		writeln("");
		writeln("");
		writeprop(" 6.*Weapon = ", creature[i].weapon);
		writenum(" 7. Points = ", creature[i].points);
		writenum(" 8.*Attack Threshold  = ", creature[i].threshold);
		writenum(" 9.   Attack counter  = ", creature[i].counter);
		writenum("10.*Attack Time Limit = ", creature[i].timethresh);
		writenum("11.   Attack timer    = ", creature[i].timecounter);
		writeprop("12. Class  = ", creature[i].oclass);
		writenum("13. Flag   = ", creature[i].flagnum);
		writeln("");
		writeln("14. Object properties and attributes.");
		writeln("");
		writeln("(Fields marked with an * are not saved or restored.)");
		writeln("");
		writestr("Field to change (0 to exit)? ");
		n = read_number();
		if (n == 0) return;
		switch (n) {
		case 1:
			tog(creature[i].hostile);
		case 2:
			tog(creature[i].groupmemb);
		case 3:
			tog(creature[i].isglobal);
		case 4:
			tog(creature[i].proper);

		case 5:
			creature[i].gender = readgender();
			break;
		case 6:
			creature[i].weapon = readval("Weapon =", AGT_ITEM | AGT_NONE);
			break;
		case 7:
			creature[i].points = readval("Points =", AGT_NUM);
			break;
		case 8:
			creature[i].threshold = readval("Threshold =", AGT_NUM);
			break;
		case 9:
			creature[i].counter = readval("Attack counter =", AGT_NUM);
			break;
		case 10:
			creature[i].timethresh = readval("Time limit =", AGT_NUM);
			break;
		case 11:
			creature[i].timecounter = readval("Timer =", AGT_NUM);
			break;
		case 12:
			creature[i].oclass = readval("Class =", AGT_ITEM | AGT_NONE);
			break;
		case 13:
			noun[i].flagnum = readval("Flag Number=", AGT_ROOMFLAG);
			break;
		case 14:
			edit_objattr(i + first_creat);
			break;
		default:
			writeln("Invalid field");
		}
	}
}

#undef tog


static void obj_edit() {
	int n;

	for (;;) {
		writeln("");
		do {
			writestr("Enter object number (0 to exit)? ");
			n = read_number();
			if (n <= 0) return;
		} while (!troom(n) && !tnoun(n) && !tcreat(n));

		if (troom(n)) room_edit(n - first_room);
		else if (tnoun(n)) noun_edit(n - first_noun);
		else if (tcreat(n)) creat_edit(n - first_creat);
		else writeln("[Not yet implemented]");

	}
}

static const char *yesnostr[] = { "No", "Yes" };

static void set_debug_options() {
	char buff[80];
	int n;

	for (;;) {
		agt_clrscr();
		writeln("DEBUGGING OPTIONS:");
		writeln("");
		sprintf(buff, "  1. Trace metacommands: %s", yesnostr[DEBUG_AGT_CMD]);
		writeln(buff);
		sprintf(buff, "  2. Trace ANY metacommands: %s", yesnostr[debug_any]);
		writeln(buff);
		sprintf(buff, "  3. Trace during disambiguation: %s",
		        yesnostr[debug_disambig]);
		writeln(buff);
		writeln("");
		writeln("(<2> and <3> are ignored if <1> is not set; option <1> can"
		        " also be changed from the main debugging menu)");
		writeln("");
		writestr("Option to toggle (0 to exit): ");
		n = read_number();
		switch (n) {
		case 0:
			return;
		case 1:
			DEBUG_AGT_CMD = !DEBUG_AGT_CMD;
			break;
		case 2:
			debug_any = !debug_any;
			break;
		case 3:
			debug_disambig = !debug_disambig;
			break;
		default:
			writeln("Not a valid option");
		}
	}
}

void get_debugcmd() {
	int n;

	for (;;) {
		writeln("DEBUGGING COMMANDS");
		writeln("");
		writeln("1. Move player          8. List Rooms");
		writeln("2. Get Noun             9. List Nouns");
		writeln("3. Move object         10. List Creatures");
		writeln("4. View/Edit object    11. List/Set Flags");
		writeln("5. Toggle Trace        12. List/Set Variables");
		writeln("6. Set Debug Options   13. List/Set Counters");
		writeln("7. Edit User Strings");
		writeln("");
		writestr("Enter choice (0 to exit): ");
		n = read_number();
		switch (n) {
		case -1:
		case 0:
			return;
		case 1:
			d_moveobj(1, -1);
			break;
		case 2:
			d_moveobj(-1, 1);
			break;
		case 3:
			d_moveobj(-1, -1);
			break;
		case 4:
			obj_edit();
			break;
		case 5:
			DEBUG_AGT_CMD = !DEBUG_AGT_CMD;
			break;
		case 6:
			set_debug_options();
			break;
		case 7:
			edit_str();
			break;
		case 8:
			d_listroom();
			break;
		case 9:
			d_listnoun();
			break;
		case 10:
			d_listcreat();
			break;
		case 11:
			var_edit(2);
			break;
		case 12:
			var_edit(0);
			break;
		case 13:
			var_edit(1);
			break;
		default:
			writeln("Not a valid option");
		}
		writeln("");
	};
}

} // End of namespace AGT
} // End of namespace Glk
