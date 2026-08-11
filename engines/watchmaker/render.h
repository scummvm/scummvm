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

#ifndef WATCHMAKER_RENDER_H
#define WATCHMAKER_RENDER_H

#include "common/array.h"
#include "watchmaker/types.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/3d/texture.h"
#include "watchmaker/3d/material.h"

namespace Watchmaker {

// Material properties
#define T3D_MATERIAL_CLIPMAP            (1<<0)          // clipmap
#define T3D_MATERIAL_OPACITY            (1<<1)          // opacity (0-24)
#define T3D_MATERIAL_ADDITIVE           (1<<2)          // additive (25-49)
#define T3D_MATERIAL_GLASS              (1<<3)          // glass (50-74)
#define T3D_MATERIAL_BOTTLE             (1<<4)          // bottle (75-100)
#define T3D_MATERIAL_ENVIRONMENT        (1<<5)          // environment
#define T3D_MATERIAL_SKY                (1<<6)          // sky
#define T3D_MATERIAL_NOLIGHTMAP         (1<<7)          // no lightmap
#define T3D_MATERIAL_MOVIE              (1<<8)          // with movie
#define T3D_MATERIAL_FLARE              (1<<9)          // flare
#define T3D_MATERIAL_FLARE_SUN          (1<<10)         // flare sun
#define T3D_MATERIAL_FLARESOFT          (1<<11)         // flare soft
#define T3D_MATERIAL_SMOKE              (1<<12)         // smoke
#define T3D_MATERIAL_MOVIEPAUSED        (1<<13)         // if movie is paused

// Backbuffer & Z-Buffer
#define rCLEARZBUFFER       (1<<0)  // clear zbuffer
#define rCLEARBACKBUFFER    (1<<1)  // clear back buffer
#define rCLEARSCREENBUFFER  (1<<2)  // clear screen buffer

// rendering modes
#define rSOLIDMODE          1       // render in solid mode
#define rWIREFRAMEMODE      2       // render in wireframe

// vertex buffer lock options
#define rVBLOCK_READONLY    1       // lock VB for reading
#define rVBLOCK_WRITEONLY   2       // lock VB for writing
#define rVBLOCK_NOSYSLOCK   4       // lock VB without hanging the system

// VertexBuffers
#define MAX_LINES                   50000   // max lines in VB
#define MAX_SHADOWS_PER_LIGHT       30      // max shadows per light

#define MAX_BATCH_BLOCKS            512     // max normal batch blocks
#define MAX_BATCH_BLOCKS_SPECIAL    128     // max special batch blocks
#define MAX_BATCH_BLOCKS_LIGHTMAPS  512     // max lightmaps batch blocks
#define MAX_BATCH_BLOCKS_SKY        4       // max sky batch blocks

#define FLEXIBLEVERTEXFLAGS (D3DFVF_XYZ | D3DFVF_DIFFUSE |D3DFVF_TEX2 )     // VB struct elements
//#define FLEXIBLEVERTEXFLAGSDEST   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE |D3DFVF_TEX2 )

// Bitmap & Texture
#define rBITMAPSURFACE      (0)     // Plain bitmap image; fit the backbuffer pixelformat
#define rTEXTURESURFACE     (1<<0)  // Use when loading an image; if flag specified the image fit the texture pixelformat
#define rSURFACESTRETCH     (1<<1)  // Used for bitmaps; strech the pic to a new dimension using DX (could produce very bad results)
#define rSURFACECOPY        (1<<2)  // Not supported yet
#define rSURFACEHALF        (1<<3)  // Used for texture; hal the dimension during loading
#define rSURFACEFLIP        (1<<4)  // Flip upside-down the loaded image

struct VertexBuffer {
	// Just cheat for now, and do this offline.
	Common::Array<gVertex> _buffer;
};

struct SHADOW {
	uint32                      totalverts, num_objverts;   // number of vertices in the shadow mesh
	uint32                      num_side_indices, num_cap_indices;  // number of indices
//	LPDIRECT3DVERTEXBUFFER7  VB,ProjVertsVB;             // holds vertices of shadow volumes
	void   *VB;
	void *ProjVertsVB;              // holds vertices of shadow volumes
	uint16                     *pwShadVolIndices;           // tri indices into vertex buffer VB for DrawPrim
	uint16                     *pwShadVolSideIndices;       // ptrs into main index array pwShadVolIndices for Side tris of shadow volume
	int16                      *pwShadVolCapIndices;        // ptrs into main index array pwShadVolIndices for cap tris of shadow volume
	gTexture                    ProjectiveTexture;          // texture to be projected
};

struct SHADOWBOX {
	SHADOW                  ShadowsList[MAX_SHADOWS_PER_LIGHT]; // shadow elements
	unsigned int            NumShadowsList;             // number of shadow elements
	int                     VBO;
	//LPDIRECT3DVERTEXBUFFER7   VB;                         // shadow VB
	uint32                  NumVerts;                   // num verts in shadow box
	uint32                  NumIndices;                 // num indices
	uint16                 *pwIndices;                  // pointer to indices list
	uint16                  Intensity;                  // shadow intensity
	uint16                  ViewMatrixNum;              // number of view matrix
};

struct gBatchBlock {
	signed short int        Texture1 = 0, Texture2 = 0; // texture ID
	unsigned int            Flags1 = 0, Flags2 = 0;     // flags
	signed short int        ViewMatrixNum = 0;              // view matrix num
	unsigned short int      NumFaces() {
		return FacesList.size();    // faces number
	}
	unsigned short int      NumVerts() {
		if (VBO) return VBO->_buffer.size();    // verts number
		else return 0;
	}
	Common::Array<uint16>     FacesList;                 // pointer to faces list
	Common::SharedPtr<VertexBuffer> VBO = nullptr;
//	LPDIRECT3DVERTEXBUFFER7  VB;                         // block VB

