/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "engines/wintermute/base/gfx/opengl/shadow_volume.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
ShadowVolume::ShadowVolume(BaseGame *inGame) : BaseClass(inGame), _color(0x7f000000) {
}

//////////////////////////////////////////////////////////////////////////
ShadowVolume::~ShadowVolume() {
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::reset() {
	// nothing to do here at the moment
	return true;
}

//////////////////////////////////////////////////////////////////////////
void ShadowVolume::addVertex(const Math::Vector3d &vertex) {
	_vertices.add(vertex);
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::render() {
	warning("ShadowVolume::render not implemented yet");
	//	CBRenderD3D* Rend = (CBRenderD3D*)Game->m_Renderer;

	//	Rend->m_LastTexture = NULL;
	//	Rend->m_Device->SetTexture(0, NULL);

	//	C3DUtils::SetFixedVertexShader(Rend->m_Device, D3DFVF_XYZ);
	//	Rend->m_Device->DrawPrimitiveUP(D3DPT_TRIANGLELIST, m_NumVertices / 3, m_Vertices, sizeof(Math::Vector3d));

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::renderToStencilBuffer() {
	warning("ShadowVolume::renderToStencilBuffer not implemented yet");
	//	CBRenderD3D* Rend = (CBRenderD3D*)Game->m_Renderer;

	//	// Disable z-buffer writes (note: z-testing still occurs), and enable the
	//	// stencil-buffer
	//	Rend->m_Device->SetRenderState(D3DRS_ZWRITEENABLE,  FALSE);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	//	// Set up stencil compare fuction, reference value, and masks.
	//	// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
	//	// Note: since we set up the stencil-test to always pass, the STENCILFAIL
	//	// renderstate is really not needed.
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILFUNC,  D3DCMP_ALWAYS);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP);

	//	Rend->m_Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
	//	Rend->m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);

	//	// If z-test passes, inc/decrement stencil buffer value
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILREF,       0x1);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILMASK,      0xffffffff);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILWRITEMASK, 0xffffffff);

	//	// Make sure that no pixels get drawn to the frame buffer
	//	Rend->m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//	Rend->m_Device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_ZERO);
	//	Rend->m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//	Rend->m_Device->SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_INCR);

	//	// Draw back-side of shadow volume in stencil/z only
	//	Rend->m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	//	Render();

	//	// Decrement stencil buffer value
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_DECR);

	//	// Draw front-side of shadow volume in stencil/z only
	//	Rend->m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
	//	Render();

	//	// Restore render states
	//	Rend->m_Device->SetRenderState(D3DRS_LIGHTING, TRUE);
	//	Rend->m_Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	//	Rend->m_Device->SetRenderState(D3DRS_CULLMODE,  D3DCULL_CCW);
	//	Rend->m_Device->SetRenderState(D3DRS_ZWRITEENABLE,     TRUE) ;
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILENABLE,    FALSE);
	//	Rend->m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::renderToScene() {
	warning("ShadowVolume::renderToScene not implemented yet");
	//	if(!m_StencilMaskVB) InitMask();

	//	CBRenderD3D* Rend = (CBRenderD3D*)Game->m_Renderer;

	//	Rend->m_Device->SetRenderState(D3DRS_ZENABLE,          FALSE);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILENABLE,    TRUE);
	//	Rend->m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	//	Rend->m_Device->SetRenderState(D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA);
	//	Rend->m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_COLOROP,   D3DTOP_MODULATE);
	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
	//	Rend->m_Device->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);

	//	// Only write where stencil val >= 1 (count indicates # of shadows that
	//	// overlap that pixel)
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILREF,  0x1 );
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);

	//	Rend->m_Device->SetRenderState(D3DRS_FOGENABLE,          FALSE);

	//	// Draw a big, gray square
	//	Rend->m_Device->SetRenderState(D3DRS_COLORVERTEX, TRUE);

	//	C3DUtils::SetFixedVertexShader(Rend->m_Device, D3DFVF_SHADOWVOLVERTEX);
	//	C3DUtils::SetStreamSource(Rend->m_Device, 0, m_StencilMaskVB, sizeof(SHADOWVOLVERTEX));
	//	Rend->m_Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	//	// Restore render states
	//	Rend->m_Device->SetRenderState(D3DRS_ZENABLE,          TRUE);
	//	Rend->m_Device->SetRenderState(D3DRS_STENCILENABLE,    FALSE);

	//	Rend->Setup3D(NULL, true);

	//	// clear stencil buffer
	//	Rend->m_Device->Clear(0, NULL, D3DCLEAR_STENCIL, 0x00, 1.0f, 0);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::initMask() {
	warning("ShadowVolume::initMask not implemented yet");
	//	CBRenderD3D* Rend = (CBRenderD3D*)Game->m_Renderer;

	//	RELEASE(m_StencilMaskVB);

	//	Rend->m_Device->CreateVertexBufferUni(4 * sizeof(SHADOWVOLVERTEX),	D3DUSAGE_WRITEONLY, D3DFVF_SHADOWVOLVERTEX,	D3DPOOL_MANAGED, &m_StencilMaskVB);
	//	SHADOWVOLVERTEX* v;
	//	m_StencilMaskVB->LockVB(0, 0, &v, 0);
	//	{
	//		v[0].p = D3DXVECTOR4(0.0f, Rend->m_RealHeight, 0.0f, 1.0f);
	//		v[1].p = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 1.0f);
	//		v[2].p = D3DXVECTOR4(Rend->m_RealWidth, Rend->m_RealHeight, 0.0f, 1.0f);
	//		v[3].p = D3DXVECTOR4(Rend->m_RealWidth, 0.0f, 0.0f, 1.0f);
	//		v[0].color = m_Color;
	//		v[1].color = m_Color;
	//		v[2].color = m_Color;
	//		v[3].color = m_Color;
	//	}
	//	m_StencilMaskVB->Unlock();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool ShadowVolume::setColor(uint32 color) {
	if (color != _color) {
		_color = color;
		return initMask();
	} else {
		return true;
	}
}

} // namespace Wintermute
