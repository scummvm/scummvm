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

#include "hpl1/engine/graphics/Graphics.h"

#include "hpl1/engine/game/Updateable.h"
#include "hpl1/engine/graphics/GraphicsDrawer.h"
#include "hpl1/engine/graphics/LowLevelGraphics.h"
#include "hpl1/engine/graphics/MaterialHandler.h"
#include "hpl1/engine/graphics/MeshCreator.h"
#include "hpl1/engine/graphics/RenderList.h"
#include "hpl1/engine/graphics/Renderer2D.h"
#include "hpl1/engine/graphics/Renderer3D.h"
#include "hpl1/engine/graphics/RendererPostEffects.h"
#include "hpl1/engine/resources/Resources.h"
#include "hpl1/engine/resources/low_level_resources.h"
#include "hpl1/engine/system/low_level_system.h"

// 2D Materials
#include "hpl1/engine/graphics/Material_BumpSpec2D.h"
#include "hpl1/engine/graphics/Material_Diffuse2D.h"
#include "hpl1/engine/graphics/Material_DiffuseAdditive2D.h"
#include "hpl1/engine/graphics/Material_DiffuseAlpha2D.h"
#include "hpl1/engine/graphics/Material_FontNormal.h"
#include "hpl1/engine/graphics/Material_Smoke2D.h"

// 3D Materials
#include "hpl1/engine/graphics/Material_Additive.h"
#include "hpl1/engine/graphics/Material_Alpha.h"
#include "hpl1/engine/graphics/Material_Bump.h"
#include "hpl1/engine/graphics/Material_BumpColorSpec.h"
#include "hpl1/engine/graphics/Material_BumpSpec.h"
#include "hpl1/engine/graphics/Material_Diffuse.h"
#include "hpl1/engine/graphics/Material_DiffuseSpec.h"
#include "hpl1/engine/graphics/Material_EnvMap_Reflect.h"
#include "hpl1/engine/graphics/Material_Flat.h"
#include "hpl1/engine/graphics/Material_Modulative.h"
#include "hpl1/engine/graphics/Material_ModulativeX2.h"
#include "hpl1/engine/graphics/Material_Water.h"

namespace hpl {

//////////////////////////////////////////////////////////////////////////
// CONSTRUCTORS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

cGraphics::cGraphics(iLowLevelGraphics *apLowLevelGraphics, LowLevelResources *apLowLevelResources) {
	mpLowLevelGraphics = apLowLevelGraphics;
	mpLowLevelResources = apLowLevelResources;

	mpDrawer = NULL;
	mpMeshCreator = NULL;
	mpMaterialHandler = NULL;
	mpRenderer2D = NULL;
	mpRenderer3D = NULL;
	mpRendererPostEffects = NULL;
}

//-----------------------------------------------------------------------

cGraphics::~cGraphics() {
	Log("Exiting Graphics Module\n");
	Log("--------------------------------------------------------\n");

	hplDelete(mpRenderer2D);
	hplDelete(mpRenderer3D);
	hplDelete(mpRendererPostEffects);
	hplDelete(mpDrawer);
	hplDelete(mpMeshCreator);
	hplDelete(mpMaterialHandler);
	hplDelete(mpRenderList);

	Log("--------------------------------------------------------\n\n");
}

//-----------------------------------------------------------------------

//////////////////////////////////////////////////////////////////////////
// PUBLIC METHODS
//////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------

bool cGraphics::Init(int alWidth, int alHeight, int alBpp, int abFullscreen,
					 int alMultisampling, const tString &asWindowCaption,
					 cResources *apResources) {
	Log("Initializing Graphics Module\n");
	Log("--------------------------------------------------------\n");

	// Setup the graphic directories:
	apResources->AddResourceDir("core/programs");
	apResources->AddResourceDir("core/textures");

	Log(" Init low level graphics\n");
	mpLowLevelGraphics->Init(alWidth, alHeight, alBpp, abFullscreen, alMultisampling, asWindowCaption);

	Log(" Creating graphic systems\n");
	mpMaterialHandler = hplNew(cMaterialHandler, (this, apResources));
	mpDrawer = hplNew(cGraphicsDrawer, (mpLowLevelGraphics, mpMaterialHandler, apResources));
	mpRenderer2D = hplNew(cRenderer2D, (mpLowLevelGraphics, apResources, mpDrawer));
	mpRenderList = hplNew(cRenderList, (this));
	mpMeshCreator = hplNew(cMeshCreator, (mpLowLevelGraphics, apResources));
	mpRenderer3D = hplNew(cRenderer3D, (mpLowLevelGraphics, apResources, mpMeshCreator, mpRenderList));
	mpRendererPostEffects = hplNew(cRendererPostEffects, (mpLowLevelGraphics, apResources, mpRenderList,
														  mpRenderer3D));
	mpRenderer3D->SetPostEffects(mpRendererPostEffects);

	// Add all the materials.
	// 2D
	Log(" Adding engine materials\n");
	mpMaterialHandler->Add(hplNew(cMaterialType_BumpSpec2D, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_DiffuseAdditive2D, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_DiffuseAlpha2D, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Diffuse2D, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Smoke2D, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_FontNormal, ()));

	// 3D
	mpMaterialHandler->Add(hplNew(cMaterialType_Diffuse, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Bump, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_DiffuseSpec, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_BumpSpec, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_BumpColorSpec, ()));

	mpMaterialHandler->Add(hplNew(cMaterialType_Additive, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Alpha, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Flat, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_Modulative, ()));
	mpMaterialHandler->Add(hplNew(cMaterialType_ModulativeX2, ()));

	mpMaterialHandler->Add(hplNew(cMaterialType_EnvMap_Reflect, ()));

	mpMaterialHandler->Add(hplNew(cMaterialType_Water, ()));

	Log("--------------------------------------------------------\n\n");

	return true;
}

//-----------------------------------------------------------------------

iLowLevelGraphics *cGraphics::GetLowLevel() {
	return mpLowLevelGraphics;
}

//-----------------------------------------------------------------------

cGraphicsDrawer *cGraphics::GetDrawer() {
	return mpDrawer;
}

//-----------------------------------------------------------------------

cRenderer2D *cGraphics::GetRenderer2D() {
	return mpRenderer2D;
}

//-----------------------------------------------------------------------

} // namespace hpl
