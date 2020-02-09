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

#ifndef TITANIC_TITLE_ENGINE_H
#define TITANIC_TITLE_ENGINE_H

#include "common/stream.h"
#include "titanic/support/string.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_response.h"
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/true_talk/tt_title_script.h"

namespace Titanic {

class CTitleEngine;

class CTitleStream : public SimpleFile {
public:
	CTitleStream() : SimpleFile() {}
};

class CTitleEngine {
public:
	CScriptHandler *_scriptHandler;
	TTscriptBase *_script;
public:
	CTitleEngine();
	virtual ~CTitleEngine();

	/**
	 * Setup the engine
	 */
	virtual void setup(int val1, VocabMode vocabMode = VOCAB_MODE_NONE);

	/**
	 * Sets a conversation reponse
	 */
	virtual int setResponse(TTscriptBase *script, TTresponse *response) { return SS_4; }

	/**
	 * Open a designated file
	 */
	virtual SimpleFile *open(const CString &name) = 0;
};

class STtitleEngine : public CTitleEngine {
private:
	Common::SeekableReadStream *_stream;
	const TTresponse *_responseP;
public:
	Common::Array<uint> _indexes;
	Common::Array<byte> _data;
public:
	STtitleEngine();
	~STtitleEngine() override;

	void reset();

	/**
	 * Setup the engine
	 */
	void setup(int val1, VocabMode vocabMode = VOCAB_MODE_NONE) override;

	/**
	 * Sets a conversation reponse
	 */
	int setResponse(TTscriptBase *script, TTresponse *response) override;

	/**
	 * Open a designated file
	 */
	SimpleFile *open(const CString &name) override;
};

} // End of namespace Titanic

#endif /* TITANIC_TITLE_ENGINE_H */
