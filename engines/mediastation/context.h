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

#ifndef MEDIASTATION_CONTEXT_H
#define MEDIASTATION_CONTEXT_H

#include "mediastation/datafile.h"
#include "mediastation/contextparameters.h"
#include "mediastation/assetheader.h"
#include "mediastation/mediascript/function.h"

namespace MediaStation {

enum ContextSectionType {
	kContextEmptySection = 0x0000,
	kContextOldStyleSection = 0x000d,
	kContextParametersSection = 0x000e,
	kContextPaletteSection = 0x05aa,
	kContextEndSection = 0x0010,
	kContextAssetHeaderSection = 0x0011,
	kContextPoohSection = 0x057a,
	kContextAssetLinkSection = 0x0013,
	kContextFunctionSection = 0x0031
};

class Context : Datafile {
public:
	Context(const Common::Path &path);
	~Context();

	bool readPreamble();

	uint32 _unk1;
	uint32 _subfileCount;
	uint32 _fileSize;
	Graphics::Palette *_palette = nullptr;
	ContextParameters *_parameters = nullptr;
	AssetHeader *_screenAsset = nullptr;

private:
	void readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk);
	void readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk);
	bool readHeaderSection(Subfile &subfile, Chunk &chunk);

	void readAssetInFirstSubfile(Chunk &chunk);
	void readAssetFromLaterSubfile(Subfile &subfile);
};

} // End of namespace MediaStation

#endif