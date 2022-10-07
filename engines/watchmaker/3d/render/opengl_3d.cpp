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

#define FORBIDDEN_SYMBOL_EXCEPTION_strcat
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy

#include "watchmaker/3d/render/opengl_3d.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/t3d.h"
#include "watchmaker/3d/math/Matrix4x4.h"
#include "watchmaker/3d/render/opengl_renderer.h"
#include "watchmaker/ll/ll_system.h"
#include "watchmaker/3d/render/opengl_2d.h"
#include "watchmaker/renderer.h"
#include "watchmaker/globvar.h"
#include "watchmaker/work_dirs.h"

#ifdef USE_OPENGL_GAME

#include "graphics/opengl/system_headers.h"
#include "watchmaker/tga_util.h"

#define MAXTEXTURES     2000

namespace Watchmaker {

// temp pixel format conversion info
struct {
	float RedScale;
	float GreenScale;
	float BlueScale;
	float AlphaScale;
	unsigned int RedShift;
	unsigned int GreenShift;
	unsigned int BlueShift;
	unsigned int AlphaShift;
	unsigned int RedMask;
	unsigned int GreenMask;
	unsigned int BlueMask;
	unsigned int AlphaMask;
} gTexturePixelConversion;

#define T3D_FASTFILE            (1<<23)     // fastfile

// Tecture formats
int             NumAvailableFormats = 0;
gAvailFormat    AvailableFormats[50];
gAvailFormat    *gCurrentFormat;

// Point VB
int g_lpD3DPointsBuffer;
unsigned int            gNumPointsBuffer;

// Tecture list
gTexture        gTextureList[MAXTEXTURES];
unsigned int    gNumTextureList = 0;

// {ines array
uint16          gLinesArray[MAX_LINES];
unsigned int    gNumLinesArray = 0;

// screen traingles
Vertex      gTriangles[100];
unsigned int    gNumTrianglesArray = 0;

// camera info
Matrix4x4       rWorldMatrix;
Matrix4x4       rProjectionMatrix;
float           gNearPlane, gFarPlane;

// saved matrix
Matrix4x4       rLinesViewMatrix;

// user matrix
#define         MAX_USER_VIEW_MATRICES      2000 // TODO: Why do we get so many of them?
Matrix4x4       rUserViewMatrix[MAX_USER_VIEW_MATRICES];
unsigned int    rNumUserViewMatrices = 0;

// user vertext buffer
void    *g_lpD3DUserVertexBuffer = nullptr;
unsigned int    g_dwD3DUserVertexBufferCounter = 0;

//***********************************************************************************************
uint16 *rGetLinesArrayPtr() {
	return &gLinesArray[gNumLinesArray];
}

//***********************************************************************************************
void rAddLinesArray() {
	gNumLinesArray++;
}

//***********************************************************************************************
void *rLockPointArray() {
	warning("TODO: Implement rLockPointArray");
#if 0
	HRESULT hResult;
	LPVOID v;

	if ((hResult = g_lpD3DPointsBuffer->Lock(DDLOCK_SURFACEMEMORYPTR, &v, NULL)) != D3D_OK) {
		char str[255];
		GetDDErrorString(hResult, str, 1);
		DebugLogFile("rLockPointArray: Unable to lock points vertexbuffer:\r\n%s", str);
		return NULL;
	}

	gVertex *gv = (gVertex *)v;
	return &gv[rGetNumPointArray()];
#endif
	return nullptr;
}

void rAddTrianglesArray(float x, float y, int r, int g, int b, int a) {
	gTriangles[gNumTrianglesArray].sx = x;
	gTriangles[gNumTrianglesArray].sy = y;
	gTriangles[gNumTrianglesArray].sz = 1.0f;
	gTriangles[gNumTrianglesArray].color = RGBA_MAKE(r, g, b, a);
	gNumTrianglesArray++;
}


//***********************************************************************************************
void rUnlockPointArray() {
	warning("TODO: Implement rUnlockPointArray");
#if 0
	g_lpD3DPointsBuffer->Unlock();
#endif
}

//***********************************************************************************************
unsigned int rGetNumPointArray() {
	return gNumPointsBuffer;
}

//***********************************************************************************************
void rAddPointArray() {
	gNumPointsBuffer++;
}

//***********************************************************************************************
bool rSetViewMatrix(float _00, float _01, float _02,
                    float _03, float _04, float _05,
                    float _06, float _07, float _08,
                    float _tx, float _ty, float _tz) {
	Matrix4x4       rViewMatrix;
	rViewMatrix.setIdentity();

	rViewMatrix.setValue(1, 1, _00);
	rViewMatrix.setValue(1, 2, _01);
	rViewMatrix.setValue(1, 3, _02);
	rViewMatrix.setValue(1, 4, _tx);

	rViewMatrix.setValue(2, 1, _03);
	rViewMatrix.setValue(2, 2, _04);
	rViewMatrix.setValue(2, 3, _05);
	rViewMatrix.setValue(2, 4, _ty);

	rViewMatrix.setValue(3, 1, -_06);
	rViewMatrix.setValue(3, 2, -_07);
	rViewMatrix.setValue(3, 3, -_08);
	rViewMatrix.setValue(3, 4, _tz);

	rViewMatrix.setValue(4, 1, 0.0f);
	rViewMatrix.setValue(4, 2, 0.0f);
	rViewMatrix.setValue(4, 3, 0.0f);
	rViewMatrix.setValue(4, 4, 1.0f);

	g_renderer->setTransformMatrix(TransformMatrix::VIEW, rViewMatrix);

	return true;
}

void rSetViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation) {
	rSetViewMatrix(viewMatrix.M[0], viewMatrix.M[1], viewMatrix.M[2],
				   viewMatrix.M[3], viewMatrix.M[4], viewMatrix.M[5],
				   viewMatrix.M[6], viewMatrix.M[7], viewMatrix.M[8],
				   translation.x, translation.y, -translation.z);
}

