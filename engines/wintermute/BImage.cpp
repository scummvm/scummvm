/*
This file is part of WME Lite.
http://dead-code.org/redir.php?target=wmelite

Copyright (c) 2011 Jan Nedoma

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include "dcgf.h"
#include "BImage.h"
//#include "FreeImage.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
CBImage::CBImage(CBGame *inGame, FIBITMAP *bitmap): CBBase(inGame) {
#if 0
	m_Bitmap = bitmap;
#endif
	m_Bitmap = NULL;
}


//////////////////////////////////////////////////////////////////////
CBImage::~CBImage() {
#if 0
	if (m_Bitmap) FreeImage_Unload(m_Bitmap);
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::SaveBMPFile(char *Filename) {
#if 0
	if (!m_Bitmap) return E_FAIL;

	if (FreeImage_Save(FIF_BMP, m_Bitmap, Filename)) return S_OK;
	else return E_FAIL;
#endif
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::Resize(int NewWidth, int NewHeight) {
#if 0
	if (!m_Bitmap) return E_FAIL;

	if (NewWidth == 0) NewWidth = FreeImage_GetWidth(m_Bitmap);
	if (NewHeight == 0) NewHeight = FreeImage_GetHeight(m_Bitmap);


	FIBITMAP *newImg = FreeImage_Rescale(m_Bitmap, NewWidth, NewHeight, FILTER_BILINEAR);
	if (newImg) {
		FreeImage_Unload(m_Bitmap);
		m_Bitmap = newImg;
		return S_OK;
	} else return E_FAIL;
#endif
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
byte *CBImage::CreateBMPBuffer(uint32 *BufferSize) {
	if (!m_Bitmap) return NULL;
#if 0
	FIMEMORY *fiMem = FreeImage_OpenMemory();
	FreeImage_SaveToMemory(FIF_PNG, m_Bitmap, fiMem);
	uint32 size;
	byte *data;
	FreeImage_AcquireMemory(fiMem, &data, &size);


	byte *Buffer = new byte[size];
	memcpy(Buffer, data, size);

	FreeImage_CloseMemory(fiMem);

	if (BufferSize) *BufferSize = size;

	return Buffer;
#endif
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBImage::CopyFrom(CBImage *OrigImage, int NewWidth, int NewHeight) {
#if 0
	if (m_Bitmap) FreeImage_Unload(m_Bitmap);

	if (NewWidth == 0) NewWidth = FreeImage_GetWidth(OrigImage->GetBitmap());
	if (NewHeight == 0) NewHeight = FreeImage_GetHeight(OrigImage->GetBitmap());

	m_Bitmap = FreeImage_Rescale(OrigImage->GetBitmap(), NewWidth, NewHeight, FILTER_BILINEAR);
#endif
	return S_OK;
}

} // end of namespace WinterMute
