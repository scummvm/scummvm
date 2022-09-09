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

#include "watchmaker/3d/render/shadows.h"
#include "watchmaker/3d/math/Matrix4x4.h"


#define MAX_SHADOW_BOXES    20                      // max shadows

namespace Watchmaker {

SHADOWBOX      *ShadowBoxesList[MAX_SHADOW_BOXES];
unsigned int    gNumShadowBoxesList = 0;

unsigned int gGetTextureListPosition();
extern unsigned int CurLoaderFlags;
#if 0
#pragma pack(1)
struct COLORVERTEX {                                // vertex structures
	D3DVECTOR p;
	D3DCOLOR  c;
};

struct TRANSCOLORVERTEX {                           // transformed vertex
	D3DVECTOR   p;
	D3DVALUE    rhw;
	D3DCOLOR    c;
};
#pragma pack()
#endif
#define MAX_SHADOW_BOXES    20                      // max shadows

bool g_bUseOneBitStencil = FALSE;                   // if Stencil buffer if 1 bit deep only
DWORD g_max_StencilVal = 255;                       // maximum value the stencil buffer will hold
#if 0
D3DSTENCILOP g_StencDecOp, g_StencIncOp;            // increment e decrement functions

LPDIRECTDRAWSURFACE7    g_pddsShadowBuffer = NULL;
LPDIRECTDRAWSURFACE7    g_pddsShadowZBuffer = NULL;
#endif
//************************************************************************************************
inline void SetIdentityMatrix(Matrix4x4 &m) {                                                    // set D3D matrix to identity
	m.setIdentity();
}

#if 0
//************************************************************************************************
int ccw(COLORVERTEX *P[], int i, int j, int k) {                                                // for convex-hull
	double  a = P[i]->p.x - P[j]->p.x,
	        b = P[i]->p.y - P[j]->p.y,
	        c = P[k]->p.x - P[j]->p.x,
	        d = P[k]->p.y - P[j]->p.y;
	return a * d - b * c <= 0; // true if points i, j, k counterclockwise
}

//*********************************************************************************************
int cmpl(const void *a, const void *b) {                                                        // for convex-hull
	float v;
	COLORVERTEX **av, **bv;

	av = (COLORVERTEX **)a;
	bv = (COLORVERTEX **)b;

	v = (*av)->p.x - (*bv)->p.x;

	if (v > 0) return 1;
	if (v < 0) return -1;

	v = (*bv)->p.y - (*av)->p.y;

	if (v > 0) return 1;
	if (v < 0) return -1;

	return 0;
}

//*********************************************************************************************
int cmph(const void *a, const void *b) {
	return cmpl(b, a);   // for convex-hull
}

//*********************************************************************************************
int make_chain(COLORVERTEX *V[], int n, int (*cmp)(const void *, const void *)) {               // for convex-hull
	int i, j, s = 1;
	COLORVERTEX *t;

	qsort(V, n, sizeof(COLORVERTEX *), cmp);
	for (i = 2; i < n; i++) {
		for (j = s; j >= 1 && ccw(V, i, j, j - 1); j--)
		{}
		s = j + 1;
		t = V[s];
		V[s] = V[i];
		V[i] = t;
	}
	return s;
}

//*********************************************************************************************
int ch2d(COLORVERTEX *P[], int n) {                                                             // for convex-hull
	int u = make_chain(P, n, cmpl);     // make lower hull
	if (!n) return 0;
	P[n] = P[0];
	return u + make_chain(P + u, n - u + 1, cmph); // make upper hull
}

//************************************************************************************************
void Find2DConvexHull(DWORD nverts, COLORVERTEX *pntptr, DWORD *cNumOutIdxs, WORD **OutHullIdxs) { // find a convex hull
	COLORVERTEX **PntPtrs;
	DWORD i;

	*cNumOutIdxs = 0;             //max space needed is n+1 indices
	*OutHullIdxs = (WORD *)malloc((nverts + 1) * (sizeof(DWORD) + sizeof(COLORVERTEX *)));

	PntPtrs = (COLORVERTEX **) & (*OutHullIdxs)[nverts + 1];

	// alg requires array of ptrs to verts (for qsort) instead of array of verts, so do the conversion
	for (i = 0; i < nverts; i++) {
		PntPtrs[i] = &pntptr[i];
	}

	*cNumOutIdxs = ch2d(PntPtrs, nverts);

	// convert back to array of idxs
	for (i = 0; i < *cNumOutIdxs; i++) {
		(*OutHullIdxs)[i] = (WORD)(PntPtrs[i] - &pntptr[0]);
	}
}

#endif

/* -----------------25/08/1999 16.41-----------------
 *      find 2D convex hull for the object
 * --------------------------------------------------*/
bool rMakeShadowVolume(SHADOWBOX *sb, gVertex *InVerts, DWORD nverts, float lightm[9]) {
	warning("Stubbed: rMakeShadowVolume\n");
#if 0
	Matrix4x4 matWorld, matView, matProj, IDmat;
	DWORD i;
	HRESULT hr;
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;
	unsigned int AlphaVal = 98;
	SHADOW *shad = &sb->ShadowsList[sb->NumShadowsList++];

	// Get a ptr to the ID3D object to create materials and/or lights. Note:
	// the Release() call just serves to decrease the ref count.
	LPDIRECT3D7 pD3D;
	pd3dDevice->GetDirect3D(&pD3D);
	pD3D->Release();

	LPDIRECT3DVERTEXBUFFER7 VB_Proj;
	D3DVERTEXBUFFERDESC vbDesc;
	vbDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
	vbDesc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
	vbDesc.dwFVF  =  D3DFVF_XYZ | D3DFVF_DIFFUSE;
	// xyz+color so we can render them in showshadvol mode

	// Create vertex buffer to hold shadow volumes verts
	if (shad->VB == NULL) {
		// now form array of indices that will make the tris
		ZeroMemory(shad, sizeof(SHADOW));
		shad->num_objverts = nverts;
		vbDesc.dwNumVertices = nverts * 2; // *2 to hold top of shadvol for infin light source

		if (FAILED(hr = pD3D->CreateVertexBuffer(&vbDesc, &shad->VB, 0)))
			return hr;

		// alloc enough to hold largest-case shadvol (max # of verts in c-hull is nverts)
		// (nverts+1)*2 for tri mesh to hold shadvol sides + nverts to hold tri-fan
		shad->pwShadVolIndices = (WORD *)t3dCalloc(sizeof(WORD) * (nverts + 1) * 2);
	}

	// create VB_Proj vertex buffer as a target for the vertex-projection operation used to compute
	// the silhouette
	vbDesc.dwNumVertices = nverts;
	vbDesc.dwFVF =  D3DFVF_XYZRHW;
	// even though RHW not used, must specify it or ProcessVerts will not consider this as a valid
	// target to xform verts into

	if (FAILED(hr = pD3D->CreateVertexBuffer(&vbDesc, &VB_Proj, NULL)))
		return hr;

	// must lock VB, then copy verts into its space.
	COLORVERTEX *VBvertptr;

	shad->VB->Lock(DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
	               (VOID **) &VBvertptr, NULL);

	// have to copy verts into VB memory.  I reformat into COLORVERTEX to do this.
	// could prevent reformat and do a straight memcpy if Find2DConvexHull used D3DVERTEX tho.
	COLORVERTEX    *cvptr   = VBvertptr;
	gVertex        *d3dvptr = InVerts;

	// reformat D3DVERTEX array to COLORVERTEX array
	for (i = 0; i < nverts; i++) {
		cvptr->p.x = d3dvptr->x;
		cvptr->p.y = d3dvptr->y;
		cvptr->p.z = d3dvptr->z;
		cvptr->c = RGBA_MAKE(0xff, 0x0, 0x0, 0xff); // shadvol is semi-transparent black
		cvptr++;
		d3dvptr++;
	}

	shad->VB->Unlock();

	// save cur matrices so we can use xform pipeln to project verts supafast
	pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD,      &matWorld);
	pd3dDevice->GetTransform(D3DTRANSFORMSTATE_VIEW,       &matView);
	pd3dDevice->GetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	SetIdentityMatrix(IDmat);

	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &IDmat);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &IDmat);

	// for view matrix, all we want is anything that projects the verts onto a plane
	// perp to light direction.  so any eyepoint is OK (try to make obj near origin though,
	// so look at one of the verts).  dont care what direction is view up vector (y).
	rSetViewMatrix(lightm[0], lightm[1], lightm[2],
	               lightm[3], lightm[4], lightm[5],
	               lightm[6], lightm[7], lightm[8],
	               0.0f, 0.0f, 0.0f);

	// do the planar projection
	VB_Proj->ProcessVertices(D3DVOP_TRANSFORM,
	                         0,  // write new verts at idx 0
	                         nverts,
	                         shad->VB,
	                         0,  // read src verts from idx 0
	                         pd3dDevice,
	                         0x0); // no flags

	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,      &matWorld);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,       &matView);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, &matProj);

	COLORVERTEX *pntptr;

	VB_Proj->Lock(DDLOCK_NOSYSLOCK | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
	              (void **) &pntptr, NULL);

	WORD *OutHullIdxs;
	DWORD n_idxs;

	Find2DConvexHull(nverts, pntptr, &n_idxs, &OutHullIdxs); // This is the function supplied with dx6

	VB_Proj->Unlock();
	VB_Proj->Release();   // just needed the indices of hull

	shad->VB->Lock(DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
	               (void **) &VBvertptr, NULL);

	// make shadow volume by taking hull verts and project them along light dir far enough
	// to be offscreen

	// add verts to end of VB
	for (i = 0; i < n_idxs; i++) {
		VBvertptr[nverts + i].p.x = VBvertptr[OutHullIdxs[i]].p.x - (2000.0f * lightm[6]); // scale factor of 10 should be enough
		VBvertptr[nverts + i].p.y = VBvertptr[OutHullIdxs[i]].p.y - (2000.0f * lightm[7]); // scale factor of 10 should be enough
		VBvertptr[nverts + i].p.z = VBvertptr[OutHullIdxs[i]].p.z - (2000.0f * lightm[8]); // scale factor of 10 should be enough
		VBvertptr[nverts + i].c = RGBA_MAKE(0x0, 0xff, 0x0, 0xff);
	}

	shad->totalverts = nverts + n_idxs;

	// now form array of indices that will make the tris
	// shad vol will have n_idxs square sides

	shad->num_side_indices = (n_idxs + 1) * 2;

	// if shadvol is not capped, shadow may be drawn in place where a backfacing cap is missing even
	// though no geometry is there

