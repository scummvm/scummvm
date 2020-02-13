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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef STARTREK_TEXT_H
#define STARTREK_TEXT_H


namespace StarTrek {

// The type to use for text references (values of "GameStringIDs" enum).
// NOTE: if this typedef is changed, certain lines in "saveload.cpp" would also need to be
// changed. Better to leave this as-is.
typedef int32 TextRef;

// Text that's loaded from "GROUND.TXT". First 0x40 pieces of text are for items.
// TODO: Floppy version has different numbers for this.
enum GroundTextIDs {
	// Generic "perform undefined action" text (ie. look at nothing, talk to wall)
	GROUNDTX_LOOK_KIRK = 0x49,
	GROUNDTX_LOOK_SPOCK = 0x4a,
	GROUNDTX_LOOK_MCCOY = 0x4b,
	GROUNDTX_LOOK_REDSHIRT = 0x4c,
	GROUNDTX_LOOK_ANYWHERE = 0x4d,
	GROUNDTX_TALK_TO_CREWMAN = 0x4e,
	GROUNDTX_NO_RESPONSE = 0x4f,

	GROUNDTX_KIRK_USE = 0x50,
	GROUNDTX_SPOCK_USE = 0x51,
	GROUNDTX_MCCOY_USE = 0x52,
	GROUNDTX_REDSHIRT_USE = 0x53,
	GROUNDTX_SPOCK_SCAN = 0x54,
	GROUNDTX_MCCOY_SCAN = 0x55,
	GROUNDTX_USE_MEDKIT = 0x56,

	GROUNDTX_PHASER_ON_MCCOY = 0x57, // 8 variations
	GROUNDTX_PHASER_ON_SPOCK = 0x5f, // 8 variations
	GROUNDTX_PHASER_ON_REDSHIRT = 0x67, // 8 variations
	GROUNDTX_PHASER_ANYWHERE = 0x6f, // 7 variations

	GROUNDTX_USE_COMMUNICATOR = 0x76,
	GROUNDTX_NOTHING_HAPPENS = 0x77,
	GROUNDTX_FAIL_TO_OBTAIN_ANYTHING = 0x78
};

// Text that's hardcoded into "RDF" files, and copied into here for a sane referencing
// scheme.
enum GameStringIDs {
	TX_BLANK = -1,
	TX_NULL = 0,
	TX_BLANK_OLD = 1,	// unused, kept as filler
	TX_DIALOG_ERROR,
	TX_ANIMATION_ERROR,

	TX_SPEAKER_KIRK,
	TX_SPEAKER_SPOCK,
	TX_SPEAKER_MCCOY,
	TX_SPEAKER_UHURA,
	TX_SPEAKER_SCOTT,
	TX_SPEAKER_SULU,
	TX_SPEAKER_CHEKOV,

	TX_SPEAKER_EVERTS,
	TX_SPEAKER_ANGIVEN,
	TX_SPEAKER_SIGN,
	TX_SPEAKER_KLINGON,
	TX_SPEAKER_KANDREY,
	TX_SPEAKER_STEPHEN,
	TX_SPEAKER_CHUB,
	TX_SPEAKER_ROBERTS,
	TX_SPEAKER_GRISNASH,
	TX_SPEAKER_NAUIAN,
	TX_SPEAKER_SHIPS_COMPUTER,

	TX_SPEAKER_CHRISTENSEN,
	TX_SPEAKER_SIMPSON,
	TX_SPEAKER_ELASI_GUARD,
	TX_SPEAKER_ELASI_CLANSMAN,
	TX_SPEAKER_ELASI_CERETH,
	TX_SPEAKER_MASADA_CREWMAN,

	TX_SPEAKER_FERRIS,
	TX_SPEAKER_COMPUTER,
	TX_SPEAKER_MARCUS,
	TX_SPEAKER_CHEEVER,
	TX_SPEAKER_PREAX,

	TX_SPEAKER_BUCHERT,
	TX_SPEAKER_MUDD,

	TX_SPEAKER_STRAGEY,
	TX_SPEAKER_QUETZECOATL,
	TX_SPEAKER_TLAOXAC,

