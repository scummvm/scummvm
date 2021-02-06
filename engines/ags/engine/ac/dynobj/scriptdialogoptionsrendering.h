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

#ifndef AGS_ENGINE_AC_DYNOBJ_SCRIPTDIALOGOPTIONSRENDERING_H
#define AGS_ENGINE_AC_DYNOBJ_SCRIPTDIALOGOPTIONSRENDERING_H

#include "ags/engine/ac/dynobj/scriptdrawingsurface.h"

namespace AGS3 {

struct ScriptDialogOptionsRendering final : AGSCCDynamicObject {
	int x, y, width, height;
	bool hasAlphaChannel;
	int parserTextboxX, parserTextboxY;
	int parserTextboxWidth;
	int dialogID;
	int activeOptionID;
	int chosenOptionID;
	ScriptDrawingSurface *surfaceToRenderTo;
	bool surfaceAccessed;
	bool needRepaint;

	// return the type name of the object
	const char *GetType() override;

	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	int Serialize(const char *address, char *buffer, int bufsize) override;

	void Unserialize(int index, const char *serializedData, int dataSize) override;

	void Reset();

	ScriptDialogOptionsRendering();
};

} // namespace AGS3

#endif
