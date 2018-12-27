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

#ifndef GLK_ALAN2_PARSE
#define GLK_ALAN2_PARSE

namespace Glk {
namespace Alan2 {

#define LISTLEN 100

class Parser {
public:
	Parser();

	/**
	 * Parse a new player command
	 */
	void parse();

private:
	void unknown(char *inputStr);
	int lookup(char *wrd);
	char *gettoken(char *tokenBuffer);
	void agetline();
	void scan();
	void nonverb();
	Abool objhere(Aword obj);
	Aword acthere(Aword act);
	Abool isHere(Aword id);
	void buildall(ParamElem list[]);
	void listCopy(ParamElem a[], ParamElem b[]);
	bool listContains(ParamElem l[], Aword e);
	void listIntersection(ParamElem a[], ParamElem b[]);
	void listCopyFromDictionary(ParamElem p[], Aword r[]);
	int listLength(ParamElem a[]);
	void listCompact(ParamElem a[]);
	void listMerge(ParamElem a[], ParamElem b[]);
	void listSubtract(ParamElem a[], ParamElem b[]);
	void unambig(ParamElem plst[]);
	void simple(ParamElem olst[]);
	void complex(ParamElem olst[]);
	bool claCheck(ClaElem *cla);
	void resolve(ParamElem plst[]);
	bool endOfTable(StxElem *addr);
	bool endOfTable(ElmElem *addr);
	bool endOfTable(ClaElem *addr);
	bool endOfTable(VrbElem *addr);
	bool endOfTable(AltElem *addr);
	bool endOfTable(ChkElem *addr);
	AltElem *findalt(Aword vrbsadr, Aword param);
	bool trycheck(Aaddr adr, bool act);
	bool possible();
	void tryMatch(ParamElem mlst[]);
	void match(ParamElem *mlst);
	void action(ParamElem plst[]);

	// TODO: Initialize / move these
	int wrds[LISTLEN / 2];	// List of parsed words
	int wrdidx;			// and an index into it

	bool plural = false;

	// Syntax Parameters
	int paramidx;			// Index in params
	ParamElem *params;		// List of params
	static ParamElem *pparams;	// Previous parameter list
	static ParamElem *mlst;		// Multiple objects list
	static ParamElem *pmlst;	// Previous multiple list

	StxElem *stxs;		// Syntax table pointer
	LocElem *locs;		// Location table pointer

	// Literals
	LitElem litValues[MAXPARAMS + 1];

	// What did the user say?
	int vrbwrd;			// The word he used
	int vrbcode;			// The code for that verb

	bool eol;	// Looking at End of line? Yes, initially
	char *token;

	WrdElem *dict;		// Dictionary pointer
	int dictsize;

	int allLength;		// No. of objects matching 'all'
};

} // End of namespace Alan2
} // Engine of namespace GLK

#endif
