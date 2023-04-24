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

#include "watchmaker/render.h"
#include "watchmaker/windows_hacks.h"
#include "watchmaker/3d/render/opengl_3d.h"
#include "watchmaker/render.h"
#include "watchmaker/3d/render/opengl_renderer.h"
#include "watchmaker/3d/render/opengl_2d.h"
#include "watchmaker/3d/render/shadows.h"
#include "watchmaker/ll/ll_regen.h"

namespace Watchmaker {

unsigned int bLightmaps = FALSE;                                                                // if lightmaps are activated
unsigned int bDisableShadows = FALSE;                                                           // if shadows should be disabled
unsigned int bDisableMultiTexturing = FALSE;                                                    // if multitexturing should be disabled

gBatchBlock BatchBlockList[MAX_BATCH_BLOCKS];
gBatchBlock BatchBlockListSpecial[MAX_BATCH_BLOCKS_SPECIAL];
//gBatchBlock BatchBlockListLightmaps[MAX_BATCH_BLOCKS_LIGHTMAPS];
gBatchBlock BatchBlockListSky[MAX_BATCH_BLOCKS_SKY];

unsigned int NumBatchBlocks = 0;
unsigned int NumBatchBlocksSpecial = 0;
//unsigned int NumBatchBlocksLightmaps=0;
unsigned int NumBatchBlocksSky = 0;

/* -----------------13/08/99 10.34-------------------
 *              Comparazione per BB
 * --------------------------------------------------*/
int cmpbb(const void *a, const void *b) {
	gBatchBlock *v1 = (gBatchBlock *)a;
	gBatchBlock *v2 = (gBatchBlock *)b;

	if (v1->Texture2 < v2->Texture2) return -1;
	else if (v1->Texture2 > v2->Texture2) return 1;
	else if (v1->Texture1 < v2->Texture1) return -1;
	else if (v1->Texture1 > v2->Texture1) return 1;
	else return 0;
}

/* -----------------31/05/99 10.12-------------------
 *          Attiva o disattiva lo ZBuffer
 * --------------------------------------------------*/
bool rSetZBufferState(bool state) {
	g_renderer->setRenderState(RenderState::ZENABLE, state);
	g_renderer->setRenderState(RenderState::ZWRITE_ENABLE, state);

	return !g_renderer->error();
}


/* -----------------25/06/99 11.14-------------------
 *  Renderizza il cielo senza considerare lo zbuffer
 * --------------------------------------------------*/
void RenderSky(void) {
	unsigned int    i;
	bool            hres;
	gBatchBlock     *bb;

	if (!NumBatchBlocksSky)
		return;

	if (!rSetZBufferState(false)) {
		DebugLogFile("Can't rSetZBufferState FALSE");
		return ;
	}

	g_renderer->setRenderState(RenderState::ALPHABLEND, TRUE);
	g_renderer->setRenderState(RenderState::ALPHAREF, 0x00000055);
	g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::INVSRCALPHA);
	if (g_renderer->error()) {
		return;
	}

	bb = &BatchBlockListSky[0];
	for (i = 0; i < NumBatchBlocksSky; i++, bb++) {
		if (bb->Texture1 < 0) continue;
		rSetUserViewMatrix(bb->ViewMatrixNum);

		if (bb->Texture1)
			g_renderer->setTexture(0, gTextureList[bb->Texture1]);
		else
			g_renderer->setTexture(0, nullptr);

		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);
		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		bb->Texture1 = -3;
		bb->Texture2 = -3;
	}

	if (!rSetZBufferState(true)) {
		DebugLogFile("Can't rSetZBufferState TRUE");
		return ;
	}
}


/* -----------------31/05/99 10.19-------------------
 *          Renderizza la Geometria
 * --------------------------------------------------*/