	gBatchBlock() {}
	gBatchBlock(signed short int texture1, signed short int texture2, unsigned int flags1, unsigned int flags2) : Texture1(texture1), Texture2(texture2), Flags1(flags1), Flags2(flags2) {}
};

class WorkDirs;

//Rendering control functions
gBatchBlock            *rNewBatchBlock(signed short int T1, unsigned int F1, signed short int T2, unsigned int F2);
void                    rBuildMaterialList(MaterialTable &MatList, unsigned int NumMat, signed short int ViewMatrixNum);
bool                    rRenderScene();
void                    rResetPipeline();

// Function declaration
bool rGrabVideo(const char *path, char flags);
bool rRenderReady();

//Materials, textures and bitmap functions
MaterialTable rCreateMaterialList(int num);
bool    rSetMovieFrame(MaterialPtr mat, uint32 dwCurrFrame);
uint32 rGetMovieFrame(MaterialPtr mat);
void rReleaseAllTextures(unsigned int NotFlags);
class WGame;
int rLoadBitmapImage(WGame &game, const char *TextName, unsigned char flags);
void rReleaseAllBitmaps(unsigned int NotFlags);
void rReleaseBitmap(int i);
void rReleaseBitmapDirect(gTexture *b);
void rSetLoaderFlags(unsigned int NewLoaderFlags);
int rCreateSurface(unsigned int dimx, unsigned int dimy, unsigned char flags);
void *rCreateSurfaceP(unsigned int dimx, unsigned int dimy, unsigned char flags);
char   *rGetBitmapName(unsigned int id);
void rSetBitmapName(unsigned int id, const char *s);
unsigned char *rLockSurface(int surf, unsigned int *pitch);
unsigned char *rLockSurfaceDirect(gTexture *t, unsigned int *pitch);
void    rUnlockSurface(int surf);
void    rUnlockSurfaceDirect(gTexture *t);
uint16  rRGBAToSurfaceFormat(unsigned int R, unsigned int G, unsigned int B, unsigned int A);
uint16  rRGBAToTextureFormat(unsigned int r, unsigned int g, unsigned int b, unsigned int a);
void    rSurfaceFormatToRGBA(uint32 color, unsigned int *R, unsigned int *G, unsigned int *B, unsigned int *A);
void    rTextureFormatToRGBA(uint16 color, unsigned int *R, unsigned int *G, unsigned int *B, unsigned int *A);
gTexture *rGetSurfaceTexture(int pos);
gTexture *rCopyTexture(gTexture *t);


//VertexBuffer and triangles functions
VertexBuffer rCreateVertexBuffer(unsigned int num);
bool rDeleteVertexBuffer(VertexBuffer &vb);
gVertex *rLockVertexPtr(void *vb, int flags);
void *rGetUserVertexBuffer();
unsigned int rGetUserVertexBufferCounter();
void rSetUserVertexBufferCounter(unsigned int uvbc);
bool rUnlockVertexPtr(void *vb);
uint16 *rGetLinesArrayPtr();
void rAddLinesArray();
unsigned int rGetNumLinesArray();
void rAddTrianglesArray(float x, float y, int r, int g, int b, int a);
int16 *rGetTrianglesArrayPtr();
unsigned int rGetNumTrianglesArray();
void *rLockPointArray();
void rUnlockPointArray();
unsigned int rGetNumPointArray();
void rAddPointArray();
void rOptimizeVertexArray(void *v);

bool rMakeShadowVolume(SHADOWBOX *sb, gVertex *InVerts, DWORD nverts, float lightm[9]);
bool rMakeShadowBox(SHADOWBOX *sb, float BoxX, float BoxY, float BoxZ, WORD intens);
bool rMakeProjectiveShadow(SHADOWBOX *sb, void *InVerts, DWORD nverts);

//Setup & clear functions
class WGame;
bool rInitialize3DEnvironment(WGame &game, char *cfg);
char               *rGetRenderDllDesc();
void                    rSetFlagsFullScreen();
bool                    rGetFlagsFullScreen();
void                    rSetRenderMode(int state);
bool                    rClearBuffers(char flags);
void                    rGetScreenInfos(unsigned int *width, unsigned int *height, unsigned int *bpp);
void                    rRelaseFontTable(unsigned short *ft);
void                    rRelaseAllFontTable();
void                    rGetBlitterViewport(unsigned int *left, unsigned int *top, unsigned int *right, unsigned int *bottom);
void                    rUpdateExtends(int x1, int y1, int x2, int y2);
void                    rGetExtends(int *x1, int *y1, int *x2, int *y2);
void                    rResetExtends();
int                     rBlitSetStandardFont(signed int color, unsigned short *table);
int                     DebugQuick(signed int StdPx, signed int StdPy, const char *format, ...);
bool                    rGetStencilBitDepth();

//Misc functions
void                    rBlitter(WGame &game, int dst, int src, int dposx, int dposy, int sposx, int sposy, int sdimx, int sdimy);

//D3d specific geometry trasf. functions
struct t3dM3X3F;
struct t3dV3F;
void rSetViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation);
int rBuildLinesViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation);
int rAddUserViewMatrix(const t3dM3X3F &viewMatrix, const t3dV3F &translation);
void rSetUserViewMatrix(int num);
void rSaveViewMatrix();
void rRestoreViewMatrix();
void rSetLinesViewMatrix();

bool checkGlError(const char *when = "");

} // End of namespace Watchmaker

#endif // WATCHMAKER_RENDER_H
