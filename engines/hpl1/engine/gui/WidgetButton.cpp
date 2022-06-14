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
 *
 * HPL1 Engine is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * HPL1 Engine is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with HPL1 Engine.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hpl1/engine/gui/WidgetButton.h"

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/math/Math.h"

#include "hpl1/engine/system/String.h"

#include "hpl1/engine/gui/Gui.h"
#include "hpl1/engine/gui/GuiSkin.h"
#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiGfxElement.h"

#include "hpl1/engine/graphics/FontData.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	cWidgetButton::cWidgetButton(cGuiSet *apSet, cGuiSkin *apSkin) : iWidget(eWidgetType_Button,apSet, apSkin)
	{
		mbPressed = false;
		mpImage = NULL;
	}

	//-----------------------------------------------------------------------

	cWidgetButton::~cWidgetButton()
	{
		if(mpImage && mbDestroyImage)
		{
			mpGui->DestroyGfx(mpImage);
		}
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cWidgetButton::SetImage(cGuiGfxElement *apImage, bool abDestroyImage)
	{
		mpImage = apImage;
		mbDestroyImage = abDestroyImage;
	}

	//-----------------------------------------------------------------------


	//////////////////////////////////////////////////////////////////////////
	// PROTECTED METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void cWidgetButton::OnLoadGraphics()
	{
		/////////////////////////
		//Up
		mpGfxBackgroundUp = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpBackground);

		mvGfxBordersUp[0] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpBorderRight);
		mvGfxBordersUp[1] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpBorderLeft);
		mvGfxBordersUp[2] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpBorderUp);
		mvGfxBordersUp[3] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpBorderDown);

		mvGfxCornersUp[0] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpCornerLU);
		mvGfxCornersUp[1] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpCornerRU);
		mvGfxCornersUp[2] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpCornerRD);
		mvGfxCornersUp[3] = mpSkin->GetGfx(eGuiSkinGfx_ButtonUpCornerLD);

		/////////////////////////
		//Down
		mpGfxBackgroundDown = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownBackground);

		mvGfxBordersDown[0] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownBorderRight);
		mvGfxBordersDown[1] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownBorderLeft);
		mvGfxBordersDown[2] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownBorderUp);
		mvGfxBordersDown[3] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownBorderDown);

		mvGfxCornersDown[0] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownCornerLU);
		mvGfxCornersDown[1] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownCornerRU);
		mvGfxCornersDown[2] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownCornerRD);
		mvGfxCornersDown[3] = mpSkin->GetGfx(eGuiSkinGfx_ButtonDownCornerLD);
	}

	//-----------------------------------------------------------------------

	void cWidgetButton::OnDraw(float afTimeStep, cGuiClipRegion *apClipRegion)
	{
		cVector3f vOffset = 0;
		if(mbPressed) vOffset = mpSkin->GetAttribute(eGuiSkinAttribute_ButtonPressedContentOffset);

		////////////////////////////////
		// Image
		if(mpImage)
		{
			mpSet->DrawGfx(	mpImage,GetGlobalPosition()+
							cVector3f(	mvSize.x/2 - mpImage->GetImageSize().x/2,
										mvSize.y/2 - mpImage->GetImageSize().y/2,0.2f) + vOffset
							);
		}

		////////////////////////////////
		// Text
		if(IsEnabled())
		{
			DrawDefaultText( msText,GetGlobalPosition()+
							cVector3f(mvSize.x/2, mvSize.y/2 - mvDefaultFontSize.y/2,0.5f)+vOffset,
							eFontAlign_Center);
		}
		else
		{
			DrawSkinText( msText,eGuiSkinFont_Disabled, GetGlobalPosition()+
						cVector3f(mvSize.x/2, mvSize.y/2 - mvDefaultFontSize.y/2,0.5f)+vOffset,
						eFontAlign_Center);
		}


		////////////////////////////////
		// Borders and background
		if(mbPressed)
		{
			DrawBordersAndCorners(	mpGfxBackgroundDown, mvGfxBordersDown, mvGfxCornersDown,
									GetGlobalPosition(), mvSize);
		}
		else
		{
			DrawBordersAndCorners(	mpGfxBackgroundUp, mvGfxBordersUp, mvGfxCornersUp,
									GetGlobalPosition(), mvSize);
		}
	}

	//-----------------------------------------------------------------------

	bool cWidgetButton::OnMouseMove(cGuiMessageData &aData)
	{
		return true;
	}

	//-----------------------------------------------------------------------

	bool cWidgetButton::OnMouseDown(cGuiMessageData &aData)
	{
		mbPressed = true;
		return true;
	}

	//-----------------------------------------------------------------------

	bool cWidgetButton::OnMouseUp(cGuiMessageData &aData)
	{
		if(GetMouseIsOver()==false) return false;

		if(mbPressed) ProcessMessage(eGuiMessage_ButtonPressed,aData);

		mbPressed = false;


		return true;
	}

	//-----------------------------------------------------------------------

	bool cWidgetButton::OnMouseEnter(cGuiMessageData &aData)
	{
		return false;
	}

	//-----------------------------------------------------------------------

	bool cWidgetButton::OnMouseLeave(cGuiMessageData &aData)
	{
		mbPressed = false;

		return false;
	}

	//-----------------------------------------------------------------------


}
