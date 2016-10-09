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
 * This is a utility for extracting needed resource data from different language
 * version of the Mortevielle executable files into a new file mort.dat - this
 * is required for the ScummVM Mortevielle module to work properly
 */

#ifndef ENGINEDATA_H
#define ENGINEDATA_H

const char *engineDataEn[] =  {
	"[2][ ][YES][NO]",
	"Go to",
	"Someone enters, looks surprised, but says nothing",
	"   Cool   ",
	"Oppressive",
	"  Tense   ",
	"Idem",
	"You",
	"are",
	"Alone",

	"Gosh! You hear some noise...",
	" | You should have noticed,   |       ",
	"% of hints...",
	"Do you want to wake up?",
	"OK",
	"",
	" Save",

	" Load",
	" Restart",
	"F3: Repeat",
	"F8: Proceed",
	"Hide self",
	"take",
	" probe    ",
	" raise    ",
	" -MORE- ",
	" -STOP-  ",
	"[1] [So, use the DEP menu] [Ok]",
	"lift",
	"read",

	"look",
	"search",
	"open",
	"put",
	"turn",
	"tie",
	"close",
	"hit",
	"pose",
	"smash",

	"smell",
	"scratch",
	"probe",
	"[1] [ | Before, use the DEP menu...] [Ok]",
	"& day",
	NULL
};

const char *engineDataFr[] = {
	"[2][ ][OUI][NON]",
	"aller",
	"quelqu'un entre, parait \202tonn\202 mais ne dit rien",
	"Cool",
	" Lourde ",
	"Malsaine",
	"Idem",
	"Vous",
	"\210tes",
	"SEUL",

	"Mince! Vous entendez du bruit...",
	" | Vous devriez avoir remarqu\202|       ",
	"% des indices...",
	"D\202sirez-vous vous r\202veiller?",
	"OK",
	"",
	" Sauvegarde",

	" Chargement",
	" Recommence  ",
	"F3: Encore",
	"F8: Suite",
	"se cacher",

	"prendre",
	"sonder",
	"soulever",
	" -SUITE- ",
	" -STOP-  ",
	"[1][Alors, utilisez le menu DEP...][ok]",
	"soulever",
	"lire",

	"regarder",
	"fouiller",
	"ouvrir",
	"mettre",
	"tourner",
	"attacher",
	"fermer",
	"frapper",
	"poser",
	"d\202foncer",

	"sentir",
	"gratter",
	"sonder",
	"[1][ | Avant, utilisez le menu DEP...][ok]",
	"& jour",
	NULL
};

const char *engineDataDe[] =  {
	"[2][ ][JA][NEIN]",
	"gehen",
	"Jemand kommt herein, scheint erstaunt, sagt nichts",
	"Cool",
	"Schwer",
	"Ungesund",
	"Idem",
	"Sie",
	"sind",
	"allein",

	"Verdammt! Sie hoeren ein Geraeush...",
	"Sie haetten ",
	"% der Hinweise|      bemerken muessen...",
	"Moechten Sie aufwachen?",
	"OK",
	"",
	" schreiben",

	" lesen",
	" wieder      ",
	"F3: nochmals",
	"F8: stop",
	" sich verstecken",
	" nehmen",
	" sondieren",
	" hochheben",
	" -WEITER- ",
	" -STOP-  ",
	"[1][ Benutzen Sie jetzt das Menue DEP...][OK]",
	"hochheben",
	"lesen",

	"anschauen",
	"durchsuchen",
	"oeffnen",
	"setzen",
	"drehen",
	"befestigen",
	"schliessen",
	"klopfen",
	"hinlegen",
	"eindruecken",

	"fuehlen",
	"abkratzen",
	"sondieren",
	"[1][ Benutzen Sie jetzt das Menue DEP...][OK]",
	"& tag",
	NULL
};

#endif