void rSaveViewMatrix() {
	g_renderer->pushModelView();
}

//***********************************************************************************************
void rRestoreViewMatrix() {
	g_renderer->popModelView();
}

//***********************************************************************************************
bool rBuildLinesViewMatrix(float _00, float _01, float _02,
                           float _03, float _04, float _05,
                           float _06, float _07, float _08,
                           float _tx, float _ty, float _tz) {
	rLinesViewMatrix.setIdentity();

	rLinesViewMatrix.setValue(1, 1, _00);
	rLinesViewMatrix.setValue(1, 2, _01);
	rLinesViewMatrix.setValue(1, 3, _02);
	rLinesViewMatrix.setValue(1, 4, _tx);

	rLinesViewMatrix.setValue(2, 1, _03);
	rLinesViewMatrix.setValue(2, 2, _04);
	rLinesViewMatrix.setValue(2, 3, _05);
	rLinesViewMatrix.setValue(2, 4, _ty);

	rLinesViewMatrix.setValue(3, 1, -_06);
	rLinesViewMatrix.setValue(3, 2, -_07);
	rLinesViewMatrix.setValue(3, 3, -_08);
	rLinesViewMatrix.setValue(3, 4, -_tz);

	rLinesViewMatrix.setValue(4, 1, 0.0f);
	rLinesViewMatrix.setValue(4, 2, 0.0f);
	rLinesViewMatrix.setValue(4, 3, 0.0f);
	rLinesViewMatrix.setValue(4, 4, 1.0f);

	return true;
}

int rBuildLinesViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation) {
	return rBuildLinesViewMatrix(viewMatrix.M[0], viewMatrix.M[1], viewMatrix.M[2],
								 viewMatrix.M[3], viewMatrix.M[4], viewMatrix.M[5],
								 viewMatrix.M[6], viewMatrix.M[7], viewMatrix.M[8],
								 translation.x, translation.y, translation.z);
}

