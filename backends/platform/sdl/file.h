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

#ifndef BACKENDS_SDL_SUBSYS_FILE_H
#define BACKENDS_SDL_SUBSYS_FILE_H

#include "backends/base-subsys-file.h"

#if defined(__SYMBIAN32__)
#include <esdl\SDL.h>
#else
#include <SDL.h>
#endif

class SdlSubSys_File : public BaseSubSys_File {
public:
	SdlSubSys_File();
	~SdlSubSys_File();

	virtual void fileInit(OSystem *mainSys);
	virtual void fileDone();

	bool hasFeature(Feature f);
	void setFeatureState(Feature f, bool enable);
	bool getFeatureState(Feature f);

	virtual Common::SeekableReadStream *createConfigReadStream();
	virtual Common::WriteStream *createConfigWriteStream();

	virtual Common::SaveFileManager *getSavefileManager();
	virtual FilesystemFactory *getFilesystemFactory();
	virtual void addSysArchivesToSearchSet(Common::SearchSet &s, int priority = 0);

protected:
	bool _inited;

	FilesystemFactory *_fsFactory;
	Common::SaveFileManager *_savefile;

private:
	OSystem *_mainSys;
};


#endif