//f shad->num_cap_indices=n_idxs;
	shad->num_cap_indices = 0;

	WORD *idxptr;

	idxptr = shad->pwShadVolSideIndices = shad->pwShadVolIndices;

	// tris for all facets but final one
	for (i = 0; i < n_idxs; i++) {
		// outhullidx[i] is the index of the ith vertex of the n_idx convex hull verts
		// nverts+i is the index of the projected vert corresponding to the OutHullIdx[i] vertex
		*idxptr++ = OutHullIdxs[i];
		*idxptr++ = (WORD)(nverts + i);
	}
	// add tris for final facet (i==n_idxs)
	*idxptr++ = OutHullIdxs[0];
	*idxptr++ = (WORD)(nverts + 0);

//m shad->pwShadVolCapIndices=idxptr;

	free(OutHullIdxs);   // allocated by Find2DConvexHull
	shad->VB->Unlock();
#endif
	return true;
}

/* -----------------25/08/1999 16.41-----------------
 *  makes a shadow box to avoid stenciled shadows over the object
 * --------------------------------------------------*/
bool rMakeShadowBox(SHADOWBOX *sb, float BoxX, float BoxY, float BoxZ, WORD intens) {
	warning("Stubbed: rMakeShadowBox\n");
#if 0
	bool hr;
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;

	// Get a ptr to the ID3D object to create materials and/or lights. Note:
	// the Release() call just serves to decrease the ref count.
	LPDIRECT3D7 pD3D;
	pd3dDevice->GetDirect3D(&pD3D);
	pD3D->Release();

	D3DVERTEXBUFFERDESC vbDesc;
	vbDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
	vbDesc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
	vbDesc.dwFVF  =  D3DFVF_XYZ | D3DFVF_DIFFUSE;
	// xyz+color so we can render them in showshadvol mode

	// Create vertex buffer to hold shadow volumes verts
	if (sb->VB == NULL) {
		sb->NumVerts = 16;
		sb->NumIndices = 12 * 3;
		vbDesc.dwNumVertices = sb->NumVerts;
		if (FAILED(hr = pD3D->CreateVertexBuffer(&vbDesc, &sb->VB, 0)))
			return hr;

		// alloc enough to hold largest-case shadvol (max # of verts in c-hull is nverts)
		// (nverts+1)*2 for tri mesh to hold shadvol sides + nverts to hold tri-fan
		sb->pwIndices = (WORD *)t3dCalloc(sizeof(WORD) * sb->NumIndices);

		WORD       *curind = sb->pwIndices;
		*curind++ = 0;
		*curind++ = 1;
		*curind++ = 2;                                          // Base scura
		*curind++ = 2;
		*curind++ = 1;
		*curind++ = 3;
		*curind++ = 0;
		*curind++ = 4;
		*curind++ = 1;                                          // Bordo sinistro scuro
		*curind++ = 1;
		*curind++ = 4;
		*curind++ = 5;
		*curind++ = 2;
		*curind++ = 6;
		*curind++ = 0;                                          // Bordo destro scuro
		*curind++ = 0;
		*curind++ = 6;
		*curind++ = 4;
		*curind++ = 12;
		*curind++ = 13;
		*curind++ = 14;                                         // Testa chiara
		*curind++ = 14;
		*curind++ = 13;
		*curind++ = 15;
		*curind++ = 11;
		*curind++ = 15;
		*curind++ = 9;                                          // Bordo sinistro chiaro
		*curind++ = 9;
		*curind++ = 15;
		*curind++ = 13;
		*curind++ = 10;
		*curind++ = 14;
		*curind++ = 11;                                         // Bordo destro chiaro
		*curind++ = 11;
		*curind++ = 14;
		*curind++ = 15;
	}
	sb->NumShadowsList = 0;
	sb->Intensity = intens;

	// must lock VB, then copy verts into its space.
	COLORVERTEX *VBvertptr;
	sb->VB->Lock(DDLOCK_NOSYSLOCK | DDLOCK_WRITEONLY | DDLOCK_WAIT | DDLOCK_SURFACEMEMORYPTR,
	             (VOID **) &VBvertptr, NULL);

	// have to copy verts into VB memory.  I reformat into COLORVERTEX to do this.
	// could prevent reformat and do a straight memcpy if Find2DConvexHull used D3DVERTEX tho.
	COLORVERTEX    *cvptr   = VBvertptr;
	D3DCOLOR        cshadow = RGBA_MAKE(0x0, 0x0, 0x0, intens);
	D3DCOLOR        clight  = RGBA_MAKE(0x0, 0x0, 0x0, 0);

//	0: Base Piu' distante scuro
	cvptr->p.x = BoxX;
	cvptr->p.y = 5.0f;
	cvptr->p.z = BoxZ;
	cvptr->c   = cshadow;
	cvptr ++;
//	1: Base Sinistra scuro
	cvptr->p.x = -BoxZ;
	cvptr->p.y = 5.0f;
	cvptr->p.z = BoxX;
	cvptr->c   = cshadow;
	cvptr ++;
//	2: Base Destra scuro
	cvptr->p.x = BoxZ;
	cvptr->p.y = 5.0f;
	cvptr->p.z = -BoxX;
	cvptr->c   = cshadow;
	cvptr ++;
//	3: Base Piu' vicino scuro
	cvptr->p.x = -BoxX;
	cvptr->p.y = 5.0f;
	cvptr->p.z = -BoxZ;
	cvptr->c   = cshadow;
	cvptr ++;
//	4: Testa Piu' distante scura
	cvptr->p.x = BoxX;
	cvptr->p.y = BoxY;
	cvptr->p.z = BoxZ;
	cvptr->c   = cshadow;
	cvptr ++;
//	5: Testa Sinistra scuro
	cvptr->p.x = -BoxZ;
	cvptr->p.y = BoxY;
	cvptr->p.z = BoxX;
	cvptr->c   = cshadow;
	cvptr ++;
//	6: Testa Destra scuro
	cvptr->p.x = BoxZ;
	cvptr->p.y = BoxY;
	cvptr->p.z = -BoxX;
	cvptr->c   = cshadow;
	cvptr ++;
//	7: Testa Piu' vicino scuro
	cvptr->p.x = -BoxX;
	cvptr->p.y = BoxY;
	cvptr->p.z = -BoxZ;
	cvptr->c   = cshadow;
	cvptr ++;
//	8: Base Piu' distante chiaro
	cvptr->p.x = BoxX;
	cvptr->p.y = 5.0f;
	cvptr->p.z = BoxZ;
	cvptr->c   = clight;
	cvptr ++;
//	9: Base Sinistra chiaro
	cvptr->p.x = -BoxZ;
	cvptr->p.y = 5.0f;
	cvptr->p.z = BoxX;
	cvptr->c   = clight;
	cvptr ++;
//	10: Base Destra chiaro
	cvptr->p.x = BoxZ;
	cvptr->p.y = 5.0f;
	cvptr->p.z = -BoxX;
	cvptr->c   = clight;
	cvptr ++;
//	11: Base Piu' vicino chiaro
	cvptr->p.x = -BoxX;
	cvptr->p.y = 5.0f;
	cvptr->p.z = -BoxZ;
	cvptr->c   = clight;
	cvptr ++;
//	12: Testa Piu' distante chiaro
	cvptr->p.x = BoxX;
	cvptr->p.y = BoxY;
	cvptr->p.z = BoxZ;
	cvptr->c   = clight;
	cvptr ++;
//	13: Testa Sinistra chiaro
	cvptr->p.x = -BoxZ;
	cvptr->p.y = BoxY;
	cvptr->p.z = BoxX;
	cvptr->c   = clight;
	cvptr ++;
//	14: Testa Destra chiaro
	cvptr->p.x = BoxZ;
	cvptr->p.y = BoxY;
	cvptr->p.z = -BoxX;
	cvptr->c   = clight;
	cvptr ++;
//	15: Testa Piu' vicino chiaro
	cvptr->p.x = -BoxX;
	cvptr->p.y = BoxY;
	cvptr->p.z = -BoxZ;
	cvptr->c   = clight;
	cvptr ++;

	sb->VB->Unlock();

	ShadowBoxesList[gNumShadowBoxesList++] = sb;
#endif
	return true;
}

