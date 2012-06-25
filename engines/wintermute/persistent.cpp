/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/Ad/AdActor.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdInventory.h"
#include "engines/wintermute/Ad/AdInventoryBox.h"
#include "engines/wintermute/Ad/AdItem.h"
#include "engines/wintermute/Ad/AdLayer.h"
#include "engines/wintermute/Ad/AdNodeState.h"
#include "engines/wintermute/Ad/AdObject.h"
#include "engines/wintermute/Ad/AdPath.h"
#include "engines/wintermute/Ad/AdPathPoint.h"
#include "engines/wintermute/Ad/AdRegion.h"
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/Ad/AdResponseBox.h"
#include "engines/wintermute/Ad/AdResponseContext.h"
#include "engines/wintermute/Ad/AdRotLevel.h"
#include "engines/wintermute/Ad/AdScaleLevel.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdSceneNode.h"
#include "engines/wintermute/Ad/AdSceneState.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Ad/AdSpriteSet.h"
#include "engines/wintermute/Ad/AdTalkDef.h"
#include "engines/wintermute/Ad/AdTalkHolder.h"
#include "engines/wintermute/Ad/AdTalkNode.h"
#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Base/BFader.h"
#include "engines/wintermute/Base/BFontBitmap.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFontTT.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BKeyboardState.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BPoint.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BScriptable.h"
#include "engines/wintermute/Base/BScriptHolder.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/Base/scriptables/ScEngine.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/SXArray.h"
#include "engines/wintermute/Base/scriptables/SXDate.h"
#include "engines/wintermute/Base/scriptables/SXFile.h"
#include "engines/wintermute/Base/scriptables/SXMath.h"
#include "engines/wintermute/Base/scriptables/SXMemBuffer.h"
#include "engines/wintermute/Base/scriptables/SxObject.h"
#include "engines/wintermute/Base/scriptables/SXStore.h"
#include "engines/wintermute/Base/scriptables/SXString.h"
#include "engines/wintermute/UI/UIButton.h"
#include "engines/wintermute/UI/UIEdit.h"
#include "engines/wintermute/UI/UIEntity.h"
#include "engines/wintermute/UI/UIText.h"
#include "engines/wintermute/UI/UITiledImage.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/video/VidTheoraPlayer.h"

// CSysClass adds these objects to the registry, thus they aren't as leaked as they look
#define REGISTER_CLASS(class_name, persistent_class)\
	new WinterMute::CSysClass(class_name::_className, class_name::PersistBuild, class_name::PersistLoad, persistent_class);

namespace WinterMute {

// This is done in a separate file, to avoid including the kitchensink in CSysClassRegistry.
void CSysClassRegistry::registerClasses() {
	REGISTER_CLASS(CAdActor, false)
	REGISTER_CLASS(CAdEntity, false)
	REGISTER_CLASS(CAdGame, true)
	REGISTER_CLASS(CAdInventory, false)
	REGISTER_CLASS(CAdInventoryBox, false)
	REGISTER_CLASS(CAdItem, false)
	REGISTER_CLASS(CAdLayer, false)
	REGISTER_CLASS(CAdNodeState, false)
	REGISTER_CLASS(CAdObject, false)
	REGISTER_CLASS(CAdPath, false)
	REGISTER_CLASS(CAdPathPoint, false)
	REGISTER_CLASS(CAdRegion, false)
	REGISTER_CLASS(CAdResponse, false)
	REGISTER_CLASS(CAdResponseBox, false)
	REGISTER_CLASS(CAdResponseContext, false)
	REGISTER_CLASS(CAdRotLevel, false)
	REGISTER_CLASS(CAdScaleLevel, false)
	REGISTER_CLASS(CAdScene, false)
	REGISTER_CLASS(CAdSceneNode, false)
	REGISTER_CLASS(CAdSceneState, false)
	REGISTER_CLASS(CAdSentence, false)
	REGISTER_CLASS(CAdSpriteSet, false)
	REGISTER_CLASS(CAdTalkDef, false)
	REGISTER_CLASS(CAdTalkHolder, false)
	REGISTER_CLASS(CAdTalkNode, false)
	REGISTER_CLASS(CAdWaypointGroup, false)

	REGISTER_CLASS(CBFader, false)
	REGISTER_CLASS(CBFont, false)
	REGISTER_CLASS(CBFontBitmap, false)
	REGISTER_CLASS(CBFontStorage, true)
	REGISTER_CLASS(CBFontTT, false)
	REGISTER_CLASS(CBFrame, false)
	REGISTER_CLASS(CBGame, true)
	REGISTER_CLASS(CBKeyboardState, false)
	REGISTER_CLASS(CBObject, false)
	REGISTER_CLASS(CBPoint, false)
	REGISTER_CLASS(CBRegion, false)
	REGISTER_CLASS(CBScriptable, false)
	REGISTER_CLASS(CBScriptHolder, false)
	REGISTER_CLASS(CBSound, false)
	REGISTER_CLASS(CBSprite, false)
	REGISTER_CLASS(CBSubFrame, false)

	REGISTER_CLASS(CBViewport, false)
	REGISTER_CLASS(CPartEmitter, false)
	REGISTER_CLASS(CScEngine, true)
	REGISTER_CLASS(CScScript, false)
	REGISTER_CLASS(CScStack, false)
	REGISTER_CLASS(CScValue, false)
	REGISTER_CLASS(CSXArray, false)
	REGISTER_CLASS(CSXDate, false)
	REGISTER_CLASS(CSXFile, false)
	REGISTER_CLASS(CSXMath, true)
	REGISTER_CLASS(CSXMemBuffer, false)
	REGISTER_CLASS(CSXObject, false)
	REGISTER_CLASS(CSXStore, false)
	REGISTER_CLASS(CSXString, false)

	REGISTER_CLASS(CUIButton, false)
	REGISTER_CLASS(CUIEdit, false)
	REGISTER_CLASS(CUIEntity, false)
	REGISTER_CLASS(CUIObject, false)
	REGISTER_CLASS(CUIText, false)
	REGISTER_CLASS(CUITiledImage, false)
	REGISTER_CLASS(CUIWindow, false)
	REGISTER_CLASS(CVidTheoraPlayer, false)
	warning("TODO: Clean this up at some proper time");
}

}