//***********************************************************************************************
int rAddUserViewMatrix(float _00, float _01, float _02,
                       float _03, float _04, float _05,
                       float _06, float _07, float _08,
                       float _tx, float _ty, float _tz) {
	Matrix4x4       rTempViewMatrix;
	Matrix4x4       *um;
	unsigned int    i;

	rTempViewMatrix.setIdentity();

	rTempViewMatrix.setValue(1, 1, _00);
	rTempViewMatrix.setValue(1, 2, _01);
	rTempViewMatrix.setValue(1, 3, _02);
	rTempViewMatrix.setValue(1, 4, _tx);

	rTempViewMatrix.setValue(2, 1, _03);
	rTempViewMatrix.setValue(2, 2, _04);
	rTempViewMatrix.setValue(2, 3, _05);
	rTempViewMatrix.setValue(2, 4, _ty);

	rTempViewMatrix.setValue(3, 1, -_06);
	rTempViewMatrix.setValue(3, 2, -_07);
	rTempViewMatrix.setValue(3, 3, -_08);
	rTempViewMatrix.setValue(3, 4, -_tz);

	rTempViewMatrix.setValue(4, 1, 0.0f);
	rTempViewMatrix.setValue(4, 2, 0.0f);
	rTempViewMatrix.setValue(4, 3, 0.0f);
	rTempViewMatrix.setValue(4, 4, 1.0f);

	auto &tmp = rTempViewMatrix;
	//warning("Adding: ");
	//tmp.print();
	for (i = 0, um = &rUserViewMatrix[0]; i < rNumUserViewMatrices; i++, um++) {
		//warning("Comparing %d", i);
		//um->print();
		if (*um == tmp) {
			return i;
		}
	}

	if (i >= MAX_USER_VIEW_MATRICES) {
		DebugLogFile("Too many UserViewMatrix %d (MAX is %d)\n", i, MAX_USER_VIEW_MATRICES);
		return -1;
	}

	*um = tmp;

	rNumUserViewMatrices ++;
	return rNumUserViewMatrices - 1;
}

int rAddUserViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation) {
	return rAddUserViewMatrix(viewMatrix.M[0], viewMatrix.M[1], viewMatrix.M[2],
							  viewMatrix.M[3], viewMatrix.M[4], viewMatrix.M[5],
							  viewMatrix.M[6], viewMatrix.M[7], viewMatrix.M[8],
							  translation.x, translation.y, translation.z);
}

//*********************************************************************************************
void *rGetUserVertexBuffer() {
	return g_lpD3DUserVertexBuffer;
}

//*********************************************************************************************
unsigned int rGetUserVertexBufferCounter() {
	return g_dwD3DUserVertexBufferCounter;
}

//*********************************************************************************************
void rSetUserVertexBufferCounter(unsigned int uvbc) {
	g_dwD3DUserVertexBufferCounter = uvbc;
}

gVertex *rLockVertexPtr(void *vb, int flags) {
	warning("TODO: Implement rLockVertexPtr");
#if 0
	LPVOID v;
	DWORD   dim, lock_flags;
	HRESULT hResult;

	LPDIRECT3DVERTEXBUFFER7 lpD3DVertexBuffer1 = (LPDIRECT3DVERTEXBUFFER7)vb;

	lock_flags = DDLOCK_SURFACEMEMORYPTR;
	if (flags & rVBLOCK_READONLY)
		lock_flags |= DDLOCK_READONLY;
	else if (flags & rVBLOCK_WRITEONLY)
		lock_flags |= DDLOCK_WRITEONLY;
	if (flags & rVBLOCK_NOSYSLOCK)
		lock_flags |= DDLOCK_NOSYSLOCK;

	if ((hResult = lpD3DVertexBuffer1->Lock(lock_flags, &v, &dim)) != D3D_OK) {
		char str[255];
		GetDDErrorString(hResult, str, 1);
		DebugLogFile("Unable to lock vertexbuffer:\r\n%s", str);
		return NULL;
	}
	return (gVertex *)v;
#endif
	return nullptr;
}

Graphics::Surface *gCreateSurface(int width, int height, void *ptr) {
	auto surface = new Graphics::Surface();
	surface->w = width;
	surface->h = height;
	surface->pitch = width * 4; // TODO
	surface->setPixels(ptr);
	return surface;
}

//***********************************************************************************************
bool rUnlockVertexPtr(void *vb) {
	warning("Implement rUnlockVertexPtr");
#if 0
	LPDIRECT3DVERTEXBUFFER7 lpD3DVertexBuffer1 = (LPDIRECT3DVERTEXBUFFER7)vb;

	if (lpD3DVertexBuffer1->Unlock() != D3D_OK) {
		DebugLogFile("VertexBuffer Unlock error");
		return FALSE;
	}
#endif
	return TRUE;
}


//***********************************************************************************************
void rSetLinesViewMatrix() {
	g_renderer->setTransformMatrix(TransformMatrix::VIEW, rLinesViewMatrix);
}

//***********************************************************************************************
void rSetUserViewMatrix(int num) {
	auto &matrix = rUserViewMatrix[num];

	g_renderer->setTransformMatrix(TransformMatrix::VIEW, matrix);
}


