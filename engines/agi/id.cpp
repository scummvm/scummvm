/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/opcodes.h"

namespace Agi {

/*
 * Determine what AGI v2 system to emulate, these are the major version
 * to emulate, thus 2.915 comes under 2.917, 2.4xxx is 2.440, etc.
 *
 * 0x2089
 * 0x2272
 * 0x2440
 * 0x2917
 * 0x2936
 */

const char *ids_database = 
"# CRC	 Int Ver  [options] Game name			# Comment						\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# PC-DOS versions																\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x484AA  0x2440  AGI Demo 1 (PC) 05/87 [AGI 2.425]	# A.K.A. Demo 5				\n"
"0x8CC43  0x2917  AGI Demo 2 (PC 5.25) 11/87 [v1] [AGI 2.915]	# Demo 1		\n"
"0x8856C  0x2917  AGI Demo 2 (PC 3.5) 11/87 [AGI 2.915]							\n"
"0x843AC  0x2917  AGI Demo 2 (PC 5.25) 01/88 [v2] [AGI 2.917]	# Demo 3		\n"
"0x89592  0x3149  AGI Demo 3 (PC) 09/88 [AGI 3.002.102]		# Demo 4			\n"
"0x24A18  0x2440  Black Cauldron (PC) 2.00 6/14/87 [AGI 2.439]					\n"
"0x22B50  0x3149  Black Cauldron (PC 5.25) 2.10 11/10/88 [AGI 3.002.098]		\n"
"0x23E0E  0x3149  Black Cauldron (PC 3.5) 2.10 11/10/88 [AGI 3.002.098]			\n"
"0xB5A25  0x3149  Gold Rush! (PC 5.25) 2.01 12/22/88 [AGI 3.002.149]			\n"
"0xB1C9E  0x3149  Gold Rush! (PC 3.5) 2.01 12/22/88 [AGI 3.002.149]				\n"
"0x49EDA  0x2917  King's Quest 1 (PC 5.25/3.5) 2.0F [AGI 2.917] # also 2.425	\n"
"0x633CB  0x2440  King's Quest 2 (PC 5.25/3.5) 2.1 [AGI 2.411]					\n"
"0x63338  0x2917  King's Quest 2 (PC 5.25/3.5) 2.2 [AGI 2.426]	 # also 2.917	\n"
"0x88673  0x2272  King's Quest 3 (PC) 1.01 11/08/86 [AGI 2.272]					\n"
"0x840D3  0x2440  King's Quest 3 (PC 5.25) 2.00 5/25/87 [AGI 2.435]				\n"
"0x83191  0x2440  King's Quest 3 (PC 3.5) 2.00 5/25/87 [AGI 2.435]				\n"
"0x83695  0x2936  King's Quest 3 (PC 3.5) 2.14 3/15/88 [AGI 2.936]				\n"
"0x8410B  0x2936  King's Quest 3 (PC 5.25) 2.14 3/15/88 [AGI 2.936]				\n"
"0xB124B  0x3086  King's Quest 4 (PC 3.5) 2.0 7/27/88 [AGI 3.002.086]			\n"
"0xB291F  0x3086  King's Quest 4 (PC 3.5) 2.2 9/27/88 [AGI 3.002.086]			\n"
"0xB3722  0x3086  King's Quest 4 (PC 5.25) 2.3 9/27/88 [AGI 3.002.086]			\n"
"0x9CB15  0x3149  King's Quest 4 demo (PC) [AGI 3.002.102]						\n"
"0x6F5E1  0x2440  Leisure Suit Larry 1 (PC 5.25/3.5) 1.00 6/1/87 [AGI 2.440]	\n"
"0x4C16D  0x3149  Manhunter NY (PC 5.25) 1.22 8/31/88 [AGI 3.002.107] # also 3.003.102	\n"
"0x49687  0x3149  Manhunter NY (PC 3.5) 1.22 8/31/88 [AGI 3.002.102]			\n"
"0x53971  0x3149  Manhunter SF (PC 3.5) 3.02 7/26/89 [AGI 3.002.149]			\n"
"0x584F9  0x3149  Manhunter SF (PC 5.25) 3.03 8/17/89 [AGI 3.002.149]			\n"
"0x5D77C  0x2917  Mixed-Up Mother Goose (PC) [AGI 2.915]						\n"
"0x5D7C6	 0x2917  Mixed Up Mother Goose (PC) [AGI 2.915] (Broken)			\n"
"0x7F18B  0x2917  Police Quest 1 (PC) 2.0A 10/23/87 [AGI 2.903/2.911]			\n"
"0x7EF35  0x2917  Police Quest 1 (PC) 2.0E 11/17/87 [AGI 2.915]					\n"
"0x7EF06  0x2917  Police Quest 1 (PC 5.25/ST) 2.0G 12/03/87 [AGI 2.917]			\n"
"0x7E0BC  0x2917  Police Quest 1 (PC 3.5) 2.0G 12/03/87 [AGI 2.917]				\n"
"0x67FCC  0x2089  Space Quest 1 (PC) 1.0 [AGI 2.089]							\n"
"0x68036  0x2089  Space Quest 1 (PC) 1.0X [AGI 2.089]							\n"
"0x67F6E  0x2272  Space Quest 1 (PC) 1.1A [AGI 2.272]							\n"
"0x68244  0x2440  Space Quest 1 (PC 5.25/3.5) 2.2 [AGI 2.426/2.917]				\n"
"0x8DB32  0x2917  Space Quest 2 (PC 5.25) 2.0A [AGI 2.912]						\n"
"0x8D825  0x2917  Space Quest 2 (PC 3.5) 2.0A [AGI 2.912]						\n"
"0x8DA3E  0x2917  Space Quest 2 (PC 5.25/ST) 2.0C/A [AGI 2.915]					\n"
"0x8E6A7  0x2917  Space Quest 2 (PC 3.5) 2.0C/B [AGI 2.917]						\n"
"0x8E29B  0x2936  Space Quest 2 (PC 3.5) 2.0F [AGI 2.936]						\n"
"0x8DF84  0x2936  Space Quest 2 (PC 5.25) 2.0D [AGI 2.936]						\n"
"0x8DE46  0x2936  Space Quest 2 (PC 3.5) 2.0D [AGI 2.936]						\n"
"0x8E310  0x2936  Space Quest 2 (PC 5.25) 2.0F [AGI 2.936]						\n"
"0x31677  0x2272  Xmas Card 1986 (PC) [AGI 2.272]								\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# Apple //gs versions															\n"
"# all guessed interpreter versions												\n"
"#																				\n"
"# Notes:																		\n"
"# - (CE) in Apple IIgs versions stands for Carlos Escobar --PDD				\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x93260  0x2917  AGI Demo 2 (IIgs) 1.0C (Censored)								\n"
"0x285FB  0x3149  Black Cauldron (Apple IIgs) 1.0O 2/24/89 (CE)	# 2.24.89 (CE)	\n"
"0xB6F67  0x3149  Gold Rush! (Apple IIgs) 1.0M 2/28/89 (CE) aka 2.01 12/22/88	\n"
"0x4A9E8  0x2272  King's Quest 1 (IIgs) 1.0S-88223								\n"
"0x79D1B  0x2917  King's Quest 2 (IIgs) 2.0A 6/16/88 (CE)						\n"
"0x85CD4  0x2917  King's Quest 3 (IIgs) 2.0A 8/28/88 (CE)						\n"
"0xAF778  0x3086  King's Quest 4 (IIgs) 1.0K 11/22/88 (CE)						\n"
"0x6E41E  0x2440  Leisure Suit Larry 1 (IIgs) 1.0E								\n"
"0x4C705  0x3149  Manhunter NY (IIgs) 2.0E 10/05/88 (CE)						\n"
"0x5F4E8	 0x2917  Mixed Up Mother Goose (IIgs)								\n"
"0x7DB3F  0x2917  Police Quest 1 (IIgs) 2.0A-88318								\n"
"0x7DBE5  0x2917  Police Quest 1 (IIgs) 2.0B-88421								\n"
"0x69EC0  0x2917  Space Quest 1 (IIgs) 2.2										\n"
"0x8E983  0x2936  Space Quest 2 (IIgs) 2.0A 7/25/88 (CE)						\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# Macintosh versions															\n"
"# all guessed interpreter versions												\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x4C02C  0x2440  King's Quest 1 (Mac) 2.0C										\n"
"0x6382E  0x2440  King's Quest 2 (Mac) 2.0R										\n"
"0x8410B  0x2440  King's Quest 3 (Mac) 2.14 3/15/88								\n"
"0x78202  0x2440  Leisure Suit Larry 1 (Mac) 1.05 6/26/87						\n"
"0x7EF06  0x2440  Police Quest 1 (Mac) 2.0G 12/3/87								\n"
"0x6A277  0x2440  Space Quest 1 (Mac) 1.5D										\n"
"0x8DF84  0x2936  Space Quest 2 (Mac) 2.0D										\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# Atari ST versions															\n"
"# all guessed interpreter versions												\n"
"#																				\n"
"# Notes:																		\n"
"# - Chris Iden wrote the Atari ST port of AGI --PDD							\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x1A7AF  0x2272  Donald Duck's Playground (ST) 1.0A 8/8/86						\n"
"0x4A079  0x2272  King's Quest 1 (ST) 1.0V										\n"
"0x882B7  0x2272  King's Quest 3 (ST) 1.02 11/18/86								\n"
"0xB68AB  0x3149  Gold Rush! (ST) 1.01 1/13/89 aka 2.01 12/22/88				\n"
"0x6F7E1  0x2440  Leisure Suit Larry 1 (ST) 1.04 6/18/87						\n"
"0x4CE19  0x3149  Manhunter NY (ST) 1.03 10/20/88								\n"
"0x5360B  0x3149  Manhunter SF (ST) 1.0 7/29/89									\n"
"0x69597  0x2440  Space Quest 1 (ST) 1.1A										\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# Amiga versions																\n"
"# Use option -A- to enable padding												\n"
"#																				\n"
"# Notes:																		\n"
"# - Amiga KQ3 (2.333) seems to need interpreter version 3.002.086				\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x25640  0x2440  [A] Black Cauldron (Amiga) 2.00 6/14/87	# guessed int		\n"
"0x1AFBA  0x2272  [A] Donald Duck's Playground (Amiga) 1.0C	# guessed int		\n"
"0xB3E1A  0x3149  [A] Gold Rush! (Amiga) 1.01 1/13/89 aka 2.05 3/9/89	# 2.316	\n"
"0x49C6B  0x2440  [A] King's Quest 1 (Amiga) 1.0U		# 2.082					\n"
"0x5D395  0x2440  [A] King's Quest 2 (Amiga) 2.0J		# guessed int			\n"
"0x5BCE6  0x2440	 [A] King's Quest 2 (Amiga) 2.0J (Broken)					\n"
"0x5F4B9  0x2440  [A] King's Quest 2 (Amiga) 2.0J (Broken)	# 2.176				\n"
"0x888C1  0x2440  [A] King's Quest 3 (Amiga) 1.01 11/8/86						\n"
"0x84793  0x3086  [A] King's Quest 3 (Amiga) 2.15 11/15/89	# 2.333				\n"
"0x6FDDB  0x2440  [A] Leisure Suit Larry 1 (Amiga) 1.05 6/26/87	# x.yyy			\n"
"0x4BA94  0x3149  [A] Manhunter NY (Amiga) 1.06 3/18/89		# x.yyy				\n"
"0x53D51  0x3086  [A] Manhunter SF (Amiga) 3.06 8/17/89		# 2.333				\n"
"0x5CFB1  0x3086  [A] Mixed-Up Mother Goose (Amiga) 1.1		# guessed int		\n"
"0x7F752  0x3149  [A] Police Quest 1 (Amiga) 2.0B 2/22/89	# 2.310				\n"
"0x696DD  0x2440  [A] Space Quest 1 (Amiga) 1.2			# 2.082					\n"
"0x8FEA6  0x2936  [A] Space Quest 2 (Amiga) 2.0F			# 2.202				\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# CoCo versions																\n"
"# what version of DOS AGI does CoCo 2.023 correspond with?						\n"
"# guessing 2.272 because the PC version 1.0 is 2.272; doesn't fit date though	\n"
"#																				\n"
"# Notes:																		\n"
"# - Chris Iden wrote the CoCo port of AGI --PDD								\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x7CBE8  0x2272  King's Quest 3 (CoCo3) 1.0C 6/27/88		# 2.023				\n"
"0x70D35  0x2440  Leisure Suit Larry 1 (CoCo3)									\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# AGDS games																	\n"
"# Use option -a- for AGDS games												\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x5501A	    0x2440	[a] Groza			# AGDS sample game					\n"
"																				\n"
"#----------------------------------------------------------------------------	\n"
"# Fan-made AGI games															\n"
"#----------------------------------------------------------------------------	\n"
"																				\n"
"0x3F2F7     0x2917	[m] AGI Mouse 0.7 Demo										\n"
"0x3F744	    0x2917	[m] AGI Mouse 1.0 Demo		# 2.917  6/24/00			\n"
"0x3F74F	    0x2917	[m] AGI Mouse 1.1 Demo		# 2.917  1/01/01			\n"
"0x17599     0x2917	[m] Sliding Tile Game v1.00	# 2.917  6/02/01				\n"
"0x785c4     0x2936	[m] Jolimie v0.6		# 2.936  2000						\n"
"#Jolimie uses AGIPal only and not AGIMouse; no way to separate these currently	\n"
"0x40D80     0x2440	AGI Trek			# 2.440  9/21/98						\n"
"0x64CB7     0x2440	Space Trek 1.0			# 2.440  12/13/98					\n"
"0x6596A     0x2917	Space Trek (remake)		# 2.917  6/09/99					\n"
"0x96909     0x2917	Operation: RECON teaser 1.1	#								\n"
"0x185A6     0x2917	AGI Piano v1.0			#   ?    1998						\n"
"0x91ACF     0x2917	Dave's Quest .07		#   ?								\n"
"0x620F6     0x2917	Time Quest demo D0.2		#   ?    1998					\n"
"0x7466F     0x2917	Tex McPhilip I			#   ?    2000						\n"
"0x9E400     0x2917	Tex McPhilip II			#   ?    2000						\n"
"0xAB9A8     0x2917	Justin Quest 1.0		# 2.917								\n"
"0x7D473     0x2917	The Ruby Cast demo 0.2		# 2.917  1998					\n"
"0xB4D7A     0x2917	Residence 44 Quest 1.0a		# 2.917  1999					\n"
"0x5D077     0x2917	Escape Quest demo		#   ?    1998						\n"
"0x5A434     0x2917	Acidopolis (1.0) demo		#   ?							\n"
"0x45CDF     0x2917	Go West, Young Hippie demo	# 2.917							\n"
"0x4C9DC     0x2917	Speeder Bike Challenge v1.0	#   ?							\n"
"0x112BF9    0x2917	Space Quest 0: Replicated 1.04	#        6/27/2003			\n"
"0x6E70F     0x2917	Space Quest: The Lost Chapter v10.0							\n"
"0x5859E     0x2917	Phantasmagoria												\n"
"0x7B5DF     0x2917	Dashiki demo												\n"
"0x9405B     0x2917	Dashiki 256-color demo (Unsupported)						\n"
"0x6ADCD     0x2917	Jen's Quest 0.1 demo										\n"
"0x4EE64     0x2917	Monkey Man													\n"
"";

int setup_v2_game(int ver, uint32 crc);
int setup_v3_game(int ver, uint32 crc);
int v4id_game(uint32 crc);

uint32 match_crc(uint32 crc, char *name, int len) {
	char *c, *t, buf[256];
	uint32 id, ver;

	Common::MemoryReadStream f((const byte *)ids_database, strlen(ids_database));

	while (!f.eos()) {
		f.readLine(buf, 256);
		c = strchr(buf, '#');
		if (c)
			*c = 0;

		/* Remove spaces/tabs at end of line */
		if (strlen(buf)) {
			int i;
			for (i = strlen(buf) - 1;
				 i >= 0 && (buf[i] == ' ' || buf[i] == '\t');
				 buf[i--] = 0) { }
		}

		t = strtok(buf, " \t\r\n");
		if (t == NULL)
			continue;
		id = strtoul(t, NULL, 0);

		t = strtok(NULL, " \t\r\n");
		if (t == NULL)
			continue;
		ver = strtoul(t, NULL, 0);

		t = strtok(NULL, "\n\r");
		for (; *t == ' ' || *t == '\t'; t++);

		if (id == crc) {
			/* Now we must check options enclosed in brackets
			 * like [A] for Amiga
			 */

			if (*t == '[') {
				while (*t != ']') {
					switch (*t++) {
					case 'A':
						opt.amiga = true;
						break;
					case 'a':
						opt.agds = true;
						break;
					case 'm':
						opt.agimouse = true;
						break;
					}
				}
				t++;

				for (; (*t == ' ' || *t == '\t') && *t; t++) {
				}
			}

			strncpy(name, t, len);
			return ver;
		}
	}

	return 0;
}

static uint32 match_version(uint32 crc) {
	int ver;
	char name[80];

	if ((ver = match_crc(crc, name, 80)) > 0)
		report("AGI game detected: %s\n\n", name);

	return ver;
}

int v2id_game() {
	int y, ver;
	uint32 len, c, crc;
	uint8 *buff;
	Common::File fp;
	const char *fn[] = { "viewdir", "logdir", "picdir", "snddir", "words.tok", "object", "" };

	buff = (uint8 *)malloc(8192);

	for (crc = y = 0; fn[y][0]; y++) {
		if (fp.open(fn[y])) {
			for (len = 1; len > 0;) {
				memset(buff, 0, 8192);
				len = fp.read(buff, 8000);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fp.close();
		}
	}
	free(buff);

	report("Computed CRC: 0x%05x\n", crc);
	ver = match_version(crc);
	game.crc = crc;
	game.ver = ver;
	debugC(2, kDebugLevelMain, "game.ver = 0x%x", game.ver);
	agi_set_release(ver);
	return setup_v2_game(ver, crc);
}

/*
 * Currently, there is no known difference between v3.002.098 -> v3.002.149
 * So version emulated;
 *
 * 0x0086,
 * 0x0149
 */

int v3id_game() {
	int ec = err_OK, y, ver;
	uint32 len, c, crc;
	uint8 *buff;
	Common::File fp;
	const char *fn[] = { "words.tok", "object", "" };
	Common::String path;

	buff = (uint8 *)malloc(8192);

	for (crc = 0, y = 0; fn[y][0] != 0x0; y++) {
		if (fp.open(fn[y])) {
			len = 1;
			while (len > 0) {
				memset(buff, 0, 8192);
				len = fp.read(buff, 8000);
				for (c = 0; c < len; c++)
					crc += *(buff + c);
			}
			fp.close();
		}
	}

	/* now do the directory file */

	path = Common::String(game.name) + DIR_;

	if (fp.open(path)) {
		for (len = 1; len > 0;) {
			memset(buff, 0, 8192);
			len = fp.read(buff, 8000);
			for (c = 0; c < len; c++)
				crc += *(buff + c);
		}
		fp.close();
	}

	free(buff);

	report("Computed CRC: 0x%05x\n", crc);
	ver = match_version(crc);
	game.crc = crc;
	game.ver = ver;
	agi_set_release(ver);

	ec = setup_v3_game(ver, crc);

	return ec;
}

/**
 *
 */
int setup_v2_game(int ver, uint32 crc) {
	int ec = err_OK;

	if (ver == 0) {
		report("Unknown v2 Sierra game: %08x\n\n", crc);
		agi_set_release(0x2917);
	}

	/* setup the differences in the opcodes and other bits in the
	 * AGI v2 specs
	 */
	if (opt.emuversion)
		agi_set_release(opt.emuversion);

	if (opt.agds)
		agi_set_release(0x2440);	/* ALL AGDS games built for 2.440 */

	switch (agi_get_release()) {
	case 0x2089:
		logic_names_cmd[0x86].num_args = 0;	/* quit: 0 args */
		logic_names_cmd[0x97].num_args = 3;	/* print.at: 3 args */
		logic_names_cmd[0x98].num_args = 3;	/* print.at.v: 3 args */
		break;
	case 0x2272:
		/* KQ3 0x88673 (2.272) requires print.at with 4 arguments */
		break;
	case 0x2440:
		break;
	case 0x2917:
		break;
	case 0x2936:
		break;
	default:
		report("** Cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

/**
 *
 */
int setup_v3_game(int ver, uint32 crc) {
	int ec = err_OK;

	if (ver == 0) {
		report("Unknown v3 Sierra game: %08x\n\n", crc);
		agi_set_release(ver = 0x3149);
	}

	if (opt.emuversion)
		agi_set_release(ver = opt.emuversion);

	switch (ver) {
	case 0x3086:
		logic_names_cmd[0xad].num_args = 1;	/* 173 : 1 args */
		break;
	case 0x3149:
		logic_names_cmd[0xad].num_args = 0;	/* 173 : 0 args */
		break;
	default:
		report("Error: cannot setup for unknown version\n");
		ec = err_UnknownAGIVersion;
		break;
	}

	return ec;
}

}                             // End of namespace Agi