void RenderGeometry(void) {
	signed short int LastViewMatrixNum, LastTexture1, LastTexture2;
	gBatchBlock *bb;
	bool hres = false;

	RenderSky();
	//warning("TODO: Texture configuration");
#if 0
	g_renderer->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	g_renderer->SetTextureStageState(1, D3DTSS_TEXCOORDINDEX, 1);
	g_renderer->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	g_renderer->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
#endif
	g_renderer->setTextureWrapMode(1, TextureWrapMode::CLAMP);

	g_renderer->setRenderState(RenderState::TEXTUREFACTOR, 0xFFFFFFFF);
	g_renderer->setRenderState(RenderState::ALPHAREF, 0x00000000);

	g_renderer->setRenderState(RenderState::ALPHABLEND, FALSE);
	if (g_renderer->error()) {
		return;
	}

	LastViewMatrixNum = LastTexture1 = LastTexture2 = -2;
	bb = &BatchBlockList[0];
	qsort(bb, NumBatchBlocks, sizeof(gBatchBlock), cmpbb);
	for (uint i = 0; i < NumBatchBlocks; i++, bb++) {
		if (bb->Texture1 < 0) continue;

		if (bb->ViewMatrixNum != LastViewMatrixNum) {
			rSetUserViewMatrix(bb->ViewMatrixNum);
			LastViewMatrixNum = bb->ViewMatrixNum;
		}
		if (bb->Texture1 != LastTexture1) {
			if (bb->Texture1)
				g_renderer->setTexture(0, gTextureList[bb->Texture1]);
			else
				g_renderer->setTexture(0, nullptr);
			LastTexture1 = bb->Texture1;
		}
		if ((g_renderer->supportsMultiTexturing()) && (bb->Texture2 != LastTexture2)) {
			if (bb->Texture2 > 0) {
				if (LastTexture2 <= 0) {
					//warning("TODO: Texture configuration");
#if 0
					g_renderer->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_MODULATE);
					g_renderer->SetTextureStageState(1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
					g_renderer->SetTextureStageState(1, D3DTSS_COLORARG2, D3DTA_CURRENT);
					g_renderer->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2);
					g_renderer->SetTextureStageState(1, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
#endif
					g_renderer->setTextureWrapMode(1, TextureWrapMode::CLAMP);
				}
				g_renderer->setTexture(1, gTextureList[bb->Texture2]);
			} else if (LastTexture2 > 0) {
				//warning("TODO: Texture configuration");
#if 0
				g_renderer->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
				g_renderer->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
#endif
				g_renderer->setTexture(1, nullptr);
			}
			LastTexture2 = bb->Texture2;
		}

		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);

		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		bb->Texture1 = -3;
	}
	//warning("TODO: Texture configuration");
#if 0
	g_renderer->SetTextureStageState(1, D3DTSS_COLOROP,   D3DTOP_DISABLE);
	g_renderer->SetTextureStageState(1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE);
#endif

//	2nd pass: lightmaps, if device doesn't support multi-texturing
	if ((!g_renderer->supportsMultiTexturing()) && (bLightmaps)) {
		g_renderer->setRenderState(RenderState::ZWRITE_ENABLE, FALSE);

#if 0 // TODO
		g_renderer->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);                        // use mapping coordinates set 1
#endif
		g_renderer->setBlendFunc(BlendFactor::ZERO, BlendFactor::SRCCOLOR);
		g_renderer->setRenderState(RenderState::ALPHABLEND, true);
		if (g_renderer->error()) {
			return;
		}

		LastViewMatrixNum = LastTexture1 = LastTexture2 = -2;
		bb = &BatchBlockList[0];
		for (uint i = 0; i < NumBatchBlocks; i++, bb++) {
			if (bb->Texture2 <= 0) continue;

			if (bb->ViewMatrixNum != LastViewMatrixNum) {
				rSetUserViewMatrix(bb->ViewMatrixNum);
				LastViewMatrixNum = bb->ViewMatrixNum;
			}
			if (bb->Texture2 != LastTexture2) {
				g_renderer->setTexture(0, gTextureList[bb->Texture2]);
				LastTexture2 = bb->Texture2;
			}

			g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);
			hres = !g_renderer->error();
			if (!hres) {
				Common::String str = g_renderer->getErrorString();
				DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
			}
		}

		g_renderer->setRenderState(RenderState::ZWRITE_ENABLE, TRUE);
#if 0 // TODO
		g_renderer->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);                        // mapping coordinates back to set 0
#endif
	}

//	Clipmaps
	g_renderer->setRenderState(RenderState::ALPHABLEND, TRUE);
	g_renderer->setRenderState(RenderState::ALPHAREF, 0x00000055);

	g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::INVSRCALPHA);
	if (g_renderer->error()) {
		return;
	}

	LastViewMatrixNum = LastTexture1 = LastTexture2 = -2;
	bb = &BatchBlockListSpecial[0];
	qsort(bb, NumBatchBlocksSpecial, sizeof(gBatchBlock), cmpbb);
	for (uint i = 0; i < NumBatchBlocksSpecial; i++, bb++) {
		if (bb->Texture1 < 0) continue;
		if (!(bb->Flags1 & T3D_MATERIAL_CLIPMAP) ||
		        (bb->Flags1 & T3D_MATERIAL_FLARE) ||
		        (bb->Flags1 & T3D_MATERIAL_SMOKE))
			continue;

		if (bb->ViewMatrixNum != LastViewMatrixNum) {
			rSetUserViewMatrix(bb->ViewMatrixNum);
			LastViewMatrixNum = bb->ViewMatrixNum;
		}
		if (bb->Texture1 != LastTexture1) {
			if (bb->Texture1)
				g_renderer->setTexture(0, gTextureList[bb->Texture1]);
			else
				g_renderer->setTexture(0, nullptr);
			LastTexture1 = bb->Texture1;
		}

		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);
		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		bb->Texture1 = -3;
		bb->Texture2 = -3;
	}