//*********************************************************************************************
unsigned int gGetTextureListPosition() {
	unsigned int pos = 1;

	while (!gTextureList[pos].isEmpty()) { // TODO: Do we need the surface?
		pos++;
	}

	if (pos > MAXTEXTURES)
		return 0;

	if (pos > gNumTextureList)
		gNumTextureList = pos;

	return pos;
}

//*********************************************************************************************
void gBuildAlternateName(char *AltName, const char *Name) {
	int len, i, j;

	if (!Name || !AltName) return;

	len = strlen(Name);
	memset(AltName, 0, len + 4);
	memcpy(AltName, Name, len);

	for (i = len - 1; i >= 0; i--) {
		if ((AltName[i] == '.') && (i < (len - 3))) {
			AltName[i + 1] = 'd';
			AltName[i + 2] = 'd';
			AltName[i + 3] = 's';
		}
		if (AltName[i] == '\\' || AltName[i] == '/') {
			for (j = len; j >= i; j--)
				AltName[j + 3] = AltName[j];

			AltName[i + 0] = 'D';
			AltName[i + 1] = 'D';
			AltName[i + 2] = 'S';
			break;
		}
	}

	//warning("Build alternate name %s -> %s", Name, AltName);
}

//*********************************************************************************************
void gBuildAlternateName(char *AltName, char *Name) {
	int len, i, j;

	if (!Name || !AltName) return;

	len = strlen(Name);
	memset(AltName, 0, len + 4);
	memcpy(AltName, Name, len);

	for (i = len - 1; i >= 0; i--) {
		if ((AltName[i] == '.') && (i < (len - 3))) {
			AltName[i + 1] = 'd';
			AltName[i + 2] = 'd';
			AltName[i + 3] = 's';
		}
		if (AltName[i] == '\\') {
			for (j = len; j >= i; j--)
				AltName[j + 3] = AltName[j];

			AltName[i + 0] = 'D';
			AltName[i + 1] = 'D';
			AltName[i + 2] = 'S';
			break;
		}
	}
}

//*********************************************************************************************
gTexture *gUserTexture(unsigned int dimx, unsigned int dimy) {
	bool        AlreadyLoaded = FALSE, bAlpha = FALSE;
	gTexture    *Texture;
	int         pos;
	//DDSURFACEDESC2    DDSurfDesc;

	pos = gGetTextureListPosition();
	if (pos == 0) {
		DebugLogFile("gUserTexture: Can't create more textures");
		return nullptr;
	}
	Texture = &gTextureList[pos];
	*Texture = gTexture();
	Texture->Flags = CurLoaderFlags;

	{
#if 0
		memset(&DDSurfDesc, 0, sizeof(DDSURFACEDESC2));
		memcpy(&DDSurfDesc.ddpfPixelFormat, &gCurrentFormat->SurfaceDesc, sizeof(DDPIXELFORMAT));
		DDSurfDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		if (gRenderFlags & gAGPSUPPORTED) {
			//Alloc texture in AGP
			DDSurfDesc.ddsCaps.dwCaps =  DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY |
			                             DDSCAPS_TEXTURE;
		} else {
			//No AGP support; alloc in sysmem
			DDSurfDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
			DDSurfDesc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
#endif
		if (dimx > 8)
			if (dimx > 16)
				if (dimx > 32)
					if (dimx > 64)
						if (dimx > 128)
							dimx = 256;
						else
							dimx = 128;
					else
						dimx = 64;
				else
					dimx = 32;
			else
				dimx = 16;
		if (dimy > 8)
			if (dimy > 16)
				if (dimy > 32)
					if (dimy > 64)
						if (dimy > 128)
							dimy = 256;
						else
							dimy = 128;
					else
						dimy = 64;
				else
					dimy = 32;
			else
				dimy = 16;

		Texture->surface = gCreateSurface(dimx, dimy, nullptr);
#if 0
		DDSurfDesc.dwWidth = dimx;
		DDSurfDesc.dwHeight = dimy;

		if (!(Texture->lpDDSurface = gCreateSurface(&DDSurfDesc, Texture->lpDDSurface))) {
			DebugLogFile("gCreateSurface FAILED: Can't create surface");
			return NULL;
		}
		strcpy(Texture->Name, "UserTexture");
		gClear(Texture->lpDDSurface, 0, 0, dimx, dimy, 0, 0, 0);
#endif
		Texture->ID = pos;

		Texture->DimX = dimx;
		Texture->DimY = dimy;
#if 0
		DDCOLORKEY  ddck;
		ddck.dwColorSpaceLowValue = 0;
		ddck.dwColorSpaceHighValue = 0;
		Texture->lpDDSurface->SetColorKey(DDCKEY_SRCBLT, &ddck);
#endif
	}

	return Texture;
}

int createTextureFromData(int width, int height, int dataSize, void *data, int texFormat) {
	unsigned int texId = 0;
	glGenTextures(1, &texId);

	glBindTexture(GL_TEXTURE_2D, texId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

	bool compressed = false;
	// TODO: Check both compiletime and runtime for the existence of EXT_texture_compression_s3tc

	switch (texFormat) {
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
		compressed = true;
		break;
	case GL_RGBA:
	case GL_RGB:
		compressed = false;
		break;
	default:
		warning("Texture format not handled: %d", texFormat);
	}

	if (compressed) {
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, dataSize, data);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, texFormat, width, height, 0, dataSize, GL_UNSIGNED_BYTE, data);
	}

	return texId;
}