	TX_SPEAKER_BENNIE,
	TX_SPEAKER_VLICT,
	TX_SPEAKER_KLINGON_GUARD,
	TX_SPEAKER_ENTITY,
	TX_SPEAKER_BIALBI,
	TX_SPEAKER_VOICE,
	TX_SPEAKER_LIGHT_OF_WAR,
	TX_SPEAKER_LIGHT_OF_KNOWLEDGE,
	TX_SPEAKER_LIGHT_OF_TRAVEL,

	TX_SPEAKER_MOSHER,

	TX_SPEAKER_KIJE,
	TX_SPEAKER_PATTERSON,
	TX_SPEAKER_ANDRADE,
	TX_SPEAKER_ELASI_CAPTAIN,
	TX_SPEAKER_ELASI_WEAPONS_MASTER,
	TX_SPEAKER_ELASI_CREWMAN,
	TX_SPEAKER_BRITTANY_MARATA,

	TX_BRIDU146,

	TX_G_014,
	TX_G_024,
	TX_G_043,

	TX_GENER004,

	TX_COMPA180,
	TX_COMPA185,
	TX_COMPA186,
	TX_COMPA187,
	TX_COMPA189,
	TX_COMPA190,
	TX_COMPA191,
	TX_COMPA193,
	TX_COMPU181,
	TX_COMPU182,
	TX_COMPU186,
	TX_COMPU187,
	TX_COMPU188,
	TX_COMPU189,
	TX_COMPU190,
	TX_COMPU192,
	TX_COMPU193,

	TX_DEM0N009,	// kept because it's also used as an enhancement in mudd1 and mudd3
	TX_DEM0C001,	// "C" = "Custom" (no ID originally assigned)
	TX_DEM3_019,	// kept because it's also used as an enhancement in venga
	TX_DEM3_A32,

	TX_TUG2J003,
	TX_TUG2C001, // Custom

	TX_LOV3NA08,
	TX_LOV3NA09,
	TX_LOV3NA20,
	TX_LOV3NA21,
	TX_LOV3NA22,
	TX_LOV3NA23,
	TX_LOV3NJ32,
	TX_LOV5C001, // Custom

	TX_MUD0_018,
	TX_MUD0_019,
	TX_MUD0_020,
	TX_MUD1N014,	// kept because it's also used as an enhancement in veng6
	TX_MUD2_040,	// kept as it's used as an enhancement in LOVEA
	TX_MUD4_018,
	TX_MUD4_019,	// kept for now because of TX_MUD4_A29
	TX_MUD4_023,	// kept for now because of TX_MUD4_A29
	TX_MUD4_A29,

	TX_FEA1_A46,

	TX_TRI1_J00,
	TX_TRI1U080,
	TX_TRI1C001, // Custom
	TX_TRI1C002,
	TX_QUIET,
	TX_TRI4_076,

	TX_SIN3_008,
	TX_SIN3_012,
	TX_SIN3_LASERSETTING001, // Custom named text
	TX_SIN3_LASERSETTING010,
	TX_SIN3_LASERSETTING100,
	TX_SIN3_LASERCANCEL,
	TX_SIN4U83B,

	TX_VEN0_010,
	TX_VEN0_016,
	TX_VEN0N016,
	TX_VEN1_004,
	TX_VEN2_028,
	TX_VEN2_066,
	TX_VEN2_098,
	TX_VEN2_SHI,
	TX_VEN2_TRA,
	TX_VEN2_WEA,
	TX_VEN2U093,
	TX_VEN2_HAIL_ELASI, // Custom named text
	TX_VEN2_HAIL_ENT,
	TX_VEN2_CANCEL,
	TX_VEN4_016,
	TX_VEN4_017,
	TX_VEN4N007,
	TX_VEN4N010,
	TX_VEN4N014,
	TX_VEN5_R19,
	TX_VEN5_R20,
	TX_VEN5N002,
	TX_VEN5N004,
	TX_VEN5N007,

	TX_SPOKCOFF,
	TX_STATICU1,

	TX_END
};

// defined in text.cpp
extern const char *const g_gameStrings[];

} // End of namespace StarTrek

#endif
