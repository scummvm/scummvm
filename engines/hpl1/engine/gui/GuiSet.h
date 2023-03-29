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

#ifndef HPL_GUI_SET_H
#define HPL_GUI_SET_H

#include "common/list.h"
#include "hpl1/engine/graphics/GraphicsTypes.h"
#include "hpl1/engine/gui/GuiTypes.h"
#include "hpl1/std/multiset.h"
#include "hpl1/std/set.h"

namespace hpl {

//---------------------------------------------

class cResources;
class cGraphics;
class cSound;
class cScene;

class FontData;

class cGui;
class cGuiSkin;
class iGuiMaterial;
class iGuiPopUp;
class iWidget;

class cWidgetWindow;
class cWidgetFrame;
class cWidgetButton;
class cWidgetLabel;
class cWidgetSlider;
class cWidgetTextBox;
class cWidgetCheckBox;
class cWidgetImage;
class cWidgetListBox;
class cWidgetComboBox;

//---------------------------------------------

typedef Common::List<iGuiPopUp *> tGuiPopUpList;
typedef tGuiPopUpList::iterator tGuiPopUpListIt;

//---------------------------------------------

class cGuiClipRegion;
class cGuiRenderObject {
public:
	cGuiGfxElement *mpGfx;
	cVector3f mvPos;
	cVector2f mvSize;
	cColor mColor;
	iGuiMaterial *mpCustomMaterial;
	cGuiClipRegion *mpClipRegion;
};

class cGuiRenderObjectCompare {
public:
	bool operator()(const cGuiRenderObject &aObjectA, const cGuiRenderObject &aObjectB) const;
};

typedef Hpl1::Std::multiset<cGuiRenderObject, cGuiRenderObjectCompare> tGuiRenderObjectSet;
typedef tGuiRenderObjectSet::iterator tGuiRenderObjectSetIt;

//-----------------------------------------------

typedef Common::List<cGuiClipRegion *> tGuiClipRegionList;
typedef tGuiClipRegionList::iterator tGuiClipRegionListIt;

class cGuiClipRegion {
public:
	cGuiClipRegion() : mRect(0, 0, -1, -1) {}
	~cGuiClipRegion();

	void Clear();
	cGuiClipRegion *CreateChild(const cVector3f &avPos, const cVector2f &avSize);

	// tGuiRenderObjectSet m_setObjects;
	cRect2f mRect;

	tGuiClipRegionList mlstChildren;
};

//-----------------------------------------------

class cGuiSet {
public:
	cGuiSet(const tString &asName, cGui *apGui, cGuiSkin *apSkin,
			cResources *apResources, cGraphics *apGraphics,
			cSound *apSound, cScene *apScene);
	~cGuiSet();

	////////////////////////////////////
	// General

	void Update(float afTimeStep);

	void DrawAll(float afTimeStep);

	bool SendMessage(eGuiMessage aMessage, cGuiMessageData &aData);

	////////////////////////////////////
	// Rendering
	void Render();

	void SetDrawOffset(const cVector3f &avOffset) { mvDrawOffset = avOffset; }
	void SetCurrentClipRegion(cGuiClipRegion *apRegion) { mpCurrentClipRegion = apRegion; }

	void DrawGfx(cGuiGfxElement *apGfx,
				 const cVector3f &avPos,
				 const cVector2f &avSize = -1,
				 const cColor &aColor = cColor(1, 1),
				 eGuiMaterial aMaterial = eGuiMaterial_LastEnum);
	void DrawFont(const tWString &asText,
				  FontData *apFont, const cVector3f &avPos,
				  const cVector2f &avSize, const cColor &aColor,
				  eFontAlign aAlign = eFontAlign_Left,
				  eGuiMaterial aMaterial = eGuiMaterial_FontNormal);

	////////////////////////////////////
	// Widget Creation
	cWidgetWindow *CreateWidgetWindow(const cVector3f &avLocalPos = 0,
									  const cVector2f &avSize = 0,
									  const tWString &asText = {},
									  iWidget *apParent = NULL,
									  const tString &asName = "");

	cWidgetFrame *CreateWidgetFrame(const cVector3f &avLocalPos = 0,
									const cVector2f &avSize = 0,
									bool abDrawFrame = false,
									iWidget *apParent = NULL,
									const tString &asName = "");

	cWidgetButton *CreateWidgetButton(const cVector3f &avLocalPos = 0,
									  const cVector2f &avSize = 0,
									  const tWString &asText = {},
									  iWidget *apParent = NULL,
									  const tString &asName = "");

	cWidgetLabel *CreateWidgetLabel(const cVector3f &avLocalPos = 0,
									const cVector2f &avSize = 0,
									const tWString &asText = {},
									iWidget *apParent = NULL,
									const tString &asName = "");

	cWidgetSlider *CreateWidgetSlider(eWidgetSliderOrientation aOrientation,
									  const cVector3f &avLocalPos = 0,
									  const cVector2f &avSize = 0,
									  int alMaxValue = 10,
									  iWidget *apParent = NULL,
									  const tString &asName = "");

	cWidgetTextBox *CreateWidgetTextBox(const cVector3f &avLocalPos = 0,
										const cVector2f &avSize = 0,
										const tWString &asText = {},
										iWidget *apParent = NULL,
										const tString &asName = "");

