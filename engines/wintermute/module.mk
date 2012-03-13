MODULE := engines/wintermute
 
MODULE_OBJS := \
	scriptables/ScEngine.o \
	scriptables/ScScript.o \
	scriptables/ScStack.o \
	scriptables/ScValue.o \
	scriptables/SXArray.o \
	scriptables/SXDate.o \
	scriptables/SXFile.o \
	scriptables/SXMath.o \
	scriptables/SxObject.o \
	scriptables/SXMemBuffer.o \
	scriptables/SXStore.o \
	scriptables/SXString.o \
	AdActor.o \
	AdActorDir.o \
	AdEntity.o \
	AdGame.o \
	AdInventory.o \
	AdInventoryBox.o \
	AdItem.o \
	AdLayer.o \
	AdNodeState.o \
	AdObject.o \
	AdPath.o \
	AdPathPoint.o \
	AdRegion.o \
	AdResponse.o \
	AdResponseBox.o \
	AdRotLevel.o \
	AdScaleLevel.o \
	AdScene.o \
	AdSceneNode.o \
	AdSceneState.o \
	AdSentence.o \
	AdSpriteSet.o \
	AdTalkDef.o \
	AdTalkHolder.o \
	AdTalkNode.o \
	AdWaypointGroup.o \
	BActiveRect.o \
	BBase.o \
	BDebugger.o \
	BDynBuffer.o \
	BFader.o \
	BFile.o \
	BFileEntry.o \
	BFileManager.o \
	BGame.o \
	BFont.o \
	BFontBitmap.o \
	BFontStorage.o \
	BFontTT.o \
	BFrame.o \
	BImage.o \
	BNamedObject.o \
	BObject.o \
	BPackage.o \
	BParser.o \
	BPersistMgr.o \
	BPkgFile.o \
	BPoint.o \
	BQuickMsg.o \
	BRegion.o \
	BRegistry.o \
	BResources.o \
	BResourceFile.o \
	BSaveThumbFile.o \
	BScriptable.o \
	BScriptHolder.o \
	BSound.o \
	BSoundBuffer.o \
	BSoundMgr.o \
	BSprite.o \
	BStringTable.o \
	BSubFrame.o \
	BSurface.o \
	BSurfaceSDL.o \
	BSurfaceStorage.o \
	BTransitionMgr.o \
	BViewport.o \
	ConvertUTF.o \
	crc.o \
	detection.o \
	FontGlyphCache.o \
	MathUtil.o \
	PathUtil.o \
	PlatformSDL.o \
	StringUtil.o \
	SysClass.o \
	SysClassRegistry.o \
	SysInstance.o \
	tinyxml.o \
	tinystr.o \
	UIButton.o \
	UIEdit.o \
	UIEntity.o \
	UIObject.o \
	UIText.o \
	UITiledImage.o \
	UIWindow.o \
	utils.o \
	Vector2.o \
	wintermute.o
 
MODULE_DIRS += \
	engines/wintermute
 
# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
