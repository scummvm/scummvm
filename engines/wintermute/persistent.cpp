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

#include "engines/wintermute/ad/AdActor.h"
#include "engines/wintermute/ad/AdEntity.h"
#include "engines/wintermute/ad/AdGame.h"
#include "engines/wintermute/ad/AdInventory.h"
#include "engines/wintermute/ad/AdInventoryBox.h"
#include "engines/wintermute/ad/AdItem.h"
#include "engines/wintermute/ad/AdLayer.h"
#include "engines/wintermute/ad/AdNodeState.h"
#include "engines/wintermute/ad/AdObject.h"
#include "engines/wintermute/ad/AdPath.h"
#include "engines/wintermute/ad/AdPathPoint.h"
#include "engines/wintermute/ad/AdRegion.h"
#include "engines/wintermute/ad/AdResponse.h"
#include "engines/wintermute/ad/AdResponseBox.h"
#include "engines/wintermute/ad/AdResponseContext.h"
#include "engines/wintermute/ad/AdRotLevel.h"
#include "engines/wintermute/ad/AdScaleLevel.h"
#include "engines/wintermute/ad/AdScene.h"
#include "engines/wintermute/ad/AdSceneNode.h"
#include "engines/wintermute/ad/AdSceneState.h"
#include "engines/wintermute/ad/AdSentence.h"
#include "engines/wintermute/ad/AdSpriteSet.h"
#include "engines/wintermute/ad/AdTalkDef.h"
#include "engines/wintermute/ad/AdTalkHolder.h"
#include "engines/wintermute/ad/AdTalkNode.h"
#include "engines/wintermute/ad/AdWaypointGroup.h"
#include "engines/wintermute/base/BFader.h"
#include "engines/wintermute/base/font/BFontBitmap.h"
#include "engines/wintermute/base/font/BFontStorage.h"
#include "engines/wintermute/base/font/BFontTT.h"
#include "engines/wintermute/base/BFrame.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/BKeyboardState.h"
#include "engines/wintermute/base/BObject.h"
#include "engines/wintermute/base/BPoint.h"
#include "engines/wintermute/base/BRegion.h"
#include "engines/wintermute/base/BScriptable.h"
#include "engines/wintermute/base/BScriptHolder.h"
#include "engines/wintermute/base/BSound.h"
#include "engines/wintermute/base/BSprite.h"
#include "engines/wintermute/base/BSubFrame.h"
#include "engines/wintermute/base/BViewport.h"
#include "engines/wintermute/base/scriptables/ScEngine.h"
#include "engines/wintermute/base/scriptables/ScScript.h"
#include "engines/wintermute/base/scriptables/ScStack.h"
#include "engines/wintermute/base/scriptables/ScValue.h"
#include "engines/wintermute/base/scriptables/SXArray.h"
#include "engines/wintermute/base/scriptables/SXDate.h"
#include "engines/wintermute/base/scriptables/SXFile.h"
#include "engines/wintermute/base/scriptables/SXMath.h"
#include "engines/wintermute/base/scriptables/SXMemBuffer.h"
#include "engines/wintermute/base/scriptables/SxObject.h"
#include "engines/wintermute/base/scriptables/SXString.h"
#include "engines/wintermute/ui/UIButton.h"
#include "engines/wintermute/ui/UIEdit.h"
#include "engines/wintermute/ui/UIEntity.h"
#include "engines/wintermute/ui/UIText.h"
#include "engines/wintermute/ui/UITiledImage.h"
#include "engines/wintermute/ui/UIWindow.h"
#include "engines/wintermute/video/VidTheoraPlayer.h"

// CSysClass adds these objects to the registry, thus they aren't as leaked as they look
#define REGISTER_CLASS(class_name, persistent_class)\
	new WinterMute::CSysClass(class_name::_className, class_name::persistBuild, class_name::persistLoad, persistent_class);

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