struct DDSHeader {
	int height = 0;
	int width = 0;
};

DDSHeader parseDDSHeader(Common::SeekableReadStream &stream) {
	DDSHeader header;
	//warning("TODO: Implement DDS Header parsing");
	uint32 retv = ' SDD'; //MAKEFOURCC( 'D','D','S',' ' );
	uint32 magic = stream.readUint32LE();
	if (magic != retv) {
		error("parseDDSHeader: Wrong Magic, expected %08X, got %08X\n", retv, magic);
	}
	uint32 size = stream.readUint32LE();
	uint32 flags = stream.readUint32LE();
	header.height = stream.readUint32LE();
	header.width = stream.readUint32LE();
	stream.seek(size + 4, SEEK_SET);
	return header;
}

struct DDSurface {
	int width;
	int height;
	int dataSize;
	void *data;
};

DDSurface *parseDDS(Common::SeekableReadStream &stream) {
	DDSHeader header = parseDDSHeader(stream);
	auto dataSize = stream.size() - stream.pos();
	auto data = new unsigned char[dataSize]();
	stream.read(data, dataSize);
	DDSurface *result = new DDSurface;
	result->width = header.width;
	result->height = header.height;
	result->data = data;
	result->dataSize = dataSize;
	return result;
}

int createTextureFromSurface(Graphics::Surface &surface, int texFormat) {
	return createTextureFromData(surface.w, surface.h, surface.pitch * surface.h, surface.getPixels(), texFormat);
}