/* -----------------25/08/1999 17.24-----------------
 *          renders a stenciled convex hull
 * --------------------------------------------------*/
bool RenderShadow(SHADOW *pShad,
                  void *lpVBuf) {
	warning("TODO: Stubbed RenderShadow\n");
#if 0
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;

	pd3dDevice->SetTexture(0, NULL);

	// Turn depth buffer off, and stencil buffer on
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,  FALSE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);    // dont want to bother interpolating color
	pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW);

	// Set up stencil compare fuction, reference value, and masks
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFUNC,     D3DCMP_ALWAYS);

	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILZFAIL, D3DSTENCILOP_KEEP);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFAIL,  D3DSTENCILOP_KEEP);

	if (g_bUseOneBitStencil) {
		// If ztest passes, write !(g_bInvertStencilBufferSense) into stencil buffer
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILREF,   0x1);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILMASK,     0x1);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILWRITEMASK, 0x1);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS,  D3DSTENCILOP_REPLACE);
	} else {
		// If ztest passes, inc/decrement stencil buffer value
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILREF,      0x1);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILMASK,     0xffffffff);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILWRITEMASK, 0xffffffff);
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS,  g_StencIncOp);
	}

	// Since destcolor=SRCBLEND * SRC_COLOR + DESTBLEND * DEST_COLOR,
	// this should result in the tri color being completely dropped
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND,  D3DBLEND_ZERO);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

	// draw front-side of shadow volume in stencil/z only
	pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLESTRIP, lpVBuf, 0, pShad->totalverts,
	                                   pShad->pwShadVolSideIndices,
	                                   pShad->num_side_indices, 0x0);

	// Now reverse cull order so back sides of shadow volume are written.
	if (g_bUseOneBitStencil) {
		// write 0's/1's into stencil buffer to erase pixels beyond back of shadow
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILREF,  0x0);
	} else {
		// increment stencil buffer value
		pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS, g_StencDecOp);
	}

	pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CCW);
	// Draw back-side of shadow volume in stencil/z only
	pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLESTRIP, lpVBuf, 0, pShad->totalverts,
	                                   pShad->pwShadVolSideIndices,
	                                   pShad->num_side_indices, 0x0);

	// Restore render states
	pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,     TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE,    FALSE);
