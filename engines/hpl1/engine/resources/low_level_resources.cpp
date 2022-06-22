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

namespace hpl {

Bitmap2D *LowLevelResources::LoadBitmap2D(tString filepath, tString type) {
#if 0
  		tString tType;
		if(asType != "")
			asFilePath = cString::SetFileExt(asFilePath,asType);

		tType = cString::GetFileExt(asFilePath);
		SDL_Surface* pSurface = NULL;

		if (tType=="bmp") {
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			SDL_PixelFormat RGBAFormat;
			RGBAFormat.palette = 0; RGBAFormat.colorkey = 0; RGBAFormat.alpha = 0;
			RGBAFormat.BitsPerPixel = 32; RGBAFormat.BytesPerPixel = 4;

			RGBAFormat.Rmask = 0xFF000000; RGBAFormat.Rshift = 0; RGBAFormat.Rloss = 0;
			RGBAFormat.Gmask = 0x00FF0000; RGBAFormat.Gshift = 8; RGBAFormat.Gloss = 0;
			RGBAFormat.Bmask = 0x0000FF00; RGBAFormat.Bshift = 16; RGBAFormat.Bloss = 0;
			RGBAFormat.Amask = 0x000000FF; RGBAFormat.Ashift = 24; RGBAFormat.Aloss = 0;

			SDL_Surface* orig = NULL;
			orig = IMG_Load(asFilePath.c_str());

			if(orig==NULL){
				//Error handling stuff?
				return NULL;
			}
			pSurface = SDL_ConvertSurface(orig, &RGBAFormat, SDL_SWSURFACE);
			SDL_FreeSurface(orig);
#else
			pSurface = IMG_Load(asFilePath.c_str());
#endif
		} else {
			pSurface= IMG_Load(asFilePath.c_str());
		}
		if(pSurface==NULL){
			//Error handling stuff?
			return NULL;
		}

		iBitmap2D* pBmp = mpLowLevelGraphics->CreateBitmap2DFromSurface(pSurface,
													cString::GetFileExt(asFilePath));
		pBmp->SetPath(asFilePath);
		return pBmp;
#endif
	return nullptr;
}

void LowLevelResources::GetSupportedImageFormats(tStringList &formats) {
	formats.insert(formats.end(), {
		"BMP","LBM","PCX","GIF","JPEG","PNG","JPG","TGA","TIFF","TIF"}); 
}

void LowLevelResources::AddMeshLoaders(cMeshLoaderHandler *ml) {
#if 0
	ml->AddLoader(hplNew(cMeshLoaderMSH, (mpLowLevelGraphics)));
	ml->AddLoader(hplNew(cMeshLoaderCollada, (mpLowLevelGraphics)));
#endif
}

void LowLevelResources::AddVideoLoaders(cVideoManager *vm) {
#ifdef INCLUDE_THORA
	apManager->AddVideoLoader(hplNew(cVideoStreamTheora_Loader, ()));
#endif
}

void LowLevelResources::FindFilesInDir(tStringList &alstStrings, tString asDir, tString asMask) {
	//FIXME: use consistent string types
	Common::String pattern = Common::String(asDir.c_str()) + '/' +  Common::String(asMask.c_str()); 
	Common::ArchiveMemberList ls; 
	if (SearchMan.listMatchingMembers(ls, pattern) == 0) 
		debugCN(Hpl1::kDebugLevelWarning, Hpl1::kDebugFilePath, "no files matching pattern %s were found", pattern.c_str());  
	
	for (auto f : ls)
		alstStrings.push_back(f->getName().c_str()); 
}

} // namespace hpl

