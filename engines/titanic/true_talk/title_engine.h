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
#include "common/winexe_pe.h"
#include "titanic/support/string.h"
#include "titanic/true_talk/script_handler.h"
#include "titanic/true_talk/tt_script_base.h"
#include "titanic/true_talk/tt_title_script.h"

namespace Titanic {

class CTitleEngine {
public:
	CScriptHandler *_scriptHandler;
	TTScriptBase *_script;
public:
	CTitleEngine();
	~CTitleEngine();

	/**
	 * Setup the engine
	 */
	virtual void setup(int val1, int val2 = 0);

	virtual int proc2(int val1, int val2) { return 2; }
	
	virtual int proc4(int unused) const = 0;
	virtual int proc5(int64 unused) const = 0;
	virtual int proc6(int64 unused) const = 0;
	virtual int proc7(int64 unused) const = 0;
	virtual int proc8() const = 0;

	/**
	 * Open a designated file
	 */
	virtual void open(const CString &name) = 0;

	/**
	 * Close the file
	 */
	virtual void close() = 0;
};

class STtitleEngine : public CTitleEngine {
private:
	Common::PEResources _resources;
	Common::SeekableReadStream *_stream;
	int _field58;
public:
	Common::Array<uint> _indexes;
	Common::Array<byte> _data;
public:
	STtitleEngine();
	~STtitleEngine();

	void reset();

	/**
	 * Setup the engine
	 */
	virtual void setup(int val1, int val2 = 0);

	virtual int proc2(int val1, int val2);

	virtual void dump(int val1, int val2);

	virtual int proc4(int unused) const { return 0; }
	virtual int proc5(int64 unused) const { return 0; }
	virtual int proc6(int64 unused) const { return 0; }
	virtual int proc7(int64 unused) const { return 0; }
	virtual int proc8() const { return 0; }

	/**
	 * Open a designated file
	 */
	virtual void open(const CString &name);

	/**
	 * Close the file
	 */
	virtual void close();
};

} // End of namespace Titanic

#endif /* TITANIC_TITLE_ENGINE_H */
