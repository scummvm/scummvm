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

#include "hpl1/engine/gui/GuiMaterialBasicTypes.h"

#include "hpl1/engine/graphics/LowLevelGraphics.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// DIFFUSE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiMaterial_Diffuse::BeforeRender() {
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_Zero);
}

//-----------------------------------------------------------------------

void cGuiMaterial_Diffuse::AfterRender() {
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// ALPHA
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiMaterial_Alpha::BeforeRender() {
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

void cGuiMaterial_Alpha::AfterRender() {
	// Not needed right?
	// mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// FONT NORMAL
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiMaterial_FontNormal::BeforeRender() {
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_SrcAlpha, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

void cGuiMaterial_FontNormal::AfterRender() {
	// Not needed right?
	// mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// ADDITIVE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiMaterial_Additive::BeforeRender() {
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_One);
}

//-----------------------------------------------------------------------

void cGuiMaterial_Additive::AfterRender() {
	// Not needed right?
	// mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// MODULATIVE
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

void cGuiMaterial_Modulative::BeforeRender() {
	mpLowLevelGraphics->SetBlendActive(true);
	mpLowLevelGraphics->SetBlendFunc(eBlendFunc_DestColor, eBlendFunc_Zero);
}

//-----------------------------------------------------------------------

void cGuiMaterial_Modulative::AfterRender() {
	// Not needed right?
	// mpLowLevelGraphics->SetBlendFunc(eBlendFunc_One, eBlendFunc_OneMinusSrcAlpha);
}

//-----------------------------------------------------------------------

} // namespace hpl
