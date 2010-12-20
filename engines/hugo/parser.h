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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_PARSER_H
#define HUGO_PARSER_H
namespace Hugo {

enum seqTextParser {
	kTBExit  = 0, kTBMaze,    kTBNoPoint, kTBNoun,       kTBVerb,
	kTBEh,        kTBUnusual, kTBHave,    kTBNoUse,      kTBDontHave,
	kTBNeed,      kTBOk,      kCmtAny1,   kCmtAny2,      kCmtAny3,
	kCmtClose,    kTBIntro,   kTBOutro,   kTBUnusual_1d, kCmtAny4,
	kCmtAny5,     kTBExit_1d, kTBEh_1d,   kTBEh_2d,      kTBNoUse_2d
};

class Parser {
public:
	Parser(HugoEngine *vm);
	virtual ~Parser();

	bool isWordPresent(char **wordArr);

	void charHandler();
	void command(const char *format, ...);

	virtual void keyHandler(Common::Event event) = 0;
	virtual void lineHandler() = 0;

protected:
	HugoEngine *_vm;

protected:
	char *findNoun();
	char *findVerb();
	void  showDosInventory();

	bool   _checkDoubleF1Fl;                        // Flag used to display user help or instructions
	uint16 _getIndex;                               // Index into ring buffer
	uint16 _putIndex;
	char   _ringBuffer[32];                         // Ring buffer
};

class Parser_v1d : public Parser {
public:
	Parser_v1d(HugoEngine *vm);
	~Parser_v1d();

	virtual void keyHandler(Common::Event event);
	virtual void lineHandler();

protected:
	virtual void  dropObject(object_t *obj);
	virtual bool  isBackgroundWord(char *noun, char *verb, objectList_t obj);
	virtual bool  isCatchallVerb(bool testNounFl, char *noun, char *verb, objectList_t obj);
	virtual bool  isGenericVerb(char *word, object_t *obj);
	virtual bool  isNear(char *verb, char *noun, object_t *obj, char *comment);
	virtual bool  isObjectVerb(char *word, object_t *obj);
	virtual void  takeObject(object_t *obj);

	char *findNextNoun(char *noun);
};

class Parser_v2d : public Parser_v1d {
public:
	Parser_v2d(HugoEngine *vm);
	~Parser_v2d();

	void lineHandler();
};

class Parser_v3d : public Parser_v1d {
public:
	Parser_v3d(HugoEngine *vm);
	~Parser_v3d();

	virtual void lineHandler();
protected:
	void  dropObject(object_t *obj);
	bool  isBackgroundWord(objectList_t obj);
	bool  isCatchallVerb(objectList_t obj);
	bool  isGenericVerb(object_t *obj, char *comment);
	bool  isNear(object_t *obj, char *verb, char *comment);
	bool  isObjectVerb(object_t *obj, char *comment);
	void  takeObject(object_t *obj);
};

class Parser_v1w : public Parser_v3d {
public:
	Parser_v1w(HugoEngine *vm);
	~Parser_v1w();

	void  keyHandler(Common::Event event);
	void  lineHandler();
};

} // End of namespace Hugo

#endif //HUGO_PARSER_H
