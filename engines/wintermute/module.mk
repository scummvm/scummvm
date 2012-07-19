MODULE := engines/wintermute
 
MODULE_OBJS := \
	ad/AdActor.o \
	ad/AdEntity.o \
	ad/AdGame.o \
	ad/AdInventory.o \
	ad/AdInventoryBox.o \
	ad/AdItem.o \
	ad/AdLayer.o \
	ad/AdNodeState.o \
	ad/AdObject.o \
	ad/AdPath.o \
	ad/AdPathPoint.o \
	ad/AdRegion.o \
	ad/AdResponse.o \
	ad/AdResponseBox.o \
	ad/AdResponseContext.o \
	ad/AdRotLevel.o \
	ad/AdScaleLevel.o \
	ad/AdScene.o \
	ad/AdSceneNode.o \
	ad/AdSceneState.o \
	ad/AdSentence.o \
	ad/AdSpriteSet.o \
	ad/AdTalkDef.o \
	ad/AdTalkHolder.o \
	ad/AdTalkNode.o \
	ad/AdWaypointGroup.o \
	base/scriptables/ScEngine.o \
	base/scriptables/ScScript.o \
	base/scriptables/ScStack.o \
	base/scriptables/ScValue.o \
	base/scriptables/SXArray.o \
	base/scriptables/SXDate.o \
	base/scriptables/SXFile.o \
	base/scriptables/SXMath.o \
	base/scriptables/SxObject.o \
	base/scriptables/SXMemBuffer.o \
	base/scriptables/SXString.o \
	base/file/BDiskFile.o \
	base/file/BFile.o \
	base/file/BFileEntry.o \
	base/file/BPackage.o \
	base/file/BSaveThumbFile.o \
	base/file/BPkgFile.o \
	base/font/BFontBitmap.o \
	base/font/BFontTT.o \
	base/font/BFont.o \
	base/font/BFontStorage.o \
	base/gfx/base_image.o \
	base/gfx/base_renderer.o \
	base/gfx/base_surface.o \
	base/gfx/osystem/base_surface_osystem.o \
	base/gfx/osystem/base_render_osystem.o \
	base/particles/PartParticle.o \
	base/particles/PartEmitter.o \
	base/particles/PartForce.o \
	base/BActiveRect.o \
	base/BBase.o \
	base/BDebugger.o \
	base/BDynBuffer.o \
	base/BFader.o \
	base/BFileManager.o \
	base/BGame.o \
	base/BFrame.o \
	base/BKeyboardState.o \
	base/BNamedObject.o \
	base/BObject.o \
	base/BParser.o \
	base/BPersistMgr.o \
	base/BPoint.o \
	base/BQuickMsg.o \
	base/BRegion.o \
	base/BRegistry.o \
	base/BResources.o \
	base/BSaveThumbHelper.o \
	base/BScriptable.o \
	base/BScriptHolder.o \
	base/BSound.o \
	base/BSoundBuffer.o \
	base/BSoundMgr.o \
	base/BSprite.o \
	base/BStringTable.o \
	base/BSubFrame.o \
	base/BSurfaceStorage.o \
	base/BTransitionMgr.o \
	base/BViewport.o \
	detection.o \
	graphics/transparentSurface.o \
	graphics/tga.o \
	math/MathUtil.o \
	math/Matrix4.o \
	math/Vector2.o \
	platform_osystem.o \
	system/SysClass.o \
	system/SysClassRegistry.o \
	system/SysInstance.o \
	tinyxml/tinyxml.o \
	tinyxml/tinystr.o \
	tinyxml/tinyxmlparser.o \
	tinyxml/tinyxmlerror.o \
	ui/UIButton.o \
	ui/UIEdit.o \
	ui/UIEntity.o \
	ui/UIObject.o \
	ui/UIText.o \
	ui/UITiledImage.o \
	ui/UIWindow.o \
	utils/ConvertUTF.o \
	utils/crc.o \
	utils/PathUtil.o \
	utils/StringUtil.o \
	utils/utils.o \
	video/VidPlayer.o \
	video/VidTheoraPlayer.o \
	video/decoders/theora_decoder.o \
	wintermute.o \
	persistent.o
 
MODULE_DIRS += \
	engines/wintermute
 
# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
