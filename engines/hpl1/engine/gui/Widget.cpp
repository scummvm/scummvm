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

#include "hpl1/engine/gui/Widget.h"

#include "hpl1/engine/system/LowLevelSystem.h"

#include "hpl1/engine/gui/GuiSet.h"
#include "hpl1/engine/gui/GuiSkin.h"
#include "hpl1/engine/gui/GuiGfxElement.h"

namespace hpl {

	//////////////////////////////////////////////////////////////////////////
	// CONSTRUCTORS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	iWidget::iWidget(eWidgetType aType,cGuiSet *apSet, cGuiSkin *apSkin)
	{
		mpSet = apSet;
		mpSkin = apSkin;
		mpGui = mpSet->GetGui();

		mType = aType;

		mvCallbackLists.resize(eGuiMessage_LastEnum);

		mpParent = NULL;

		mvPosition =0;
		mvSize = 0;

		mbEnabled = true;
		mbVisible = true;

		mbClipsGraphics = false;

		mbMouseIsOver = false;

		msText =_W("");

		mbPositionIsUpdated = true;

		mlPositionCount =0;

		mbConnectedToChildren = true;

		if(mpSkin)  mpPointerGfx = mpSkin->GetGfx(eGuiSkinGfx_PointerNormal);
		else		mpPointerGfx = NULL;
	}

	//-----------------------------------------------------------------------