int createCompressedTextureFromSurface(DDSurface &surface) {
	return createTextureFromData(surface.width, surface.height, surface.dataSize, surface.data, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
}

//*********************************************************************************************
Common::SharedPtr<gMovie> gLoadMovie(WorkDirs &workDirs, const char *TextName) {
	bool        AlreadyLoaded = FALSE, bAlpha = FALSE;
	uint32 i;
	char finalName[MAX_PATH];

	auto Movie = Common::SharedPtr<gMovie>(new gMovie());
	if (!Movie) {
		DebugLogFile("gLoadMovie FAILED: Can't alloc Movie struct");
		return nullptr;
	}
	*Movie = gMovie();

	//convert .avi name in .wmm
	strcpy(finalName, TextName);
	{
		int i = strlen(finalName) - 1;
		while (i > 0) {
			if (finalName[i] == '.') {
				finalName[i] = '\0';
				break;
			}
			i--;
		}
		strcat(finalName, ".wmm");
	}


	//load movie file
	Movie->stream = workDirs.resolveFile(finalName);
//	Movie->fp=fopen("c:\\wm\\TMaps2DDS\\fiammata.wmm","rb");
//	Movie->fp=fopen("c:\\wm\\TMaps2DDS\\medicalshow.wmm","rb");
	if (!Movie->stream) {
		DebugLogFile("gLoadMovie FAILED: Can't find movie file\n");
		return nullptr;
	}

	Movie->numFrames = Movie->stream->readUint16LE();
	Movie->width = Movie->stream->readUint16LE();
	Movie->height = Movie->stream->readUint16LE();
	Movie->keyFrame = Movie->stream->readByte();
	Movie->frameRate = Movie->stream->readByte();
	//uint32 readMagic = Movie->stream->readUint32LE();

	DDSHeader header = parseDDSHeader(*Movie->stream);
	Movie->numBlocks = Movie->width * Movie->height / 16;
	Movie->curFrame = 0xFFFF;

	Movie->frameOffsets = (uint32 *)t3dMalloc(sizeof(uint32) * Movie->numFrames);
	if (!Movie->frameOffsets) {
		DebugLogFile("gLoadMovie FAILED: Can't alloc Movie->frameOffsets struct");
		return nullptr;
	}

	uint16 maxlen = (Movie->numBlocks / 8) + 8 * Movie->numBlocks; //bit array + max different blocks
	Movie->buffer = (uint8 *)t3dMalloc(maxlen);
	if (!Movie->buffer) {
		DebugLogFile("gLoadMovie FAILED: Can't alloc Movie->buffer struct");
		return nullptr;
	}

	//read frame offsets
	for (i = 0; i < Movie->numFrames; i++) {
		Movie->frameOffsets[i] = Movie->stream->readUint32LE();
	}

	//check if the files are ok

	{
		//create surface
		//warning("TODO: Create compressed surface");
#if 0
		DDSURFACEDESC2 ddsd2;

		//create compressed surface
		memcpy(&ddsd2, &Movie->surfDesc, sizeof(DDSURFACEDESC2));

		ddsd2.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;

		if (!(Movie->surf = gCreateSurface(&ddsd2, Movie->surf))) {
			DebugLogFile("gLoadMovie: gCreateSurface FAILED: Can't create surface DDS");
			return NULL;
		}
#endif
	}

//	Movie->frameRate=240;
	return Movie;
}

//*********************************************************************************************
gTexture *gLoadTexture(WorkDirs &workDirs, const char *TextName, unsigned int LoaderFlags) {
	bool        bAlpha = FALSE, bUseAlternate = FALSE;
	gTexture    *Texture = nullptr;
	int32      pos = 0;
	char        AlternateName[500] {};
	uint32      date1 = 0, date2 = 0, time1 = 0, time2 = 0;
	//uint32        magic,retv;
	unsigned long   dwWidth = 0, dwHeight = 0;
	//DDSURFACEDESC2            DDSurfDesc;
	Graphics::Surface *lpSSource = nullptr;

	if (!TextName) return nullptr;
	lpSSource = nullptr;

	//warning("gLoadTexture(%s)", TextName);
//	Check if already loaded
	for (uint32 i = 0; i < gNumTextureList; i++) {
		if (gTextureList[i].name.equalsIgnoreCase(TextName)) {
			//Texture already loaded; just assign pointers
			Texture = &gTextureList[i];
			Texture->ID = i;
			return Texture;
		}
	}

	/*      VERSIONE ORIGINALE by FAB
	    gBuildAlternateName( AlternateName, TextName );
	    if( ( CurLoaderFlags & T3D_FASTFILE ) || ( !t3dGetFileDate( &date1, &time1, TextName ) ) )
	        bUseAlternate = TRUE;
	    if( !t3dGetFileDate( &date2, &time2, AlternateName ) )
	    {
	         if( bUseAlternate == TRUE )
	        {
	            DebugLogFile("gAddMaterial:gLoadTexture: Cannot find %s and alternate %s.\n", TextName, AlternateName );
	            return NULL;
	        }
	    }
	    else if( bUseAlternate == FALSE )
	    {
	        if( ( date2 > date1 ) || ( ( date2 == date1 ) && ( time2 >= time1 ) ) )
	            bUseAlternate = TRUE;
	    }
	*/

	gBuildAlternateName(AlternateName, TextName);

	if (!t3dGetFileDate(&date1, &time1, TextName))    // if it doesn't find the .tga texture, try the dds
		bUseAlternate = TRUE;

	if (!t3dGetFileDate(&date2, &time2, AlternateName)) {     //se non trova la texture .dds
		if (bUseAlternate == TRUE) { // does not find the .dds texture and furthermore the .tga does not exist
			DebugLogFile("gAddMaterial:gLoadTexture: Cannot find %s and alternate %s.\n", TextName, AlternateName);
			return nullptr;
		}
	} else if (bUseAlternate == FALSE) { // if there is the .dds and there is also the .tga see which is newer
		if ((date2 > date1) || ((date2 == date1) && (time2 >= time1)))
			bUseAlternate = TRUE;
	}


	if ((pos = gGetTextureListPosition()) == 0) {
		DebugLogFile("gLoadTexture: Can't create more textures");
		return nullptr;
	}
	Texture = &gTextureList[pos];
	*Texture = gTexture();

	if (bUseAlternate) {
		auto stream = workDirs.resolveFile(AlternateName);
		if (!stream) {
			DebugLogFile("gAddMaterial:gLoadTexture: Cannot find %s.\n", AlternateName);
			return nullptr;
		}
		auto compressed = parseDDS(*stream);
		dwWidth = compressed->width;
		dwHeight = compressed->height;
		Texture->texId = createCompressedTextureFromSurface(*compressed);
		delete compressed;
		lpSSource = nullptr;
#if 0
		if (gRenderFlags & gDXT1SUPPORTED) {
			/*          if( gRenderFlags & gAGPSUPPORTED )
			            {   // Alloc texture in AGP
			                DDSurfDesc.ddsCaps.dwCaps=  DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY | DDSCAPS_TEXTURE;
			            }
			            else
			*/          {   //No AGP support; alloc in sysmem
				DDSurfDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
				DDSurfDesc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
			}
			lpSSource = Texture->lpDDSurface;
		} else
			DDSurfDesc.ddsCaps.dwCaps |= DDSCAPS_SYSTEMMEMORY;
		if (!(lpSSource = gCreateSurface(&DDSurfDesc, lpSSource))) {
			DebugLogFile("gLoadTexture: gCreateSurface FAILED: Can't create surface DDS");
			return NULL;
		}

		if ((lpSSource->Lock(NULL, &DDSurfDesc, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL))) {                // Lock and fill with the dds
			DebugLogFile("gLoadTexture: Can't lock surface DDS");
			return NULL;
		}

		t3dReadData(DDSurfDesc.lpSurface, DDSurfDesc.dwLinearSize);

		if ((lpSSource->Unlock(NULL))) {
			DebugLogFile("gLoadTexture: Can't unlock surface DDS");
			return NULL;
		}
#endif
		stream = nullptr;
	} else { // TGA
		auto stream = workDirs.resolveFile(TextName);
		auto image = ReadTgaImage(TextName, *stream, Graphics::PixelFormat(4, 8, 8, 8, 8, 0, 8, 16, 24), 0); // TODO Flags
		createTextureFromSurface(*image, GL_RGBA);
#if 0
		//warning("TODO: Handle TGA");
		if (!t3dOpenFile(TextName)) {
			DebugLogFile("gAddMaterial:gLoadTexture: Cannot find %s.\n", TextName);
			return NULL;
		}
//		Parse the PPM header
		if (!loadTGAHeader(&dwWidth, &dwHeight)) {
			t3dCloseFile();
			DebugLogFile("gAddMaterial: gLoadTexture: Could not load or parse TGA header in %s.\n", TextName);
			return NULL;
		}
#endif
	}
#if 0 // Replaced by createTextureFromSurface
	if (!(gRenderFlags & gDXT1SUPPORTED) || (bUseAlternate == FALSE)) {
		memset(&DDSurfDesc, 0, sizeof(DDSURFACEDESC2));
		memcpy(&DDSurfDesc.ddpfPixelFormat, &gCurrentFormat->SurfaceDesc, sizeof(DDPIXELFORMAT));
		DDSurfDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		/*      if (gRenderFlags&gAGPSUPPORTED)
		        {   //Alloc texture in AGP
		            DDSurfDesc.ddsCaps.dwCaps=  DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY |
		                                        DDSCAPS_TEXTURE;
		        }
		        else
		*/      {   //No AGP support; alloc in sysmem
			DDSurfDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
			DDSurfDesc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		DDSurfDesc.dwWidth = dwWidth;
		DDSurfDesc.dwHeight = dwHeight;
		if (!(Texture->lpDDSurface = gCreateSurface(&DDSurfDesc, Texture->lpDDSurface))) {
			DebugLogFile("gLoadTexture: gCreateSurface FAILED: Can't create surface");
			return NULL;
		}
	}
#endif
	Texture->name = TextName;

	if (bUseAlternate) {
#if 0
		//      DebugFile( "Carico |%s|", AlternateName );
		if (!(gRenderFlags & gDXT1SUPPORTED)) {
			if (Texture->lpDDSurface->Blt(NULL, lpSSource, NULL, DDBLT_WAIT, NULL) != DD_OK) {
				DebugLogFile("gLoadTexture: Can't Blit DDS texture");
				return NULL;
			}
			lpSSource->Release();
		} else
			Texture->lpDDSurface = lpSSource;
#endif
	} else {
#if 0
		//      DebugFile( "Carico |%s|", TextName );
		Texture->lpDDSurface->Lock(NULL, &DDSurfDesc, DDLOCK_NOSYSLOCK | DDLOCK_WAIT, NULL); //Lock and fill with the tga
		if (gCurrentFormat->abits)
			bAlpha = TRUE;

		if (!ReadTgaImage(TextName, &DDSurfDesc, bAlpha)) {
			DebugLogFile("gLoadTexture: Error reading TGA file");
			return NULL;
		}
		Texture->lpDDSurface->Unlock(NULL);
#endif
	}
	Texture->RealDimX = dwWidth;
	Texture->RealDimY = dwHeight;

	if (LoaderFlags & rSURFACEHALF) {
		warning("Half-res loading not implemented");
#if 0
		LPDIRECTDRAWSURFACE7    surf;
		HRESULT err;

		dwWidth /= 2;
		dwHeight /= 2;

		memset(&DDSurfDesc, 0, sizeof(DDSURFACEDESC2));
		memcpy(&DDSurfDesc.ddpfPixelFormat, &gCurrentFormat->SurfaceDesc, sizeof(DDPIXELFORMAT));
		DDSurfDesc.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		/*      if (gRenderFlags&gAGPSUPPORTED)
		        {   //Alloc texture in AGP
		            DDSurfDesc.ddsCaps.dwCaps=  DDSCAPS_NONLOCALVIDMEM | DDSCAPS_VIDEOMEMORY |
		                                        DDSCAPS_TEXTURE;
		        }
		        else
		*/      {   //No AGP support; alloc in sysmem
			DDSurfDesc.ddsCaps.dwCaps = DDSCAPS_TEXTURE;
			DDSurfDesc.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		surf = NULL;
		DDSurfDesc.dwWidth = dwWidth;
		DDSurfDesc.dwHeight = dwHeight;
		if (!(surf = gCreateSurface(&DDSurfDesc, surf))) {
			DebugLogFile("gLoadTexture (rSURFACEHALF) FAILED: Can't create surface");
			return NULL;
		}
		if ((err = surf->Blt(NULL, Texture->lpDDSurface, NULL, 0, NULL)) != DD_OK) {
			char str[255];
			GetDDErrorString(err, str, 1);
			DebugLogFile("gLoadTexture: BltStretch failed.\n%s", str);
		}

		//Release old texture
		Texture->lpDDSurface->Release();
		//Assign the newone
		Texture->lpDDSurface = surf;
#endif
	}

	Texture->ID = pos;

	Texture->Flags = CurLoaderFlags;
	Texture->DimX = dwWidth;
	Texture->DimY = dwHeight;

	return Texture;
}


MaterialPtr Renderer::addMaterial(MaterialPtr MList, const Common::String &name, int NumFaces, unsigned int LoaderFlags) {
	MaterialPtr Material = MList;
	bool            AlreadyLoaded = FALSE;
	int             len = name.size();

	//warning("AddMaterial(%s)", name.c_str());
	if (((name[len - 1 - 0] == 'i') || (name[len - 1 - 0] == 'I')) &&
	        ((name[len - 1 - 1] == 'v') || (name[len - 1 - 1] == 'V')) &&
	        ((name[len - 1 - 2] == 'a') || (name[len - 1 - 2] == 'A'))) {
		if ((Material->Movie = gLoadMovie(*_workDirs, name.c_str())) == nullptr)
			return nullptr;
		if ((Material->Texture = gUserTexture(64,
		                                      128)) == nullptr)
//		if( (Material->Texture=gUserTexture(    Material->Movie->g_psiStreamInfo.rcFrame.right,
//										Material->Movie->g_psiStreamInfo.rcFrame.bottom)) == NULL )
			return nullptr;
		Material->addProperty(T3D_MATERIAL_MOVIE);
	} else {
		if ((Material->Texture = gLoadTexture(*_workDirs, name.c_str(), LoaderFlags)) == nullptr)
			return nullptr;
	}

//f
	Material->addProperty(T3D_MATERIAL_NOLIGHTMAP);
	return Material;
}

} // End of namespace Watchmaker

#endif // USE_OPENGL_GAME