//f    pd3dDevice->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE, FALSE );
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
#endif
	return true;
}

/* -----------------25/08/1999 17.25-----------------
 *      renders an object stenciled shadows box
 * --------------------------------------------------*/
bool RenderShadowBox(SHADOWBOX *pSB,
                     int lpVBuf) {
	warning("TODO: RenderShadowBox\n");
#if 0
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;

	pd3dDevice->SetTexture(0, NULL);

	// Turn depth buffer off, and stencil buffer on
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,  FALSE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

	// Set up stencil compare fuction, reference value, and masks
	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILREF,  0x1);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFUNC,  D3DCMP_LESSEQUAL);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILZFAIL, D3DSTENCILOP_ZERO);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFAIL,  D3DSTENCILOP_ZERO);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS,  D3DSTENCILOP_ZERO);

	// Since destcolor=SRCBLEND * SRC_COLOR + DESTBLEND * DEST_COLOR,
	// this should result in the tri color being completely dropped
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, lpVBuf, 0, pSB->NumVerts,
	                                   pSB->pwIndices,
	                                   pSB->NumIndices, 0x0);

	// Restore render states
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE,     TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE,    FALSE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD);
#endif
	return true;
}

/* -----------------25/08/1999 17.25-----------------
 * Name: DrawShadow()
 * Desc: Draws a big grey polygon over scene, and blend it with pixels with
 *       stencil 1, which are in shadow.  Could optimize this by keeping track
 *       of rendered 2D extent rect of all shadow vols.
 * --------------------------------------------------*/
