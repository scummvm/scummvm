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

#include "common/str.h"
#include "common/path.h"
#include "common/hashmap.h"
#include "graphics/palette.h"

#include "mediastation/datafile.h"
#include "mediastation/asset.h"
#include "mediastation/mediascript/function.h"

namespace MediaStation {

enum ContextParametersSectionType {
	kContextParametersEmptySection = 0x0000,
	kContextParametersVariable = 0x0014,
	kContextParametersName = 0x0bb9,
	kContextParametersFileNumber = 0x0011,
	kContextParametersBytecode = 0x0017
};

enum ContextSectionType {
	kContextEmptySection = 0x0000,
	kContextOldStyleSection = 0x000d,
	kContextParametersSection = 0x000e,
	kContextPaletteSection = 0x05aa,
	kContextUnkAtEndSection = 0x0010,
	kContextAssetHeaderSection = 0x0011,
	kContextPoohSection = 0x057a,
	kContextAssetLinkSection = 0x0013,
	kContextFunctionSection = 0x0031
};

class Screen;

class Context : public Datafile {
public:
	Context(const Common::Path &path);
	~Context();

	uint32 _unk1;
	uint32 _subfileCount;
	uint32 _fileSize;
	Graphics::Palette *_palette = nullptr;
	Screen *_screenAsset = nullptr;

	Asset *getAssetById(uint assetId);
	Asset *getAssetByChunkReference(uint chunkReference);
	Function *getFunctionById(uint functionId);
	ScriptValue *getVariable(uint variableId);

private:
	// This is not an internal file ID, but the number of the file
	// as it appears in the filename. For instance, the context in
	// "100.cxt" would have file number 100.
	uint _fileNumber = 0;
	Common::String _contextName;

	Common::HashMap<uint, Asset *> _assets;
	Common::HashMap<uint, Function *> _functions;
	Common::HashMap<uint, Asset *> _assetsByChunkReference;
	Common::HashMap<uint, ScriptValue *> _variables;

	void readOldStyleHeaderSections(Subfile &subfile, Chunk &chunk);
	void readNewStyleHeaderSections(Subfile &subfile, Chunk &chunk);

	bool readHeaderSection(Chunk &chunk);
	void readCreateContextData(Chunk &chunk);
	Asset *readCreateAssetData(Chunk &chunk);
	void readCreateVariableData(Chunk &chunk);

	void readAssetInFirstSubfile(Chunk &chunk);
	void readAssetFromLaterSubfile(Subfile &subfile);
};

} // End of namespace MediaStation

#endif
