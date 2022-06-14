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

/*
 * Copyright (C) 2006-2010 - Frictional Games
 *
 * This file is part of HPL1 Engine.
 */

#include "hpl1/engine/impl/SDLFontData.h"
#include "hpl1/engine/graphics/GraphicsDrawer.h"
#include "hpl1/engine/impl/SDLBitmap2D.h"
#include "hpl1/engine/impl/tinyXML/tinyxml.h"
#include "hpl1/engine/system/LowLevelSystem.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cSDLFontData::cSDLFontData(const tString &asName, iLowLevelGraphics *apLowLevelGraphics)
	: iFontData(asName, apLowLevelGraphics) {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cSDLFontData::CreateFromBitmapFile(const tString &asFileName) {
#if 0
  tString sPath = cString::GetFilePath(asFileName);

		////////////////////////////////////////////
		// Load xml file
		TiXmlDocument *pXmlDoc = hplNew( TiXmlDocument,(asFileName.c_str()) );

		if(pXmlDoc->LoadFile()==false)
		{
			Error("Couldn't load angle code font file '%s'\n",asFileName.c_str());
			hplDelete(pXmlDoc);
			return false;
		}

		TiXmlElement *pRootElem = pXmlDoc->RootElement();

		////////////////////////////////////////////
		// Load Common info
		TiXmlElement *pCommonElem = pRootElem->FirstChildElement("common");

		int lLineHeight = cString::ToInt(pCommonElem->Attribute("lineHeight"),0);
		int lBase = cString::ToInt(pCommonElem->Attribute("base"),0);

		mlFirstChar =0;
		mlLastChar = 3000; //Get this num from something.

		mfHeight = (float)lLineHeight;

		mvGlyphs.resize(3000, NULL);

		mvSizeRatio.x = (float)lBase / (float)lLineHeight;
		mvSizeRatio.y = 1;

		////////////////////////////////////////////
		// Load bitmaps
		std::vector<cSDLBitmap2D *> vBitmaps;

		TiXmlElement *pPagesRootElem = pRootElem->FirstChildElement("pages");

		TiXmlElement *pPageElem = pPagesRootElem->FirstChildElement("page");
		for(; pPageElem != NULL; pPageElem = pPageElem->NextSiblingElement("page"))
		{
			tString sFileName = pPageElem->Attribute("file");
			tString sFilePath = cString::SetFilePath(sFileName,sPath);

			cSDLBitmap2D *pBitmap = static_cast<cSDLBitmap2D*>(mpLowLevelResources->LoadBitmap2D(sFilePath));
			if(pBitmap==NULL)
			{
				Error("Couldn't load bitmap %s for FNT file '%s'\n",sFilePath.c_str(),asFileName.c_str());
				hplDelete(pXmlDoc);
				return false;
			}

			vBitmaps.push_back(pBitmap);
		}

		////////////////////////////////////////////
		// Load glyphs
		TiXmlElement *pCharsRootElem = pRootElem->FirstChildElement("chars");

		TiXmlElement *pCharElem = pCharsRootElem->FirstChildElement("char");
		for(; pCharElem != NULL; pCharElem = pCharElem->NextSiblingElement("char"))
		{
			//Get the info on the character
			int lId = cString::ToInt(pCharElem->Attribute("id"),0);
			int lX = cString::ToInt(pCharElem->Attribute("x"),0);
			int lY = cString::ToInt(pCharElem->Attribute("y"),0);

			int lW = cString::ToInt(pCharElem->Attribute("width"),0);
			int lH = cString::ToInt(pCharElem->Attribute("height"),0);

			int lXOffset = cString::ToInt(pCharElem->Attribute("xoffset"),0);
			int lYOffset = cString::ToInt(pCharElem->Attribute("yoffset"),0);

			int lAdvance = cString::ToInt(pCharElem->Attribute("xadvance"),0);

			int lPage = cString::ToInt(pCharElem->Attribute("page"),0);

			//Get the bitmap where the character graphics is
			cSDLBitmap2D *pSourceBitmap = vBitmaps[lPage];

			//Create a bitmap for the character.
			cVector2l vSize(lW, lH);
			cSDLBitmap2D *pBmp = static_cast<cSDLBitmap2D*>(mpLowLevelGraphics->CreateBitmap2D(vSize,32));

			//Copy from source to character bitmap
			SDL_Rect srcRect;
			srcRect.x = lX; srcRect.y = lY;
			srcRect.w = lW; srcRect.h = lH;
			SDL_BlitSurface(pSourceBitmap->GetSurface(),&srcRect, pBmp->GetSurface(), NULL);

			int lBmpSize = pBmp->GetSurface()->format->BytesPerPixel;
			unsigned char* PixBuffer = (unsigned char*)pBmp->GetSurface()->pixels;

			//Set proper alpha (dunno if this is needed)
			for(unsigned int y=0;y<pBmp->GetHeight();y++)
				for(unsigned int x=0;x<pBmp->GetWidth();x++) {
					unsigned char* Pix = &PixBuffer[y*pBmp->GetWidth()*lBmpSize + x*lBmpSize];
					Pix[3] = Pix[0];
				}

			//Create glyph and place it correctly.
			cGlyph *pGlyph = CreateGlyph(pBmp,cVector2l(lXOffset,lYOffset),cVector2l(lW,lH),
										cVector2l(lBase,lLineHeight),lAdvance);

			mvGlyphs[lId] = pGlyph;

			hplDelete(pBmp);
		}



		//Destroy bitmaps
		STLDeleteAll(vBitmaps);

		//Destroy XML
		hplDelete(pXmlDoc);
		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

bool cSDLFontData::CreateFromFontFile(const tString &asFileName, int alSize, unsigned short alFirstChar,
									  unsigned short alLastChar) {
#if 0
  		cGlyph* pGlyph=NULL;

		mlFirstChar = alFirstChar;
		mlLastChar = alLastChar;

		TTF_Font* pFont = TTF_OpenFont(asFileName.c_str(), alSize);
		if(pFont==NULL){
			Error("Error when opening '%s': %s\n",asFileName.c_str(),TTF_GetError());
			return false;
		}

		//Create bitmaps from all of the characters and create
		//Images from those.
		for(int i=alFirstChar; i<=alLastChar;i++)
		{
			unsigned short lUniCode = i;
			/*char c = (char)i;

			if(c == '�')lUniCode = 'o';
			else if(c == '�')lUniCode = 'O';*/

			pGlyph = RenderGlyph(pFont, lUniCode, alSize);
			AddGlyph(pGlyph);
		}

		//Get the properties
		mfHeight = (float)TTF_FontHeight(pFont);

		mvSizeRatio = 1;

		//Cleanup
		TTF_CloseFont(pFont);

		return true;
#endif
	return false;
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PRIVATE METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGlyph *cSDLFontData::RenderGlyph(TTF_Font *apFont, unsigned short aChar, int alFontSize) {
#if 0
  		//If the font is saved to disk, then load the bitmap from disk instead.

		cVector2l vMin;
		cVector2l vMax;
		int lAdvance=0;

		TTF_GlyphMetrics(apFont, aChar, &vMin.x, &vMax.x, &vMin.y, &vMax.y, &lAdvance);

		//Create the bitmap we want to draw upon
		cVector2l vSize = vMax - vMin;
		cSDLBitmap2D *pBmp = static_cast<cSDLBitmap2D*>(mpLowLevelGraphics->CreateBitmap2D(vSize,32));

		pBmp->FillRect(cRect2l(),cColor(0,1));

		//create a surface with the glyph
		SDL_Color Col;Col.r=255;Col.g=255;Col.b=255;
		SDL_Surface* pGlyphSurface = TTF_RenderGlyph_Blended(apFont,aChar,Col);

		//Blit the surface using blending. This way it should create a nice
		//b&w image where the font is white.
		SDL_SetAlpha(pBmp->GetSurface(),0,0);
		SDL_SetAlpha(pGlyphSurface,SDL_SRCALPHA,0);
		SDL_BlitSurface(pGlyphSurface, NULL, pBmp->GetSurface(),NULL);
		SDL_SetAlpha(pBmp->GetSurface(),0,0);

		//Set the alpha of the bitmap to the average color.
		//So we get some alpha bledning.
		int lBmpSize = pBmp->GetSurface()->format->BytesPerPixel;
		unsigned char* PixBuffer = (unsigned char*)pBmp->GetSurface()->pixels;

		for(unsigned int y=0;y<pBmp->GetHeight();y++)
			for(unsigned int x=0;x<pBmp->GetWidth();x++)
			{
				unsigned char* Pix = &PixBuffer[y*pBmp->GetWidth()*lBmpSize +
					x*lBmpSize];

				Pix[3] = Pix[0];
			}

		//Create the Glyph
		int lHeight = TTF_FontHeight(apFont);
		cVector2l vOffset = cVector2l(vMin.x, alFontSize - vMax.y);//(lHeight - vSize.y) - vMin.y);

		cGlyph* pGlyph = CreateGlyph(pBmp,vOffset,vSize,alFontSize,lAdvance);

		hplDelete(pBmp);
		SDL_FreeSurface(pGlyphSurface);

		return pGlyph;
#endif
	return {};
}

//-----------------------------------------------------------------------

} // namespace hpl