//	Render Smoke or Flare materials
	g_renderer->setRenderState(RenderState::ALPHAREF, 0x00000055);
	g_renderer->setRenderState(RenderState::ZWRITE_ENABLE, FALSE);
	if (g_renderer->error()) {
		return;
	}

	LastViewMatrixNum = LastTexture1 = LastTexture2 = -2;
	bb = &BatchBlockListSpecial[0];
	for (uint i = 0; i < NumBatchBlocksSpecial; i++, bb++) {
		if (bb->Texture1 < 0) continue;
		if (!(bb->Flags1 & T3D_MATERIAL_FLARE) &&
		        !(bb->Flags1 & T3D_MATERIAL_SMOKE))
			continue;

		if (bb->Flags1 & T3D_MATERIAL_FLARE_SUN) {
			g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::INVSRCCOLOR);
		} else if (bb->Flags1 & T3D_MATERIAL_FLARESOFT) {
			g_renderer->setRenderState(RenderState::ZENABLE, FALSE);
			g_renderer->setBlendFunc(BlendFactor::DESTCOLOR, BlendFactor::ONE);
		} else if (bb->Flags1 & T3D_MATERIAL_SMOKE) {
			g_renderer->setBlendFunc(BlendFactor::SRCALPHA, BlendFactor::ONE);
		} else {
			g_renderer->setBlendFunc(BlendFactor::SRCCOLOR, BlendFactor::ONE);
		}
		if (g_renderer->error()) {
			return;
		}

		if (bb->ViewMatrixNum != LastViewMatrixNum) {
			rSetUserViewMatrix(bb->ViewMatrixNum);
			LastViewMatrixNum = bb->ViewMatrixNum;
		}
		if (bb->Texture1 != LastTexture1) {
			if (bb->Texture1)
				g_renderer->setTexture(0, gTextureList[bb->Texture1]);
			else
				g_renderer->setTexture(0, nullptr);
			LastTexture1 = bb->Texture1;
		}

		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		bb->Texture1 = -3;
		bb->Texture2 = -3;
	}
	g_renderer->setRenderState(RenderState::ZWRITE_ENABLE, TRUE);
	hres = !g_renderer->error();
	if (!hres) {
		return;
	}

//	Print transparent/translucent materials
	g_renderer->setRenderState(RenderState::ALPHAREF, 0x0000002);

	LastViewMatrixNum = LastTexture1 = LastTexture2 = -2;
	bb = &BatchBlockListSpecial[0];
	for (uint i = 0; i < NumBatchBlocksSpecial; i++, bb++) {
		if (bb->Texture1 < 0) continue;
		if ((bb->Flags1 & T3D_MATERIAL_FLARE) ||
		        (bb->Flags1 & T3D_MATERIAL_SMOKE))
			continue;

		if (bb->Flags1 & T3D_MATERIAL_GLASS) {
			g_renderer->setBlendFunc(BlendFactor::DESTCOLOR, BlendFactor::ZERO);
			return;
		} else if (bb->Flags1 & T3D_MATERIAL_OPACITY) {
			g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::SRCCOLOR);
		} else if (bb->Flags1 & T3D_MATERIAL_BOTTLE) {
			g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::INVDESTCOLOR);
		} else if (bb->Flags1 & T3D_MATERIAL_ADDITIVE) {
			g_renderer->setBlendFunc(BlendFactor::DESTCOLOR, BlendFactor::ONE);
		} else {
			continue;
		}
		if (g_renderer->error()) {
			return;
		}

		if (bb->ViewMatrixNum != LastViewMatrixNum) {
			rSetUserViewMatrix(bb->ViewMatrixNum);
			LastViewMatrixNum = bb->ViewMatrixNum;
		}
		if (bb->Texture1 != LastTexture1) {
			if (bb->Texture1)
				g_renderer->setTexture(0, gTextureList[bb->Texture1]);
			else
				g_renderer->setTexture(0, nullptr);
			LastTexture1 = bb->Texture1;
		}

		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::TRIANGLE, *bb);
		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		bb->Texture1 = -3;
		bb->Texture2 = -3;
	}

