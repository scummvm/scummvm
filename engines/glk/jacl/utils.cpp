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

#include "glk/jacl/jacl.h"
#include "glk/jacl/language.h"
#include "glk/jacl/types.h"
#include "glk/jacl/prototypes.h"

namespace Glk {
namespace JACL {

extern char                     function_name[];

extern struct object_type       *object[];
extern struct variable_type     *variable[];

extern int                      objects;
extern int                      player;

extern char                     game_file[];
extern char                     game_path[];
extern char                     prefix[];
extern char                     blorb[];
extern char                     bookmark[];
extern char                     walkthru[];
extern char                     include_directory[];
extern char                     temp_directory[];
extern char                     data_directory[];
extern char                     temp_buffer[];

void eachturn() {
	/* INCREMENT THE TOTAL NUMBER OF MOVES MADE AND CALL THE 'EACHTURN'
	 * FUNCTION FOR THE CURRENT LOCATION AND THE GLOBAL 'EACHTURN'
	 * FUNCTION. THESE FUNCTIONS CONTAIN ANY CODE THAT SIMULATED EVENTS
	 * OCCURING DUE TO THE PASSING OF TIME */
	TOTAL_MOVES->value++;
	execute("+eachturn");
	strcpy(function_name, "eachturn_");
	strcat(function_name, object[HERE]->label);
	execute(function_name);
	execute("+system_eachturn");

	/* SET TIME TO FALSE SO THAT NO MORE eachturn FUNCTIONS ARE EXECUTED
	 * UNTIL EITHER THE COMMAND PROMPT IS RETURNED TO (AND TIME IS SET
	 * TO TRUE AGAIN, OR IT IS MANUALLY SET TO TRUE BY A VERB THAT CALLS
	 * MORE THAN ONE proxy COMMAND. THIS IS BECAUSE OTHERWISE A VERB THAT
	 * USES A proxy COMMAND TO TRANSLATE THE VERB IS RESULT IN TWO MOVES
	 * (OR MORE) HAVING PASSED FOR THE ONE ACTION. */
	TIME->value = FALSE;
}

int get_here() {
	/* THIS FUNCTION RETURNS THE VALUE OF 'here' IN A SAFE, ERROR CHECKED
	 * WAY */
	if (player < 1 || player > objects) {
		badplrrun(player);
		terminate(44);
		return 0;
	} else if (object[player]->PARENT < 1 || object[player]->PARENT > objects || object[player]->PARENT == player) {
		badparrun();
		terminate(44);
		return 0;
	} else {
		return (object[player]->PARENT);
	}
}

char *strip_return(char *string) {
	/* STRIP ANY TRAILING RETURN OR NEWLINE OFF THE END OF A STRING */
	int index;
	int length = strlen(string);

	for (index = 0; index < length; index++) {
		switch (string[index]) {
		case '\r':
		case '\n':
			string[index] = 0;
			break;
		default:
			break;
		}
	}

	return string;
}

int random_number() {
	return g_vm->getRandomNumber(0x7fffffff);
}

void create_paths(char *full_path) {
	int       index;
	char      *last_slash;

	/* SAVE A COPY OF THE SUPPLIED GAMEFILE NAME */
	strcpy(game_file, full_path);

	/* FIND THE LAST SLASH IN THE SPECIFIED GAME PATH AND REMOVE THE GAME
	 * FILE SUFFIX IF ANY EXISTS */
	last_slash = (char *)NULL;

	/* GET A POINTER TO THE LAST SLASH IN THE FULL PATH */
	last_slash = strrchr(full_path, DIR_SEPARATOR);

	for (index = strlen(full_path); index >= 0; index--) {
		if (full_path[index] == DIR_SEPARATOR) {
			/* NO '.' WAS FOUND BEFORE THE LAST SLASH WAS REACHED,
			 * THERE IS NO FILE EXTENSION */
			break;
		} else if (full_path[index] == '.') {
			full_path[index] = 0;
			break;
		}
	}

	/* STORE THE GAME PATH AND THE GAME FILENAME PARTS SEPARATELY */
	if (last_slash == (const char *) NULL) {
		/* GAME MUST BE IN CURRENT DIRECTORY SO THERE WILL BE NO GAME PATH */
		strcpy(prefix, full_path);
		game_path[0] = 0;

		/* THIS ADDITION OF ./ TO THE FRONT OF THE GAMEFILE IF IT IS IN THE
		 * CURRENT DIRECTORY IS REQUIRED TO KEEP Gargoyle HAPPY. */
#ifdef __NDS__
		sprintf(temp_buffer, "%c%s", DIR_SEPARATOR, game_file);
#else
		sprintf(temp_buffer, ".%c%s", DIR_SEPARATOR, game_file);
#endif
		strcpy(game_file, temp_buffer);
	} else {
		/* STORE THE DIRECTORY THE GAME FILE IS IN WITH THE TRAILING
		 * SLASH IF THERE IS ONE */
		last_slash++;
		strcpy(prefix, last_slash);
		*last_slash = '\0';
		strcpy(game_path, full_path);
	}

#ifdef GLK
	/* SET DEFAULT WALKTHRU FILE NAME */
	sprintf(walkthru, "%s.walkthru", prefix);

	/* SET DEFAULT SAVED GAME FILE NAME */
	sprintf(bookmark, "%s.bookmark", prefix);

	/* SET DEFAULT BLORB FILE NAME */
	sprintf(blorb, "%s.blorb", prefix);
#endif

	/* SET DEFAULT FILE LOCATIONS IF NOT SET BY THE USER IN CONFIG */
	if (include_directory[0] == 0) {
		strcpy(include_directory, game_path);
		strcat(include_directory, INCLUDE_DIR);
	}

	if (temp_directory[0] == 0) {
		strcpy(temp_directory, game_path);
		strcat(temp_directory, TEMP_DIR);
	}

	if (data_directory[0] == 0) {
		strcpy(data_directory, game_path);
		strcat(data_directory, DATA_DIR);
	}
}

int jacl_whitespace(char character) {
	/* CHECK IF A CHARACTER IS CONSIDERED WHITE SPACE IN THE JACL LANGUAGE */
	switch (character) {
	case ':':
	case '\t':
	case ' ':
		return (TRUE);
	default:
		return (FALSE);
	}
}

char *stripwhite(char *string) {
	int i;

	/* STRIP WHITESPACE FROM THE START AND END OF STRING. */
	while (jacl_whitespace(*string)) string++;

	i = strlen(string) - 1;

	while (i >= 0 && ((jacl_whitespace(*(string + i))) || *(string + i) == '\n' || *(string + i) == '\r')) i--;

#ifdef WIN32
	i++;
	*(string + i) = '\r';
#endif
	i++;
	*(string + i) = '\n';
	i++;
	*(string + i) = '\0';

	return string;
}

void jacl_encrypt(char *string) {
	int index, length;

	length = strlen(string);

	for (index = 0; index < length; index++) {
		if (string[index] == '\n' || string[index] == '\r') {
			return;
		}
		string[index] = string[index] ^ 255;
	}
}

void jacl_decrypt(char *string) {
	int index, length;

	length = strlen(string);

	for (index = 0; index < length; index++) {
		if (string[index] == '\n' || string[index] == '\r') {
			return;
		}
		string[index] = string[index] ^ 255;
	}
}

} // End of namespace JACL
} // End of namespace Glk