bool DrawShadow(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, WORD intens) {
	warning("Stubbed: DrawShadow\n");
#if 0
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;

	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);

	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE,    TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);

	// Since destcolor=SRCBLEND * SRC_COLOR + DESTBLEND * DEST_COLOR,
	// this results in destcolor= (AlphaSrc) * SRC_COLOR + (1-AlphaSrc)*DestColor
	pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// stencil cmp func is defined as (ref cmpfn stencbufval).

	// Only write where stencil val >= 1.  (count indicates # of shadows that overlap that pixel)
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILREF,  0x1);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILFUNC, D3DCMP_LESSEQUAL);

//f    pd3dDevice->SetRenderState( D3DRENDERSTATE_STENCILPASS, D3DSTENCILOP_KEEP );
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILPASS, D3DSTENCILOP_ZERO);

	// Set the world matrix to identity to draw the big grey square
	D3DMATRIX matWorld, matIdentity;
	pd3dDevice->GetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
	SetIdentityMatrix(matIdentity);
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matIdentity);
//    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW, &matIdentity );
//    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &matIdentity );

	TRANSCOLORVERTEX sqverts[4];
	WORD sqindices[6];

	sqverts[0].p.z = sqverts[1].p.z = sqverts[2].p.z = sqverts[3].p.z = 0.0f;
	sqverts[0].rhw = sqverts[1].rhw = sqverts[3].rhw = sqverts[3].rhw = 0.0f;
	sqverts[0].c = sqverts[1].c = sqverts[2].c = sqverts[3].c = RGBA_MAKE(0x0, 0x0, 0x0, intens);

	sqindices[0] = 0;
	sqindices[1] = 2;
	sqindices[2] = 1;
	sqindices[3] = 0;
	sqindices[4] = 3;
	sqindices[5] = 2;

	sqverts[0].p.x = (float)x1;
	sqverts[0].p.y = (float)y1;
	sqverts[1].p.x = (float)x2;
	sqverts[1].p.y = (float)y1;
	sqverts[2].p.x = (float)x2;
	sqverts[2].p.y = (float)y2;
	sqverts[3].p.x = (float)x1;
	sqverts[3].p.y = (float)y2;

	pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, (D3DFVF_XYZRHW | D3DFVF_DIFFUSE),
	                                 sqverts, 4, sqindices, 6, 0x0);

	// Restore render states
	pd3dDevice->SetTransform(D3DTRANSFORMSTATE_WORLD, &matWorld);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE,          TRUE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_STENCILENABLE,    FALSE);
	pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
