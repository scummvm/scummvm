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

%require "3.0"
%defines "engines/hypno/tokens_arc.h"
%output "engines/hypno/grammar_arc.cpp"
%define api.prefix {HYPNO_ARC_}

%{

#include "common/array.h"
#include "hypno/hypno.h"

#undef yyerror
#define yyerror	 HYPNO_ARC_xerror

Hypno::Shoot *shoot;

extern int HYPNO_ARC_lex();
extern int HYPNO_ARC_lineno;
uint32 HYPNO_ARC_default_sound_rate = 0;

void HYPNO_ARC_xerror(const char *str) {
	error("%s at line %d", str, HYPNO_ARC_lineno);
}

int HYPNO_ARC_wrap() {
	return 1;
}

using namespace Hypno;

void parseSN(const char *sn, const char *path, const char *enc, const char *flag) {
	uint32 sampleRate = 11025;
	if (Common::String("22K") == enc || Common::String("22k") == enc)
		sampleRate = 22050;
	else if (HYPNO_ARC_default_sound_rate > 0)
		sampleRate = HYPNO_ARC_default_sound_rate;

	if (Common::String("S0") == sn) {
		g_parsedArc->music = path;
		g_parsedArc->musicRate = sampleRate;
	} else if (Common::String("S1") == sn) {
		g_parsedArc->shootSound = path;
		g_parsedArc->shootSoundRate = sampleRate;
	} else if (Common::String("S2") == sn) {
		g_parsedArc->hitSound = path;
		g_parsedArc->hitSoundRate = sampleRate;
	} else if (Common::String("S4") == sn) {
		g_parsedArc->enemySound = path;
		g_parsedArc->enemySoundRate = sampleRate;
	} else if (Common::String("S5") == sn) {
		g_parsedArc->additionalSound = path;
		g_parsedArc->additionalSoundRate = sampleRate;
	} else if (Common::String("S7") == sn) {
		g_parsedArc->noAmmoSound = path;
		g_parsedArc->noAmmoSoundRate = sampleRate;
	} else if (Common::String("S8") == sn) {
		g_parsedArc->additionalSound = path;
		g_parsedArc->additionalSoundRate = sampleRate;
	}
	debugC(1, kHypnoDebugParser, "SN %s", path);
}

%}

%union {
	char *s; /* string value */
	int i;	 /* integer value */
}

%token<s> NAME FILENAME BNTOK SNTOK KNTOK YXTOK FNTOK ENCTOK ONTOK H12TOK
%token<i> NUM BYTE
// header
%token COMMENT ALTOK AVTOK ABTOK CTOK DTOK HTOK HETOK HLTOK HUTOK RETTOK QTOK RESTOK
%token PTOK FTOK TTOK TATOK TPTOK TSTOK ATOK VTOK OTOK LTOK MTOK NTOK NRTOK NSTOK RTOK R0TOK R1TOK
%token ITOK I1TOK GTOK JTOK J0TOK KTOK UTOK ZTOK

// body
%token NONETOK A0TOK P0TOK WTOK

// end
%token XTOK

// bytes??
%token CB3TOK C02TOK

%type<s> enc flag

%%

start: YXTOK { g_parsedArc->mode = $1; } header ZTOK RETTOK body XTOK
	| RETTOK start
	;

header: hline header
	| RETTOK header
	| /* nothing */
	;

