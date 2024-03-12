
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

#ifndef M4_ADV_DB_R_DATABASE_H
#define M4_ADV_DB_R_DATABASE_H

#include "m4/m4_types.h"

namespace M4 {

#define STR_DB_TREE "db tree node"
#define STR_DB_TOKEN "db token"
#define STR_DB_STRING "db string"
#define STR_GENERICLIST "GenericList"

#define NOTHING                         0
#define INTEGER                         1
#define FLOAT                           2
#define DELIMITER                       3
#define IDENTIFIER                      4
#define STRING                          5
#define COMMENT                         6
#define OPERATOR                        7
#define MATH                            8
#define BITWISE                         9
#define LOGICAL                         10
#define DUMMY                           11

#define OPENB                           400
#define CLOSEB                          401
#define O_BRACE                         408
#define C_BRACE                         409
#define COMMA                           414
#define COLON                           479

#define	_T_SCENEFILE	101
#define	_T_VERSION		102
#define	_T_SCENE		103
#define	_T_ARTBASE		104
#define	_T_HOTSPOTS		105
#define	_T_VERB			107
#define	_T_SYNTAX		108
#define	_T_FACING		109
#define	_T_FEETX		110
#define	_T_FEETY		111
#define	_T_CURSOR		112
#define	_T_FRONTSCALE	113
#define	_T_BACKSCALE	114
#define	_T_FRONTY		115
#define	_T_BACKY		116
#define	_T_RAILS		117
#define	_T_DEPTHS		118
#define	_T_PREP			123
#define  _T_PARALLAX	124
#define  _T_SPRITE		125
#define  _T_PROPS		126

#define PR_ERR_PARSE	   -1
#define	_T_NAME				146
#define	_T_ARTIST			103
#define	_T_ITEM				107
#define	_T_TITLE			108
#define	_T_NEXTASSET		109
#define	_T_BACKGROUNDS		110
#define	_T_SPRITES			111
#define	_T_CONVERSATIONS	112
#define  _T_ANIMATIONS		144
#define	_T_SOUNDS			113
#define	_T_MUSICSCORES		114
#define	_T_CODE 			115
#define	_T_ROOM 			116
#define _T_ASSET			118
#define	_T_OPENBRACE		119
#define	_T_CLOSEBRACE		120
#define	_T_OPENBRACKET		121
#define	_T_CLOSE_BRACKET	122
#define _T_ROOMPATH			123
#define _T_STARTTIME		124
#define _T_LASTMODIFIED		125
#define _T_APPROVEDTIME		126
#define _T_COMMENTS			127
#define _T_PRIORITY			128
#define _T_STATUS		   	129
#define _T_SECRETTAG		130
#define _T_PATH				131
#define _T_WALK				141     
#define _T_DEPTH			142     
#define _T_SPECIAL			143     
#define _T_VOLUME 			145     
#define _T_TASK_APPROVALS	132
#define _T_DEPTHCODES		133
#define _T_WALKCODES	   	134
#define _T_SPECIALCODES		135


#define name_size            20
#define num_reserved_words   10
#define string_size        1024

struct tree {
	int n;
	int index;
	int seen;
	int   line_num;
	char *label;
	token *data;
	tree *left, *right;
	tree *parent;
};

struct  treeclass {
	tree *t;
	tree *r;
};

} // End of namespace M4

#endif