#endif
	return true;
}

/* -----------------25/08/1999 17.28-----------------
 *      prepare shadow for projection to a texture
 * --------------------------------------------------*/
bool rMakeProjectiveShadow(SHADOWBOX *sb, void *InVerts, DWORD nverts) {
	warning("Stubbed. rMakeProjectiveShadow\n");
#if 0
	SHADOW *shad = &sb->ShadowsList[0];
	D3DVERTEXBUFFERDESC vbDesc;
	LPDIRECT3D7 pD3D;
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;
	int pos;

	if (shad->VB == NULL) {
		pd3dDevice->GetDirect3D(&pD3D);
		pD3D->Release();
		vbDesc.dwSize = sizeof(D3DVERTEXBUFFERDESC);
		vbDesc.dwCaps = D3DVBCAPS_SYSTEMMEMORY;
		vbDesc.dwFVF  =  D3DFVF_XYZRHW | D3DFVF_DIFFUSE;

		// now form array of indices that will make the tris
		shad->num_objverts = nverts;
		vbDesc.dwNumVertices = nverts; // *2 to hold top of shadvol for infin light source

		if (FAILED(pD3D->CreateVertexBuffer(&vbDesc, &shad->VB, 0))) {
			DebugLogFile("Unable to create VertexBuffer");
			return FALSE;
		}

		//Alloc indices for triangles
		shad->pwShadVolSideIndices = (WORD *)t3dCalloc(sizeof(WORD) * (nverts + 1) * 3);
	}

	if (!g_pddsShadowBuffer) {
		HRESULT LastError;
		DDSURFACEDESC2  ddsd;

		memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		memcpy(&ddsd, &GraphicsModes[gCurrGraphicsMode].SurfDesc, sizeof(DDSURFACEDESC2));
		ddsd.dwFlags = DDSD_WIDTH | DDSD_HEIGHT | DDSD_CAPS | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE;

		ddsd.dwWidth  = 256;
		ddsd.dwHeight = 256;

		g_pddsShadowBuffer = gCreateSurface(&ddsd, g_pddsShadowBuffer);
		if (!g_pddsShadowBuffer) {
			DebugLogFile("Unable to create ShadowBuffer");
			return FALSE;
		}

		memset(&ddsd, 0, sizeof(DDSURFACEDESC2));
		ddsd.dwSize = sizeof(DDSURFACEDESC2);
		g_pddsZBuffer->GetSurfaceDesc(&ddsd);

		ddsd.dwFlags        = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_PIXELFORMAT;
		ddsd.ddsCaps.dwCaps = DDSCAPS_ZBUFFER;
		ddsd.dwWidth  = 256;
		ddsd.dwHeight = 256;

		g_pddsShadowZBuffer = gCreateSurface(&ddsd, g_pddsZBuffer);
		if (!g_pddsShadowZBuffer) {
			char str[200];
			LastError = 0;
			GetDDErrorString(LastError, str, 1);

			DebugLogFile("CreateSurface for ShadowBuffer Z-buffer failed.\r\n%s", str);
			return FALSE;
		}
		LastError = g_pddsShadowBuffer->AddAttachedSurface(g_pddsShadowZBuffer);
		if (LastError != DD_OK) {
			char str[200];
			GetDDErrorString(LastError, str, 1);
			DebugLogFile("AddAttachedBuffer To ShadowBuffer failed for Z-Buffer.%s", str);
			return FALSE;
		}
	}

	if (!shad->ProjectiveTexture.lpDDSurface) {
		DDSURFACEDESC2  DDSurfDesc;
		memset(&DDSurfDesc, 0, sizeof(DDSURFACEDESC2));
		memcpy(&DDSurfDesc.ddpfPixelFormat, &gCurrentFormat->SurfaceDesc, sizeof(DDPIXELFORMAT));
		memcpy(&DDSurfDesc, &GraphicsModes[gCurrGraphicsMode].SurfDesc, sizeof(DDSURFACEDESC2));
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
		DDSurfDesc.dwWidth = 256;
		DDSurfDesc.dwHeight = 256;
		if (!(shad->ProjectiveTexture.lpDDSurface = gCreateSurface(&DDSurfDesc, shad->ProjectiveTexture.lpDDSurface))) {
			DebugLogFile("rMakeProjectiveShadow: gCreateSurface FAILED: Can't create surface");
			return FALSE;
		}

		pos = gGetTextureListPosition();
		if (pos == 0) {
			DebugLogFile("rMakeProjectiveShadow: Can't create more textures");
			return -1;
		}

		memset(&gTextureList[pos], 0, sizeof(gTexture));
		strcpy(gTextureList[pos].Name, "Texture Buffer Shadow");
		gTextureList[pos].lpDDSurface = shad->ProjectiveTexture.lpDDSurface;
		gTextureList[pos].RealDimX = 256;
		gTextureList[pos].RealDimY = 256;
		gTextureList[pos].DimX = 256;
		gTextureList[pos].DimY = 256;
		gTextureList[pos].Flags = CurLoaderFlags;
	}

	ShadowBoxesList[gNumShadowBoxesList++] = sb;
	sb->NumShadowsList = 1;
#endif
	return TRUE;
}

