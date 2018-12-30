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
 *
 *              Originally written by Syn9 in FreeBASIC with SDL
 *              http://syn9.thehideoutgames.com/index_backup.php
 *
 *            Ported to plain C for GCW-Zero handheld by Dmitry Smagin
 *                http://github.com/dmitrysmagin/griffon_legend
 *
 *
 *                 Programming/Graphics: Daniel "Syn9" Kennedy
 *                     Music/Sound effects: David Turner
 *
 *                   Beta testing and gameplay design help:
 *                    Deleter, Cha0s, Aether Fox, and Kiz
 *
 */

#include "griffon/griffon.h"
#include "griffon/state.h"

namespace Griffon {

char player_sav[256] = "data/player%i.sav";

#define PRINT(A,B)          \
	do {                    \
		char line[256];         \
		sprintf(line, A "\n", B);   \
		fputs(line, fp);        \
	} while(0)

#define INPUT(A, B)         \
	do {                    \
		char line[256];         \
		fgets(line, sizeof(line), fp);  \
		sscanf(line, A, B);     \
	} while(0)

// externs (to be removed later)
extern PLAYERTYPE playera;
extern int asecstart;

int state_load(int slotnum) {
#if 0
	FILE *fp;
	char line[256];

	sprintf(line, player_sav, slotnum);

	fp = fopen(line, "r");
	if (fp) {
		INPUT("%i", &player.level);

		if (player.level > 0) {
			INPUT("%i", &secstart);
			INPUT("%s", line);

			INPUT("%f", &player.px);
			INPUT("%f", &player.py);
			INPUT("%f", &player.opx);
			INPUT("%f", &player.opy);
			INPUT("%i", &player.walkdir);
			INPUT("%f", &player.walkframe);
			INPUT("%f", &player.walkspd);
			INPUT("%f", &player.attackframe);
			INPUT("%f", &player.attackspd);
			INPUT("%i", &player.hp);
			INPUT("%i", &player.maxhp);
			INPUT("%f", &player.hpflash);
			INPUT("%i", &player.level);
			INPUT("%i", &player.maxlevel);
			INPUT("%i", &player.sword);
			INPUT("%i", &player.shield);
			INPUT("%i", &player.armour);
			for (int i = 0; i < 5; i++) {
				INPUT("%i", &player.foundspell[i]);
				INPUT("%f", &player.spellcharge[i]);
			}
			for (int a = 0; a < 5; a++) {
				INPUT("%i", &player.inventory[a]);
			}
			INPUT("%f", &player.attackstrength);
			INPUT("%i", &player.spelldamage);
			INPUT("%i", &player.sworddamage);
			INPUT("%i", &player.exp);
			INPUT("%i", &player.nextlevel);
			for (int a = 0; a <= 99; a++) {
				for (int b = 0; b <= 9; b++) {
					INPUT("%i", &scriptflag[a][b]);
				}
			}
			INPUT("%i", &curmap);

			for (int a = 0; a <= 999; a++) {
				for (int b = 0; b <= 20; b++) {
					for (int c = 0; c <= 14; c++) {
						INPUT("%i", &objmapf[a][b][c]);
					}
				}
			}

			for (int a = 0; a <= 200; a++) {
				INPUT("%i", &roomlocks[a]);
			}

			INPUT("%f", &player.spellstrength);

			fclose(fp);

			return 1; // success
		}

		fclose(fp);
	}

#endif
	return 0; // fail
}

/* fill PLAYERTYPE playera; */
int state_load_player(int slotnum) {
#if 0
	FILE *fp;
	char line[256];

	sprintf(line, player_sav, slotnum);

	playera.level = 0;

	fp = fopen(line, "r");
	if (fp) {
		INPUT("%i", &playera.level);

		if (playera.level > 0) {
			INPUT("%i", &asecstart);
			INPUT("%s", line);

			INPUT("%f", &playera.px);
			INPUT("%f", &playera.py);
			INPUT("%f", &playera.opx);
			INPUT("%f", &playera.opy);
			INPUT("%i", &playera.walkdir);
			INPUT("%f", &playera.walkframe);
			INPUT("%f", &playera.walkspd);
			INPUT("%f", &playera.attackframe);
			INPUT("%f", &playera.attackspd);
			INPUT("%i", &playera.hp);
			INPUT("%i", &playera.maxhp);
			INPUT("%f", &playera.hpflash);
			INPUT("%i", &playera.level);
			INPUT("%i", &playera.maxlevel);
			INPUT("%i", &playera.sword);
			INPUT("%i", &playera.shield);
			INPUT("%i", &playera.armour);
			for (int i = 0; i < 5; i++) {
				INPUT("%i", &playera.foundspell[i]);
				INPUT("%f", &playera.spellcharge[i]);
			}
			for (int a = 0; a < 5; a++) {
				INPUT("%i", &playera.inventory[a]);
			}
			INPUT("%f", &playera.attackstrength);
			INPUT("%i", &playera.spelldamage);
			INPUT("%i", &playera.sworddamage);
			INPUT("%i", &playera.exp);
			INPUT("%i", &playera.nextlevel);

			fclose(fp);

			return 1; // success
		}

		fclose(fp);
	}
#endif

	return 0; // fail
}

int state_save(int slotnum) {
#if 0
	FILE *fp;
	char line[256];

	sprintf(line, player_sav, slotnum);

	fp = fopen(line, "w");
	if (fp) {
		PRINT("%i", player.level);

		if (player.level > 0) {
			PRINT("%i", (secstart + secsingame));
			PRINT("%s", "a");

			PRINT("%f", player.px);
			PRINT("%f", player.py);
			PRINT("%f", player.opx);
			PRINT("%f", player.opy);
			PRINT("%i", player.walkdir);
			PRINT("%f", player.walkframe);
			PRINT("%f", player.walkspd);
			PRINT("%f", player.attackframe);
			PRINT("%f", player.attackspd);
			PRINT("%i", player.hp);
			PRINT("%i", player.maxhp);
			PRINT("%f", player.hpflash);
			PRINT("%i", player.level);
			PRINT("%i", player.maxlevel);
			PRINT("%i", player.sword);
			PRINT("%i", player.shield);
			PRINT("%i", player.armour);
			for (int i = 0; i < 5; i++) {
				PRINT("%i", player.foundspell[i]);
				PRINT("%f", player.spellcharge[i]);
			}
			for (int a = 0; a < 5; a++) {
				PRINT("%i", player.inventory[a]);
			}
			PRINT("%f", player.attackstrength);
			PRINT("%i", player.spelldamage);
			PRINT("%i", player.sworddamage);
			PRINT("%i", player.exp);
			PRINT("%i", player.nextlevel);
			for (int a = 0; a <= 99; a++) {
				for (int b = 0; b <= 9; b++) {
					PRINT("%i", scriptflag[a][b]);
				}
			}
			PRINT("%i", curmap);

			for (int a = 0; a <= 999; a++) {
				for (int b = 0; b <= 20; b++) {
					for (int c = 0; c <= 14; c++) {
						PRINT("%i", objmapf[a][b][c]);
					}
				}
			}

			for (int a = 0; a <= 200; a++) {
				PRINT("%i", roomlocks[a]);
			}

			PRINT("%f", player.spellstrength);
		}

		fclose(fp);

		return 1; // success
	}

#endif
	return 0; // fail
}

} // end of namespace Griffon