hline: 	CTOK NUM {
		g_parsedArc->id = $2;
		HYPNO_ARC_default_sound_rate = 0;
		debugC(1, kHypnoDebugParser, "C %d", $2); }
	| FTOK NUM {
		HYPNO_ARC_default_sound_rate = $2;
		debugC(1, kHypnoDebugParser, "F %d", $2);
	}
	| DTOK NUM  {
		g_parsedArc->frameDelay = $2;
		debugC(1, kHypnoDebugParser, "D %d", $2);
	}
	| PTOK NUM NUM { debugC(1, kHypnoDebugParser, "P %d %d", $2, $3); }
	| ATOK NUM NUM { debugC(1, kHypnoDebugParser, "A %d %d", $2, $3); }
	| MTOK FILENAME {
		debugC(1, kHypnoDebugParser, "M %s", $2);
		g_parsedArc->maskVideo = $2;
	}
	| UTOK NUM NUM NUM NUM {
		debugC(1, kHypnoDebugParser, "U %d %d %d %d", $2, $3, $4, $5);
		ScriptInfo si($2, $3, $4, $5);
		g_parsedArc->script.push_back(si);
	}
	| VTOK NUM NUM {
		debugC(1, kHypnoDebugParser, "V %d %d", $2, $3);
		g_parsedArc->mouseBox = Common::Rect(0, 0, $2, $3);
	}
	| VTOK RESTOK {
		debugC(1, kHypnoDebugParser, "V 320,200");
		g_parsedArc->mouseBox = Common::Rect(0, 0, 320, 200);
	}
	| OTOK NUM NUM {
		g_parsedArc->objKillsRequired[0] = $2;
		g_parsedArc->objMissesAllowed[0] = $3;
		debugC(1, kHypnoDebugParser, "O %d %d", $2, $3);
	}
	| ONTOK NUM NUM {
		if (Common::String("O0") == $1) {
			g_parsedArc->objKillsRequired[0] = $2;
			g_parsedArc->objMissesAllowed[0] = $3;
		} else if (Common::String("O1") == $1) {
			g_parsedArc->objKillsRequired[1] = $2;
			g_parsedArc->objMissesAllowed[1] = $3;
		} else
			error("Invalid objective: '%s'", $1);
		debugC(1, kHypnoDebugParser, "ON %d %d", $2, $3); }
	| ONTOK NUM {
		if (Common::String("O0") == $1) {
			g_parsedArc->objKillsRequired[0] = $2;
		} else if (Common::String("O1") == $1) {
			g_parsedArc->objKillsRequired[1] = $2;
		} else
			error("Invalid objective: '%s'", $1);
		debugC(1, kHypnoDebugParser, "ON %d", $2);
	}
	| TPTOK NONETOK NUM FILENAME {
		ArcadeTransition at("NONE", $4, "", 0, $3);
		g_parsedArc->transitions.push_back(at);
		debugC(1, kHypnoDebugParser, "Tp %s %d %s", "NONE", $3, $4);
	}
	| TSTOK FILENAME NUM NUM {
		debugC(1, kHypnoDebugParser, "Ts %s %d %d", $2, $3, $4);
	}
	| TPTOK FILENAME NUM FILENAME {
		ArcadeTransition at($2, $4, "", 0, $3);
		g_parsedArc->transitions.push_back(at);
		debugC(1, kHypnoDebugParser, "Tp %s %d %s", $2, $3, $4);
	}
	| TATOK NUM FILENAME flag enc {
		uint32 sampleRate = 11025;
		if (Common::String("22K") == $5 || Common::String("22k") == $5)
			sampleRate = 22050;

		ArcadeTransition at("", "", $3, sampleRate, $2);
		g_parsedArc->transitions.push_back(at);
		debugC(1, kHypnoDebugParser, "Ta %d %s", $2, $3);
	}
	| TTOK FILENAME NUM {
		ArcadeTransition at($2, "", "", 0, $3);
		g_parsedArc->transitions.push_back(at);
		debugC(1, kHypnoDebugParser, "T %s %d", $2, $3);
	}
	| TTOK NONETOK NUM {
		ArcadeTransition at("NONE", "", "", 0, $3);
		g_parsedArc->transitions.push_back(at);
		debugC(1, kHypnoDebugParser, "T NONE %d", $3); }
	| NTOK FILENAME  {
		g_parsedArc->backgroundVideo = $2;
		debugC(1, kHypnoDebugParser, "N %s", $2);
	}
	| NSTOK FILENAME  {
		g_parsedArc->backgroundVideo = $2;
		debugC(1, kHypnoDebugParser, "N* %s", $2);
	}
	| RTOK FILENAME  {
		g_parsedArc->backgroundPalette = $2;
		debugC(1, kHypnoDebugParser, "R %s", $2); }
	| ITOK FILENAME {
		g_parsedArc->player = $2;
		debugC(1, kHypnoDebugParser, "I %s", $2);
	}
	| I1TOK FILENAME {
		debugC(1, kHypnoDebugParser, "I1 %s", $2);
	}
	| QTOK NUM NUM { debugC(1, kHypnoDebugParser, "Q %d %d", $2, $3); }
	| BNTOK FILENAME {
		if (Common::String("B0") == $1)
			g_parsedArc->beforeVideo = $2;
		else if (Common::String("B1") == $1)
			g_parsedArc->additionalVideo = $2;
		else if (Common::String("B2") == $1)
			g_parsedArc->nextLevelVideo = $2;
		else if (Common::String("B3") == $1)
			g_parsedArc->defeatNoEnergyFirstVideo = $2;
		else if (Common::String("B4") == $1)
			g_parsedArc->defeatMissBossVideo = $2;
		else if (Common::String("B5") == $1)
			g_parsedArc->defeatNoEnergySecondVideo = $2;
		else if (Common::String("B6") == $1)
			g_parsedArc->hitBoss1Video = $2;
		else if (Common::String("B7") == $1)
			g_parsedArc->missBoss1Video = $2;
		else if (Common::String("B8") == $1)
			g_parsedArc->hitBoss2Video = $2;
		else if (Common::String("B9") == $1)
			g_parsedArc->missBoss2Video = $2;
		else if (Common::String("BA") == $1)
			g_parsedArc->briefingVideo = $2;
		else if (Common::String("BB") == $1)
			g_parsedArc->postStatsVideo = $2;

		debugC(1, kHypnoDebugParser, "BN %s", $2);
	}
	| SNTOK FILENAME enc flag {
		parseSN($1, $2, $3, $4);
	}
	| SNTOK FILENAME flag enc {
		parseSN($1, $2, $4, $3);
	}
	| HETOK BYTE NUM NUM {
		Segment segment($2, $4, $3);
		segment.end = true;
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "HE %x %d %d", $2, $3, $4);
	}
	| HLTOK BYTE NUM NUM {
		Segment segment($2, $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "HL %x %d %d", $2, $3, $4);
	}
	| HUTOK BYTE NUM NUM {
		Segment segment($2, $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "HU %x %d %d", $2, $3, $4);
	}
	| HTOK NAME NUM NUM {
		assert(Common::String($2).size() == 1);
		Segment segment($2[0], $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H %s %d %d", $2, $3, $4);
	}
	| HTOK RTOK NUM NUM { // Workaround for BYTE == R
		Segment segment('R', $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H R %d %d", $3, $4);
	}
	| HTOK ATOK NUM NUM { // Workaround for BYTE == A
		Segment segment('A', $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H A %d %d", $3, $4);
	}
	| HTOK PTOK NUM NUM { // Workaround for BYTE == P
		Segment segment('P', $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H P %d %d", $3, $4);
	}
	| HTOK LTOK NUM NUM { // Workaround for BYTE == P
		Segment segment('L', $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H P %d %d", $3, $4);
	}
	| H12TOK BYTE NUM NUM {
		Segment segment($2, $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "HN %x %d %d", $2, $3, $4);
	}
	| HTOK BYTE NUM NUM {
		Segment segment($2, $4, $3);
		g_parsedArc->segments.push_back(segment);
		debugC(1, kHypnoDebugParser, "H %x %d %d", $2, $3, $4);
	}
	;

enc: ENCTOK          { $$ = $1; }
	| /* nothing */  { $$ = scumm_strdup(""); }
	;

flag: NAME           { $$ = $1; }
	| /* nothing */  { $$ = scumm_strdup(""); }
	;

body: bline body
	| RETTOK body
	| /* nothing */
	;

bline: FNTOK FILENAME {
		shoot = new Shoot();
		if (Common::String("F0") == $1)
			shoot->animation = $2;
		else if (Common::String("F4") == $1)
			shoot->explosionAnimation = $2;
		debugC(1, kHypnoDebugParser, "FN %s", $2);
	}
	| AVTOK NUM {
		assert($2 == 0);
		shoot->nonHostile = true;
		debugC(1, kHypnoDebugParser, "AV %d", $2);
	}
	| ALTOK NUM {
		debugC(1, kHypnoDebugParser, "AL %d", $2);
	}
	| ABTOK NUM {
		assert($2 == 1);
		shoot->playInteractionAudio = true;
		debugC(1, kHypnoDebugParser, "AB %d", $2);
	}
	| DTOK LTOK  { debugC(1, kHypnoDebugParser, "D L");
	}
	| J0TOK NUM {
		debugC(1, kHypnoDebugParser, "J0 %d", $2);
	}
	| FNTOK NONETOK {
		shoot = new Shoot();
		shoot->animation = "NONE";
		debugC(1, kHypnoDebugParser, "FN NONE");
	}
	| FTOK FILENAME {
		shoot = new Shoot();
		shoot->animation = $2;
		debugC(1, kHypnoDebugParser, "FN %s", $2);
	}
	| ITOK NAME  {
		shoot->name = $2;
		debugC(1, kHypnoDebugParser, "I %s", $2);
	}
	| ITOK BNTOK  {  // Workaround for NAME == B1
		shoot->name = $2;
		debugC(1, kHypnoDebugParser, "I %s", $2);
	}
	| ITOK ATOK  { // Workaround for NAME == A
		shoot->name = "A";
		debugC(1, kHypnoDebugParser, "I A");
	}
	| ITOK CTOK  { // Workaround for NAME == C
		shoot->name = "C";
		debugC(1, kHypnoDebugParser, "I C");
	}
	| ITOK DTOK  { // Workaround for NAME == D
		shoot->name = "D";
		debugC(1, kHypnoDebugParser, "I D");
	}
	| ITOK FTOK  { // Workaround for NAME == F
		shoot->name = "F";
		debugC(1, kHypnoDebugParser, "I F");
	}
	| ITOK GTOK  { // Workaround for NAME == G
		shoot->name = "G";
		debugC(1, kHypnoDebugParser, "I G");
	}
	| ITOK HTOK  { // Workaround for NAME == H
		shoot->name = "H";
		debugC(1, kHypnoDebugParser, "I H");
	}
	| ITOK H12TOK  { // Workaround for NAME == H1/H2
		shoot->name = $2;
		debugC(1, kHypnoDebugParser, "I %s", $2);
	}
	| ITOK ITOK  { // Workaround for NAME == I
		shoot->name = "I";
		debugC(1, kHypnoDebugParser, "I I");
	}
	| ITOK JTOK  { // Workaround for NAME == J
		shoot->name = "J";
		debugC(1, kHypnoDebugParser, "I J");
	}
	| ITOK KTOK  { // Workaround for NAME == K
		shoot->name = "K";
		debugC(1, kHypnoDebugParser, "I K");
	}
	| ITOK NTOK  { // Workaround for NAME == N
		shoot->name = "N";
		debugC(1, kHypnoDebugParser, "I N");
	}
	| ITOK OTOK  { // Workaround for NAME == O
		shoot->name = "O";
		debugC(1, kHypnoDebugParser, "I O");
	}
	| ITOK PTOK  { // Workaround for NAME == P
		shoot->name = "P";
		debugC(1, kHypnoDebugParser, "I P");
	}
	| ITOK QTOK  { // Workaround for NAME == Q
		shoot->name = "Q";
		debugC(1, kHypnoDebugParser, "I Q");
	}
	| ITOK RTOK  { // Workaround for NAME == R
		shoot->name = "R";
		debugC(1, kHypnoDebugParser, "I R");
	}
	| ITOK SNTOK  {  // Workaround for NAME == S1
		shoot->name = $2;
		debugC(1, kHypnoDebugParser, "I %s", $2);
	}
	| ITOK TTOK  { // Workaround for NAME == T
		shoot->name = "T";
		debugC(1, kHypnoDebugParser, "I T");
	}
	| ITOK LTOK  { // Workaround for NAME == L
		shoot->name = "L";
		debugC(1, kHypnoDebugParser, "I L");
	}
	| ITOK MTOK  { // Workaround for NAME == M
		shoot->name = "M";
		debugC(1, kHypnoDebugParser, "I M");
	}
	| ITOK UTOK  { // Workaround for NAME == U
		shoot->name = "U";
		debugC(1, kHypnoDebugParser, "I U");
	}
	| JTOK NUM  {
		debugC(1, kHypnoDebugParser, "J %d", $2);
	}
	| A0TOK NUM NUM {
		shoot->position = Common::Point($2, $3);
		debugC(1, kHypnoDebugParser, "A0 %d %d", $2, $3);
	}
	| RTOK NUM NUM  {
		shoot->objKillsCount = $2;
		shoot->objMissesCount = $3;
		debugC(1, kHypnoDebugParser, "R %d %d", $2, $3);
	}
	| R0TOK NUM NUM  {
		shoot->objKillsCount = $2;
		shoot->objMissesCount = $3;
		debugC(1, kHypnoDebugParser, "R0 %d %d", $2, $3);
	}
	| R1TOK NUM NUM  {
		debugC(1, kHypnoDebugParser, "R1 %d %d", $2, $3);
	}
	| BNTOK NUM NUM {
		FrameInfo fi($3, $2);
		shoot->bodyFrames.push_back(fi);
		debugC(1, kHypnoDebugParser, "BN %d %d", $2, $3);
	}
	| KNTOK NUM NUM {
		FrameInfo fi($3, $2);
		shoot->explosionFrames.push_back(fi);
		debugC(1, kHypnoDebugParser, "KN %d %d", $2, $3);
	}
	| P0TOK NUM NUM {
		shoot->paletteSize = $2;
		shoot->paletteOffset = $3;
		debugC(1, kHypnoDebugParser, "P0 %d %d", $2, $3); }
	| OTOK NUM NUM {
		if ($2 == 0 && $3 == 0)
			error("Invalid O command (0, 0)");
		shoot->deathPosition = Common::Point($2, $3);
		debugC(1, kHypnoDebugParser, "O %d %d", $2, $3);
	}
	| CTOK NUM  {
		shoot->timesToShoot = $2;
		debugC(1, kHypnoDebugParser, "C %d", $2);
	}
	| HTOK NUM  {
		shoot->attackFrames.push_back($2);
		debugC(1, kHypnoDebugParser, "H %d", $2); }
	| VTOK NUM  { debugC(1, kHypnoDebugParser, "V %d", $2); }
	| VTOK { debugC(1, kHypnoDebugParser, "V"); }
	| WTOK NUM  {
		shoot->attackWeight = $2;
		debugC(1, kHypnoDebugParser, "W %d", $2); }
	| DTOK NUM  {
		shoot->pointsToShoot = $2;
		debugC(1, kHypnoDebugParser, "D %d", $2);
	}
	| LTOK NUM NUM {
		debugC(1, kHypnoDebugParser, "L %d %d", $2, $3);
	}
	| LTOK NUM {
		debugC(1, kHypnoDebugParser, "L %d", $2);
		FrameInfo fi($2-1, 0);
		shoot->bodyFrames.push_back(fi);
	}
	| MTOK NUM { debugC(1, kHypnoDebugParser, "M %d", $2);
		shoot->missedAnimation = $2;
	}
	| KTOK { debugC(1, kHypnoDebugParser, "K"); }
	| KTOK NUM { debugC(1, kHypnoDebugParser, "K %d", $2);
		FrameInfo fi($2, 1);
		shoot->explosionFrames.push_back(fi);
	}
	| KTOK NUM NUM NUM {
		debugC(1, kHypnoDebugParser, "K %d %d %d", $2, $3, $4);
	}
	| KTOK NUM NUM { debugC(1, kHypnoDebugParser, "K %d %d", $2, $3);
		FrameInfo fi($2, 1);
		shoot->explosionFrames.push_back(fi);
	}
	| SNTOK FILENAME enc {
		if (Common::String("S0") == $1)
			shoot->enemySound = $2;
		else if (Common::String("S1") == $1)
			shoot->deathSound = $2;
		else if (Common::String("S2") == $1)
			shoot->hitSound = $2;
		else if (Common::String("S4") == $1)
			shoot->animalSound = $2;

		debugC(1, kHypnoDebugParser, "SN %s", $2); }
	| SNTOK {
		debugC(1, kHypnoDebugParser, "SN");
	}

	| GTOK { debugC(1, kHypnoDebugParser, "G"); }
	| TTOK NUM NUM NUM {
		debugC(1, kHypnoDebugParser, "T %d %d %d", $2, $3, $4);
	}
	| TTOK NUM {
		shoot->interactionFrame = $2;
		debugC(1, kHypnoDebugParser, "T %d", $2);
	}
	| TTOK {
		shoot->isAnimal = true;
		debugC(1, kHypnoDebugParser, "T");
	}
	| MTOK {
		debugC(1, kHypnoDebugParser, "M");
	}
	| NTOK {
		shoot->noEnemySound = true;
		debugC(1, kHypnoDebugParser, "N"); }
	| NRTOK {
		debugC(1, kHypnoDebugParser, "NR"); }
	| ZTOK {
		g_parsedArc->shoots.push_back(*shoot);
		//delete shoot;
		//shoot = nullptr;
		debugC(1, kHypnoDebugParser, "Z");
	}
	;