/* -----------------25/08/1999 17.28-----------------
 *      renders shadow to a specific texture
 * --------------------------------------------------*/
bool RenderProjectiveShadow(SHADOW *pShad) {
	warning("Stubbed. RenderPojectiveShadow\n");
#if 0
	HRESULT hres;
	LPDIRECT3DDEVICE7 pd3dDevice = g_pd3dDevice;
	LPDIRECTDRAWSURFACE7 OldRenderTarget;
	D3DVIEWPORT7 NewViewport;

	memset(&gOldViewport, 0, sizeof(D3DVIEWPORT7));
	if (hres = pd3dDevice->GetViewport(&gOldViewport) != D3D_OK) {
		DebugLogFile("Unable to get ViewPort2");
		return hres;
	}
	memcpy(&NewViewport, &gOldViewport, sizeof(D3DVIEWPORT7));

	if (hres = pd3dDevice->GetRenderTarget(&OldRenderTarget) != D3D_OK) {
		DebugLogFile("Can't GetRenderTarget for shadow");
		return hres;
	}
	if (hres = pd3dDevice->SetRenderTarget(g_pddsShadowBuffer, 0) != D3D_OK) {
		DebugLogFile("Can't SetRenderTarget for shadow");
		return hres;
	}

	NewViewport.dwHeight = NewViewport.dwWidth = 256;
	if (hres = pd3dDevice->SetViewport(&NewViewport) != D3D_OK) {
		DebugLogFile("Unable to set ShadowBuffer viewport");
		return hres;
	}

	D3DRECT rect;
	rect.x1 = rect.y1 = 0;
	rect.x2 = 256;
	rect.y2 = 256;
	if (hres = pd3dDevice->Clear(1, &rect, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xFFFFFFFF, 1.0f, 0) != D3D_OK) {
		DebugLogFile("Unable to clear2 ShadowBuffer viewport");
		return hres;
	}

	if (hres = g_pd3dDevice->BeginScene() != D3D_OK) {
		DebugLogFile("Unable to begin Shadow scene");
		return hres;
	}

	if (hres = pd3dDevice->SetTexture(0, NULL)) return hres;

	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, D3DZB_FALSE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_FLAT)) return hres; // dont want to bother interpolating color
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE)) return hres;

	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ZERO)) return hres;

	if (hres = pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST, pShad->VB, 0, pShad->totalverts,
	           pShad->pwShadVolSideIndices,
	           pShad->num_side_indices, 0x0) != D3D_OK) {
		DebugLogFile("Unable to DrawIndexedPrimitive for projected shadow");
		return hres;
	}

	if (hres = g_pd3dDevice->EndScene() != D3D_OK) {
		DebugLogFile("Unable to end shadow scene");
		return hres;
	}

	if (hres = g_pd3dDevice->SetRenderTarget(OldRenderTarget, 0) != D3D_OK) {
		DebugLogFile("Can't SetRenderTarget for shadows");
		return hres;
	}

	if (hres = pd3dDevice->SetViewport(&gOldViewport) != D3D_OK) {
		DebugLogFile("Unable to restore viewport");
		return hres;
	}

	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_SHADEMODE, D3DSHADE_GOURAUD)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, TRUE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, D3DZB_TRUE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE)) return hres;
	if (hres = pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE)) return hres;