	iWidget::~iWidget()
	{
		////////////////////////
		//Remove all children
		tWidgetListIt it = mlstChildren.begin();
		while(it != mlstChildren.end())
		{
			RemoveChild(*it);
			it = mlstChildren.begin();
		}

		////////////////////////////
		//Remove from parent
		if(mpParent) mpParent->RemoveChild(this);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PUBLIC METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	void iWidget::Update(float afTimeStep)
	{
		OnUpdate(afTimeStep);
	}

	//-----------------------------------------------------------------------

	void iWidget::Draw(float afTimeStep, cGuiClipRegion *apClipRegion)
	{
		if(mbVisible==false) return;

		OnDraw(afTimeStep, apClipRegion);

		cGuiClipRegion *pChildRegion = apClipRegion;
		if(mbClipsGraphics)
		{
			pChildRegion = apClipRegion->CreateChild(GetGlobalPosition(), mvSize);
			mpSet->SetCurrentClipRegion(pChildRegion);
		}

		OnDrawAfterClip(afTimeStep,apClipRegion);

		/////////////////////////////////
		//Draw callbacks
		cGuiMessageData data;
		data.mfVal = afTimeStep;
		data.mpData = apClipRegion;
		ProcessMessage(eGuiMessage_OnDraw, data);

		/////////////////////////////////
		//Draw children
		tWidgetListIt it = mlstChildren.begin();
		for(; it != mlstChildren.end(); ++it)
		{
			iWidget *pChild = *it;

			pChild->Draw(afTimeStep, pChildRegion);
		}

		if(mbClipsGraphics) mpSet->SetCurrentClipRegion(apClipRegion);
	}

	//-----------------------------------------------------------------------

	void iWidget::Init()
	{
		OnInit();
		LoadGraphics();
	}

	//-----------------------------------------------------------------------

	bool iWidget::ProcessMessage(eGuiMessage aMessage, cGuiMessageData &aData)
	{
		if(IsEnabled()==false) return false;

		aData.mMessage = aMessage;

		bool bRet = false;
		bRet = OnMessage(aMessage,aData); //This can override any message.

		/////////////////////////////////////////
		//Call the correct virtual function
		if(bRet==false)
		{
			switch(aMessage)
			{
				case eGuiMessage_MouseMove:			bRet = OnMouseMove(aData); break;
				case eGuiMessage_MouseDown:			bRet = OnMouseDown(aData); break;
				case eGuiMessage_MouseUp:			bRet = OnMouseUp(aData); break;
				case eGuiMessage_MouseDoubleClick:	bRet = OnMouseDoubleClick(aData); break;
				case eGuiMessage_MouseEnter:		bRet = OnMouseEnter(aData); break;
				case eGuiMessage_MouseLeave:		bRet = OnMouseLeave(aData); break;
				case eGuiMessage_KeyPress:			bRet = OnKeyPress(aData); break;
				case eGuiMessage_GotFocus:			bRet = OnGotFocus(aData); break;
				case eGuiMessage_LostFocus:			bRet = OnLostFocus(aData); break;
			}
		}

		/////////////////////////////////////////
		//Process user callbacks for the event.
		if(ProcessCallbacks(aMessage,aData)) bRet = true;

		return bRet;
	}

	//-----------------------------------------------------------------------

	void iWidget::AddCallback(eGuiMessage aMessage,void *apObject,tGuiCallbackFunc apFunc)
	{
		mvCallbackLists[aMessage].push_back(cWidgetCallback(apObject, apFunc));
	}

	//-----------------------------------------------------------------------

	bool iWidget::PointIsInside(const cVector2f& avPoint, bool abOnlyClipped)
	{
		if(mpParent && mpParent->ClipsGraphics())
		{
			if(mpParent->PointIsInside(avPoint, true)==false)
			{
				return false;
			}
		}

		if(abOnlyClipped && mbClipsGraphics==false) return true;

		cVector3f vGlobalPos = GetGlobalPosition();

		if(	avPoint.x < vGlobalPos.x || avPoint.x > vGlobalPos.x + mvSize.x ||
			avPoint.y < vGlobalPos.y || avPoint.y > vGlobalPos.y + mvSize.y)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	//-----------------------------------------------------------------------

	void iWidget::AttachChild(iWidget *apChild)
	{
		if(apChild->mpParent)
		{
			iWidget *pParent = apChild->mpParent;
			pParent->RemoveChild(apChild);
			apChild->SetPosition(apChild->mvPosition + pParent->GetGlobalPosition());
			apChild->SetPosition(apChild->mvPosition - GetGlobalPosition());
		}
		apChild->mpParent = this;
		apChild->SetPositionUpdated();
		mlstChildren.push_back(apChild);
	}

	void iWidget::RemoveChild(iWidget *apChild)
	{
		tWidgetListIt it = mlstChildren.begin();
		for(; it != mlstChildren.end(); ++it)
		{
			iWidget *pChild = *it;

			if(pChild == apChild)
			{
				mlstChildren.erase(it);

				pChild->mpParent = NULL;
				pChild->SetPositionUpdated();
				pChild->SetPosition(pChild->mvPosition + GetGlobalPosition());

				break;
			}
		}
	}

	//-----------------------------------------------------------------------

	void iWidget::SetEnabled(bool abX)
	{
		if(mbEnabled == abX) return;

		mbEnabled = abX;

	}

	bool iWidget::IsEnabled()
	{
		if(mpParent)
		{
			if(mpParent->IsEnabled()) return mbEnabled;
			else					return false;
		}

		return mbEnabled;
	}

	//-----------------------------------------------------------------------

	void iWidget::SetVisible(bool abX)
	{
		if(mbVisible == abX) return;

		mbVisible = abX;
	}

	bool iWidget::IsVisible()
	{
		if(mpParent)
		{
			if(mpParent->IsVisible()) return mbVisible;
			else					return false;
		}

		return mbVisible;
	}

	//-----------------------------------------------------------------------

	bool iWidget::HasFocus()
	{
		return mpSet->GetFocusedWidget() == this;
	}

	//-----------------------------------------------------------------------

	void iWidget::SetText(const tWString& asText)
	{
		if(asText == msText) return;

		msText = asText;

		OnChangeText();
		cGuiMessageData data = cGuiMessageData();
		ProcessMessage(eGuiMessage_TextChange,data);
	}

	//-----------------------------------------------------------------------

	void iWidget::SetPosition(const cVector3f &avPos)
	{
		mvPosition = avPos;

		SetPositionUpdated();
	}

	void iWidget::SetGlobalPosition(const cVector3f &avPos)
	{
		SetPosition(avPos - mpParent->GetGlobalPosition());
	}

	const cVector3f& iWidget::GetLocalPosition()
	{
		return mvPosition;
	}

	const cVector3f& iWidget::GetGlobalPosition()
	{
		if(mpParent)
		{
			if(mbPositionIsUpdated)
			{
				mbPositionIsUpdated = false;
				mvGlobalPosition = mpParent->GetGlobalPosition() + mvPosition;
			}
			return mvGlobalPosition;
		}
		else
		{
			return mvPosition;
		}
	}

	//-----------------------------------------------------------------------

	void iWidget::SetSize(const cVector2f &avSize)
	{
		mvSize = avSize;

		OnChangeSize();
	}

	//-----------------------------------------------------------------------

	bool iWidget::ClipsGraphics()
	{
		if(mpParent && mpParent->ClipsGraphics()) return true;

		return mbClipsGraphics;
	}

	//-----------------------------------------------------------------------

	bool iWidget::IsConnectedTo(iWidget *apWidget, bool abIsStartWidget)
	{
		if(abIsStartWidget == false && mbConnectedToChildren==false) return false;

		if(apWidget == NULL) return false;
		if(apWidget == this) return true;

		if(mpParent) return mpParent->IsConnectedTo(apWidget,false);

		return false;
	}


	//-----------------------------------------------------------------------

	cGuiGfxElement* iWidget::GetPointerGfx()
	{
		return mpPointerGfx;
	}

	//-----------------------------------------------------------------------


	//////////////////////////////////////////////////////////////////////////
	// PROTECTED METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool iWidget::OnGotFocus(cGuiMessageData &aData)
	{
		return mbEnabled;
	}
	//-----------------------------------------------------------------------

	cVector3f iWidget::WorldToLocalPosition(const cVector3f &avPos)
	{
		return avPos - GetGlobalPosition();
	}

	//-----------------------------------------------------------------------

	cVector2f iWidget::GetPosRelativeToMouse(cGuiMessageData &aData)
	{
		cVector3f vTemp = GetGlobalPosition() - aData.mvPos;
		return cVector2f(vTemp.x, vTemp.y);
	}

	//-----------------------------------------------------------------------

	void iWidget::DrawBordersAndCorners(cGuiGfxElement *apBackground,
										cGuiGfxElement **apBorderVec,cGuiGfxElement **apCornerVec,
										const cVector3f &avPosition, const cVector2f &avSize)
	{
		mpSet->SetDrawOffset(avPosition);

		///////////////////////
		// Background
		if(apBackground)
		{

			mpSet->DrawGfx(apBackground,cVector3f(	apCornerVec[0]->GetActiveSize().x,
												apCornerVec[0]->GetActiveSize().y,0 ),
							avSize - apCornerVec[2]->GetActiveSize()-apCornerVec[0]->GetActiveSize(),
							cColor(1,1));
		}



		///////////////////////
		// Borders
		//Right
		mpSet->DrawGfx(	apBorderVec[0],
			cVector3f(	avSize.x - apBorderVec[0]->GetActiveSize().x,
			apCornerVec[1]->GetActiveSize().y,0),
			cVector2f(	apBorderVec[0]->GetImageSize().x,
			avSize.y - (apCornerVec[2]->GetActiveSize().y +
			apCornerVec[1]->GetActiveSize().y)));
		//Left
		mpSet->DrawGfx(	apBorderVec[1],
			cVector3f(	0,apCornerVec[0]->GetActiveSize().y,0),
			cVector2f(	apBorderVec[1]->GetImageSize().x,
			avSize.y - (apCornerVec[3]->GetActiveSize().y +
			apCornerVec[0]->GetActiveSize().y)));

		//Up
		mpSet->DrawGfx(	apBorderVec[2],
			cVector3f(	apCornerVec[0]->GetActiveSize().x,0,0),
			cVector2f(	avSize.x - (apCornerVec[0]->GetActiveSize().x+
			apCornerVec[1]->GetActiveSize().x),
			apBorderVec[2]->GetImageSize().y));

		//Down
		mpSet->DrawGfx(	apBorderVec[3],
			cVector3f(	apCornerVec[3]->GetActiveSize().x,
			avSize.y - apBorderVec[3]->GetActiveSize().y,0),
			cVector2f(	avSize.x - (apCornerVec[2]->GetActiveSize().x+
			apCornerVec[3]->GetActiveSize().x),
			apBorderVec[3]->GetImageSize().y));


		///////////////////////
		// Corners
		//Left Up
		mpSet->DrawGfx(apCornerVec[0], cVector3f(0,0,0));
		//Right Up
		mpSet->DrawGfx(apCornerVec[1], cVector3f(	avSize.x - apCornerVec[1]->GetActiveSize().x,0,0));

		//Right Down
		mpSet->DrawGfx(apCornerVec[2], cVector3f(	avSize.x -  apCornerVec[2]->GetActiveSize().x,
			avSize.y -  apCornerVec[2]->GetActiveSize().y,0));
		//Left Down
		mpSet->DrawGfx(apCornerVec[3], cVector3f(	0,avSize.y - apCornerVec[3]->GetActiveSize().y,0));


		mpSet->SetDrawOffset(0);
	}

	//-----------------------------------------------------------------------

	void iWidget::DrawSkinText(	const tWString& asText,eGuiSkinFont aFont,
								const cVector3f& avPosition,eFontAlign aAlign)
	{
		cGuiSkinFont *pFont = mpSkin->GetFont(aFont);
		mpSet->DrawFont(asText,pFont->mpFont,avPosition,pFont->mvSize,pFont->mColor,
						aAlign);
	}

	//-----------------------------------------------------------------------

	void iWidget::DrawDefaultText(	const tWString& asText,
									const cVector3f& avPosition,eFontAlign aAlign)
	{
		if(mpDefaultFontType==NULL) return;

		mpSet->DrawFont(asText,mpDefaultFontType,avPosition,mvDefaultFontSize,
						mDefaultFontColor, aAlign);
	}

	//-----------------------------------------------------------------------

	//////////////////////////////////////////////////////////////////////////
	// PRIVATE METHODS
	//////////////////////////////////////////////////////////////////////////

	//-----------------------------------------------------------------------

	bool iWidget::ProcessCallbacks(eGuiMessage aMessage, cGuiMessageData &aData)
	{
		tWidgetCallbackList &lstCallbacks = mvCallbackLists[aMessage];

		if(lstCallbacks.empty()) return false;

		bool bRet = false;
		tWidgetCallbackListIt it = lstCallbacks.begin();
		for(; it != lstCallbacks.end(); ++it)
		{
			cWidgetCallback &callback = *it;

			bool bX = (callback.mpFunc)(callback.mpObject,this, aData);
			if(bX) bRet = true;
		}

		return bRet;
	}

	//-----------------------------------------------------------------------

	void iWidget::LoadGraphics()
	{
		if(mpSkin)
		{
			mpDefaultFont = mpSkin->GetFont(eGuiSkinFont_Default);

			mpDefaultFontType = mpDefaultFont->mpFont;
			mDefaultFontColor = mpDefaultFont->mColor;
			mvDefaultFontSize = mpDefaultFont->mvSize;
		}
		else
		{
			mpDefaultFont = NULL;
		}

		OnLoadGraphics();
	}

	//-----------------------------------------------------------------------
	void iWidget::SetPositionUpdated()
	{
		mbPositionIsUpdated = true;
		mlPositionCount++;

		OnChangePosition();

		tWidgetListIt it = mlstChildren.begin();
		for(; it != mlstChildren.end(); ++it)
		{
			iWidget *pChild = *it;
			pChild->SetPositionUpdated();
		}
	}

	//-----------------------------------------------------------------------


}
