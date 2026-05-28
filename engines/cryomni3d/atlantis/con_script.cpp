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

#include "common/array.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "cryomni3d/atlantis/con_script.h"

namespace CryOmni3D {
namespace Atlantis {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

// Strip trailing whitespace and carriage returns from a String.
static void rtrim(Common::String &s) {
	while (!s.empty()) {
		char c = s.lastChar();
		if (c == ' ' || c == '\t' || c == '\r')
			s.deleteLastChar();
		else
			break;
	}
}

// Parse a compound condition string of the form:
//   (key op val)&(key op val)&...
// where op is '=', '<', or '>'.
// Keys are lowercased. Unknown/malformed fragments are silently skipped.
static void parseConditions(const Common::String &str, Common::Array<ConCondition> &out) {
	const char *p = str.c_str();
	while (*p) {
		// Skip to the next '('
		while (*p && *p != '(') p++;
		if (!*p) break;
		p++; // consume '('

		// Find matching ')'
		const char *start = p;
		while (*p && *p != ')') p++;
		if (!*p) break;
		Common::String cond(start, (uint32)(p - start));
		p++; // consume ')'

		// Find the operator character
		const char *cp = cond.c_str();
		const char *opPtr = nullptr;
		char op = '=';
		for (const char *q = cp; *q; q++) {
			if (*q == '=' || *q == '<' || *q == '>' || *q == '!') {
				opPtr = q;
				op = *q;
				break;
			}
		}
		if (!opPtr) continue;

		ConCondition cc;
		cc.key = Common::String(cp, (uint32)(opPtr - cp));
		cc.key.toLowercase();
		cc.op    = op;
		cc.value = atoi(opPtr + 1);

		// Record the separator that FOLLOWS this condition, so the evaluator
		// can decide whether it is an AND-link or an OR-link.  See the comment
		// on ConCondition for the left-to-right short-circuit semantics this
		// drives (mirrors atlantis.exe FUN_004298ac).
		while (*p == ' ') p++;
		if (*p == '|') {
			cc.nextSep = '|';
			p++;
		} else if (*p == '&') {
			cc.nextSep = '&';
			p++;
		}
		while (*p == ' ') p++;

		out.push_back(cc);
	}
}

// ---------------------------------------------------------------------------
// ConScript
// ---------------------------------------------------------------------------

void ConScript::reset() {
	_sections.clear();
	_initCmds.clear();
}

void ConScript::parse(const char *buf, uint32 len) {
	reset();

	const char *p   = buf;
	const char *end = buf + len;
	bool inInit     = false;
	ConSection *cur = nullptr;

	while (p < end) {
		// Read one line.
		const char *sol = p;
		while (p < end && *p != '\r' && *p != '\n') p++;
		Common::String line(sol, (uint32)(p - sol));
		while (p < end && (*p == '\r' || *p == '\n')) p++;
		rtrim(line);
		if (line.empty()) continue;

		// /INIT / /FININIT markers.
		if (line.equalsIgnoreCase("/INIT"))    { inInit = true;  cur = nullptr; continue; }
		if (line.equalsIgnoreCase("/FININIT")) { inInit = false;               continue; }

		if (inInit) {
			// Collect /set(...) content.
			if (line.size() > 5 && scumm_strnicmp(line.c_str(), "/set(", 5) == 0) {
				uint endPos = line.size();
				if (line.lastChar() == ')') endPos--;
				if (endPos > 5)
					_initCmds.push_back(line.substr(5, endPos - 5));
			}
			continue;
		}

		// Section header: /N where every character after / is a digit.
		if (line[0] == '/' && line.size() >= 2 && Common::isDigit(line[1])) {
			bool allDigits = true;
			for (uint i = 1; i < line.size(); i++)
				if (!Common::isDigit(line[i])) { allDigits = false; break; }
			if (allDigits) {
				int sid = atoi(line.c_str() + 1);
				if (sid == 0) break;  // /000 = end of script
				_sections.push_back(ConSection());
				cur = &_sections.back();
				cur->id = sid;
				continue;
			}
		}

		if (!cur) continue;

		// /con(...) or /con*(...) — player-triggered condition.
		if (line.size() >= 4 && scumm_strnicmp(line.c_str(), "/con", 4) == 0) {
			cur->isCon = true;
			uint i = 4;
			if (i < line.size() && line[i] == '*') { cur->needsItem = true; i++; }
			Common::String condStr = (i < line.size()) ? line.substr(i) : Common::String();
			parseConditions(condStr, cur->conditions);
			// Extract clicPerso and clicZone for engine lookup.
			for (const ConCondition &cc : cur->conditions) {
				if (cc.key == "clicperso") cur->clicPerso = cc.value;
				if (cc.key == "cliczone")  cur->clicZone  = cc.value;
			}
			continue;
		}

		// /sel(...) — automatic condition (view events, game-state checks).
		if (line.size() >= 4 && scumm_strnicmp(line.c_str(), "/sel", 4) == 0) {
			cur->isCon = false;
			Common::String condStr = (4u < line.size()) ? line.substr(4) : Common::String();
			parseConditions(condStr, cur->conditions);
			continue;
		}

		// /timN(...) — timer-driven automatic condition (timerNum = the digit after /tim).
		if (line.size() >= 4 && scumm_strnicmp(line.c_str(), "/tim", 4) == 0) {
			cur->isCon    = false;
			cur->isTimer  = true;
			// Extract timer number (digit(s) between "/tim" and "(").
			uint i = 4;
			uint numStart = i;
			while (i < line.size() && Common::isDigit(line[i])) i++;
			cur->timerNum = (i > numStart) ? atoi(line.c_str() + numStart) : 1;
			Common::String condStr = (i < line.size()) ? line.substr(i) : Common::String();
			parseConditions(condStr, cur->conditions);
			continue;
		}

		// /sujN[,M[,...]] -- subject gate, supports a comma-separated
		// list of subject ids.  The section fires when ANY listed
		// subject is enabled for the clicked perso (see ConSection::sujIds).
		if (line.size() >= 5 && scumm_strnicmp(line.c_str(), "/suj", 4) == 0
		        && Common::isDigit(line[4])) {
			const char *p = line.c_str() + 4;
			while (*p) {
				int v = atoi(p);
				cur->sujIds.push_back(v);
				while (*p && *p != ',') p++;
				if (*p == ',') p++;
			}
			if (!cur->sujIds.empty())
				cur->sujId = cur->sujIds[0];
			continue;
		}

		// /goN — redirect.
		if (line.size() >= 4 && scumm_strnicmp(line.c_str(), "/go", 3) == 0
		        && Common::isDigit(line[3])) {
			cur->gotoId = atoi(line.c_str() + 3);
			continue;
		}

		// /set(...) or /Set(...) — game-state command; preserve position relative to dialog lines.
		if (line.size() > 5 && scumm_strnicmp(line.c_str(), "/set(", 5) == 0) {
			uint endPos = line.size();
			if (line.lastChar() == ')') endPos--;
			if (endPos > 5) {
				ConSectionItem item;
				item.isSet  = true;
				item.setCmd = line.substr(5, endPos - 5);
				cur->items.push_back(item);
			}
			continue;
		}

		// [Speaker,params] text — dialog line; preserve position relative to /set commands.
		if (line[0] == '[') {
			uint close = 1;
			while (close < line.size() && line[close] != ']') close++;
			if (close < line.size()) {
				Common::String block = line.substr(1, close - 1);
				Common::String text;
				if (close + 1 < line.size()) {
					if (line[close + 1] == ' ')
						text = line.substr(close + 2);
					else
						text = line.substr(close + 1);
				}

				ConLine cl;
				uint comma = block.find(',');
				if (comma == Common::String::npos) {
					cl.speaker = block;
				} else {
					cl.speaker = block.substr(0, comma);
					cl.params  = block.substr(comma + 1);
				}
				cl.text = text;

				// Extract optional numeric angle tokens.  The original CON
				// syntax is `[Speaker, X, Y, camN]` — the two tokens after
				// the speaker are pixel-space panorama offsets that the
				// engine writes directly into the cyclo view (see header
				// comment on ConLine).  When only `camN` or `off` follows
				// the speaker, no angle is encoded.
				//
				// Implementation: split params by comma, recognise a leading
				// pair of integer-only tokens as (X, Y).  Anything else (a
				// "camN", "off", or trailing string token) terminates the
				// scan.  Tokens are trimmed of surrounding whitespace.
				if (!cl.params.empty()) {
					Common::Array<Common::String> partTok;
					const char *q = cl.params.c_str();
					while (*q) {
						while (*q == ' ' || *q == '\t') q++;
						const char *qe = q;
						while (*qe && *qe != ',') qe++;
						const char *trimEnd = qe;
						while (trimEnd > q &&
						       (trimEnd[-1] == ' ' || trimEnd[-1] == '\t'))
							trimEnd--;
						partTok.push_back(
						    Common::String(q, (uint32)(trimEnd - q)));
						if (*qe == ',') q = qe + 1;
						else q = qe;
					}
					auto isIntToken = [](const Common::String &s) -> bool {
						if (s.empty()) return false;
						uint i = 0;
						if (s[0] == '-' || s[0] == '+') i = 1;
						if (i >= s.size()) return false;
						for (; i < s.size(); ++i)
							if (!Common::isDigit(s[i])) return false;
						return true;
					};
					if (partTok.size() >= 2 &&
					    isIntToken(partTok[0]) && isIntToken(partTok[1])) {
						cl.hasAngle = true;
						cl.angleX   = atoi(partTok[0].c_str());
						cl.angleY   = atoi(partTok[1].c_str());
					}
				}

				ConSectionItem item;
				item.isSet = false;
				item.line  = cl;
				cur->items.push_back(item);
			}
			continue;
		}

		// Anything else — ignored.
	}
}

} // namespace Atlantis
} // namespace CryOmni3D