#endif
	return true;
}

/* -----------------25/08/1999 17.31-----------------
 *      draw projected shadow texture to the room
 * --------------------------------------------------*/
bool DrawProjectiveShadow(SHADOW *pShad) {
	warning("Stubbed: DrawProjectiveShadow\n");
#if 0
	bool        hres;

	if (hres = g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_CLAMP) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE) != DD_OK) return hres;

	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE) != DD_OK) return hres;

	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_CW) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetTexture(0, pShad->ProjectiveTexture.lpDDSurface) != DD_OK) return hres;

	if (hres = g_pd3dDevice->DrawIndexedPrimitiveVB(D3DPT_TRIANGLELIST,
	           pShad->ProjVertsVB, 0, pShad->totalverts,
	           pShad->pwShadVolCapIndices,
	           pShad->num_cap_indices, 0x0) != DD_OK) {
		char str[255];
		GetDDErrorString(hres, str, 1);
		DebugLogFile("DrawIndexedPrimitiveVB ERROR:\n\r%s", str);
	}
	pShad->num_cap_indices = 0;

	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, TRUE) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetTextureStageState(0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP) != DD_OK) return hres;
	if (hres = g_pd3dDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE) != DD_OK) return hres;
#endif
	return true;
}

} // End of namespace Watchmaker