//	Render blank materials
//	???
}

/* -----------------31/05/99 10.55-------------------
 *              Renderizza la scena
 * --------------------------------------------------*/
bool rRenderScene(void) {
	unsigned int    i, j;
	bool            hres;
	static unsigned int dwFrameCount = 0;

	g_renderer->setRenderState(RenderState::LIGHT, false);
	g_renderer->setRenderState(RenderState::CLIP, true);
	g_renderer->setRenderState(RenderState::EXTENT, false);

	// Added:
	ResetScreenBuffer();

	if (!gStencilBitDepth && !bDisableShadows) {
//tb
		//Render Projected shadow into shadow surface
		for (j = 0; j < gNumShadowBoxesList; j++) {
			SHADOWBOX *sb = ShadowBoxesList[j];
			rSetUserViewMatrix(sb->ViewMatrixNum);
			if (bDisableShadows) break;
			warning("TODO: Shadows");
#if 0
			for (i = 0; i < sb->NumShadowsList; i++) {
				if (!(hres = RenderProjectiveShadow(&sb->ShadowsList[i]))) {
					Common::String str = g_renderer->getErrorString();
					DebugLogFile("Unable to RenderProjectiveShadow into a texture: %s | %d %d | %d", str.c_str(), S_OK, DD_OK, hres);
					bDisableShadows = TRUE;
					break;
				}

				if (sb->ShadowsList[i].ProjectiveTexture.lpDDSurface) {
					if (hres = sb->ShadowsList[i].ProjectiveTexture.lpDDSurface->BltFast(0, 0, g_pddsShadowBuffer, NULL, DDBLTFAST_NOCOLORKEY) != S_OK) {
						Common::String str = g_renderer->getErrorString();
						DebugLogWindow("Unable to blitfast Shadowbuffer into a texture: %s", str.c_str());
						bDisableShadows = TRUE;
						break;
					}

				}
				/*              if (g_pddsBackBuffer && !j)
				                {
				                    RECT    rect;
				                    rect.left=rect.top=0;
				                    rect.right=rect.bottom=256;

				                    if (FAILED(hres=g_pddsBackBuffer->BltFast(0,0,sb->ShadowsList[i].ProjectiveTexture.lpDDSurface,&rect,DDBLTFAST_NOCOLORKEY)))
				                    {
				                        Common::String str = g_renderer->getErrorString();
				                        DebugLogWindow("Unable to blit projective texture: %s",str);
				                        bDisableShadows=TRUE;
				                        break;
				                    }
				                }*/
			}
#endif
		}
//tb
	}

	// Begin the scene.
#if 0
	if (FAILED(g_pd3dDevice->BeginScene()))
		goto closescene;
#endif
	RenderGeometry();

	if (gStencilBitDepth && !bDisableShadows) {
//sb
		rSaveViewMatrix();
		g_renderer->setRenderState(RenderState::ALPHAREF, 0x00000002);
		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("Unable to SetRenderState for Shadowbuffer: %s", str.c_str());
			bDisableShadows = TRUE;
		}

		//Render Shadows volumes
		for (j = 0; j < gNumShadowBoxesList; j++) {
			SHADOWBOX *sb = ShadowBoxesList[j];
			rSetUserViewMatrix(sb->ViewMatrixNum);
			for (i = 0; i < sb->NumShadowsList; i++)
				RenderShadow(&sb->ShadowsList[i], sb->ShadowsList[i].VB);

			if (i) {
				RenderShadowBox(sb, sb->VBO);

				unsigned int width, height, bpp;
				rGetScreenInfos(&width, &height, &bpp);
				DrawShadow(0, 0, width, height, sb->Intensity);
			}
		}
		rRestoreViewMatrix();
//sb
	} else if (!bDisableShadows) {
//tb
		for (j = 0; j < gNumShadowBoxesList; j++) {
			SHADOWBOX *sb = ShadowBoxesList[j];
			rSetUserViewMatrix(sb->ViewMatrixNum);
			for (i = 0; i < sb->NumShadowsList; i++)
				if (!(hres = DrawProjectiveShadow(&sb->ShadowsList[i]))) {
					Common::String str = g_renderer->getErrorString();
					DebugLogFile("Unable to DrawProjectiveShadow: %s", str.c_str());
					bDisableShadows = TRUE;
				}
		}
//tb
	}

	if (gNumLinesArray && gNumPointsBuffer) {
		rSaveViewMatrix();
		rSetLinesViewMatrix();

		g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::ZERO);
		g_renderer->setRenderState(RenderState::ALPHABLEND, false);
		if (g_renderer->error()) {
			goto closescene;
		}

		g_renderer->setTexture(0, nullptr);
		/*
		hres=g_pd3dDevice->DrawIndexedPrimitiveVB(  D3DPT_LINELIST,
		                                              g_lpD3DPointsBuffer,0,gNumPointsBuffer,
		                                              gLinesArray, gNumLinesArray,0);
		*/
		g_renderer->drawIndexedPrimitivesVBO(PrimitiveType::LINE, g_lpD3DPointsBuffer, 0, gNumPointsBuffer, gLinesArray, gNumLinesArray/*, 0*/);
		hres = g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		rRestoreViewMatrix();
		gNumLinesArray = 0;
	}


	if (gNumTrianglesArray) {
		rSetZBufferState(FALSE);

		g_renderer->setRenderState(RenderState::ALPHAREF, 0x0000000);
		g_renderer->setBlendFunc(BlendFactor::ONE, BlendFactor::SRCALPHA);
		g_renderer->setRenderState(RenderState::ALPHABLEND, true);
		if (g_renderer->error()) {
			goto closescene;
		}

		g_renderer->setTexture(0, nullptr);
		g_renderer->drawPrimitives(PrimitiveType::TRIANGLE,
		                           /*D3DFVF_TLVERTEX,*/ gTriangles,
		                           gNumTrianglesArray/*, 0x0*/);
		hres = !g_renderer->error();
		if (!hres) {
			Common::String str = g_renderer->getErrorString();
			DebugLogWindow("DrawIndexedPrimitiveVB ERROR:\n\r%s", str.c_str());
		}
		gNumTrianglesArray = 0;
		rSetZBufferState(TRUE);
	}