	cWidgetCheckBox *CreateWidgetCheckBox(const cVector3f &avLocalPos = 0,
										  const cVector2f &avSize = 0,
										  const tWString &asText = {},
										  iWidget *apParent = NULL,
										  const tString &asName = "");

	cWidgetImage *CreateWidgetImage(const tString &asFile = "",
									const cVector3f &avLocalPos = 0,
									const cVector2f &avSize = -1,
									eGuiMaterial aMaterial = eGuiMaterial_Alpha,
									bool abAnimate = false,
									iWidget *apParent = NULL,
									const tString &asName = "");

	cWidgetListBox *CreateWidgetListBox(const cVector3f &avLocalPos = 0,
										const cVector2f &avSize = 0,
										iWidget *apParent = NULL,
										const tString &asName = "");

	cWidgetComboBox *CreateWidgetComboBox(const cVector3f &avLocalPos = 0,
										  const cVector2f &avSize = 0,
										  const tWString &asText = {},
										  iWidget *apParent = NULL,
										  const tString &asName = "");

	iWidget *GetWidgetFromName(const tString &asName);

	void DestroyWidget(iWidget *apWidget);

	////////////////////////////////////
	// Popup
	void CreatePopUpMessageBox(const tWString &asLabel, const tWString &asText,
							   const tWString &asButton1, const tWString &asButton2,
							   void *apCallbackObject, tGuiCallbackFunc apCallback);

	void DestroyPopUp(iGuiPopUp *apPopUp);

	////////////////////////////////////
	// Properties
	void SetActive(bool abX);
	bool IsActive() { return mbActive; }

	cGui *GetGui() { return mpGui; }

	void SetDrawMouse(bool abX);
	bool GetDrawMouse() { return mbDrawMouse; }

	void SetMouseZ(float afZ) { mfMouseZ = afZ; }
	float GetMouse() { return mfMouseZ; }

	void SetRootWidgetClips(bool abX);
	bool GetRootWidgetClips();

	void SetVirtualSize(const cVector2f &avSize, float afMinZ, float afMaxZ);
	const cVector2f &GetVirtualSize() { return mvVirtualSize; }

	void SetFocusedWidget(iWidget *apWidget);
	iWidget *GetFocusedWidget() { return mpFocusedWidget; }

	void SetAttentionWidget(iWidget *apWidget);
	iWidget *GetAttentionWidget() { return mpAttentionWidget; }

	void SetIs3D(bool abX);
	bool Is3D() { return mbIs3D; }

	int GetDrawPriority() { return mlDrawPrio; }
	void SetDrawPriority(int alPrio) { mlDrawPrio = alPrio; }

	void SetCurrentPointer(cGuiGfxElement *apGfx);
	cGuiGfxElement *GetCurrentPointer() { return mpGfxCurrentPointer; }

	void Set3DSize(const cVector3f &avSize);
	cVector3f Get3DSize() { return mv3DSize; }

	void SetCullBackface(bool abX) { mbCullBackface = abX; }
	bool GetCullBackface() { return mbCullBackface; }

	void Set3DTransform(const cMatrixf &a_mtxTransform);
	cMatrixf Get3DTransform() { return m_mtx3DTransform; }

	bool HasFocus();

	void SetSkin(cGuiSkin *apSkin);
	cGuiSkin *GetSkin() { return mpSkin; }

	cResources *GetResources() { return mpResources; }

	bool IsDestroyingSet() { return mbDestroyingSet; }

private:
	void RenderClipRegion();

	void AddWidget(iWidget *apWidget, iWidget *apParent);

	bool OnMouseMove(cGuiMessageData &aData);
	bool OnMouseDown(cGuiMessageData &aData);
	bool OnMouseUp(cGuiMessageData &aData);
	bool OnMouseDoubleClick(cGuiMessageData &aData);

	bool OnKeyPress(cGuiMessageData &aData);

	bool DrawMouse(iWidget *apWidget, cGuiMessageData &aData);
	kGuiCalllbackDeclarationEnd(DrawMouse);

	cGui *mpGui;
	cGuiSkin *mpSkin;

	tString msName;

	cResources *mpResources;
	cGraphics *mpGraphics;
	cSound *mpSound;
	cScene *mpScene;

	iWidget *mpAttentionWidget;

	iWidget *mpFocusedWidget;

	iWidget *mpWidgetRoot;
	tWidgetList mlstWidgets;

	tGuiRenderObjectSet m_setRenderObjects;

	cVector2f mvVirtualSize;
	float mfVirtualMinZ;
	float mfVirtualMaxZ;

	cVector3f mvDrawOffset;

	bool mbCullBackface;
	bool mbIs3D;
	cVector3f mv3DSize;
	cMatrixf m_mtx3DTransform;
	int mlDrawPrio;

	bool mbActive;
	bool mbDrawMouse;
	float mfMouseZ;
	cGuiGfxElement *mpGfxCurrentPointer;

	bool mvMouseDown[3];
	cVector2f mvMousePos;

	tGuiPopUpList mlstPopUps;

	cGuiClipRegion mBaseClipRegion;
	cGuiClipRegion *mpCurrentClipRegion;

	bool mbDestroyingSet;
};

} // namespace hpl

#endif // HPL_GUI_SET_H
