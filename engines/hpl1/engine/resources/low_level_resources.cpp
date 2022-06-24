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

#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/resources/MeshLoaderHandler.h"
#include "hpl1/engine/resources/VideoManager.h"
#include "common/fs.h"
#include "hpl1/debug.h"
#include "common/file.h"

namespace hpl {

static tString getImageType(const tString &filepath) {
	//FIXME: use proper string types
	Common::String str(filepath.c_str()); 
	str.toLowercase(); 
	size_t pos = str.findLastOf(".");
	if (pos != Common::String::npos)
		return str.substr(pos + 1).c_str();
	return "";
}

Bitmap2D *LowLevelResources::loadBitmap2D(const tString &filepath) {
	return new Bitmap2D(filepath, getImageType(filepath));
}

void LowLevelResources::getSupportedImageFormats(tStringList &formats) {
	formats.insert(formats.end(), {
		"BMP","GIF","JPEG","PNG","JPG","TGA"}); 
}

void LowLevelResources::addMeshLoaders(cMeshLoaderHandler *ml) {
#if 0
	ml->AddLoader(hplNew(cMeshLoaderMSH, (mpLowLevelGraphics)));
	ml->AddLoader(hplNew(cMeshLoaderCollada, (mpLowLevelGraphics)));
#endif
}

void LowLevelResources::addVideoLoaders(cVideoManager *vm) {
#ifdef INCLUDE_THORA
	apManager->AddVideoLoader(hplNew(cVideoStreamTheora_Loader, ()));
#endif
}

void LowLevelResources::findFilesInDir(tStringList &alstStrings, tString asDir, tString asMask) {
	//FIXME: use consistent string types
	Common::String pattern = Common::String(asDir.c_str()) + '/' +  Common::String(asMask.c_str()); 
	Common::ArchiveMemberList ls; 
	if (SearchMan.listMatchingMembers(ls, pattern) == 0) 
		debugCN(Hpl1::kDebugLevelWarning, Hpl1::kDebugFilePath, "no files matching pattern %s were found", pattern.c_str());  
	
	for (auto f : ls)
		alstStrings.push_back(f->getName().c_str()); 
}

} // namespace hpl