#if 0
	g_pd3dDevice->EndScene();
#endif
	NumBatchBlocks = 0;
	NumBatchBlocksSpecial = 0;
//	NumBatchBlocksLightmaps=0;
	NumBatchBlocksSky = 0;

	dwFrameCount++;

	return TRUE;
	// End the scene.

closescene:
#if 0
	g_pd3dDevice->EndScene();
#endif
	DebugLogFile("Error during renderscene");
	return FALSE;
}


/* -----------------13/08/99 10.34-------------------
 *          Aggiunge un nuovo BatchBlock
 * --------------------------------------------------*/
gBatchBlock *rNewBatchBlock(signed short int T1, unsigned int F1, signed short int T2, unsigned int F2) {
	gBatchBlock *bb;

	bb = nullptr;
	if ((F1 & T3D_MATERIAL_GLASS) ||
	        (F1 & T3D_MATERIAL_OPACITY) ||
	        (F1 & T3D_MATERIAL_CLIPMAP) ||
	        (F1 & T3D_MATERIAL_SMOKE) ||
	        (F1 & T3D_MATERIAL_BOTTLE) ||
	        (F1 & T3D_MATERIAL_FLARE) ||
	        (F1 & T3D_MATERIAL_ADDITIVE)) {
		if ((NumBatchBlocksSpecial + 1) < MAX_BATCH_BLOCKS_SPECIAL)
			bb = &BatchBlockListSpecial[NumBatchBlocksSpecial++];
		else
			DebugLogFile("Too many BB Special: %d (MAX is %d)!", NumBatchBlocksSpecial, MAX_BATCH_BLOCKS_SPECIAL);
	}
//	else if( (T2>0) )
//	{
//		if( (NumBatchBlocksLightmaps+1) < MAX_BATCH_BLOCKS_LIGHTMAPS )
//			bb=&BatchBlockList[NumBatchBlocksLightmaps++];
//		else
//			DebugLogFile("Too many BB LightMaps: %d (MAX is %d)!",NumBatchBlocksLightmaps,MAX_BATCH_BLOCKS_LIGHTMAPS);
//	}
	else if ((F1 & T3D_MATERIAL_SKY)) {
		if ((NumBatchBlocksSky + 1) < MAX_BATCH_BLOCKS_SKY)
			bb = &BatchBlockListSky[NumBatchBlocksSky++];
		else
			DebugLogFile("Too many BB Sky: %d (MAX is %d)!", NumBatchBlocksSky, MAX_BATCH_BLOCKS_SKY);
	} else {
		if ((NumBatchBlocks + 1) < MAX_BATCH_BLOCKS)
			bb = &BatchBlockList[NumBatchBlocks++];
		else
			DebugLogFile("Too many BB: %d (MAX is %d)!", NumBatchBlocks, MAX_BATCH_BLOCKS);
	}

	if (!bb) return nullptr;

	*bb = gBatchBlock(T1, T2, F1, F2);
	return bb;
}

} // End of namespace Watchmaker
