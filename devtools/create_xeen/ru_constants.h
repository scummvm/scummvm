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

#pragma once

#include "constants.h"

class RU : public LangConstants {
public:
	/*
	"\v012\t000\x3"
	"c\f35���� � ����������� ��������:\n"
	"\f17���� ��� �������\x3"
	"l\n"
	"\n"
	"\t025\f35������������:\n"
	"\t035\f17���� ��������\n"
	"\t035���� �������\n"
	"\n"
	"\t025\f35������������ � ����:\n"
	"\t035\f17���� ��������\n"
	"\n"
	"\t025\f35������ � ����:\n"
	"\t035\f17��� �����\n"
	"\n"
	"\t025\f35��������:\n"
	"\t035\f17��� ������\n"
	"\t035����� ̸���\n"
	"\t035���� ��� �������\v012\n"
	"\n"
	"\n"
	"\t180\f35���������:\n"
	"\t190\f17��� �������\n"
	"\t190�������� �. ����\n"
	"\t190������ ����-������\n"
	"\t190������ �����\n"
	"\t190������� �������\n"
	"\n"
	"\t180\f35�������:\n"
	"\t190\f17��������� ����\n"
	"\t190����� ���������\n"
	"\t190������ ����\n"
	"\t190����� ���������\n"
	"\t190������� ������\n"
	"\t190����� ������\x3"
	"c";
	*/
	const char *CLOUDS_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35\x88\xA4\xA5\xEF \xA8 \xE0\xE3\xAA\xAE\xA2\xAE\xA4\xE1\xE2\xA2\xAE \xAF\xE0\xAE\xA5\xAA\xE2\xAE\xAC:\n"
			   "\f17\x84\xA6\xAE\xAD \x82\xA0\xAD \x8A\xA0\xAD\xA5\xA3\xA5\xAC\x3"
			   "l\n"
			   "\n"
			   "\t025\f35\x8F\xE0\xAE\xA3\xE0\xA0\xAC\xAC\xA8\xE1\xE2\xEB:\n"
			   "\t035\f17\x8C\xA0\xE0\xAA \x8A\xAE\xAB\xA4\xE3\xED\xAB\xAB\n"
			   "\t035\x84\xA5\xA9\xA2 \x95\xED\xE2\xED\xE3\xED\xA9\n"
			   "\n"
			   "\t025\f35\x80\xE3\xA4\xA8\xAE\xE1\xA8\xE1\xE2\xA5\xAC\xA0 \xA8 \xA7\xA2\xE3\xAA:\n"
			   "\t035\f17\x92\xAE\xA4\xA4 \x95\xA5\xAD\xA4\xE0\xA8\xAA\xE1\n"
			   "\n"
			   "\t025\f35\x8C\xE3\xA7\xEB\xAA\xA0 \xA8 \xE0\xA5\xE7\xEC:\n"
			   "\t035\f17\x92\xA8\xAC \x92\xA0\xAB\xAB\xA8\n"
			   "\n"
			   "\t025\f35\x91\xE6\xA5\xAD\xA0\xE0\xA8\xA9:\n"
			   "\t035\f17\x8F\xAE\xAB \x90\xA0\xE2\xAD\xA5\xE0\n"
			   "\t035\x84\xA5\xA1\xA1\xA8 \x8C\xF1\xE0\xE4\xA8\n"
			   "\t035\x84\xA6\xAE\xAD \x82\xA0\xAD \x8A\xA0\xAD\xA5\xA3\xA5\xAC\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35\x95\xE3\xA4\xAE\xA6\xAD\xA8\xAA\xA8:\n"
			   "\t190\f17\x8B\xE3\xA8 \x84\xA6\xAE\xAD\xE1\xAE\xAD\n"
			   "\t190\x84\xA6\xAE\xAD\xA0\xE2\xA0\xAD \x8F. \x83\xA2\xA8\xAD\n"
			   "\t190\x81\xAE\xAD\xA8\xE2\xA0 \x8B\xAE\xAD\xA3-\x95\xA5\xAC\xE1\xA0\xE2\n"
			   "\t190\x84\xA6\xE3\xAB\xA8\xEF \x93\xAB\xA0\xAD\xAE\n"
			   "\t190\x90\xA8\xAA\xA0\xE0\xA4\xAE \x81\xA0\xE0\xE0\xA5\xE0\xA0\n"
			   "\n"
			   "\t180\f35\x92\xA5\xE1\xE2\xA5\xE0\xEB:\n"
			   "\t190\f17\x81\xA5\xAD\xA4\xA6\xA0\xAC\xA8\xAD \x81\xA5\xAD\xE2\n"
			   "\t190\x8C\xA0\xE0\xA8\xAE \x9D\xE1\xAA\xA0\xAC\xA8\xAB\xAB\xA0\n"
			   "\t190\x90\xA8\xE7\xA0\xE0\xA4 \x9D\xE1\xAF\xA8\n"
			   "\t190\x91\xAA\xAE\xE2\xE2 \x8C\xA0\xAA\xA4\xED\xAD\xA8\xA5\xAB\n"
			   "\t190\x8A\xAB\xA5\xA9\xE2\xAE\xAD \x90\xA5\xE2\xA7\xA5\xE0\n"
			   "\t190\x8C\xA0\xA9\xAA\xAB \x91\xE3\xA0\xE0\xA5\xE1\x3"
			   "c";
	}

	const char *DARK_SIDE_CREDITS() {
		return "\v012\t000\x3"
			   "c\f35Designed and Directed By:\n"
			   "\f17Jon Van Caneghem\x3l\n"
			   "\n"
			   "\t025\f35Programming:\n"
			   "\t035\f17Mark Caldwell\n"
			   "\t035Dave Hathaway\n"
			   "\n"
			   "\t025\f35Sound System & FX:\n"
			   "\t035\f17Mike Heilemann\n"
			   "\n"
			   "\t025\f35Music & Speech:\n"
			   "\t035\f17Tim Tully\n"
			   "\n"
			   "\t025\f35Writing:\n"
			   "\t035\f17Paul Rattner\n"
			   "\t035Debbie Van Caneghem\n"
			   "\t035Jon Van Caneghem\v012\n"
			   "\n"
			   "\n"
			   "\t180\f35Graphics:\n"
			   "\t190\f17Jonathan P. Gwyn\n"
			   "\t190Bonita Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n"
			   "\n"
			   "\t180\f35Testing:\n"
			   "\t190\f17Benjamin Bent\n"
			   "\t190Christian Dailey\n"
			   "\t190Mario Escamilla\n"
			   "\t190Marco Hunter\n"
			   "\t190Robert J. Lupo\n"
			   "\t190Clayton Retzer\n"
			   "\t190David Vela\x3"
			   "c";
	}

	const char *SWORDS_CREDITS1() {
		return "\v012\x3"
			   "c\f35Published By New World Computing, Inc.\f17\n"
			   "Developed By CATware, Inc.\x3l\n"
			   "\f01Design and Direction\t180Series Created by\n"
			   "\t020Bill Fawcett\t190John Van Caneghem\n"
			   "\n"
			   "\t010Story Contributions\t180Producer & Manual\n"
			   "\t020Ellen Guon\t190Dean Rettig\n"
			   "\n"
			   "\t010Programming & Ideas\t180Original Programming\n"
			   "\t020David Potter\t190Mark Caldwell\n"
			   "\t020Rod Retterath\t190Dave Hathaway\n"
			   "\n"
			   "\t010Manual Illustrations\t180Graphic Artists\n"
			   "\t020Todd Cameron Hamilton\t190Jonathan P. Gwyn\n"
			   "\t020James Clouse\t190Bonnie Long-Hemsath\n"
			   "\t190Julia Ulano\n"
			   "\t190Ricardo Barrera\n";
	}

	const char *SWORDS_CREDITS2() {
		return "\f05\v012\t000\x3l\n"
			   "\t100Sound Programming\n"
			   "\t110Todd Hendrix\n"
			   "\n"
			   "\t100Music\n"
			   "\t110Tim Tully\n"
			   "\t110Quality Assurance Manager\n"
			   "\t110Peter Ryu\n"
			   "\t100Testers\n"
			   "\t110Walter Johnson\n"
			   "\t110Bryan Farina\n"
			   "\t110David Baton\n"
			   "\t110Jack Nalls\n";
	}

	/*
	"\r\x1\x3"
	"c\fd��� � �����\n"
	"%s �����\x2\n"
	"\v%.3dCopyright (c) %d NWC, Inc.\n"
	"All Rights Reserved\x1";
	*/
	const char *OPTIONS_MENU() {
		return "\r\x1\x3"
			   "c\fd\x8C\xA5\xE7 \xA8 \x8C\xA0\xA3\xA8\xEF\n"
			   "%s \x8A\xE1\xA8\xAD\xA0\x2\n"
			   "\v%.3dCopyright (c) %d NWC, Inc.\n"
			   "All Rights Reserved\x1";
	}
	/*
	"������", "Darkside", "World"
	*/
	const char **GAME_NAMES() {
		delete[] _gameNames;
		_gameNames = new const char *[3] { "\x8E\xA1\xAB\xA0\xAA\xA0", "Darkside", "World" };
		return _gameNames;
	}

	/*
	"\v012����� ��������� � ������!"
	*/
	const char *THE_PARTY_NEEDS_REST() {
		return "\v012\x8E\xE2\xE0\xEF\xA4 \xAD\xE3\xA6\xA4\xA0\xA5\xE2\xE1\xEF \xA2 \xAE\xE2\xA4\xEB\xE5\xA5!";
	}

	const char *WHO_WILL() {
		return "\x3"
			   "c\v000\t000%s\n\n%s?\n\v055F1 - F%d";
	}

	/*
	"\x3""c�������\n\n"
	*/
	const char *HOW_MUCH() {
		return "\x3"
			   "c\x91\xAA\xAE\xAB\xEC\xAA\xAE\n\n";
	}

	const char *WHATS_THE_PASSWORD() {
		return "\x3"
			   "cWhat's the Password?\n"
			   "\n"
			   "Please turn to page %u, go to\n"
			   "line %u, and type in word %u.\v067\t000Spaces are not counted as words or lines.  "
			   "Hyphenated words are treated as one word.  Any line that has any text is considered a line."
			   "\x3"
			   "c\v040\t000\n";
	}

	const char *PASSWORD_INCORRECT() {
		return "\x3"
			   "c\v040\n"
			   "\f32Incorrect!\fd";
	}

	/*
	"\v007%s �� � ��������� ���-���� �������!"
	*/
	const char *IN_NO_CONDITION() {
		return "\v007%s \xAD\xA5 \xA2 \xE1\xAE\xE1\xE2\xAE\xEF\xAD\xA8\xA8 \xE7\xE2\xAE-\xAB\xA8\xA1\xAE \xE1\xA4\xA5\xAB\xA0\xE2\xEC!";
	}

	/*
	"\x3""c\v010����� ������ ���."
	*/
	const char *NOTHING_HERE() {
		return "\x3"
			   "c\v010\x87\xA4\xA5\xE1\xEC \xAD\xA8\xE7\xA5\xA3\xAE \xAD\xA5\xE2.";
	}

	/*
	"��� ������",
	"��� �������",
	"��� ������",
	"��� ����� ������",
	"��� ���������",
	"��� ������",
	"���� �������",
	"��� ������",
	"��� ������",
	"��� ������",
	"��� ���� ������",
	"��� ��������",
	"��� ��������",
	"��� ���������",
	"��� ��� �������",
	"��� ���������",
	"��� ��� ���������",
	"��� ��������",
	"��� ���������",
	"��� ������ �������",
	"pray",
	"��� ������ ������",
	"act",
	"��� ����� �������",
	"��� �����",
	"rub",
	"pick",
	"eat",
	"sign",
	"close",
	"look",
	"try"
	*/
	const char **WHO_ACTIONS() {
		delete[] _whoActions;
		_whoActions = new const char *[32] {
			"\x8A\xE2\xAE \xAE\xA1\xEB\xE9\xA5\xE2",
				"\x8A\xE2\xAE \xAE\xE2\xAA\xE0\xAE\xA5\xE2",
				"\x8A\xE2\xAE \xA2\xEB\xAF\xEC\xA5\xE2",
				"\x8A\xE2\xAE \xA1\xE3\xA4\xA5\xE2 \xAA\xAE\xAF\xA0\xE2\xEC",
				"\x8A\xE2\xAE \xAF\xAE\xE2\xE0\xAE\xA3\xA0\xA5\xE2",
				"\x8A\xE2\xAE \xAF\xE0\xAE\xE7\xE2\xF1\xE2",
				"\x8A\xAE\xA3\xAE \xAD\xA0\xE3\xE7\xA8\xE2\xEC",
				"\x8A\xE2\xAE \xA2\xAE\xA7\xEC\xAC\xF1\xE2",
				"\x8A\xE2\xAE \xE3\xA4\xA0\xE0\xA8\xE2",
				"\x8A\xE2\xAE \xE3\xAA\xE0\xA0\xA4\xF1\xE2",
				"\x8A\xE2\xAE \xA4\xA0\xE1\xE2 \xA2\xA7\xEF\xE2\xAA\xE3",
				"\x8A\xE2\xAE \xA7\xA0\xAF\xAB\xA0\xE2\xA8\xE2",
				"\x8A\xE2\xAE \xAF\xE0\xA8\xE1\xEF\xA4\xA5\xE2",
				"\x8A\xE2\xAE \xAF\xAE\xAF\xE0\xAE\xA1\xE3\xA5\xE2",
				"\x8A\xE2\xAE \xA5\xA3\xAE \xAF\xAE\xA2\xA5\xE0\xAD\xF1\xE2",
				"\x8A\xE2\xAE \xA8\xE1\xAA\xE3\xAF\xAD\xF1\xE2\xE1\xEF",
				"\x8A\xE2\xAE \xA5\xA3\xAE \xE3\xAD\xA8\xE7\xE2\xAE\xA6\xA8\xE2",
				"\x8A\xE2\xAE \xA2\xEB\xA4\xA5\xE0\xAD\xA5\xE2",
				"\x8A\xE2\xAE \xE1\xAF\xE3\xE1\xE2\xA8\xE2\xE1\xEF",
				"\x8A\xE2\xAE \xA1\xE0\xAE\xE1\xA8\xE2 \xAC\xAE\xAD\xA5\xE2\xAA\xE3",
				"pray",
				"\x8A\xE2\xAE \xE1\xE2\xA0\xAD\xA5\xE2 \xE7\xAB\xA5\xAD\xAE\xAC",
				"act",
				"\x8A\xE2\xAE \xE5\xAE\xE7\xA5\xE2 \xE1\xEB\xA3\xE0\xA0\xE2\xEC",
				"\x8A\xE2\xAE \xAD\xA0\xA6\xAC\xF1\xE2",
				"rub",
				"pick",
				"eat",
				"sign",
				"close",
				"look",
				"try"
		};
		return _whoActions;
	}

	/*
	"������� �������",
	"������� �����",
	"�������� ������",
	"�������� ���������"
	*/
	const char **WHO_WILL_ACTIONS() {
		delete[] _whoWillActions;
		_whoWillActions = new const char *[4] {
			"\x8E\xE2\xAA\xE0\xEB\xE2\xEC \xE0\xA5\xE8\xF1\xE2\xAA\xE3",
				"\x8E\xE2\xAA\xE0\xEB\xE2\xEC \xA4\xA2\xA5\xE0\xEC",
				"\x90\xA0\xE1\xAA\xE0\xEB\xE2\xEC \xE1\xA2\xA8\xE2\xAE\xAA",
				"\x82\xEB\xA1\xA5\xE0\xA8\xE2\xA5 \xAF\xA5\xE0\xE1\xAE\xAD\xA0\xA6\xA0"
		};
		return _whoWillActions;
	}

	const char **DIRECTION_TEXT_UPPER() {
		delete[] _directionTextUpper;
		_directionTextUpper = new const char *[4] { "NORTH", "EAST", "SOUTH", "WEST" };
		return _directionTextUpper;
	}

	/*
	"�����",
	"������",
	"��",
	"�����"
	*/
	const char **DIRECTION_TEXT() {
		delete[] _directionText;
		_directionText = new const char *[4] {
			"\x91\xA5\xA2\xA5\xE0",
				"\x82\xAE\xE1\xE2\xAE\xAA",
				"\x9E\xA3",
				"\x87\xA0\xAF\xA0\xA4"
		};
		return _directionText;
	}

	/*
	"�������",
	"����",
	"�����",
	"����",
	"�������"
	*/
	const char **RACE_NAMES() {
		delete[] _raceNames;
		_raceNames = new const char *[5] {
			"\x97\xA5\xAB\xAE\xA2\xA5\xAA",
				"\x9D\xAB\xEC\xE4",
				"\x84\xA2\xAE\xE0\xE4",
				"\x83\xAD\xAE\xAC",
				"\x8F\xAE\xAB\xE3\xAE\xE0\xAA"
		};
		return _raceNames;
	}

	/*
	"�������",
	"�������",
	"����(-��)"
	*/
	const char **ALIGNMENT_NAMES() {
		delete[] _alignmentNames;
		_alignmentNames = new const char *[3] {
			"\x95\xAE\xE0\xAE\xE8\xA8\xA9",
				"\x8D\xA5\xA9\xE2\xE0\xA0\xAB",
				"\x87\xAB\xAE\xA9(-\xA0\xEF)"
		};
		return _alignmentNames;
	}

	/*
	"�������",
	"�������"
	*/
	const char **SEX_NAMES() {
		delete[] _sexNames;
		_sexNames = new const char *[2] {
			"\x8C\xE3\xA6\xE7\xA8\xAD\xA0",
				"\x86\xA5\xAD\xE9\xA8\xAD\xA0"
		};
		return _sexNames;
	}

	/*
	"���������\t100",
	"���������",
	"��������",
	"�����",
	"���������",
	"�����������",
	"������� �����������",
	"��������",
	"�����",
	"��������",
	"�������",
	"��������",
	"������� �������",
	"�������� �������",
	"������",
	"�������",
	"����������",
	"������� ���������"
	*/
	const char **SKILL_NAMES() {
		delete[] _skillNames;
		_skillNames = new const char *[18] {
			"\x82\xAE\xE0\xAE\xA2\xE1\xE2\xA2\xAE\t100",
				"\x8E\xE0\xE3\xA6\xA5\xA9\xAD\xA8\xAA",
				"\x80\xE1\xE2\xE0\xAE\xAB\xAE\xA3",
				"\x80\xE2\xAB\xA5\xE2",
				"\x8A\xA0\xE0\xE2\xAE\xA3\xE0\xA0\xE4",
				"\x8A\xE0\xA5\xE1\xE2\xAE\xAD\xAE\xE1\xA5\xE6",
				"\x97\xE3\xA2\xE1\xE2\xA2\xAE \xAD\xA0\xAF\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xEF",
				"\x9F\xA7\xEB\xAA\xAE\xA2\xA5\xA4",
				"\x8A\xE3\xAF\xA5\xE6",
				"\x91\xAA\xA0\xAB\xAE\xAB\xA0\xA7",
				"\x98\xE2\xE3\xE0\xAC\xA0\xAD",
				"\x91\xAB\xA5\xA4\xAE\xAF\xEB\xE2",
				"\x8C\xA0\xA3\xA8\xE1\xE2\xE0 \xAC\xAE\xAB\xA8\xE2\xA2\xEB",
				"\x8B\xAE\xA2\xAA\xAE\xE1\xE2\xEC \xAF\xA0\xAB\xEC\xE6\xA5\xA2",
				"\x8F\xAB\xAE\xA2\xA5\xE6",
				"\x8E\xE5\xAE\xE2\xAD\xA8\xAA",
				"\x82\xAE\xE1\xAF\xE0\xA8\xEF\xE2\xA8\xA5",
				"\x97\xE3\xA2\xE1\xE2\xA2\xAE \xAE\xAF\xA0\xE1\xAD\xAE\xE1\xE2\xA8"
		};
		return _skillNames;
	}

	/*
	"������",
	"�������",
	"������",
	"������",
	"���",
	"���",
	"������",
	"������",
	"�����",
	"��������",
	nullptr
	*/
	const char **CLASS_NAMES() {
		delete[] _classNames;
		_classNames = new const char *[11] {
			"\x90\xEB\xE6\xA0\xE0\xEC",
				"\x8F\xA0\xAB\xA0\xA4\xA8\xAD",
				"\x8B\xE3\xE7\xAD\xA8\xAA",
				"\x8A\xAB\xA8\xE0\xA8\xAA",
				"\x8C\xA0\xA3",
				"\x82\xAE\xE0",
				"\x8D\xA8\xAD\xA4\xA7\xEF",
				"\x82\xA0\xE0\xA2\xA0\xE0",
				"\x84\xE0\xE3\xA8\xA4",
				"\x91\xAB\xA5\xA4\xAE\xAF\xEB\xE2",
				nullptr
		};
		return _classNames;
	}

	/*
	"���������",
	"Heart Broken",
	"��������",
	"����������",
	"�������",
	"�������",
	"������(-�)",
	"��������",
	"���",
	"� ������",
	"� ��������",
	"�������",
	"�.��������",
	"�����",
	"������",
	"���������",
	"�������"
	*/
	const char **CONDITION_NAMES() {
		delete[] _conditionNames;
		_conditionNames = new const char *[17] {
			"\x8F\xE0\xAE\xAA\xAB\xEF\xE2\xA8\xA5",
				"Heart Broken",
				"\x91\xAB\xA0\xA1\xAE\xE1\xE2\xEC",
				"\x8E\xE2\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xA5",
				"\x81\xAE\xAB\xA5\xA7\xAD\xEC",
				"\x81\xA5\xA7\xE3\xAC\xA8\xA5",
				"\x82\xAB\xEE\xA1\xAB\xF1\xAD(-\xA0)",
				"\x8D\xA0\xA2\xA5\xE1\xA5\xAB\xA5",
				"\x91\xAE\xAD",
				"\x82 \xE3\xAD\xEB\xAD\xA8\xA8",
				"\x82 \xE1\xAC\xEF\xE2\xA5\xAD\xA8\xA8",
				"\x8F\xA0\xE0\xA0\xAB\xA8\xE7",
				"\x81.\xE1\xAE\xA7\xAD\xA0\xAD\xA8\xEF",
				"\x8C\xA5\xE0\xE2\xA2",
				"\x8A\xA0\xAC\xA5\xAD\xEC",
				"\x93\xAD\xA8\xE7\xE2\xAE\xA6\xA5\xAD",
				"\x95\xAE\xE0\xAE\xE8\xA5\xA5"
		};
		return _conditionNames;
	}

	/*
	"�������"
	*/
	const char *GOOD() {
		return "\x95\xAE\xE0\xAE\xE8\xA5\xA5";
	}

	/*
	"\n\t014�������������\t095%+d"
	*/
	const char *BLESSED() {
		return "\n\t014\x81\xAB\xA0\xA3\xAE\xE1\xAB\xAE\xA2\xA5\xAD\xA8\xA5\t095%+d";
	}

	/*
	"\n\t014������� ���\t095%+d";
	*/
	const char *POWER_SHIELD() {
		return "\n\t014\x91\xA8\xAB\xAE\xA2\xAE\xA9 \xE9\xA8\xE2\t095%+d";
	}

	/*
	"\n\t014������ ���\t095%+d"
	*/
	const char *HOLY_BONUS() {
		return "\n\t014\x91\xA2\xEF\xE2\xAE\xA9 \xA4\xA0\xE0\t095%+d";
	}

	/*
	"\n\t014��������\t095%+d"
	*/
	const char *HEROISM() {
		return "\n\t014\x8C\xE3\xA6\xA5\xE1\xE2\xA2\xAE\t095%+d";
	}

	/*
	"\f15� ������\fd"
	*/
	const char *IN_PARTY() {
		return "\f15\x82 \xAE\xE2\xE0\xEF\xA4\xA5\fd";
	}

	const char *PARTY_DETAILS() {
		return "\015\003l\002\014"
			   "00"
			   "\013"
			   "001"
			   "\011"
			   "035%s"
			   "\013"
			   "009"
			   "\011"
			   "035%s"
			   "\013"
			   "017"
			   "\011"
			   "035%s"
			   "\013"
			   "025"
			   "\011"
			   "035%s"
			   "\013"
			   "001"
			   "\011"
			   "136%s"
			   "\013"
			   "009"
			   "\011"
			   "136%s"
			   "\013"
			   "017"
			   "\011"
			   "136%s"
			   "\013"
			   "025"
			   "\011"
			   "136%s"
			   "\013"
			   "044"
			   "\011"
			   "035%s"
			   "\013"
			   "052"
			   "\011"
			   "035%s"
			   "\013"
			   "060"
			   "\011"
			   "035%s"
			   "\013"
			   "068"
			   "\011"
			   "035%s"
			   "\013"
			   "044"
			   "\011"
			   "136%s"
			   "\013"
			   "052"
			   "\011"
			   "136%s"
			   "\013"
			   "060"
			   "\011"
			   "136%s"
			   "\013"
			   "068"
			   "\011"
			   "136%s";
	}

	/*
	"%s\x2\x3""c\v106\t013�����\t048����\t083\f37�\fd���\t118\f37�\fd���"
	"\t153\f37�\fd���\t188�\f37�\fd���\x1";
	*/
	const char *PARTY_DIALOG_TEXT() {
		return "%s\x2\x3"
			   "c\v106\t013\x82\xA2\xA5\xE0\xE5\t048\x82\xAD\xA8\xA7\t083\f37\x93\fd\xA4\xA0\xAB\t118\f37\x82\fd\xEB\xA3\xAD"
			   "\t153\f37\x91\fd\xAE\xA7\xA4\t188\x82\f37\xEB\fd\xE5\xAE\xA4\x1";
	}

	/*
	"��� �� � ��� ��������������"
	*/
	const char *NO_ONE_TO_ADVENTURE_WITH() {
		return "\x82\xA0\xAC \xAD\xA5 \xE1 \xAA\xA5\xAC \xAF\xE3\xE2\xA5\xE8\xA5\xE1\xE2\xA2\xAE\xA2\xA0\xE2\xEC";
	}

	/*
	"��� ������ �����!"
	*/
	const char *YOUR_ROSTER_IS_FULL() {
		return "\x82\xA0\xE8 \xE1\xAF\xA8\xE1\xAE\xAA \xAF\xAE\xAB\xAE\xAD!";
	}

	/*
	"\fd\x3""c\t000\v002���������..."
	*/
	const char *PLEASE_WAIT() {
		return "\fd\x3"
			   "c\t000\v002\x8F\xAE\xA4\xAE\xA6\xA4\xA8\xE2\xA5...";
	}

	/*
	"\x3""c\t000\v002���..."
	*/
	const char *OOPS() {
		return "\x3"
			   "c\t000\v002\x93\xAF\xE1...";
	}

	/*
	"\r\x2\x3""c\v122\t013"
	"\f37�\fd��\t040\f37�\fd�\t067ESC"
	"\x1\t000\v000���� �����\v015\n"
	"����\x3""l\n"
	"���\x3""r\t000%s\x3""l\n"
	"����\x3""r\t000%s\x3""c\n"
	"\n"
	"�����\x3""l\n"
	"���\x3""r\t000%s\x3""l\n"
	"����\x3""r\t000%s";
	*/
	const char *BANK_TEXT() {
		return "\r\x2\x3"
			   "c\v122\t013"
			   "\f37\x82\fd\xAA\xAB\t040\f37\x91\fd\xAD\t067ESC"
			   "\x1\t000\v000\x81\xA0\xAD\xAA \x8A\xE1\xA8\xAD\xA0\v015\n"
			   "\x81\xA0\xAD\xAA\x3"
			   "l\n"
			   "\x87\xAE\xAB\x3"
			   "r\t000%s\x3"
			   "l\n"
			   "\x80\xAB\xAC\xA7\x3"
			   "r\t000%s\x3"
			   "c\n"
			   "\n"
			   "\x8E\xE2\xE0\xEF\xA4\x3"
			   "l\n"
			   "\x87\xAE\xAB\x3"
			   "r\t000%s\x3"
			   "l\n"
			   "\x80\xAB\xAC\xA7\x3"
			   "r\t000%s";
	}

	/*
	"\x1\r\x3""c\v000\t000"
	"�����\t039\v027%s\x3""l\v046\n"
	"\t011\f37�\fd�������\n"
	"\t000\v090���\x3""r\t000%s"
	"\x2\x3""c\v122\t040ESC\x1";
	*/
	const char *BLACKSMITH_TEXT() {
		return "\x1\r\x3"
			   "c\v000\t000"
			   "\x8B\xA0\xA2\xAA\xA0\t039\v027%s\x3"
			   "l\v046\n"
			   "\t011\f37\x91\fd\xAC\xAE\xE2\xE0\xA5\xE2\xEC\n"
			   "\t000\v090\x87\xAE\xAB\x3"
			   "r\t000%s"
			   "\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	/*
	"\n�� ������ ���� ������ �������, ����� �������� ����������."
	*/
	const char *GUILD_NOT_MEMBER_TEXT() {
		return "\n\x82\xEB \xA4\xAE\xAB\xA6\xAD\xEB \xA1\xEB\xE2\xEC \xE7\xAB\xA5\xAD\xAE\xAC \xA3\xA8\xAB\xEC\xA4\xA8\xA8, \xE7\xE2\xAE\xA1\xEB \xAF\xAE\xAA\xE3\xAF\xA0\xE2\xEC \xA7\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xEF.";
	}

	/*
	"\x3""c\v027\t039%s"
	"\x3""l\v046\n"
	"\t012\f37�\fd�����\n"
	"\t012\f37�\fd�������";
	*/
	const char *GUILD_TEXT() {
		return "\x3"
			   "c\v027\t039%s"
			   "\x3"
			   "l\v046\n"
			   "\t012\f37\x8A\fd\xE3\xAF\xA8\xE2\xEC\n"
			   "\t012\f37\x8E\fd\xAF\xA8\xE1\xA0\xAD\xA8\xEF";
	}

	/*
	"\r\x3""c\v000\t000�������\t039"
	"\v027%s%s\x3""l\t000"
	"\v090���\x3""r\t000%s\x2\x3""c\v122"
	"\t021\f37�\fd����\t060ESC\x1";
	*/
	const char *TAVERN_TEXT() {
		return "\r\x3"
			   "c\v000\t000\x92\xA0\xA2\xA5\xE0\xAD\xA0\t039"
			   "\v027%s%s\x3"
			   "l\t000"
			   "\v090\x87\xAE\xAB\x3"
			   "r\t000%s\x2\x3"
			   "c\v122"
			   "\t021\f37\x8E\fd\xE2\xA4\xEB\xE5\t060ESC\x1";
	}

	/*
	"\x3""l\t017\v046\f37�\fd�����\n"
	"\t017\f37�\fd��\n"
	"\t017\f37�\fd�����\n"
	"\t017\f37�\fd����";
	*/
	const char *FOOD_AND_DRINK() {
		return "\x3"
			   "l\t017\v046\f37\x82\fd\xEB\xAF\xA8\xE2\xEC\n"
			   "\t017\f37\x85\fd\xA4\xA0\n"
			   "\t017\f37\x97\fd\xA0\xA5\xA2\xEB\xA5\n"
			   "\t017\f37\x91\fd\xAB\xE3\xE5\xA8";
	}

	/*
	"\n"
	"\n"
	"�������\n"
	"\n"
	"������� �������!";
	*/
	const char *GOOD_STUFF() {
		return "\n"
			   "\n"
			   "\x8D\xA5\xA4\xE3\xE0\xAD\xAE\n"
			   "\n"
			   "\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3!";
	}

	/*
	"\n������ �������\n\n������� �������!"
	*/
	const char *HAVE_A_DRINK() {
		return "\n\x91\xAF\xA5\xE0\xA2\xA0 \xA2\xEB\xAF\xA5\xA9\xE2\xA5\n\n\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3!";
	}

	/*
	"\n\n�� �����\n\n������� �������!"
	*/
	const char *YOURE_DRUNK() {
		return "\n\n\x82\xEB \xAF\xEC\xEF\xAD\xEB\n\n\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3!";
	}

	/*
	"\r\x3""c\v000\t000����"
	"\t039\v027%s\x3""l\t000\v046"
	"\f37�\fd�����\x3""r\t000%lu\x3""l\n"
	"\f37�\fd��\x3""r\t000%lu\x3""l\n"
	"\f37�\fd������\x3""r\t000%s"
	"\x3""l\t000\v090���\x3""r\t000%s"
	"\x2\x3""c\v122\t040ESC\x1";
	*/
	const char *TEMPLE_TEXT() {
		return "\r\x3"
			   "c\v000\t000\x95\xE0\xA0\xAC"
			   "\t039\v027%s\x3"
			   "l\t000\v046"
			   "\f37\x8B\fd\xA5\xE7\xA8\xE2\xEC\x3"
			   "r\t000%lu\x3"
			   "l\n"
			   "\f37\x84\fd\xA0\xE0\x3"
			   "r\t000%lu\x3"
			   "l\n"
			   "\f37\x91\fd\xAD\x8F\xE0\xAE\xAA\xAB\x3"
			   "r\t000%s"
			   "\x3"
			   "l\t000\v090\x87\xAE\xAB\x3"
			   "r\t000%s"
			   "\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	/*
	"%s\n����� %lu ����� ����� ��� ������ %u."
	*/
	const char *EXPERIENCE_FOR_LEVEL() {
		return "%s\n\x8D\xE3\xA6\xAD\xAE %lu \xAE\xE7\xAA\xAE\xA2 \xAE\xAF\xEB\xE2\xA0 \xA4\xAB\xEF \xE3\xE0\xAE\xA2\xAD\xEF %u.";
	}

	/*
	"%s\n�� ��� ������ ��, ���� �� ����� ��� �������!"
	*/
	const char *TRAINING_LEARNED_ALL() {
		return "%s\n\x82\xEB \xE3\xA6\xA5 \xA7\xAD\xA0\xA5\xE2\xA5 \xA2\xE1\xF1, \xE7\xA5\xAC\xE3 \xAC\xEB \xAC\xAE\xA6\xA5\xAC \xA2\xA0\xE1 \xAD\xA0\xE3\xE7\xA8\xE2\xEC!";
	}

	/*
	"%s ����� ��������� �� ������ %d.\x3""l\n"
	"\v081����\x3""r\t000%lu";
	*/
	const char *ELIGIBLE_FOR_LEVEL() {
		return "%s \xAC\xAE\xA6\xA5\xE2 \xAE\xA1\xE3\xE7\xA0\xE2\xEC\xE1\xEF \xA4\xAE \xE3\xE0\xAE\xA2\xAD\xEF %d.\x3"
			   "l\n"
			   "\v081\x96\xA5\xAD\xA0\x3"
			   "r\t000%lu";
	}

	/*
	"\r\x3""c��������\n"
	"\n"
	"%s\x3""l\v090\t000���\x3""r\t000%s\x2\x3""c\v122\t021"
	"\f37�\fd�������\t060ESC\x1";
	*/
	const char *TRAINING_TEXT() {
		return "\r\x3"
			   "c\x8E\xA1\xE3\xE7\xA5\xAD\xA8\xA5\n"
			   "\n"
			   "%s\x3"
			   "l\v090\t000\x87\xAE\xAB\x3"
			   "r\t000%s\x2\x3"
			   "c\v122\t021"
			   "\f37\x8E\fd\xA1\xE3\xE7\xA5\xAD\xA8\xA5\t060ESC\x1";
	}

	/*
	"\x3""c\v000\t000%s\x3""l\n"
	"\n"
	"���\x3""r\t000%s\x3""l\n"
	"����\x3""r\t000%s\x2\x3""c\v096\t007\f37�\fd���\t035\f37�\fd��\t067ESC\x1";
	*/
	const char *GOLD_GEMS() {
		return "\x3"
			   "c\v000\t000%s\x3"
			   "l\n"
			   "\n"
			   "\x87\xAE\xAB\x3"
			   "r\t000%s\x3"
			   "l\n"
			   "\x80\xAB\xAC\xA7\x3"
			   "r\t000%s\x2\x3"
			   "c\v096\t007\f37\x80\fd\xAB\xAC\xA7\t035\f37\x87\fd\xAE\xAB\t067ESC\x1";
	}

	/*
	"\t000\v000\x3""c%s\x3""l\n"
	"\n"
	"\x4""077���\x3""r\t000%s\x3""l\n"
	"\x4""077����\x3""r\t000%s\x3""l\t000\v051\x4""077\n"
	"\x4""077";
	*/
	const char *GOLD_GEMS_2() {
		return "\t000\v000\x3"
			   "c%s\x3"
			   "l\n"
			   "\n"
			   "\x4"
			   "077\x87\xAE\xAB\x3"
			   "r\t000%s\x3"
			   "l\n"
			   "\x4"
			   "077\x80\xAB\xAC\xA7\x3"
			   "r\t000%s\x3"
			   "l\t000\v051\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	/*
	"�������� �� ����", "����� �� �����"
	*/
	const char **DEPOSIT_WITHDRAWL() {
		delete[] _depositWithdrawl;
		_depositWithdrawl = new const char *[2] { "\x8F\xAE\xAB\xAE\xA6\xA8\xE2\xEC \xAD\xA0 \xE1\xE7\xF1\xE2", "\x91\xAD\xEF\xE2\xEC \xE1\xAE \xE1\xE7\xF1\xE2\xA0" };
		return _depositWithdrawl;
	}

	/*
	"\x3""c\v012������������ %s %s!\x3""l"
	*/
	const char *NOT_ENOUGH_X_IN_THE_Y() {
		return "\x3"
			   "c\v012\x8D\xA5\xA4\xAE\xE1\xE2\xA0\xE2\xAE\xE7\xAD\xAE %s %s!\x3"
			   "l";
	}

	/*
	"\x3""c\v012��� %s %s!\x3""l"
	*/
	const char *NO_X_IN_THE_Y() {
		return "\x3"
			   "c\v012\x8D\xA5\xE2 %s %s!\x3"
			   "l";
	}

	/*
	"����", "���������", "���� ����", "��������", "��������",
	"��������", "�����", "�������", "�������", "����� ������", "��������",
	"���� �����", "������������", "������", "�������", "����"
	*/
	const char **STAT_NAMES() {
		delete[] _statNames;
		_statNames = new const char *[16] {
			"\x91\xA8\xAB\xA0", "\x88\xAD\xE2\xA5\xAB\xAB\xA5\xAA\xE2", "\x91\xA8\xAB\xA0 \xA4\xE3\xE5\xA0", "\x91\xAB\xAE\xA6\xA5\xAD\xA8\xA5", "\x91\xAA\xAE\xE0\xAE\xE1\xE2\xEC",
				"\x8C\xA5\xE2\xAA\xAE\xE1\xE2\xEC", "\x93\xA4\xA0\xE7\xA0", "\x82\xAE\xA7\xE0\xA0\xE1\xE2", "\x93\xE0\xAE\xA2\xA5\xAD\xEC", "\x8A\xAB\xA0\xE1\xE1 \xA7\xA0\xE9\xA8\xE2\xEB", "\x87\xA4\xAE\xE0\xAE\xA2\xEC\xA5",
				"\x8E\xE7\xAA\xA8 \xAC\xA0\xA3\xA8\xA8", "\x93\xE1\xE2\xAE\xA9\xE7\xA8\xA2\xAE\xE1\xE2\xA8", "\x8D\xA0\xA2\xEB\xAA\xA8", "\x8D\xA0\xA3\xE0\xA0\xA4\xEB", "\x8E\xAF\xEB\xE2"
		};
		return _statNames;
	}

	/*
	"������", "������", "����", "���������"
	*/
	const char **CONSUMABLE_NAMES() {
		delete[] _consumableNames;
		_consumableNames = new const char *[4] { "\x87\xAE\xAB\xAE\xE2\xAE", "\x80\xAB\xAC\xA0\xA7\xEB", "\x8F\xA8\xE9\xA0", "\x91\xAE\xE1\xE2\xAE\xEF\xAD\xA8\xA5" };
		return _consumableNames;
	}

	/*
	"� ������", "� �����"
	*/
	const char **WHERE_NAMES() {
		delete[] _whereNames;
		_whereNames = new const char *[2] { "\xE3 \xAE\xE2\xE0\xEF\xA4\xA0", "\xA2 \xA1\xA0\xAD\xAA\xA5" };
		return _whereNames;
	}

	/*
	"\x3""c\t000\v051�������\x3""l\n"
	*/
	const char *AMOUNT() {
		return "\x3"
			   "c\t000\v051\x91\xAA\xAE\xAB\xEC\xAA\xAE\x3"
			   "l\n";
	}

	/*
	"\v007���� ������� ��� ���������!"
	*/
	const char *FOOD_PACKS_FULL() {
		return "\v007\x82\xA0\xE8\xA8 \xE0\xEE\xAA\xA7\xA0\xAA\xA8 \xE3\xA6\xA5 \xAD\xA0\xAF\xAE\xAB\xAD\xA5\xAD\xEB!";
	}

	/*
	"\x3""c\v027\t039%s\x3""l\v046\n"
	"\t012\f37�\fd�����\n"
	"\t012\f37�\fd�������";
	*/
	const char *BUY_SPELLS() {
		return "\x3"
			   "c\v027\t039%s\x3"
			   "l\v046\n"
			   "\t012\f37\x8A\fd\xE3\xAF\xA8\xE2\xEC\n"
			   "\t012\f37\x8E\fd\xAF\xA8\xE1\xA0\xAD\xA8\xEF";
	}

	/*
	"\x1\r\f00\x3""c\v000\t000�������\n"
	"%s\x3""l\t000\v090���"
	"\x3""r\t000%s\x2\x3""c\v122\t040ESC\x1";
	*/
	const char *GUILD_OPTIONS() {
		return "\x1\r\f00\x3"
			   "c\v000\t000\x83\xA8\xAB\xEC\xA4\xA8\xEF\n"
			   "%s\x3"
			   "l\t000\v090\x87\xAE\xAB"
			   "\x3"
			   "r\t000%s\x2\x3"
			   "c\v122\t040ESC\x1";
	}

	/*
	"�� �� �����������..."
	*/
	const char *NOT_A_SPELL_CASTER() {
		return "\x82\xEB \xAD\xA5 \xA7\xA0\xAA\xAB\xA8\xAD\xA0\xE2\xA5\xAB\xEC...";
	}

	/*
	"�� ��� ������ ��,\n"
	"\t010���� �� ����� ���\n"
	"\t010�������!\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n"
	"\n";
	*/
	const char *SPELLS_LEARNED_ALL() {
		return "\x82\xEB \xE3\xA6\xA5 \xA7\xAD\xA0\xA5\xE2\xA5 \xA2\xE1\xF1,\n"
			   "\t010\xE7\xA5\xAC\xE3 \xAC\xEB \xAC\xAE\xA6\xA5\xAC \xA2\xA0\xE1\n"
			   "\t010\xAD\xA0\xE3\xE7\xA8\xE2\xEC!\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n"
			   "\n";
	}

	/*
	"\r\fd%s\x2\x3""c\t000\v002����������"
	*/
	const char *SPELLS_FOR() {
		return "\r\fd%s\x2\x3"
			   "c\t000\v002\x87\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xEF";
	}

	const char *SPELL_LINES_0_TO_9() {
		return "\x2\x3l\v015\t0011\n2\n3\n4\n5\n6\n7\n8\n9\n0";
	}

	const char *SPELLS_DIALOG_SPELLS() {
		return "\x3l\v015"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l\n"
			   "\t010\f%2u%s\fd\x3l"
			   "\t004\v110%s - %u\x1";
	}

	/*
	"����"
	*/
	const char *SPELL_PTS() {
		return "\x8C\xA0\xAD\xA0";
	}

	/*
	"���"
	*/
	const char *GOLD() {
		return "\x87\xAE\xAB";
	}

	/*
	"\x3""c\f09%s\fd\x3""l\n"
	"\n"
	"%s\x3""c\t000\v110�. �������!";
	*/
	const char *SPELL_INFO() {
		return "\x3"
			   "c\f09%s\fd\x3"
			   "l\n"
			   "\n"
			   "%s\x3"
			   "c\t000\v110\x8D. \xAA\xAB\xA0\xA2\xA8\xE8\xE3!";
	}

	/*
	"\x3""l\v000\t000\fd%s  ������� ���������� "
	"\f09%s\fd �� %u?";
	*/
	const char *SPELL_PURCHASE() {
		return "\x3"
			   "l\v000\t000\fd%s  \x86\xA5\xAB\xA0\xA5\xE2\xA5 \xAF\xE0\xA8\xAE\xA1\xE0\xA5\xE1\xE2\xA8 "
			   "\f09%s\fd \xA7\xA0 %u?";
	}

	/*
	"\x3""c\v000\t000%s\x3l\v139"
	"\t000X = %d\x3r\t000Y = %d\x3""c\t000%s";
	*/
	const char *MAP_TEXT() {
		return "\x3"
			   "c\v000\t000%s\x3l\v139"
			   "\t000X = %d\x3r\t000Y = %d\x3"
			   "c\t000%s";
	}

	/*
	"\x3""l\n\n\t024����\x3""r\t124%u";
	*/
	const char *LIGHT_COUNT_TEXT() {
		return "\x3"
			   "l\n\n\t024\x91\xA2\xA5\xE2\x3"
			   "r\t124%u";
	}

	/*
	"%c%s�����%s%u"
	*/
	const char *FIRE_RESISTENCE_TEXT() {
		return "%c%s\x8E\xA3\xAE\xAD\xEC%s%u";
	}

	/*
	"%c%s����%s%u"
	*/
	const char *ELECRICITY_RESISTENCE_TEXT() {
		return "%c%s\x83\xE0\xAE\xAC%s%u";
	}

	/*
	"%c%s�����%s%u"
	*/
	const char *COLD_RESISTENCE_TEXT() {
		return "%c%s\x95\xAE\xAB\xAE\xA4%s%u";
	}

	/*
	"%c%s��/�������%s%u"
	*/
	const char *POISON_RESISTENCE_TEXT() {
		return "%c%s\x9F\xA4/\x8A\xA8\xE1\xAB\xAE\xE2\xA0%s%u";
	}

	/*
	"%c%s�����������%s"
	*/
	const char *CLAIRVOYANCE_TEXT() {
		return "%c%s\x9F\xE1\xAD\xAE\xA2\xA8\xA4\xA5\xAD\xA8\xA5%s";
	}

	/*
	"%c%s���������%s"
	*/
	const char *LEVITATE_TEXT() {
		return "%c%s\x8B\xA5\xA2\xA8\xE2\xA0\xE6\xA8\xEF%s";
	}

	/*
	"%c%s�������� �� ����"
	*/
	const char *WALK_ON_WATER_TEXT() {
		return "%c%s\x95\xAE\xA6\xA4\xA5\xAD\xA8\xA5 \xAF\xAE \xA2\xAE\xA4\xA5";
	}

	/*
	"\r\x3""c\t000\v001\f37%s �����\fd\n"
	"������� ����������\n"
	"\n"
	"������� \f37%s\fd\n"
	"\n"
	"\t032�����\t072����\t112���\n"
	"\t032\f37%d:%02d%c\t072%u\t112%u\fd%s";
	*/
	const char *GAME_INFORMATION() {
		return "\r\x3"
			   "c\t000\v001\f37%s \x8A\xE1\xA8\xAD\xA0\fd\n"
			   "\x88\xA3\xE0\xAE\xA2\xA0\xEF \xA8\xAD\xE4\xAE\xE0\xAC\xA0\xE6\xA8\xEF\n"
			   "\n"
			   "\x91\xA5\xA3\xAE\xA4\xAD\xEF \f37%s\fd\n"
			   "\n"
			   "\t032\x82\xE0\xA5\xAC\xEF\t072\x84\xA5\xAD\xEC\t112\x83\xAE\xA4\n"
			   "\t032\f37%d:%02d%c\t072%u\t112%u\fd%s";
	}

	const char *WORLD_GAME_TEXT() {
		return "World";
	}

	const char *DARKSIDE_GAME_TEXT() {
		return "Darkside";
	}

	/*
	"������"
	*/
	const char *CLOUDS_GAME_TEXT() {
		return "\x8E\xA1\xAB\xA0\xAA\xA0";
	}

	const char *SWORDS_GAME_TEXT() {
		return "Swords";
	}

	/*
	"����������", "���������", "���������", "����������", "�����������",
	"��������",   "���������", "��������",  "����������", "����������"
	*/
	const char **WEEK_DAY_STRINGS() {
		delete[] _weekDayStrings;
		_weekDayStrings = new const char *[10] {
			"\xA4\xA5\xE1\xEF\xE2\xA8\xA4\xA5\xAD\xEC", "\xAF\xA5\xE0\xA2\xAE\xA4\xA5\xAD\xEC", "\xA2\xE2\xAE\xE0\xAE\xA4\xA5\xAD\xEC", "\xE2\xE0\xA5\xE2\xEC\xA5\xA4\xA5\xAD\xEC", "\xE7\xA5\xE2\xA2\xA5\xE0\xAE\xA4\xA5\xAD\xEC",
				"\xAF\xEF\xE2\xA8\xA4\xA5\xAD\xEC", "\xE8\xA5\xE1\xE2\xA8\xA4\xA5\xAD\xEC", "\xE1\xA5\xAC\xA8\xA4\xA5\xAD\xEC", "\xA2\xAE\xE1\xEC\xAC\xA8\xA4\xA5\xAD\xEC", "\xA4\xA5\xA2\xEF\xE2\xA8\xA4\xA5\xAD\xEC"
		};
		return _weekDayStrings;
	}

	const char *CHARACTER_DETAILS() {
		return "\x3"
			   "l\v041\t196%s\t000\v002%s : %s %s %s"
			   "\x3"
			   "r\t053\v028\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3"
			   "l\t131\f%02u%d\fd\t196\f15%lu\fd\x3"
			   "r"
			   "\t053\v051\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3"
			   "l\t131\f%02u%u\fd\t196\f15%lu\fd"
			   "\x3"
			   "r\t053\v074\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3"
			   "l\t131\f15%u\fd\t196\f15%lu\fd"
			   "\x3"
			   "r\t053\v097\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3"
			   "l\t131\f15%u\fd\t196\f15%u %s\fd"
			   "\x3"
			   "r\t053\v120\f%02u%u\fd\t103\f%02u%u\fd"
			   "\x3"
			   "l\t131\f15%u\fd\t196\f%02u%s\fd"
			   "\t245%s%s%s%s\fd";
	}

	/*
	"����", "���", "����"
	*/
	const char **DAYS() {
		delete[] _days;
		_days = new const char *[3] { "\xA4\xA5\xAD\xEC", "\xA4\xAD\xEF", "\xA4\xAD\xA5\xA9" };
		return _days;
	}

	/*
	"������"
	*/
	const char *PARTY_GOLD() {
		return "\x87\xAE\xAB\xAE\xE2\xAE";
	}

	const char *PLUS_14() {
		return "\f14+";
	}

	/*
	"\x1\f00\r\x3""l\t029\v018���\t080���\t131����\t196����"
	"\t029\v041���\t080���\t131��\t029\v064���\t080���"
	"\t131���\t196������\t029\v087���\t080���\t131���"
	"\t196����\t029\v110���\t080��\t131����\t196���������"
	"\x3""c\t290\v025\f37�\fd���\t290\v057\f37�"
	"\fd���\t290\v089\f37�\fd���\t290\v121�����\x3""l%s";
	*/
	const char *CHARACTER_TEMPLATE() {
		return "\x1\f00\r\x3"
			   "l\t029\v018\x91\xA8\xAB\t080\x8C\xE2\xAA\t131\x87\xA4\xAE\xE0\t196\x8E\xAF\xEB\xE2"
			   "\t029\v041\x88\xAD\xE2\t080\x93\xA4\xE7\t131\x8E\x8C\t029\v064\x84\xE3\xE5\t080\x82\xA7\xE0"
			   "\t131\x93\xE1\xE2\t196\x80\xAB\xAC\xA0\xA7\xEB\t029\v087\x91\xAB\xA6\t080\x93\xE0\xA2\t131\x8D\xA2\xAA"
			   "\t196\x8F\xA8\xE9\xA0\t029\v110\x91\xAA\xE0\t080\x8A\x87\t131\x8D\xA0\xA3\xE0\t196\x91\xAE\xE1\xE2\xAE\xEF\xAD\xA8\xA5"
			   "\x3"
			   "c\t290\v025\f37\x82\fd\xA5\xE9\xA8\t290\v057\f37\x81"
			   "\fd\xE1\xE2\xE0\t290\v089\f37\x91\fd\xAC\xA5\xAD\t290\v121\x82\xEB\xE5\xAE\xA4\x3"
			   "l%s";
	}

	/*
	"\x3""c\v007\t000�� ����� ����� ��������� �������� �������!"
	*/
	const char *EXCHANGING_IN_COMBAT() {
		return "\x3"
			   "c\v007\t000\x82\xAE \xA2\xE0\xA5\xAC\xEF \xA1\xA8\xE2\xA2\xEB \xA7\xA0\xAF\xE0\xA5\xE9\xA5\xAD\xAE \xAC\xA5\xAD\xEF\xE2\xEC\xE1\xEF \xAC\xA5\xE1\xE2\xA0\xAC\xA8!";
	}

	/*
	"\x2\x3""c%s\n"
	"������ / ��������\n"
	"\x3""r\t054%lu\x3""l\t058/ %lu\n"
	"\x3""c����: %s\x1";
	*/
	const char *CURRENT_MAXIMUM_RATING_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "\x91\xA5\xA9\xE7\xA0\xE1 / \x8C\xA0\xAA\xE1\xA8\xAC\xE3\xAC\n"
			   "\x3"
			   "r\t054%lu\x3"
			   "l\t058/ %lu\n"
			   "\x3"
			   "c\x90\xA0\xAD\xA3: %s\x1";
	}

	/*
	"\x2\x3""c%s\n"
	"������ / ��������\n"
	"\x3""r\t054%d\x3""l\t058/ %lu";
	*/
	const char *CURRENT_MAXIMUM_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "\x91\xA5\xA9\xE7\xA0\xE1 / \x8C\xA0\xAA\xE1\xA8\xAC\xE3\xAC\n"
			   "\x3"
			   "r\t054%d\x3"
			   "l\t058/ %lu";
	}

	/*
	"���", "��������", "������", "��.������", "������", "�������", "�������",
	"��������", "�������", "����������", "�������", "��������", "������������",
	"�����������", "����������", "������������", "������������", "�������������",
	"�����������", "���������", "������������", "�����������", "����������",
	"����������"
	*/
	const char **RATING_TEXT() {
		delete[] _ratingText;
		_ratingText = new const char *[24] {
			"\x8D\xA5\xE2", "\x8C\xA8\xA7\xA5\xE0\xAD\xEB\xA9", "\x86\xA0\xAB\xAA\xA8\xA9", "\x8E\xE7.\xAD\xA8\xA7\xAA\xA8\xA9", "\x8D\xA8\xA7\xAA\xA8\xA9", "\x91\xE0\xA5\xA4\xAD\xA8\xA9", "\x95\xAE\xE0\xAE\xE8\xA8\xA9",
				"\x8E\xE2\xAB\xA8\xE7\xAD\xEB\xA9", "\x82\xEB\xE1\xAE\xAA\xA8\xA9", "\x82\xEB\xE1\xAE\xE7\xA5\xAD\xAD\xEB\xA9", "\x81\xAE\xAB\xEC\xE8\xAE\xA9", "\x8E\xA3\xE0\xAE\xAC\xAD\xEB\xA9", "\x93\xA4\xA8\xA2\xA8\xE2\xA5\xAB\xEC\xAD\xEB\xA9",
				"\x8D\xA5\xA2\xA5\xE0\xAE\xEF\xE2\xAD\xEB\xA9", "\x83\xA8\xA3\xA0\xAD\xE2\xE1\xAA\xA8\xA9", "\x94\xA0\xAD\xE2\xA0\xE1\xE2\xA8\xE7\xAD\xEB\xA9", "\x88\xA7\xE3\xAC\xA8\xE2\xA5\xAB\xEC\xAD\xEB\xA9", "\x8F\xAE\xE0\xA0\xA7\xA8\xE2\xA5\xAB\xEC\xAD\xEB\xA9",
				"\x83\xE0\xA0\xAD\xA4\xA8\xAE\xA7\xAD\xEB\xA9", "\x83\xE0\xAE\xAC\xA0\xA4\xAD\xEB\xA9", "\x8A\xAE\xAB\xAE\xE1\xE1\xA0\xAB\xEC\xAD\xEB\xA9", "\x8F\xAE\xE2\xE0\xEF\xE1\xA0\xEE\xE9\xA8\xA9", "\x97\xE3\xA4\xAE\xA2\xA8\xE9\xAD\xEB\xA9",
				"\x82\xEB\xE1\xAE\xE7\xA0\xA9\xE8\xA8\xA9"
		};
		return _ratingText;
	}

	/*
	"\x2\x3""c%s\n"
	"������� / ���������\n"
	"\x3""r\t057%u\x3""l\t061/ %u\n"
	"\x3""c�����%s: %u / %u\x1";
	*/
	const char *AGE_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "\x92\xA5\xAA\xE3\xE9\xA8\xA9 / \x8D\xA0\xE1\xE2\xAE\xEF\xE9\xA8\xA9\n"
			   "\x3"
			   "r\t057%u\x3"
			   "l\t061/ %u\n"
			   "\x3"
			   "c\x90\xAE\xA4\xA8\xAB%s: %u / %u\x1";
	}

	/*
	"\x2\x3""c%s\n"
	"������� / ��������\n"
	"\x3""r\t054%u\x3""l\t058/ %u\n"
	"\x3""c%u ����%s/�����\x1";
	*/
	const char *LEVEL_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "\x92\xA5\xAA\xE3\xE9\xA8\xA9 / \x8C\xA0\xAA\xE1\xA8\xAC\xE3\xAC\n"
			   "\x3"
			   "r\t054%u\x3"
			   "l\t058/ %u\n"
			   "\x3"
			   "c%u \x80\xE2\xA0\xAA%s/\x90\xA0\xE3\xAD\xA4\x1";
	}

	/*
	"\x2\x3""c%s\x3""l\n"
	"\t020�����\t100%u\n"
	"\t020�����\t100%u\n"
	"\t020����\t100%u\n"
	"\t020��\t100%u\n"
	"\t020�������\t100%u\n"
	"\t020�����\t100%u";
	*/
	const char *RESISTENCES_TEXT() {
		return "\x2\x3"
			   "c%s\x3"
			   "l\n"
			   "\t020\x8E\xA3\xAE\xAD\xEC\t100%u\n"
			   "\t020\x95\xAE\xAB\xAE\xA4\t100%u\n"
			   "\t020\x83\xE0\xAE\xAC\t100%u\n"
			   "\t020\x9F\xA4\t100%u\n"
			   "\t020\x9D\xAD\xA5\xE0\xA3\xA8\xEF\t100%u\n"
			   "\t020\x8C\xA0\xA3\xA8\xEF\t100%u";
	}

	/*
	"\n\t012���"
	*/
	const char *NONE() {
		return "\n\t012\x8D\xA5\xE2";
	}

	/*
	"\x2\x3""c%s\x3""l\n"
	"\t005�������:\t070%lu\n"
	"\t005��. �������:\t070%s\x1";
	*/
	const char *EXPERIENCE_TEXT() {
		return "\x2\x3"
			   "c%s\x3"
			   "l\n"
			   "\t005\x92\xA5\xAA\xE3\xE9\xA8\xA9:\t070%lu\n"
			   "\t005\x91\xAB. \xE3\xE0\xAE\xA2\xA5\xAD\xEC:\t070%s\x1";
	}

	/*
	"\f12��������\fd"
	*/
	const char *ELIGIBLE() {
		return "\f12\x84\xAE\xE1\xE2\xE3\xAF\xA5\xAD\fd";
	}

	/*
	"\x2\x3""c%s\n"
	"%lu � �����\n"
	"%lu � �����\x1\x3""l";
	*/
	const char *IN_PARTY_IN_BANK() {
		return "\x2\x3"
			   "c%s\n"
			   "%lu \xE1 \xE1\xAE\xA1\xAE\xA9\n"
			   "%lu \xA2 \xA1\xA0\xAD\xAA\xA5\x1\x3"
			   "l";
	}

	/*
	"\x2\x3""c%s\n"
	"%u �����%c\n"
	"������ �� %u %s\x1\x3""l";
	*/
	const char *FOOD_TEXT() {
		return "\x2\x3"
			   "c%s\n"
			   "%u \xAF\xAE\xE0\xE6\xA8%c\n"
			   "\x95\xA2\xA0\xE2\xA8\xE2 \xAD\xA0 %u %s\x1\x3"
			   "l";
	}

	/*
	"\t010\v005���������� � ���?"
	*/
	const char *EXCHANGE_WITH_WHOM() {
		return "\t010\v005\x8F\xAE\xAC\xA5\xAD\xEF\xE2\xEC\xE1\xEF \xE1 \xAA\xA5\xAC?";
	}

	const char *QUICK_REF_LINE() {
		return "\v%3d\t007%u)\t027%s\t110%c%c%c\x3r\t160\f%02u%u\fd"
			   "\x3l\t170\f%02u%d\fd\t208\f%02u%u\fd\t247\f"
			   "%02u%u\fd\t270\f%02u%c%c%c%c\fd";
	}

	/*
	"\r\x3""c������� c������\v012\x3""l"
	"\t007#\t027���\t110���\t147���\t174����"
	"\t214��\t242��\t270����"
	"%s%s%s%s%s%s%s%s"
	"\v110\t064\x3""c������\t144������\t224����\v119"
	"\t064\f15%lu\t144%lu\t224%u %s\fd";
	*/
	const char *QUICK_REFERENCE() {
		return "\r\x3"
			   "c\x81\xEB\xE1\xE2\xE0\xA0\xEF c\xAF\xE0\xA0\xA2\xAA\xA0\v012\x3"
			   "l"
			   "\t007#\t027\x88\xAC\xEF\t110\x8A\xAB\xE1\t147\x93\xE0\xA2\t174\x87\xA4\xAE\xE0"
			   "\t214\x8E\x8C\t242\x8A\x87\t270\x91\xAE\xE1\xE2"
			   "%s%s%s%s%s%s%s%s"
			   "\v110\t064\x3"
			   "c\x87\xAE\xAB\xAE\xE2\xAE\t144\x80\xAB\xAC\xA0\xA7\xEB\t224\x8F\xA8\xE9\xA0\v119"
			   "\t064\f15%lu\t144%lu\t224%u %s\fd";
	}

	/*
	"\r\x2\x3""c\v021\t017���\f37�\fd\t051\f37�\fd����\t085"
	"\f37�\fd����\t119\f37�\fd���\t153%s\t187%s\t221%s\t255%s\t289�����";
	*/
	const char *ITEMS_DIALOG_TEXT1() {
		return "\r\x2\x3"
			   "c\v021\t017\x8E\xE0\xE3\f37\xA6\fd\t051\f37\x81\fd\xE0\xAE\xAD\xEF\t085"
			   "\f37\x93\fd\xAA\xE0\xA0\xE8\t119\f37\x90\fd\xA0\xA7\xAD\t153%s\t187%s\t221%s\t255%s\t289\x82\xEB\xE5\xAE\xA4";
	}

	/*
	"\r\x2\x3""c\v021\t017���\f37�\fd\t051\f37�\fd����\t085"
	"\f37�\fd����\t119\f37�\fd���\t153\f37%s\t289�����";
	*/
	const char *ITEMS_DIALOG_TEXT2() {
		return "\r\x2\x3"
			   "c\v021\t017\x8E\xE0\xE3\f37\xA6\fd\t051\f37\x81\fd\xE0\xAE\xAD\xEF\t085"
			   "\f37\x93\fd\xAA\xE0\xA0\xE8\t119\f37\x90\fd\xA0\xA7\xAD\t153\f37%s\t289\x82\xEB\xE5\xAE\xA4";
	}

	/*
	"\x3r\f%02u\t023%2d)\x3l\t028%s\n"
	*/
	const char *ITEMS_DIALOG_LINE1() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\n";
	}

	/*
	"\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n"
	*/
	const char *ITEMS_DIALOG_LINE2() {
		return "\x3r\f%02u\t023%2d)\x3l\t028%s\x3r\t000%u\n";
	}

	/*
	"\f37�\fd��"
	*/
	const char *BTN_BUY() {
		return "\f37\x8A\fd\xE3\xAF";
	}

	/*
	"\f37�\fd���"
	*/
	const char *BTN_SELL() {
		return "\f37\x8F\fd\xE0\xAE\xA4";
	}

	/*
	"\f37�\fd����"
	*/
	const char *BTN_IDENTIFY() {
		return "\f37\x8E\fd\xAF\xAE\xA7\xAD";
	}

	/*
	"\f37�\fd�����"
	*/
	const char *BTN_FIX() {
		return "\f37\x97\fd\xA8\xAD\xA8\xE2\xEC";
	}

	/*
	"\f37�\fd��"
	*/
	const char *BTN_USE() {
		return "\f37\x88\fd\xE1\xAF";
	}

	/*
	"\f37�\fd��"
	*/
	const char *BTN_EQUIP() {
		return "\f37\x8D\fd\xA0\xA4";
	}

	/*
	"\f37�\fd����"
	*/
	const char *BTN_REMOVE() {
		return "\f37\x91\fd\xAD\xEF\xE2\xEC";
	}

	/*
	"\f37�\fd���"
	*/
	const char *BTN_DISCARD() {
		return "\f37\x82\fd\xEB\xA1\xE0";
	}

	/*
	"\f37�\fd����"
	*/
	const char *BTN_QUEST() {
		return "\f37\x87\fd\xA0\xA4\xA0\xAD";
	}

	/*
	"�\fd����"
	*/
	const char *BTN_ENCHANT() {
		return "\x87\fd\xA0\xE7\xA0\xE0";
	}

	/*
	"�\fd�����"
	*/
	const char *BTN_RECHARGE() {
		return "\x8F\fd\xA5\xE0\xA5\xA7\xE0";
	}

	/*
	"�\fd�����"
	*/
	const char *BTN_GOLD() {
		return "\x87\fd\xAE\xAB\xAE\xE2\xAE";
	}

	/*
	"\f32������� "
	*/
	const char *ITEM_BROKEN() {
		return "\f32\xE1\xAB\xAE\xAC\xA0\xAD\xAE ";
	}

	/*
	"\f09�������� "
	*/
	const char *ITEM_CURSED() {
		return "\f09\xAF\xE0\xAE\xAA\xAB\xEF\xE2\xAE ";
	}

	const char *ITEM_OF() {
		return "";
	}

	/*
	"", "������ ��������", "����������� ������", "��������� �������",
	"�������� ���������", "������ ��������", "������ ������"
	*/
	const char **BONUS_NAMES() {
		delete[] _bonusNames;
		_bonusNames = new const char *[7] {
			"", "\x93\xA1\xA8\xA9\xE6\xEB \xA4\xE0\xA0\xAA\xAE\xAD\xAE\xA2", "\x88\xA7\xA3\xAE\xAD\xEF\xEE\xE9\xA5\xA3\xAE \xAC\xF1\xE0\xE2\xA2\xEB\xE5", "\x8A\xE0\xE3\xE8\xA8\xE2\xA5\xAB\xEF \xA3\xAE\xAB\xA5\xAC\xAE\xA2",
				"\x84\xA0\xA2\xA8\xE2\xA5\xAB\xEF \xAD\xA0\xE1\xA5\xAA\xAE\xAC\xEB\xE5", "\x93\xA1\xA8\xA9\xE6\xEB \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2", "\x93\xA1\xA8\xA9\xE6\xEB \xA7\xA2\xA5\xE0\xA5\xA9"
		};
		return _bonusNames;
	}

	/*
	nullptr, "��� ", "������� ", "����� ", "������ ",
	"������ ", "����� ", "������ ", "����� ", "������ ", "������� ",
	"��������� ", "������ ", "������ ", "��� ", "���� ", "����� ", "����� ",
	"������ ", "����� ", "�������� ", "���� ", "�������� ", "�������� ",
	"����� ", "������ ����� ", "�������� ", "������ ����� ", "������ ",
	"����� ������ ", "��� ", "������� ��� ", "������� ", "����� ",
	"��� - ������ ����� ",
	"Elder LongSword ", "Elder Dagger ", "Elder Mace ", "Elder Spear ",
	"Elder Staff ", "Elder LongBow "
	*/
	const char **WEAPON_NAMES() {
		delete[] _weaponNames;
		_weaponNames = new const char *[41] {
			nullptr, "\xAC\xA5\xE7 ", "\xA3\xAB\xA0\xA4\xA8\xE3\xE1 ", "\xAF\xA0\xAB\xA0\xE8 ", "\xEF\xE2\xA0\xA3\xA0\xAD ",
				"\xAA\xAE\xE0\xE2\xA8\xAA ", "\xE1\xA0\xA1\xAB\xEF ", "\xA4\xE3\xA1\xA8\xAD\xA0 ", "\xE2\xAE\xAF\xAE\xE0 ", "\xAA\xA0\xE2\xA0\xAD\xA0 ", "\xAD\xE3\xAD\xE7\xA0\xAA\xA8 ",
				"\xA2\xA0\xAA\xA8\xA4\xA7\xA0\xE1\xA8 ", "\xAA\xA8\xAD\xA6\xA0\xAB ", "\xA1\xE3\xAB\xA0\xA2\xA0 ", "\xE6\xA5\xAF ", "\xA6\xA5\xA7\xAB ", "\xAC\xAE\xAB\xAE\xE2 ", "\xAA\xAE\xAF\xEC\xF1 ",
				"\xA1\xA5\xE0\xA4\xEB\xE8 ", "\xA3\xAB\xA5\xE4\xA0 ", "\xA0\xAB\xA5\xA1\xA0\xE0\xA4\xA0 ", "\xAF\xA8\xAA\xA0 ", "\xE4\xAB\xA0\xAC\xA1\xA5\xE0\xA3 ", "\xE2\xE0\xA5\xA7\xE3\xA1\xA5\xE6 ",
				"\xAF\xAE\xE1\xAE\xE5 ", "\xA1\xAE\xA5\xA2\xAE\xA9 \xAC\xAE\xAB\xAE\xE2 ", "\xAD\xA0\xA3\xA8\xAD\xA0\xE2\xA0 ", "\xA1\xAE\xA5\xA2\xAE\xA9 \xE2\xAE\xAF\xAE\xE0 ", "\xE1\xA5\xAA\xA8\xE0\xA0 ",
				"\xE2\xAE\xAF\xAE\xE0 \xAF\xA0\xAB\xA0\xE7\xA0 ", "\xAB\xE3\xAA ", "\xA1\xAE\xAB\xEC\xE8\xAE\xA9 \xAB\xE3\xAA ", "\xA0\xE0\xA1\xA0\xAB\xA5\xE2 ", "\xAF\xE0\xA0\xE9\xA0 ",
				"\x8C\xA5\xE7 - \xE3\xA1\xA8\xA9\xE6\xA0 \x8A\xE1\xA8\xAD\xA0 ",
				"Elder LongSword ", "Elder Dagger ", "Elder Mace ", "Elder Spear ",
				"Elder Staff ", "Elder LongBow "
		};
		return _weaponNames;
	}

	/*
	nullptr, "������ ", "����� ", "������� ", "�������� ",
	"����� ", "�������� ", "���� ", "��� ",
	"���� ", "������ ", "���� ", "������� ", "�������� "
	*/
	const char **ARMOR_NAMES() {
		delete[] _armorNames;
		_armorNames = new const char *[14] {
			nullptr, "\xAC\xA0\xAD\xE2\xA8\xEF ", "\xE7\xA5\xE8\xE3\xEF ", "\xAF\xA0\xAD\xE6\xA8\xE0\xEC ", "\xAA\xAE\xAB\xEC\xE7\xE3\xA3\xA0 ",
				"\xEE\xE8\xAC\xA0\xAD ", "\xA1\xA5\xE5\xE2\xA5\xE0\xA5\xE6 ", "\xAB\xA0\xE2\xEB ", "\xE9\xA8\xE2 ",
				"\xE8\xAB\xA5\xAC ", "\xE1\xA0\xAF\xAE\xA3\xA8 ", "\xAF\xAB\xA0\xE9 ", "\xAD\xA0\xAA\xA8\xA4\xAA\xA0 ", "\xAF\xA5\xE0\xE7\xA0\xE2\xAA\xA8 "
		};
		return _armorNames;
	}

	/*
	nullptr, "������ ", "���� ", "����� ", "������ ", "�������� ", "����� ",
	"�������� ", "����� ", "�������� ", "������ "
	*/
	const char **ACCESSORY_NAMES() {
		delete[] _accessoryNames;
		_accessoryNames = new const char *[11] {
			nullptr, "\xAA\xAE\xAB\xEC\xE6\xAE ", "\xAF\xAE\xEF\xE1 ", "\xA1\xE0\xAE\xE8\xEC ", "\xAC\xA5\xA4\xA0\xAB\xEC ", "\xE2\xA0\xAB\xA8\xE1\xAC\xA0\xAD ", "\xAA\xA0\xAC\xA5\xEF ",
				"\xE1\xAA\xA0\xE0\xA0\xA1\xA5\xA9 ", "\xAA\xE3\xAB\xAE\xAD ", "\xAE\xA6\xA5\xE0\xA5\xAB\xEC\xA5 ", "\xA0\xAC\xE3\xAB\xA5\xE2 "
		};
		return _accessoryNames;
	}

	/*
	nullptr, "������� ", "�������� ", "������ ", "�������� ", "����� ", "��� ",
	"������� ", "���� ", "������� ", "����� ", "������ ", "��������", "��������",
	"��������", "��������", "��������", "��������", "��������", "��������",
	"��������", "��������"
	*/
	const char **MISC_NAMES() {
		delete[] _miscNames;
		_miscNames = new const char *[22] {
			nullptr, "\xE1\xAA\xA8\xAF\xA5\xE2\xE0 ", "\xE1\xA0\xAC\xAE\xE6\xA2\xA5\xE2 ", "\xAA\xA0\xAC\xA5\xAD\xEC ", "\xE8\xAA\xA0\xE2\xE3\xAB\xAA\xA0 ", "\xE1\xE4\xA5\xE0\xA0 ", "\xE0\xAE\xA3 ",
				"\xAC\xAE\xAD\xA5\xE2\xAA\xA0 ", "\xA6\xA5\xA7\xAB ", "\xE1\xA2\xA8\xE1\xE2\xAE\xAA ", "\xA7\xA5\xAB\xEC\xA5 ", "\xE1\xA2\xA8\xE2\xAE\xAA ", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0",
				"\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0",
				"\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0", "\xAF\xAE\xA4\xA4\xA5\xAB\xAA\xA0"
		};
		return _miscNames;
	}

	/*
	nullptr, "�����", "�����������", "��������� �����", "������ ������",
	"�������", "��������������� ������", "���������", "������� ��������",
	"�������", "����", "��������", "����������", "�������� �������",
	"������ �� ������", "����", "�������", "�������� ��� ���������",
	"�����������", "�������� ������", "���������", "���������� �����",
	"�������������", "��������� ��������", "������", "������ �����", "���������",
	"���������� �������", "������", "����", "��������", "�������",
	"�������� �� ����", "����������� ������", "������ ��������",
	"�������� �����", "����� ������", "�����������", "���������� �������",
	"���������� ���������", "��������� ��������", "����������", "������������",
	"������", "���������� ��������", "��������� ������", "�������� ������",
	"������������ ���", "�������", "��� ������", "��� �����", "����",
	"��������� ����", "�����������", "������", "��������� ��������",
	"������ ����������", "��������� ������", "����������������",
	"��������� �������", "������ �����", "��������� ����",
	"��������������� �����", "���", "�����������", "������ ����", "�����������",
	"����", "���������", "�������", "��������� �����", "��������",
	"������ ������", "�����!"
	*/
	const char **SPECIAL_NAMES() {
		delete[] _specialNames;
		_specialNames = new const char *[74] {
			nullptr, "\xE1\xA2\xA5\xE2\xA0", "\xAF\xE0\xAE\xA1\xE3\xA6\xA4\xA5\xAD\xA8\xEF", "\xA2\xAE\xAB\xE8\xA5\xA1\xAD\xEB\xE5 \xE1\xE2\xE0\xA5\xAB", "\xAF\xA5\xE0\xA2\xAE\xA9 \xAF\xAE\xAC\xAE\xE9\xA8",
				"\xAA\xE3\xAB\xA0\xAA\xAE\xA2", "\xED\xAD\xA5\xE0\xA3\xA5\xE2\xA8\xE7\xA5\xE1\xAA\xAE\xA3\xAE \xA2\xA7\xE0\xEB\xA2\xA0", "\xE3\xE1\xEB\xAF\xAB\xA5\xAD\xA8\xEF", "\xAB\xA5\xE7\xA5\xAD\xA8\xEF \xE1\xAB\xA0\xA1\xAE\xE1\xE2\xA8",
				"\xAB\xA5\xE7\xA5\xAD\xA8\xEF", "\xA8\xE1\xAA\xE0", "\xE8\xE0\xA0\xAF\xAD\xA5\xAB\xA8", "\xE0\xA5\xAF\xA5\xAB\xAB\xA5\xAD\xE2\xA0", "\xEF\xA4\xAE\xA2\xA8\xE2\xEB\xE5 \xAE\xA1\xAB\xA0\xAA\xAE\xA2",
				"\xA7\xA0\xE9\xA8\xE2\xEB \xAE\xE2 \xE1\xE2\xA8\xE5\xA8\xA9", "\xA1\xAE\xAB\xA8", "\xAF\xE0\xEB\xA6\xAA\xAE\xA2", "\xAA\xAE\xAD\xE2\xE0\xAE\xAB\xEF \xAD\xA0\xA4 \xAC\xAE\xAD\xE1\xE2\xE0\xA0\xAC\xA8",
				"\xEF\xE1\xAD\xAE\xA2\xA8\xA4\xA5\xAD\xA8\xEF", "\xA8\xA7\xA3\xAD\xA0\xAD\xA8\xEF \xAD\xA5\xA6\xA8\xE2\xA8", "\xAB\xA5\xA2\xA8\xE2\xA0\xE6\xA8\xA8", "\xA2\xAE\xAB\xE8\xA5\xA1\xAD\xAE\xA3\xAE \xA3\xAB\xA0\xA7\xA0",
				"\xA1\xAB\xA0\xA3\xAE\xE1\xAB\xAE\xA2\xA5\xAD\xA8\xEF", "\xAE\xAF\xAE\xA7\xAD\xA0\xAD\xA8\xEF \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2", "\xAC\xAE\xAB\xAD\xA8\xA9", "\xE1\xA2\xEF\xE2\xEB\xE5 \xA4\xA0\xE0\xAE\xA2", "\xA8\xE1\xE6\xA5\xAB\xA5\xAD\xA8\xEF",
				"\xAF\xE0\xA8\xE0\xAE\xA4\xAD\xAE\xA3\xAE \xAB\xA5\xE7\xA5\xAD\xA8\xEF", "\xAC\xA0\xEF\xAA\xAE\xA2", "\xE9\xA8\xE2\xA0", "\xAC\xE3\xA6\xA5\xE1\xE2\xA2\xA0", "\xA3\xA8\xAF\xAD\xAE\xA7\xA0",
				"\xE5\xAE\xA6\xA4\xA5\xAD\xA8\xEF \xAF\xAE \xA2\xAE\xA4\xA5", "\xAE\xA1\xA6\xA8\xA3\xA0\xEE\xE9\xA5\xA3\xAE \xE5\xAE\xAB\xAE\xA4\xA0", "\xAF\xAE\xA8\xE1\xAA\xA0 \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2",
				"\xAE\xA3\xAD\xA5\xAD\xAD\xEB\xE5 \xE8\xA0\xE0\xAE\xA2", "\xAB\xE3\xE7\xA5\xA9 \xE5\xAE\xAB\xAE\xA4\xA0", "\xAF\xE0\xAE\xE2\xA8\xA2\xAE\xEF\xA4\xA8\xEF", "\xE0\xA0\xE1\xAF\xEB\xAB\xA5\xAD\xA8\xEF \xAA\xA8\xE1\xAB\xAE\xE2\xEB",
				"\xA2\xE0\xA5\xAC\xA5\xAD\xAD\xAE\xA3\xAE \xA8\xE1\xAA\xA0\xA6\xA5\xAD\xA8\xEF", "\xE3\xE1\xEB\xAF\xAB\xA5\xAD\xA8\xEF \xA4\xE0\xA0\xAA\xAE\xAD\xAE\xA2", "\xA2\xA0\xAA\xE6\xA8\xAD\xA0\xE6\xA8\xA8", "\xE2\xA5\xAB\xA5\xAF\xAE\xE0\xE2\xA0\xE6\xA8\xA8",
				"\xE1\xAC\xA5\xE0\xE2\xA8", "\xE1\xA2\xAE\xA1\xAE\xA4\xAD\xAE\xA3\xAE \xA4\xA2\xA8\xA6\xA5\xAD\xA8\xEF", "\xAE\xE1\xE2\xA0\xAD\xAE\xA2\xAA\xA8 \xA3\xAE\xAB\xA5\xAC\xA0", "\xEF\xA4\xAE\xA2\xA8\xE2\xEB\xE5 \xA7\xA0\xAB\xAF\xAE\xA2",
				"\xE1\xAC\xA5\xE0\xE2\xA5\xAB\xEC\xAD\xAE\xA3\xAE \xE0\xAE\xEF", "\xE3\xA1\xA5\xA6\xA8\xE9\xA0", "\xA4\xAD\xEF \xA7\xA0\xE9\xA8\xE2\xEB", "\xA4\xAD\xEF \xAC\xA0\xA3\xA8\xA8", "\xAF\xA8\xE0\xA0",
				"\xAE\xA3\xAD\xA5\xAD\xAD\xAE\xA3\xAE \xE6\xA5\xAF\xA0", "\xAF\xA5\xE0\xA5\xA7\xA0\xE0\xEF\xA4\xAA\xA8", "\xAC\xAE\xE0\xAE\xA7\xA0", "\xA3\xAE\xE0\xAE\xA4\xE1\xAA\xA8\xE5 \xAF\xAE\xE0\xE2\xA0\xAB\xAE\xA2",
				"\xE1\xAD\xEF\xE2\xA8\xEF \xAE\xAA\xA0\xAC\xA5\xAD\xA5\xAD\xA8\xEF", "\xAE\xA6\xA8\xA2\xAB\xA5\xAD\xA8\xEF \xAC\xF1\xE0\xE2\xA2\xEB\xE5", "\xA4\xA5\xAC\xA0\xE2\xA5\xE0\xA8\xA0\xAB\xA8\xA7\xA0\xE6\xA8\xA8",
				"\xE2\xA0\xAD\xE6\xE3\xEE\xE9\xA8\xE5 \xAA\xAB\xA8\xAD\xAA\xAE\xA2", "\xAB\xE3\xAD\xAD\xEB\xE5 \xAB\xE3\xE7\xA5\xA9", "\xA8\xA7\xAC\xA5\xAD\xA5\xAD\xA8\xEF \xA2\xA5\xE1\xA0",
				"\xAF\xE0\xA8\xA7\xAC\xA0\xE2\xA8\xE7\xA5\xE1\xAA\xAE\xA3\xAE \xE1\xA2\xA5\xE2\xA0", "\xE7\xA0\xE0", "\xA8\xE1\xAF\xA5\xAF\xA5\xAB\xA5\xAD\xA8\xEF", "\xE1\xA2\xEF\xE2\xEB\xE5 \xE1\xAB\xAE\xA2", "\xA2\xAE\xE1\xAA\xE0\xA5\xE8\xA5\xAD\xA8\xEF",
				"\xA1\xE3\xE0\xEC", "\xAC\xA5\xA3\xA0\xA2\xAE\xAB\xEC\xE2", "\xA8\xAD\xE4\xA5\xE0\xAD\xAE", "\xE1\xAE\xAB\xAD\xA5\xE7\xAD\xEB\xE5 \xAB\xE3\xE7\xA5\xA9", "\xA8\xAC\xAF\xAB\xAE\xA7\xA8\xA9",
				"\xA2\xA7\xE0\xEB\xA2\xA0 \xA7\xA2\xA5\xA7\xA4\xEB", "\x81\x8E\x83\x8E\x82!"
		};
		return _specialNames;
	}

	/*
	"����", "�����", "������", "�������/���", "�������", "�����"
	*/
	const char **ELEMENTAL_NAMES() {
		delete[] _elementalNames;
		_elementalNames = new const char *[6] {
			"\x8E\xA3\xAD\xEF", "\x83\xE0\xAE\xAC\xA0", "\x95\xAE\xAB\xAE\xA4\xA0", "\x8A\xA8\xE1\xAB\xAE\xE2\xEB/\x9F\xA4\xA0", "\x9D\xAD\xA5\xE0\xA3\xA8\xA8", "\x8C\xA0\xA3\xA8\xA8"
		};
		return _elementalNames;
	}

	/*
	"����", "���������", "���� ����", "��������", "��������", "�����",
	"��������", "���� �����", "����� ������", "���������"
	*/
	const char **ATTRIBUTE_NAMES() {
		delete[] _attributeNames;
		_attributeNames = new const char *[10] {
			"\x91\xA8\xAB\xA0", "\x88\xAD\xE2\xA5\xAB\xAB\xA5\xAA\xE2", "\x91\xA8\xAB\xA0 \xA4\xE3\xE5\xA0", "\x91\xAA\xAE\xE0\xAE\xE1\xE2\xEC", "\x8C\xA5\xE2\xAA\xAE\xE1\xE2\xEC", "\x93\xA4\xA0\xE7\xA0",
				"\x87\xA4\xAE\xE0\xAE\xA2\xEC\xA5", "\x8E\xE7\xAA\xA8 \xAC\xA0\xA3\xA8\xA8", "\x8A\xAB\xA0\xE1\xE1 \xA7\xA0\xE9\xA8\xE2\xEB", "\x82\xAE\xE0\xAE\xA2\xE1\xE2\xA2\xAE"
		};
		return _attributeNames;
	}

	/*
	nullptr, "��������", "̸�����", "�������", "���������", "��������", "������"
	*/
	const char **EFFECTIVENESS_NAMES() {
		delete[] _effectivenessNames;
		_effectivenessNames = new const char *[7] {
			nullptr, "\x84\xE0\xA0\xAA\xAE\xAD\xAE\xA2", "\x8C\xF1\xE0\xE2\xA2\xEB\xE5", "\x83\xAE\xAB\xA5\xAC\xAE\xA2", "\x8D\xA0\xE1\xA5\xAA\xAE\xAC\xEB\xE5", "\x8C\xAE\xAD\xE1\xE2\xE0\xAE\xA2", "\x87\xA2\xA5\xE0\xA5\xA9"
		};
		return _effectivenessNames;
	}

	/*
	"����� �������� ���������",
	"����������� ���� �� ��������� �����",
	"������� ��� ����� �������",
	"���� �� ����� ������ �����",
	"����������� ������ ��������� �������",
	"������ ������ ������",
	"���������� ������ �������",
	"���������� ������ ���",
	"������� ���������� ���������",
	"��� �������",
	"������� ��������������",
	"��������� ���� ���",
	"������� ��������� �������",
	"��������� ����� ������",
	"�������� ����������",
	"��������� ������������������",
	"������ ��������",
	"������ �����",
	"�������� ������� �������",
	"��������� ����� ������",
	"��������� ����� ������",
	"��������� ������ ����",
	"��������� �������� �������� �����",
	"��������� �������� ����",
	"��������� ���� �����",
	"����� ������� ����� �����",
	"���������� ������",
	"���������� �� ��������",
	"�������",
	"����� ����",
	"����� ��������",
	"����� ��������",
	"����� ��������",
	"����� �����",
	"����������",
	"Pass to Castleview",
	"Pass to Sandcaster",
	"Pass to Lakeside",
	"Pass to Necropolis",
	"Pass to Olympus",
	"Key to Great Western Tower",
	"Key to Great Southern Tower",
	"Key to Great Eastern Tower",
	"Key to Great Northern Tower",
	"Key to Ellinger's Tower",
	"Key to Dragon Tower",
	"Key to Darkstone Tower",
	"Key to Temple of Bark",
	"Key to Dungeon of Lost Souls",
	"Key to Ancient Pyramid",
	"Key to Dungeon of Death",
	"Amulet of the Southern Sphinx",
	"Dragon Pharoah's Orb",
	"Cube of Power",
	"Chime of Opening",
	"Gold ID Card",
	"Silver ID Card",
	"Vulture Repellant",
	"Bridle",
	"Enchanted Bridle",
	"Treasure Map (Goto E1 x1, y11)",
	"",
	"Fake Map",
	"Onyx Necklace",
	"Dragon Egg",
	"Tribble",
	"Golden Pegasus Statuette",
	"Golden Dragon Statuette",
	"Golden Griffin Statuette",
	"Chalice of Protection",
	"Jewel of Ages",
	"Songbird of Serenity",
	"Sandro's Heart",
	"Ector's Ring",
	"Vespar's Emerald Handle",
	"Queen Kalindra's Crown",
	"Caleb's Magnifying Glass",
	"Soul Box",
	"Soul Box with Corak inside",
	"Ruby Rock",
	"Emerald Rock",
	"Sapphire Rock",
	"Diamond Rock",
	"Monga Melon",
	"Energy Disk"
	*/
	const char **QUEST_ITEM_NAMES() {
		delete[] _questItemNames;
		_questItemNames = new const char *[85] {
			"\x8F\xE0\xA0\xA2\xAE \xA2\xAB\xA0\xA4\xA5\xAD\xA8\xEF \x8D\xEC\xEE\xAA\xA0\xE1\xAB\xAE\xAC",
				"\x95\xE0\xE3\xE1\xE2\xA0\xAB\xEC\xAD\xEB\xA9 \xAA\xAB\xEE\xE7 \xAE\xE2 \x82\xA5\xA4\xEC\xAC\xA8\xAD\xAE\xA9 \xA1\xA0\xE8\xAD\xA8",
				"\x8E\xE2\xAC\xEB\xE7\xAA\xA0 \xA4\xAB\xEF \xA1\xA0\xE8\xAD\xA8 \x84\xA0\xE0\xA7\xAE\xA3\xA0",
				"\x8A\xAB\xEE\xE7 \xAE\xE2 \x81\xA0\xE8\xAD\xA8 \xA2\xEB\xE1\xE8\xA5\xA9 \xAC\xA0\xA3\xA8\xA8",
				"\x84\xE0\xA0\xA3\xAE\xE6\xA5\xAD\xAD\xEB\xA9 \xA0\xAC\xE3\xAB\xA5\xE2 \x91\xA5\xA2\xA5\xE0\xAD\xAE\xA3\xAE \x91\xE4\xA8\xAD\xAA\xE1\xA0",
				"\x8A\xA0\xAC\xA5\xAD\xEC \x92\xEB\xE1\xEF\xE7\xA8 \x93\xA6\xA0\xE1\xAE\xA2",
				"\x8E\xE2\xAF\xA8\xE0\xA0\xEE\xE9\xA8\xA9 \xAA\xA0\xAC\xA5\xAD\xEC \xA3\xAE\xAB\xA5\xAC\xAE\xA2",
				"\x8E\xE2\xAF\xA8\xE0\xA0\xEE\xE9\xA8\xA9 \xAA\xA0\xAC\xA5\xAD\xEC \x9F\xAA\xA0",
				"\x91\xAA\xA8\xAF\xA5\xE2\xE0 \x82\xE0\xA5\xAC\xA5\xAD\xAD\xAE\xA3\xAE \x88\xE1\xAA\xA0\xA6\xA5\xAD\xA8\xEF",
				"\x90\xAE\xA3 \x94\xA0\xAB\xA8\xE1\xE2\xEB",
				"\x9D\xAB\xA8\xAA\xE1\xA8\xE0 \x82\xAE\xE1\xE1\xE2\xA0\xAD\xAE\xA2\xAB\xA5\xAD\xA8\xEF",
				"\x82\xAE\xAB\xE8\xA5\xA1\xAD\xEB\xA9 \xA6\xA5\xA7\xAB \xE4\xA5\xA9",
				"\x84\xA8\xA0\xA4\xA5\xAC\xA0 \xAF\xE0\xA8\xAD\xE6\xA5\xE1\xE1\xEB \x90\xAE\xAA\xE1\xA0\xAD\xEB",
				"\x91\xA2\xEF\xE9\xA5\xAD\xAD\xA0\xEF \x8A\xAD\xA8\xA3\xA0 \x9D\xAB\xEC\xE4\xAE\xA2",
				"\x91\xAA\xA0\xE0\xA0\xA1\xA5\xA9 \x82\xAE\xAF\xAB\xAE\xE9\xA5\xAD\xA8\xEF",
				"\x8A\xE0\xA8\xE1\xE2\xA0\xAB\xAB\xEB \x8F\xEC\xA5\xA7\xAE\xED\xAB\xA5\xAA\xE2\xE0\xA8\xE7\xA5\xE1\xE2\xA2\xA0",
				"\x91\xA2\xA8\xE2\xAE\xAA \x8C\xE3\xA4\xE0\xAE\xE1\xE2\xA8",
				"\x8A\xAE\xE0\xA5\xAD\xEC \xE4\xA8\xE0\xAD\xEB",
				"\x8A\xAE\xE1\xE2\xEF\xAD\xAE\xA9 \xE1\xA2\xA8\xE1\xE2\xAE\xAA \x8E\xE0\xAE\xE4\xA8\xAD\xA0",
				"\x82\xAE\xAB\xE8\xA5\xA1\xAD\xEB\xA9 \xAA\xE3\xAB\xAE\xAD \x81\xA0\xE0\xAE\xAA\xA0",
				"\x8F\xE0\xAE\xAF\xA0\xA2\xE8\xA8\xA9 \xE7\xA5\xE0\xA5\xAF \x8B\xA8\xA3\xAE\xAD\xAE",
				"\x8F\xAE\xE1\xAB\xA5\xA4\xAD\xA8\xA9 \xE6\xA2\xA5\xE2\xAE\xAA \xAB\xA5\xE2\xA0",
				"\x8F\xAE\xE1\xAB\xA5\xA4\xAD\xEF\xEF \xA4\xAE\xA6\xA4\xA5\xA2\xA0\xEF \xAA\xA0\xAF\xA5\xAB\xEC\xAA\xA0 \xA2\xA5\xE1\xAD\xEB",
				"\x8F\xAE\xE1\xAB\xA5\xA4\xAD\xEF\xEF \xE1\xAD\xA5\xA6\xA8\xAD\xAA\xA0 \xA7\xA8\xAC\xEB",
				"\x8F\xAE\xE1\xAB\xA5\xA4\xAD\xA8\xA9 \xAB\xA8\xE1\xE2 \xAE\xE1\xA5\xAD\xA8",
				"\x82\xA5\xE7\xAD\xAE \xA3\xAE\xE0\xEF\xE7\xA8\xA9 \xAA\xE3\xE1\xAE\xAA \xAF\xA5\xAC\xA7\xEB",
				"\x8C\xA5\xA3\xA0\xAA\xE0\xA5\xA4\xA8\xE2 \xAA\xAE\xE0\xAE\xAB\xEF",
				"\x90\xA0\xA7\xE0\xA5\xE8\xA5\xAD\xA8\xA5 \xAD\xA0 \xE0\xA0\xE1\xAA\xAE\xAF\xAA\xA8",
				"\x8A\xE3\xAA\xAE\xAB\xAA\xA0",
				"\x8A\xE3\xAA\xAB\xA0 \x91\xA8\xAB\xEB",
				"\x8A\xE3\xAA\xAB\xA0 \x91\xAA\xAE\xE0\xAE\xE1\xE2\xA8",
				"\x8A\xE3\xAA\xAB\xA0 \x91\xAB\xAE\xA6\xA5\xAD\xA8\xEF",
				"\x8A\xE3\xAA\xAB\xA0 \x8C\xA5\xE2\xAA\xAE\xE1\xE2\xA8",
				"\x8A\xE3\xAA\xAB\xA0 \x93\xA4\xA0\xE7\xA8",
				"\x93\xE1\xE2\xE0\xAE\xA9\xE1\xE2\xA2\xAE",
				"Pass to Castleview",
				"Pass to Sandcaster",
				"Pass to Lakeside",
				"Pass to Necropolis",
				"Pass to Olympus",
				"Key to Great Western Tower",
				"Key to Great Southern Tower",
				"Key to Great Eastern Tower",
				"Key to Great Northern Tower",
				"Key to Ellinger's Tower",
				"Key to Dragon Tower",
				"Key to Darkstone Tower",
				"Key to Temple of Bark",
				"Key to Dungeon of Lost Souls",
				"Key to Ancient Pyramid",
				"Key to Dungeon of Death",
				"Amulet of the Southern Sphinx",
				"Dragon Pharoah's Orb",
				"Cube of Power",
				"Chime of Opening",
				"Gold ID Card",
				"Silver ID Card",
				"Vulture Repellant",
				"Bridle",
				"Enchanted Bridle",
				"Treasure Map (Goto E1 x1, y11)",
				"",
				"Fake Map",
				"Onyx Necklace",
				"Dragon Egg",
				"Tribble",
				"Golden Pegasus Statuette",
				"Golden Dragon Statuette",
				"Golden Griffin Statuette",
				"Chalice of Protection",
				"Jewel of Ages",
				"Songbird of Serenity",
				"Sandro's Heart",
				"Ector's Ring",
				"Vespar's Emerald Handle",
				"Queen Kalindra's Crown",
				"Caleb's Magnifying Glass",
				"Soul Box",
				"Soul Box with Corak inside",
				"Ruby Rock",
				"Emerald Rock",
				"Sapphire Rock",
				"Diamond Rock",
				"Monga Melon",
				"Energy Disk"
		};
		return _questItemNames;
	}

	const char **QUEST_ITEM_NAMES_SWORDS() {
		delete[] _questItemNamesSwords;
		_questItemNamesSwords = new const char *[51] {
			"Pass to Hart", "Pass to Impery", "Pass to town3", "Pass to town4", "Pass to town5",
				"Key to Hart Sewers", "Key to Rettig's Pyramid", "Key to the Old Temple",
				"Key to Canegtut's Pyramid", "Key to Ascihep's Pyramid", "Key to Dragon Tower",
				"Key to Darkstone Tower", "Key to Temple of Bark", "Key to Dungeon of Lost Souls",
				"Key to Ancient Pyramid", "Key to Dungeon of Death", "Red Magic Hammer",
				"Green Magic Hammer", "Golden Magic Wand", "Silver Magic Hammer", "Magic Coin",
				"Ruby", "Diamond Mineral", "Emerald", "Sapphire", "Treasure Map (Goto E1 x1, y11)",
				"NOTUSED", "Melon", "Princess Crown", "Emerald Wand", "Druid Carving", "High Sign",
				"Holy Wheel", "Double Cross", "Sky Hook", "Sacred Cow", "Staff of the Mountain",
				"Hard Rock", "Soft Rock", "Rock Candy", "Ivy Plant", "Spirit Gem", "Temple of Sun holy lamp oil",
				"Noams Hammer", "Positive Orb", "Negative Orb", "FireBane Staff", "Diamond Edged Pick",
				"Monga Melon", "Energy Disk", "Old XEEN Quest Item"
		};
		return _questItemNamesSwords;
	}

	/*
	"\t000\v007\x3""c%s �� ����� ������������ %s!"
	*/
	const char *NOT_PROFICIENT() {
		return "\t000\v007\x3"
			   "c%s \xAD\xA5 \xAC\xAE\xA6\xA5\xE2 \xA8\xE1\xAF\xAE\xAB\xEC\xA7\xAE\xA2\xA0\xE2\xEC %s!";
	}

	/*
	"\x3""c\n\t000��� �����."
	*/
	const char *NO_ITEMS_AVAILABLE() {
		return "\x3"
			   "c\n\t000\x8D\xA5\xE2 \xA2\xA5\xE9\xA5\xA9.";
	}

	/*
	"������", "�����", "���������", "������"
	*/
	const char **CATEGORY_NAMES() {
		delete[] _categoryNames;
		_categoryNames = new const char *[4] { "\x8E\xE0\xE3\xA6\xA8\xA5", "\x81\xE0\xAE\xAD\xEF", "\x93\xAA\xE0\xA0\xE8\xA5\xAD\xA8\xEF", "\x90\xA0\xA7\xAD\xAE\xA5" };
		return _categoryNames;
	}

	const char *X_FOR_THE_Y() {
		return "\x1\fd\r%s\v000\t000%s - %s %s%s\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	const char *X_FOR_Y() {
		return "\x1\fd\r\x3"
			   "l\v000\t000%s - %s\x3"
			   "r\t000%s\x3"
			   "l\v011"
			   "\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	/*
	"\x1\fd\r\x3""l\v000\t000%s - %s\t150������ - %lu%s\x3""l\v011"
	"\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	*/
	const char *X_FOR_Y_GOLD() {
		return "\x1\fd\r\x3"
			   "l\v000\t000%s - %s\t150\x87\xAE\xAB\xAE\xE2\xAE - %lu%s\x3"
			   "l\v011"
			   "\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	/*
	"\x3""r\t000�������\x3""l"
	*/
	const char *FMT_CHARGES() {
		return "\x3"
			   "r\t000\x87\xA0\xE0\xEF\xA4\xAE\xA2\x3"
			   "l";
	}

	/*
	"\x1\fd\r\x3""l\v000\t000%s\t150������ - %lu\x3""r\t000����"
	"\x3""l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	*/
	const char *AVAILABLE_GOLD_COST() {
		return "\x1\fd\r\x3"
			   "l\v000\t000%s\t150\x87\xAE\xAB\xAE\xE2\xAE - %lu\x3"
			   "r\t000\x96\xA5\xAD\xA0"
			   "\x3"
			   "l\v011\x2%s%s%s%s%s%s%s%s%s\x1\fd";
	}

	/*
	"�������"
	*/
	const char *CHARGES() {
		return "\x87\xA0\xE0\xEF\xA4\xAE\xA2";
	}

	/*
	"����"
	*/
	const char *COST() {
		return "\x96\xA5\xAD\xA0";
	}

	/*
	"������", "�����", "������������", "��������", "����������", "������������",
	"���������� � ������"
	*/
	const char **ITEM_ACTIONS() {
		delete[] _itemActions;
		_itemActions = new const char *[7] {
			"\xAD\xA0\xA4\xA5\xE2\xEC", "\xE1\xAD\xEF\xE2\xEC", "\xA8\xE1\xAF\xAE\xAB\xEC\xA7\xAE\xA2\xA0\xE2\xEC", "\xA2\xEB\xAA\xA8\xAD\xE3\xE2\xEC", "\xA7\xA0\xE7\xA0\xE0\xAE\xA2\xA0\xE2\xEC", "\xAF\xA5\xE0\xA5\xA7\xA0\xE0\xEF\xA4\xA8\xE2\xEC",
				"\x8F\xE0\xA5\xA2\xE0\xA0\xE2\xA8\xE2\xEC \xA2 \xA7\xAE\xAB\xAE\xE2\xAE"
		};
		return _itemActions;
	}

	/*
	"\v000����� ������� �� ������� %s?"
	*/
	const char *WHICH_ITEM() {
		return "\v000\x8A\xA0\xAA\xAE\xA9 \xAF\xE0\xA5\xA4\xAC\xA5\xE2 \xA2\xEB \xA6\xA5\xAB\xA0\xA5\xE2\xA5 %s?";
	}

	/*
	"\v007���� ����������?\n"
	"�������, ���� �� ���������� ������!";
	*/
	const char *WHATS_YOUR_HURRY() {
		return "\v007\x97\xA5\xA3\xAE \xE2\xAE\xE0\xAE\xAF\xA8\xE8\xEC\xE1\xEF?\n"
			   "\x8F\xAE\xA4\xAE\xA6\xA4\xA8, \xAF\xAE\xAA\xA0 \xAD\xA5 \xA2\xEB\xA1\xA5\xE0\xA5\xE8\xEC\xE1\xEF ���!";
	}

	/*
	"\v007����� ������������ ������� � �����, ������� �� ������ '���' �� ������� ������."
	*/
	const char *USE_ITEM_IN_COMBAT() {
		return "\v007\x97\xE2\xAE\xA1\xEB \xA8\xE1\xAF\xAE\xAB\xEC\xA7\xAE\xA2\xA0\xE2\xEC \xAF\xE0\xA5\xA4\xAC\xA5\xE2 \xA2 \xA1\xA8\xE2\xA2\xA5, \xAD\xA0\xA6\xAC\xA8\xE2\xA5 \xAD\xA0 \xAA\xAD\xAE\xAF\xAA\xE3 '\x88\xE1\xAF' \xAD\xA0 \xA1\xAE\xAA\xAE\xA2\xAE\xA9 \xAF\xA0\xAD\xA5\xAB\xA8.";
	}

	/*
	"\v005\x3""c%s\fd�� ����� ������ �������!"
	*/
	const char *NO_SPECIAL_ABILITIES() {
		return "\v005\x3"
			   "c%s\fd\xAD\xA5 \xA8\xAC\xA5\xA5\xE2 \xAE\xE1\xAE\xA1\xEB\xE5 \xE1\xA2\xAE\xA9\xE1\xE2\xA2!";
	}

	/*
	"\x3""c\v007���������� %s ������ ������������ � �����!"
	*/
	const char *CANT_CAST_WHILE_ENGAGED() {
		return "\x3"
			   "c\v007\x87\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xA5 %s \xAD\xA5\xAB\xEC\xA7\xEF \xA8\xE1\xAF\xAE\xAB\xEC\xA7\xAE\xA2\xA0\xE2\xEC \xA2 \xA1\xA8\xE2\xA2\xA5!";
	}

	/*
	"\x3""c\v007�� �� ������ ������ ������ ���� %s!"
	*/
	const char *EQUIPPED_ALL_YOU_CAN() {
		return "\x3"
			   "c\v007\x82\xEB \xAD\xA5 \xAC\xAE\xA6\xA5\xE2\xA5 \xAD\xA0\xA4\xA5\xE2\xEC \xA1\xAE\xAB\xEC\xE8\xA5 \xA4\xA2\xE3\xE5 %s!";
	}

	/*
	"\x3""c\v007��� ����� ����� %s����� ������ %s\b!"
	*/
	const char *REMOVE_X_TO_EQUIP_Y() {
		return "\x3"
			   "c\v007\x82\xA0\xAC \xAD\xE3\xA6\xAD\xAE \xE1\xAD\xEF\xE2\xEC %s\xE7\xE2\xAE\xA1\xEB \xAD\xA0\xA4\xA5\xE2\xEC %s\b!";
	}

	/*
	"�����"
	*/
	const char *RING() {
		return "\xAA\xAE\xAB\xA5\xE6";
	}

	/*
	"�������"
	*/
	const char *MEDAL() {
		return "\xAC\xA5\xA4\xA0\xAB\xA5\xA9";
	}

	/*
	"\x3""c�� �� ������ ����� ��������� ����!"
	*/
	const char *CANNOT_REMOVE_CURSED_ITEM() {
		return "\x3"
			   "c\x82\xEB \xAD\xA5 \xAC\xAE\xA6\xA5\xE2\xA5 \xE1\xAD\xEF\xE2\xEC \xAF\xE0\xAE\xAA\xAB\xEF\xE2\xE3\xEE \xA2\xA5\xE9\xEC!";
	}

	/*
	"\x3""c�� �� ������ �������� ��������� ����!"
	*/
	const char *CANNOT_DISCARD_CURSED_ITEM() {
		return "\x3"
			   "c\x82\xEB \xAD\xA5 \xAC\xAE\xA6\xA5\xE2\xA5 \xA2\xEB\xAA\xA8\xAD\xE3\xE2\xEC \xAF\xE0\xAE\xAA\xAB\xEF\xE2\xE3\xEE \xA2\xA5\xE9\xEC!";
	}

	/*
	"\v000\t000\x3""l�������� ��������? %s"
	*/
	const char *PERMANENTLY_DISCARD() {
		return "\v000\t000\x3"
			   "l\x82\xEB\xAA\xA8\xAD\xE3\xE2\xEC \xAD\xA0\xA2\xE1\xA5\xA3\xA4\xA0? %s";
	}

	/*
	"\v005\x3""c\fd%s! ��� ������ �����."
	*/
	const char *BACKPACK_IS_FULL() {
		return "\v005\x3"
			   "c\fd%s! \x82\xA0\xE8 \xE0\xEE\xAA\xA7\xA0\xAA \xAF\xAE\xAB\xAE\xAD.";
	}

	/*
	"\v010\t000\x3""c%s! ��� ������ �����.",
	"\v010\t000\x3""c%s! ��� ������ �����.",
	"\v010\t000\x3""c%s! ��� ������ �����.",
	"\v010\t000\x3""c%s! ��� ������ �����."
	*/
	const char **CATEGORY_BACKPACK_IS_FULL() {
		delete[] _categoryBackpackIsFull;
		_categoryBackpackIsFull = new const char *[4] {
			"\v010\t000\x3"
			"c%s! \x82\xA0\xE8 \xE0\xEE\xAA\xA7\xA0\xAA \xAF\xAE\xAB\xAE\xAD.",
				"\v010\t000\x3"
				"c%s! \x82\xA0\xE8 \xE0\xEE\xAA\xA7\xA0\xAA \xAF\xAE\xAB\xAE\xAD.",
				"\v010\t000\x3"
				"c%s! \x82\xA0\xE8 \xE0\xEE\xAA\xA7\xA0\xAA \xAF\xAE\xAB\xAE\xAD.",
				"\v010\t000\x3"
				"c%s! \x82\xA0\xE8 \xE0\xEE\xAA\xA7\xA0\xAA \xAF\xAE\xAB\xAE\xAD."
		};
		return _categoryBackpackIsFull;
	}

	/*
	"\x3""l\v000\t000\fd������ %s\fd �� %lu �����%s?"
	*/
	const char *BUY_X_FOR_Y_GOLD() {
		return "\x3"
			   "l\v000\t000\fd\x8A\xE3\xAF\xA8\xE2\xEC %s\fd \xA7\xA0 %lu \xA7\xAE\xAB\xAE\xE2%s?";
	}

	/*
	"\x3""l\v000\t000\fd������� %s\fd �� %lu �����%s?"
	*/
	const char *SELL_X_FOR_Y_GOLD() {
		return "\x3"
			   "l\v000\t000\fd\x8F\xE0\xAE\xA4\xA0\xE2\xEC %s\fd \xA7\xA0 %lu \xA7\xAE\xAB\xAE\xE2%s?";
	}

	/*
	"��", "��"
	*/
	const char **SELL_X_FOR_Y_GOLD_ENDINGS() {
		delete[] _sellXForYGoldEndings;
		_sellXForYGoldEndings = new const char *[2] { "\xAE\xA9", "\xEB\xE5" };
		return _sellXForYGoldEndings;
	}

	/*
	"\v005\x3""c\fd��� �� ���������� %s\fd!"
	*/
	const char *NO_NEED_OF_THIS() {
		return "\v005\x3"
			   "c\fd\x8D\xA0\xE1 \xAD\xA5 \xA8\xAD\xE2\xA5\xE0\xA5\xE1\xE3\xA5\xE2 %s\fd!";
	}

	/*
	"\v007\x3""c\fd���������� ������������.\n%s"
	*/
	const char *NOT_RECHARGABLE() {
		return "\v007\x3"
			   "c\fd\x8D\xA5\xA2\xAE\xA7\xAC\xAE\xA6\xAD\xAE \xAF\xA5\xE0\xA5\xA7\xA0\xE0\xEF\xA4\xA8\xE2\xEC.\n%s";
	}

	/*
	"\v007\t000\x3""c���������� ����������.\n%s"
	*/
	const char *NOT_ENCHANTABLE() {
		return "\v007\t000\x3"
			   "c\x8D\xA5\xA2\xAE\xA7\xAC\xAE\xA6\xAD\xAE \xA7\xA0\xE7\xA0\xE0\xAE\xA2\xA0\xE2\xEC.\n%s";
	}

	/*
	"���������� �� ���������!"
	*/
	const char *SPELL_FAILED() {
		return "\x87\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xA5 \xAD\xA5 \xE1\xE0\xA0\xA1\xAE\xE2\xA0\xAB\xAE!";
	}

	/*
	"\fd���� ������� �� ������!"
	*/
	const char *ITEM_NOT_BROKEN() {
		return "\fd\x9D\xE2\xAE\xE2 \xAF\xE0\xA5\xA4\xAC\xA5\xE2 \xAD\xA5 \xE1\xAB\xAE\xAC\xA0\xAD!";
	}

	/*
	"��������", "��������"
	*/
	const char **FIX_IDENTIFY() {
		delete[] _fixIdentify;
		_fixIdentify = new const char *[2] { "\x8F\xAE\xE7\xA8\xAD\xA8\xE2\xEC", "\x8E\xAF\xAE\xA7\xAD\xA0\xE2\xEC" };
		return _fixIdentify;
	}

	/*
	"\x3""l\v000\t000%s %s\fd �� %lu �����%s?"
	*/
	const char *FIX_IDENTIFY_GOLD() {
		return "\x3"
			   "l\v000\t000%s %s\fd \xA7\xA0 %lu \xA7\xAE\xAB\xAE\xE2%s?";
	}

	/*
	"\fd\v000\t000\x3""c�������� �������\x3""l\n"
	"\n"
	"\v012%s\fd\n"
	"\n"
	"%s";
	*/
	const char *IDENTIFY_ITEM_MSG() {
		return "\fd\v000\t000\x3"
			   "c\x8E\xAF\xAE\xA7\xAD\xA0\xE2\xEC \xAF\xE0\xA5\xA4\xAC\xA5\xE2\x3"
			   "l\n"
			   "\n"
			   "\v012%s\fd\n"
			   "\n"
			   "%s";
	}

	/*
	"���������� ������\t132:\t140%s\n"
	"����������� �����\t132:\t140%s\n"
	"���������� ����\t132:\t140%s\n"
	"���� �� ������\t132:\t140%s\n"
	"������ �� ������\t132:\t140%s\n"
	"����� ������\t132:\t140%s\n"
	"����� ��������\t132:\t140%s\n"
	"������ ��������\t132:\t140%s";
	*/
	const char *ITEM_DETAILS() {
		return "\x88\xE1\xAF\xAE\xAB\xEC\xA7\xE3\xEE\xE2 \xAA\xAB\xA0\xE1\xE1\xEB\t132:\t140%s\n"
			   "\x8C\xAE\xA4\xA8\xE4\xA8\xAA\xA0\xE2\xAE\xE0 \xE3\xE0\xAE\xAD\xA0\t132:\t140%s\n"
			   "\x94\xA8\xA7\xA8\xE7\xA5\xE1\xAA\xA8\xA9 \xE3\xE0\xAE\xAD\t132:\t140%s\n"
			   "\x93\xE0\xAE\xAD \xAE\xE2 \x91\xE2\xA8\xE5\xA8\xA9\t132:\t140%s\n"
			   "\x87\xA0\xE9\xA8\xE2\xA0 \xAE\xE2 \x91\xE2\xA8\xE5\xA8\xA9\t132:\t140%s\n"
			   "\x8A\xAB\xA0\xE1\xE1 \xA7\xA0\xE9\xA8\xE2\xEB\t132:\t140%s\n"
			   "\x81\xAE\xAD\xE3\xE1 \xE1\xA2\xAE\xA9\xE1\xE2\xA2\xA0\t132:\t140%s\n"
			   "\x8E\xE1\xAE\xA1\xAE\xA5 \xE1\xA2\xAE\xA9\xE1\xE2\xA2\xAE\t132:\t140%s";
	}

	/*
	"���"
	*/
	const char *ALL() {
		return "\x82\xE1\xA5";
	}

	/*
	"���"
	*/
	const char *FIELD_NONE() {
		return "\x8D\xA5\xE2";
	}

	const char *DAMAGE_X_TO_Y() {
		return "%d-%d";
	}

	/*
	%+d ����� �� %s"
	*/
	const char *ELEMENTAL_XY_DAMAGE() {
		return "%+d \xE3\xE0\xAE\xAD\xA0 \xAE\xE2 %s";
	}

	const char *ATTR_XY_BONUS() {
		return "%+d %s";
	}

	/*
	"x3 ������ %s"
	*/
	const char *EFFECTIVE_AGAINST() {
		return "x3 \xAF\xE0\xAE\xE2\xA8\xA2 %s";
	}

	/*
	"\r\x2\x3""c\v021\t017\f37�\fd���\t085\f37�\fd������\t153"
	"�\f37�\fd�����\t221�����\t255����"
	"\t289�����";
	*/
	const char *QUESTS_DIALOG_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t017\f37\x82\fd\xA5\xE9\xA8\t085\f37\x87\fd\xA0\xA4\xA0\xAD\xA8\xEF\t153"
			   "\x87\f37\xA0\fd\xAC\xA5\xE2\xAA\xA8\t221\x82\xA2\xA5\xE0\xE5\t255\x82\xAD\xA8\xA7"
			   "\t289\x82\xEB\xE5\xAE\xA4";
	}

	/*
	"\b \b*-- \f04������ �����\fd --"
	*/
	const char *CLOUDS_OF_XEEN_LINE() {
		return "\b \b*-- \f04\x8E\xA1\xAB\xA0\xAA\xA0 \x8A\xE1\xA8\xAD\xA0\fd --";
	}

	const char *DARKSIDE_OF_XEEN_LINE() {
		return "\b \b*-- \f04Darkside of Xeen\fd --";
	}

	const char *SWORDS_OF_XEEN_LINE() {
		return "\b \b*-- \f04Swords of Xeen\fd --";
	}

	/*
	"\r\x1\fd\x3""c\v000\t000�������� �������\x3""l\x2\n"
	"\n"
	"\x3""c��� ���������";
	*/
	const char *NO_QUEST_ITEMS() {
		return "\r\x1\fd\x3"
			   "c\v000\t000\x8F\xE0\xA5\xA4\xAC\xA5\xE2\xEB \xA7\xA0\xA4\xA0\xAD\xA8\xA9\x3"
			   "l\x2\n"
			   "\n"
			   "\x3"
			   "c\x8D\xA5\xE2 \xAF\xE0\xA5\xA4\xAC\xA5\xE2\xAE\xA2";
	}

	/*
	"\x3""c\v000\t000\n"
	"\n"
	"��� �������";
	*/
	const char *NO_CURRENT_QUESTS() {
		return "\x3"
			   "c\v000\t000\n"
			   "\n"
			   "\x8D\xA5\xE2 \xA7\xA0\xA4\xA0\xAD\xA8\xA9";
	}

	/*
	"\x3""c������� ���"
	*/
	const char *NO_AUTO_NOTES() {
		return "\x3"
			   "c\x87\xA0\xAC\xA5\xE2\xAE\xAA \xAD\xA5\xE2";
	}

	/*
	"\r\x1\fd\x3""c\v000\t000�������� �������\x3""l\x2\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s\n"
	"\f04 * \fd%s";
	*/
	const char *QUEST_ITEMS_DATA() {
		return "\r\x1\fd\x3"
			   "c\v000\t000\x8F\xE0\xA5\xA4\xAC\xA5\xE2\xEB \xA7\xA0\xA4\xA0\xAD\xA8\xA9\x3"
			   "l\x2\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s\n"
			   "\f04 * \fd%s";
	}

	/*
	"\r\x1\fd\x3""c\t000\v000�������\x3""l\x2\n"
	"%s\n"
	"\n"
	"%s\n"
	"\n"
	"%s";
	*/
	const char *CURRENT_QUESTS_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000\x87\xA0\xA4\xA0\xAD\xA8\xEF\x3"
			   "l\x2\n"
			   "%s\n"
			   "\n"
			   "%s\n"
			   "\n"
			   "%s";
	}

	/*
	"\r\x1\fd\x3""c\t000\v000�������\x3""l\x2\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l\n"
	"%s\x3""l";
	*/
	const char *AUTO_NOTES_DATA() {
		return "\r\x1\fd\x3"
			   "c\t000\v000\x87\xA0\xAC\xA5\xE2\xAA\xA8\x3"
			   "l\x2\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l\n"
			   "%s\x3"
			   "l";
	}

	/*
	"\v000\t000������ 8 �����. ����� ��������\n"
	"%s\n"
	"���� ������� %d ����.";
	*/
	const char *REST_COMPLETE() {
		return "\v000\t000\x8F\xE0\xAE\xE8\xAB\xAE 8 \xE7\xA0\xE1\xAE\xA2. \x8E\xE2\xA4\xEB\xE5 \xA7\xA0\xAA\xAE\xAD\xE7\xA5\xAD\n"
			   "%s\n"
			   "\x81\xEB\xAB\xAE \xE1\xEA\xA5\xA4\xA5\xAD\xAE %d \xAF\xA8\xE9\xA8.";
	}

	/*
	"\f07����� ��������!\fd"
	*/
	const char *PARTY_IS_STARVING() {
		return "\f07\x8E\xE2\xE0\xEF\xA4 \xA3\xAE\xAB\xAE\xA4\xA0\xA5\xE2!\fd";
	}

	/*
	"�������� � ���� �������������"
	*/
	const char *HIT_SPELL_POINTS_RESTORED() {
		return "\x87\xA4\xAE\xE0\xAE\xA2\xEC\xA5 \xA8 \xAC\xA0\xAD\xA0 \xA2\xAE\xE1\xE1\xE2\xA0\xAD\xAE\xA2\xAB\xA5\xAD\xEB";
	}

	/*
	"����� ������� ������ ��������!"
	*/
	const char *TOO_DANGEROUS_TO_REST() {
		return "\x87\xA4\xA5\xE1\xEC \xE1\xAB\xA8\xE8\xAA\xAE\xAC \xAE\xAF\xA0\xE1\xAD\xAE \xAE\xE2\xA4\xEB\xE5\xA0\xE2\xEC!";
	}

	/*
	"���-�� �� ������ ����� �������. �� �� ����� ������� ���������?"
	*/
	const char *SOME_CHARS_MAY_DIE() {
		return "\x8A\xE2\xAE-\xE2\xAE \xA8\xA7 \xAE\xE2\xE0\xEF\xA4\xA0 \xAC\xAE\xA6\xA5\xE2 \xE3\xAC\xA5\xE0\xA5\xE2\xEC. \x82\xEB \xA2\xE1\xF1 \xE0\xA0\xA2\xAD\xAE \xA6\xA5\xAB\xA0\xA5\xE2\xA5 \xAE\xE2\xA4\xAE\xE5\xAD\xE3\xE2\xEC?";
	}

	/*
	"\r\t010\v005���� �������?"
	*/
	const char *DISMISS_WHOM() {
		return "\r\t010\v005\x8A\xAE\xA3\xAE \xA2\xEB\xA3\xAD\xA0\xE2\xEC?";
	}

	/*
	"�� �� ������ ������� ������ ���������� ���������!"
	*/
	const char *CANT_DISMISS_LAST_CHAR() {
		return "\x82\xEB \xAD\xA5 \xAC\xAE\xA6\xA5\xE2\xA5 \xA2\xEB\xA3\xAD\xA0\xE2\xEC \xE1\xA2\xAE\xA5\xA3\xAE \xAF\xAE\xE1\xAB\xA5\xA4\xAD\xA5\xA3\xAE \xAF\xA5\xE0\xE1\xAE\xAD\xA0\xA6\xA0!";
	}

	const char *DELETE_CHAR_WITH_ELDER_WEAPON() {
		return "\v000\t000This character has an Elder Weapon and cannot be deleted!";
	}

	/*
	"�������", "�������"
	*/
	const char **REMOVE_DELETE() {
		delete[] _removeDelete;
		_removeDelete = new const char *[2] { "\x82\xEB\xA3\xAD\xA0\xE2\xEC", "\xE3\xA4\xA0\xAB\xA8\xE2\xEC" };
		return _removeDelete;
	}

	/*
	"\x3""l\t010\v005���� %s?"
	*/
	const char *REMOVE_OR_DELETE_WHICH() {
		return "\x3"
			   "l\t010\v005\x8A\xAE\xA3\xAE %s?";
	}

	/*
	"\v007��� ����� �����!"
	*/
	const char *YOUR_PARTY_IS_FULL() {
		return "\v007\x82\xA0\xE8 \xAE\xE2\xE0\xEF\xA4 \xAF\xAE\xAB\xAE\xAD!";
	}

	/*
	"\v000\t000���� �������� �� ����� ���� �����, ��� ��� � ���� ���-������ �����!"
	*/
	const char *HAS_SLAYER_SWORD() {
		return "\v000\t000\x9D\xE2\xAE\xE2 \xAF\xA5\xE0\xE1\xAE\xAD\xA0\xA6 \xAD\xA5 \xAC\xAE\xA6\xA5\xE2 \xA1\xEB\xE2\xEC \xE3\xA4\xA0\xAB\xF1\xAD, \xE2\xA0\xAA \xAA\xA0\xAA \xE3 \xAD\xA5\xA3\xAE \x8C\xA5\xE7-\xE3\xA1\xA8\xA9\xE6\xA0 \x8A\xE1\xA8\xAD\xA0!";
	}

	/*
	"%s %s. �� �������, ��� ������ ������� ����� �����?"
	*/
	const char *SURE_TO_DELETE_CHAR() {
		return "%s %s. \x82\xEB \xE3\xA2\xA5\xE0\xA5\xAD\xEB, \xE7\xE2\xAE \xE5\xAE\xE2\xA8\xE2\xA5 \xE3\xA4\xA0\xAB\xA8\xE2\xEC \xED\xE2\xAE\xA3\xAE \xA3\xA5\xE0\xAE\xEF?";
	}

	/*
	"\f04\x3""c\x2\t144\v119\f37�\f04�����\t144\v149\f37�\f04���"
	"\t144\v179\f37ESC\f04\x3""l\x1\t195\v021��\f37�\f04"
	"\t195\v045\f37�\f04��\t195\v069\f37�\f04��\t195\v093��\f37�\f04"
	"\t195\v116�\f37�\f04�\t195\v140\f37�\f04��\t195\v164\f37�\f04��%s";
	*/
	const char *CREATE_CHAR_DETAILS() {
		return "\f04\x3"
			   "c\x2\t144\v119\f37\x81\f04\xE0\xAE\xE1\xAE\xAA\t144\v149\f37\x91\f04\xAE\xA7\xA4"
			   "\t144\v179\f37ESC\f04\x3"
			   "l\x1\t195\v021\x91\xA8\f37\xAB\f04"
			   "\t195\v045\f37\x88\f04\xAD\xE2\t195\v069\f37\x84\f04\xE3\xE5\t195\v093\x91\xAB\f37\xA6\f04"
			   "\t195\v116\x91\f37\xAA\f04\xE0\t195\v140\f37\x8C\f04\xE2\xAA\t195\v164\f37\x93\f04\xA4\xE7%s";
	}

	/*
	"\f04\x3""l\t017\v148����\t055: %s\n"
	"\t017���\t055: %s\n"
	"\t017�����\t055:\n"
	"\x3""r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
	"\t215\v151%d\t215\v175%d\x3""l\t242\v020\f%2d������\t242\v031\f%2d"
	"�������\t242\v042\f%2d������\t242\v053\f%2d������\t242\v064\f%2d"
	"���\t242\v075\f%2d���\t242\v086\f%2d������\t242\v097\f%2d"
	"������\t242\v108\f%2d�����\t242\v119\f%2d��������\f04\x3""c"
	"\t265\v142������\x2\x3""l\t223\v155%s\t223\v170%s%s\x1";
	*/
	const char *NEW_CHAR_STATS() {
		return "\f04\x3"
			   "l\t017\v148\x90\xA0\xE1\xA0\t055: %s\n"
			   "\t017\x8F\xAE\xAB\t055: %s\n"
			   "\t017\x8A\xAB\xA0\xE1\xE1\t055:\n"
			   "\x3"
			   "r\t215\v031%d\t215\v055%d\t215\v079%d\t215\v103%d\t215\v127%d"
			   "\t215\v151%d\t215\v175%d\x3"
			   "l\t242\v020\f%2d\x90\xEB\xE6\xA0\xE0\xEC\t242\v031\f%2d"
			   "\x8F\xA0\xAB\xA0\xA4\xA8\xAD\t242\v042\f%2d\x8B\xE3\xE7\xAD\xA8\xAA\t242\v053\f%2d\x8A\xAB\xA8\xE0\xA8\xAA\t242\v064\f%2d"
			   "\x8C\xA0\xA3\t242\v075\f%2d\x82\xAE\xE0\t242\v086\f%2d\x8D\xA8\xAD\xA4\xA7\xEF\t242\v097\f%2d"
			   "\x82\xA0\xE0\xA2\xA0\xE0\t242\v108\f%2d\x84\xE0\xE3\xA8\xA4\t242\v119\f%2d\x91\xAB\xA5\xA4\xAE\xAF\xEB\xE2\f04\x3"
			   "c"
			   "\t265\v142\x8D\xA0\xA2\xEB\xAA\xA8\x2\x3"
			   "l\t223\v155%s\t223\v170%s%s\x1";
	}

	/*
	"\x3""c������� ��� ���������\n\n"
	*/
	const char *NAME_FOR_NEW_CHARACTER() {
		return "\x3"
			   "c\x82\xA2\xA5\xA4\xA8\xE2\xA5 \xA8\xAC\xEF \xAF\xA5\xE0\xE1\xAE\xAD\xA0\xA6\xA0\n\n";
	}

	/*
	"\v003\x3""c����� ����������� �������� �����.\x3""l"
	*/
	const char *SELECT_CLASS_BEFORE_SAVING() {
		return "\v003\x3"
			   "c\x8F\xA5\xE0\xA5\xA4 \xE1\xAE\xE5\xE0\xA0\xAD\xA5\xAD\xA8\xA5\xAC \xA2\xEB\xA1\xA5\xE0\xA8\xE2\xA5 \xAA\xAB\xA0\xE1\xE1.\x3"
			   "l";
	}

	/*
	"�������� %s �..."
	*/
	const char *EXCHANGE_ATTR_WITH() {
		return "\x8F\xAE\xAC\xA5\xAD\xEF\xE2\xEC %s \xE1...";
	}

	/*
	"\r\f00\x3""c\v000\t000\x2%s%s%s\x1"
	*/
	const char *COMBAT_DETAILS() {
		return "\r\f00\x3"
			   "c\v000\t000\x2%s%s%s\x1";
	}

	/*
	"\x3""c\v000������������ %s, ����� ��������� %s"
	*/
	const char *NOT_ENOUGH_TO_CAST() {
		return "\x3"
			   "c\v000\x8D\xA5\xA4\xAE\xE1\xE2\xA0\xE2\xAE\xE7\xAD\xAE %s, \xE7\xE2\xAE\xA1\xEB \xA7\xA0\xAA\xAB\xA8\xAD\xA0\xE2\xEC %s";
	}

	/*
	"����� �����", "�������"
	*/
	const char **SPELL_CAST_COMPONENTS() {
		delete[] _spellCastComponents;
		_spellCastComponents = new const char *[2] { "\xAE\xE7\xAA\xAE\xA2 \xAC\xA0\xA3\xA8\xA8", "\xA0\xAB\xAC\xA0\xA7\xAE\xA2" };
		return _spellCastComponents;
	}

	/*
	"\r\x2\x3""c\v122\t013\f37�\fd���\t040\f37�\fd��"
	"\t067ESC\x1\t000\v000\x3""c����������\n"
	"\n"
	"%s\x3""c\n"
	"\n"
	"     �������:\x3""c\n"
	"\n"
	"\f09%s\fd\x2\x3""l\n"
	"\v082����\x3""r\t000%u/%u\x3""l\n"
	"��� ��\x3""r\t000%u\x1";
	*/
	const char *CAST_SPELL_DETAILS() {
		return "\r\x2\x3"
			   "c\v122\t013\f37\x87\fd\xA0\xAA\xAB\t040\f37\x8D\fd\xAE\xA2"
			   "\t067ESC\x1\t000\v000\x3"
			   "c\x87\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xA5\n"
			   "\n"
			   "%s\x3"
			   "c\n"
			   "\n"
			   "     \x83\xAE\xE2\xAE\xA2\xAE\xA5:\x3"
			   "c\n"
			   "\n"
			   "\f09%s\fd\x2\x3"
			   "l\n"
			   "\v082\x96\xA5\xAD\xA0\x3"
			   "r\t000%u/%u\x3"
			   "l\n"
			   "\x92\xA5\xAA \x8E\x8C\x3"
			   "r\t000%u\x1";
	}

	/*
	"\x3""c�������:\n"
	"\n"
	"\x3""r\t000%lu ���\n"
	"%lu ����";
	*/
	const char *PARTY_FOUND() {
		return "\x3"
			   "c\x8D\xA0\xA9\xA4\xA5\xAD\xAE:\n"
			   "\n"
			   "\x3"
			   "r\t000%lu \x87\xAE\xAB\n"
			   "%lu \x80\xAB\xAC\xA7";
	}

	/*
	"\v007\f12��������! ������� �����������!\fd\n"
	"������� �������";
	*/
	const char *BACKPACKS_FULL_PRESS_KEY() {
		return "\v007\f12\x82\xAD\xA8\xAC\xA0\xAD\xA8\xA5! \x90\xEE\xAA\xA7\xA0\xAA\xA8 \xAF\xA5\xE0\xA5\xAF\xAE\xAB\xAD\xA5\xAD\xEB!\fd\n"
			   "\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3";
	}

	/*
	"\x3""l\v120\t000\x4""077\x3""c\f37�. �������\fd"
	*/
	const char *HIT_A_KEY() {
		return "\x3"
			   "l\v120\t000\x4"
			   "077\x3"
			   "c\f37\x8D. \xAA\xAB\xA0\xA2\xA8\xE8\xE3\fd";
	}

	const char *GIVE_TREASURE_FORMATTING() {
		return "\x3l\v060\t000\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077\n"
			   "\x4"
			   "077";
	}

	/*
	"\v060\t000\x3""c%s ���%s: %s"
	*/
	const char *X_FOUND_Y() {
		return "\v060\t000\x3"
			   "c%s \xAD\xA0\xE8%s: %s";
	}

	/*
	"\x3""c\v009����?"
	*/
	const char *ON_WHO() {
		return "\x3"
			   "c\v009\x8A\xAE\xA3\xAE?";
	}

	/*
	"\r\x3""c\x1����� ������?\x2\v034\t014\f15�\fd����\t044"
	"\f15�\fd���\t074\f15�\fd����\t104\f15�\fd�\x1";
	*/
	const char *WHICH_ELEMENT1() {
		return "\r\x3"
			   "c\x1\x8A\xA0\xAA\xA0\xEF \x91\xE2\xA8\xE5\xA8\xEF?\x2\v034\t014\f15\x8E\fd\xA3\xAE\xAD\xEC\t044"
			   "\f15\x83\fd\xE0\xAE\xAC\t074\f15\x95\fd\xAE\xAB\xAE\xA4\t104\f15\x9F\fd\xA4\x1";
	}

	/*
	"\r\x3""c����� ������?\x2\v034\t014\f15�\fd����\t044"
	"\f15�\fd���\t074\f15�\fd����\t104\f15�\fd�\x1";
	*/
	const char *WHICH_ELEMENT2() {
		return "\r\x3"
			   "c\x8A\xA0\xAA\xA0\xEF \x91\xE2\xA8\xE5\xA8\xEF?\x2\v034\t014\f15\x8E\fd\xA3\xAE\xAD\xEC\t044"
			   "\f15\x83\fd\xE0\xAE\xAC\t074\f15\x95\fd\xAE\xAB\xAE\xA4\t104\f15\x9F\fd\xA4\x1";
	}

	/*
	"\x3""c����� ��������"
	*/
	const char *DETECT_MONSTERS() {
		return "\x3"
			   "c\x8F\xAE\xA8\xE1\xAA \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2";
	}

	/*
	"\r\x3""c\v000\t000\x1���� ������\n"
	"\n"
	"��������� �����\n"
	"\n"
	"%s\x3""l\n"
	"x = %d\x3""r\t000y = %d\x3""c\x2\v122\t021\f15�\fd����\t060\f15�\fd���\x1";
	*/
	const char *LLOYDS_BEACON() {
		return "\r\x3"
			   "c\v000\t000\x1\x8C\xA0\xEF\xAA \x8B\xAB\xAE\xA9\xA4\xA0\n"
			   "\n"
			   "\x8F\xAE\xE1\xAB\xA5\xA4\xAD\xA5\xA5 \xAC\xA5\xE1\xE2\xAE\n"
			   "\n"
			   "%s\x3"
			   "l\n"
			   "x = %d\x3"
			   "r\t000y = %d\x3"
			   "c\x2\v122\t021\f15\x93\fd\xE1\xE2\xA0\xAD\t060\f15\x82\fd\xA5\xE0\xAD\x1";
	}

	/*
	"\x3""c��������\n������� ������ �� %s (1-9)\n"
	*/
	const char *HOW_MANY_SQUARES() {
		return "\x3"
			   "c\x92\xA5\xAB\xA5\xAF\xAE\xE0\xE2\n\x91\xAA\xAE\xAB\xEC\xAA\xAE \xAA\xAB\xA5\xE2\xAE\xAA \xAD\xA0 %s (1-9)\n";
	}

	/*
	"\x3""c��������� ������\x3""l\n"
	"\n"
	"\t0101. %s\n"
	"\t0102. %s\n"
	"\t0103. %s\n"
	"\t0104. %s\n"
	"\t0105. %s\x3""c\n"
	"\n"
	"� ����� ����� (1-5)\n"
	"\n";
	*/
	const char *TOWN_PORTAL() {
		return "\x3"
			   "c\x83\xAE\xE0\xAE\xA4\xE1\xAA\xAE\xA9 \xAF\xAE\xE0\xE2\xA0\xAB\x3"
			   "l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\n"
			   "\t0104. %s\n"
			   "\t0105. %s\x3"
			   "c\n"
			   "\n"
			   "\x82 \xAA\xA0\xAA\xAE\xA9 \xA3\xAE\xE0\xAE\xA4 (1-5)\n"
			   "\n";
	}

	const char *TOWN_PORTAL_SWORDS() {
		return "\x3"
			   "cTown Portal\x3l\n"
			   "\n"
			   "\t0101. %s\n"
			   "\t0102. %s\n"
			   "\t0103. %s\x3"
			   "c\n"
			   "\n"
			   "To which Town (1-3)\n"
			   "\n";
	}

	const char *MONSTER_DETAILS() {
		return "\x3l\n"
			   "%s\x3"
			   "c\t100%s\t140%u\t180%u\x3r\t000%s";
	}

	/*
	"���", "�����", "����", "�����", "������", "����������", "�������",
	"�������", "�������", "���", "��������", "�����", "��������", "���������",
	"�������", "�������", "��������", "�������", "��������", "�������",
	"�������+5", "�����", "������"
	*/
	const char **MONSTER_SPECIAL_ATTACKS() {
		delete[] _monsterSpecialAttacks;
		_monsterSpecialAttacks = new const char *[23] {
			"\x8D\xA5\xE2", "\x8C\xA0\xA3\xA8\xEF", "\x8E\xA3\xAD\xEF", "\x83\xE0\xAE\xAC\xA0", "\x95\xAE\xAB\xAE\xA4\xA0", "\x8E\xE2\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xA5", "\x9D\xAD\xA5\xE0\xA3\xA8\xA8",
				"\x81\xAE\xAB\xA5\xA7\xAD\xEC", "\x81\xA5\xA7\xE3\xAC\xA8\xA5", "\x91\xAE\xAD", "\x8F\xE0\xAA\xAB\x8F\xE0\xA4\xAC", "\x82\xAB\xEE\xA1\xAB", "\x8E\xE1\xE3\xE8\x87\xA4\xAE\xE0", "\x8F\xE0\xAE\xAA\xAB\xEF\xE2\xA8\xA5",
				"\x8F\xA0\xE0\xA0\xAB\xA8\xE7", "\x81\xA5\xE1\xE1\xAE\xA7\xAD", "\x91\xAC\xEF\xE2\xA5\xAD\xA8\xA5", "\x8B\xAE\xAC\x81\xE0\xAE\xAD", "\x91\xAB\xA0\xA1\xAE\xE1\xE2\xEC", "\x93\xAD\xA8\xE7\xE2\xAE\xA6",
				"\x82\xAE\xA7\xE0\xA0\xE1\xE2+5", "\x8C\xA5\xE0\xE2\xA2", "\x8A\xA0\xAC\xA5\xAD\xEC"
		};
		return _monsterSpecialAttacks;
	}

	/*
	"������\x3""c\t155��\t195��\t233#����\x3""r\t000������%s%s%s"
	*/
	const char *IDENTIFY_MONSTERS() {
		return "\x8C\xAE\xAD\xE1\xE2\xE0\x3"
			   "c\t155\x8E\x87\t195\x8A\x87\t233#\x80\xE2\xA0\xAA\x3"
			   "r\t000\x8E\xE1\xAE\xA1\xAE\xA5%s%s%s";
	}

	/*
	"\x3""c\v012\t000���� ���� �� ������� � ���, �� �� ������� ������� �� Ҹ���� ������� �����"
	*/
	const char *MOONS_NOT_ALIGNED() {
		return "\x3"
			   "c\v012\t000\x8F\xAE\xAA\xA0 \xAB\xE3\xAD\xEB \xAD\xA5 \xA2\xE1\xE2\xA0\xAD\xE3\xE2 \xA2 \xE0\xEF\xA4, \xA2\xEB \xAD\xA5 \xE1\xAC\xAE\xA6\xA5\xE2\xA5 \xAF\xA5\xE0\xA5\xA9\xE2\xA8 \xAD\xA0 \x92\xF1\xAC\xAD\xE3\xEE \xE1\xE2\xAE\xE0\xAE\xAD\xE3 \x8A\xE1\xA8\xAD\xA0";
	}

	/*
	"\r\x1\fd\x3""c\v000\t000%s %s: ������� \x3""l\x2\n"
	"%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";
	*/
	const char *AWARDS_FOR() {
		return "\r\x1\fd\x3"
			   "c\v000\t000%s %s: \x8D\xA0\xA3\xE0\xA0\xA4\xEB \x3"
			   "l\x2\n"
			   "%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\n%s\x1";
	}

	/*
	"\r\x2\x3""c\v021\t221�����\t255����\t289�����"
	*/
	const char *AWARDS_TEXT() {
		return "\r\x2\x3"
			   "c\v021\t221\x82\xA2\xA5\xE0\xE5\t255\x82\xAD\xA8\xA7\t289\x82\xEB\xE5\xAE\xA4";
	}

	/*
	"\x3""c� ��������� ��� ������"
	*/
	const char *NO_AWARDS() {
		return "\x3"
			   "c\x93 \xAF\xA5\xE0\xE1\xAE\xAD\xA0\xA6\xA0 \xAD\xA5\xE2 \xAD\xA0\xA3\xE0\xA0\xA4";
	}

	/*
	"�����\n\t125������ �����"
	*/
	const char *WARZONE_BATTLE_MASTER() {
		return "\x80\xE0\xA5\xAD\xA0\n\t125\x8C\xA0\xE1\xE2\xA5\xE0 \xA1\xA8\xE2\xA2\xEB";
	}

	/*
	"���!  ����� ��?  �� ����������� � ���, ��� �� ����� ���� �����!"
	*/
	const char *WARZONE_MAXED() {
		return "\x97\xE2\xAE!  \x8E\xAF\xEF\xE2\xEC \xA2\xEB?  \x8D\xA5 \xAF\xE0\xA8\xE1\xE2\xA0\xA2\xA0\xA9\xE2\xA5 \xAA \xE2\xA5\xAC, \xAA\xE2\xAE \xAD\xA5 \xAC\xAE\xA6\xA5\xE2 \xA4\xA0\xE2\xEC \xE1\xA4\xA0\xE7\xA8!";
	}

	/*
	"����� ������� ��������? (1-10)\n"
	*/
	const char *WARZONE_LEVEL() {
		return "\x8A\xA0\xAA\xAE\xA9 \xE3\xE0\xAE\xA2\xA5\xAD\xEC \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2? (1-10)\n";
	}

	/*
	"������� ��������? (1-20)\n"
	*/
	const char *WARZONE_HOW_MANY() {
		return "\x91\xAA\xAE\xAB\xEC\xAA\xAE \xAC\xAE\xAD\xE1\xE2\xE0\xAE\xA2? (1-20)\n";
	}

	/*
	"\x3""c\v010%s �������%s �����!\n������� �������."
	*/
	const char *PICKS_THE_LOCK() {
		return "\x3"
			   "c\v010%s \xA2\xA7\xAB\xAE\xAC\xA0\xAB%s \xA7\xA0\xAC\xAE\xAA!\n\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3.";
	}

	/*
	"\x3""c\v010%s �� ����%s �������� �����!\n������� �������."
	*/
	const char *UNABLE_TO_PICK_LOCK() {
		return "\x3"
			   "c\v010%s \xAD\xA5 \xE1\xAC\xAE\xA3%s \xA2\xA7\xAB\xAE\xAC\xA0\xE2\xEC \xA7\xA0\xAC\xAE\xAA!\n\x8D\xA0\xA6\xAC\xA8\xE2\xA5 \xAA\xAB\xA0\xA2\xA8\xE8\xE3.";
	}

	/*
	"\x1\f00\x3""c\v000\t000������ ����������\x3""r"
	"\v022\t045��\f06�\fd�:\t124\f06�\fd���:"
	"\v041\t045\f06�\fd��.:\t124�\f06�\fd��:\v060\t045\f06�\fd���:"
	"\t124\f06�\fd����"
	"\v080\t084\f06�\fd����� %s\t137����\t000\x1";
	*/
	const char *CONTROL_PANEL_TEXT() {
		return "\x1\f00\x3"
			   "c\v000\t000\x8F\xA0\xAD\xA5\xAB\xEC \xE3\xAF\xE0\xA0\xA2\xAB\xA5\xAD\xA8\xEF\x3"
			   "r"
			   "\v022\t045\x87\xA2\f06\xE3\fd\xAA:\t124\f06\x91\fd\xAE\xE5\xE0:"
			   "\v041\t045\f06\x8C\fd\xE3\xA7.:\t124\x91\f06\xAE\fd\xE5\xE0:\v060\t045\f06\x87\fd\xA0\xA3\xE0:"
			   "\t124\f06\x82\fd\xEB\xE5\xAE\xA4"
			   "\v080\t084\f06\x8F\fd\xAE\xAC\xAE\xE9\xEC %s\t137\x8C\xA0\xA3\xA0\t000\x1";
	}

	const char *CONTROL_PANEL_BUTTONS() {
		return "\x3"
			   "c\f11"
			   "\v022\t062load\t141%s"
			   "\v041\t062save\t141%s"
			   "\v060\t062exit"
			   "\v079\t102Help\fd";
	}

	/*
	"\f15���\f11"
	*/
	const char *ON() {
		return "\f15\xA2\xAA\xAB\f11";
	}

	/*
	"\f32���\f11"
	*/
	const char *OFF() {
		return "\f32\xA2\xEB\xAA\f11";
	}

	/*
	"�� �������, ��� ������ �����?"
	*/
	const char *CONFIRM_QUIT() {
		return "\x82\xEB \xE3\xA2\xA5\xE0\xA5\xAD\xEB, \xE7\xE2\xAE \xE5\xAE\xE2\xA8\xE2\xA5 \xA2\xEB\xA9\xE2\xA8?";
	}

	/*
	"�� ����� ������ ��������������� ������� ��. ����?"
	*/
	const char *MR_WIZARD() {
		return "\x82\xEB \xE2\xAE\xE7\xAD\xAE \xE5\xAE\xE2\xA8\xE2\xA5 \xA2\xAE\xE1\xAF\xAE\xAB\xEC\xA7\xAE\xA2\xA0\xE2\xEC\xE1\xEF \xAF\xAE\xAC\xAE\xE9\xEC\xEE \x8C\xE0. \x8C\xA0\xA3\xA0?";
	}

	/*
	"������� �������� �� ����� �����!"
	*/
	const char *NO_LOADING_IN_COMBAT() {
		return "\x8D\xA8\xAA\xA0\xAA\xA8\xE5 \xA7\xA0\xA3\xE0\xE3\xA7\xAE\xAA \xA2\xAE \xA2\xE0\xA5\xAC\xEF \xA1\xA8\xE2\xA2\xEB!";
	}

	/*
	"������� ���������� �� ����� �����!"
	*/
	const char *NO_SAVING_IN_COMBAT() {
		return "\x8D\xA8\xAA\xA0\xAA\xA8\xE5 \xE1\xAE\xE5\xE0\xA0\xAD\xA5\xAD\xA8\xA9 \xA2\xAE \xA2\xE0\xA5\xAC\xEF \xA1\xA8\xE2\xA2\xEB!";
	}

	/*
	"\r\fd\x3""c\v000\t000������� ��������\n\n"
	"%s\x3""l\n\n"
	"�������\x3""r\n"
	"\t000%s\x2\x3""c\v122\t019\f37�\f04���\t055�����\x1";
	*/
	const char *QUICK_FIGHT_TEXT() {
		return "\r\fd\x3"
			   "c\v000\t000\x81\xEB\xE1\xE2\xE0\xAE\xA5 \xA4\xA5\xA9\xE1\xE2\xA2\xA8\xA5\n\n"
			   "%s\x3"
			   "l\n\n"
			   "\x92\xA5\xAA\xE3\xE9\xA5\xA5\x3"
			   "r\n"
			   "\t000%s\x2\x3"
			   "c\v122\t019\f37\x91\f04\xAB\xA5\xA4\t055\x82\xEB\xE5\xAE\xA4\x1";
	}

	/*
	"�����", "����������", "����", "�������"
	*/
	const char **QUICK_FIGHT_OPTIONS() {
		delete[] _quickFightOptions;
		_quickFightOptions = new const char *[4] { "\x80\xE2\xA0\xAA\xA0", "\x87\xA0\xAA\xAB\xA8\xAD\xA0\xAD\xA8\xA5", "\x81\xAB\xAE\xAA", "\x81\xA5\xA3\xE1\xE2\xA2\xAE" };
		return _quickFightOptions;
	}

	const char **WORLD_END_TEXT() {
		delete[] _worldEndText;
		_worldEndText = new const char *[9] {
			"\n\n\n\n\n\n\n"
			"Congratulations Adventurers!\n\n"
			"Let the unification ceremony begin!",
				"And so the call went out to the people throughout the lands of Xeen"
				" that the prophecy was nearing completion.",
				"They came in great numbers to witness the momentous occasion.",
				"\v026The Dragon Pharoah presided over the ceremony.",
				"\v026Queen Kalindra presented the Cube of Power.",
				"\v026Prince Roland presented the Xeen Sceptre.",
				"\v026Together, they placed the Cube of Power...",
				"\v026and the Sceptre, onto the Altar of Joining.",
				"With the prophecy complete, the two sides of Xeen were united as one",
		};
		return _worldEndText;
	}

	const char *WORLD_CONGRATULATIONS() {
		return "\x3"
			   "cCongratulations\n\n"
			   "Your Final Score is:\n\n"
			   "%010lu\n"
			   "\x3l\n"
			   "Please send this score to the Ancient's Headquarters where "
			   "you'll be added to the Hall of Legends!\n\n"
			   "Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078";
	}

	const char *WORLD_CONGRATULATIONS2() {
		return "\n\n\n\n\n\n"
			   "But wait... there's more!\n"
			   "\n\n"
			   "Include the message\n"
			   "\"%s\"\n"
			   "with your final score and receive a special bonus.";
	}

	const char *CLOUDS_CONGRATULATIONS1() {
		return "\f23\x3l"
			   "\v000\t000Please send this score to the Ancient's Headquarters "
			   "where you'll be added to the Hall of Legends!\f33\x3"
			   "c"
			   "\v070\t000Press a Key";
	}

	const char *CLOUDS_CONGRATULATIONS2() {
		return "\f23\x3l"
			   "\v000\t000Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078-4302\f33\x3"
			   "c"
			   "\v070\t000Press a Key";
	}

	const char **GOOBER() {
		delete[] _goober;
		_goober = new const char *[3] {
			"", "I am a Goober!", "I am a Super Goober!"
		};
		return _goober;
	}

	/*
	"\v000\t000\x3""c�������� ������� �����"
	*/
	const char *DIFFICULTY_TEXT() {
		return "\v000\t000\x3"
			   "c\x82\xEB\xA1\xA5\xE0\xA8\xE2\xA5 \xA8\xA3\xE0\xAE\xA2\xAE\xA9 \xE0\xA5\xA6\xA8\xAC";
	}

	/*
	"\x3""c\v002\t000���� �������������� ��� ������� ��� ����� ���������!\n"
	"���, ����� ������ �����������";
	*/
	const char *SAVE_OFF_LIMITS() {
		return "\x3"
			   "c\v002\t000\x81\xAE\xA3\xA8 \x82\xAE\xE1\xE1\xE2\xA0\xAD\xAE\xA2\xAB\xA5\xAD\xA8\xEF \x88\xA3\xE0 \xE1\xE7\xA8\xE2\xA0\xEE\xE2 \xED\xE2\xAE \xAC\xA5\xE1\xE2\xAE \xA7\xA0\xAF\xE0\xA5\xE2\xAD\xEB\xAC!\n"
			   "\x93\xA2\xEB, \xA7\xA4\xA5\xE1\xEC \xAD\xA5\xAB\xEC\xA7\xEF \xE1\xAE\xE5\xE0\xA0\xAD\xEF\xE2\xEC\xE1\xEF";
	}

	const char *CLOUDS_INTRO1() {
		return "\f00\v082\t040\x3"
			   "cKing Burlock\v190\t040Peasants\v082\t247"
			   "Lord Xeen\v190\t258Xeen's Pet\v179\t150Crodo";
	}

	const char *DARKSIDE_ENDING1() {
		return "\n\x3"
			   "cCongratulations\n"
			   "\n"
			   "Your Final Score is:\n"
			   "\n"
			   "%010lu\n"
			   "\x3"
			   "l\n"
			   "Please send this score to the Ancient's Headquarters "
			   "where you'll be added to the Hall of Legends!\n"
			   "\n"
			   "Ancient's Headquarters\n"
			   "New World Computing, Inc.\n"
			   "P.O. Box 4302\n"
			   "Hollywood, CA 90078";
	}

	const char *DARKSIDE_ENDING2() {
		return "\n"
			   "Adventurers,\n"
			   "\n"
			   "I will save your game in Castleview.\n"
			   "\n"
			   "The World of Xeen still needs you!\n"
			   "\n"
			   "Load your game afterwards and come visit me in the "
			   "Great Pyramid for further instructions";
	}

	const char *PHAROAH_ENDING_TEXT1() {
		return "\fd\v001\t001%s\x3"
			   "c\t000\v180Press a Key!\x3"
			   "l";
	}

	const char *PHAROAH_ENDING_TEXT2() {
		return "\f04\v000\t000%s\x3"
			   "c\t000\v180Press a Key!\x3"
			   "l\fd";
	}
};


