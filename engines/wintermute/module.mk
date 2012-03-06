MODULE := engines/wintermute
 
MODULE_OBJS := \
	scriptables/ScScript.o \
	scriptables/ScStack.o \
	scriptables/ScValue.o \
	scriptables/SXArray.o \
	scriptables/SXDate.o \
	scriptables/SXMath.o \
	scriptables/SXMemBuffer.o \
	BActiveRect.o \
	BDebugger.o \
	BDynBuffer.o \
	BBase.o \
	BFader.o \
	BFile.o \
	BFileEntry.o \
	BFrame.o \
	BImage.o \
	BNamedObject.o \
	BPackage.o \
	BParser.o \
	BPersistMgr.o \
	BPoint.o \
	BQuickMsg.o \
	BRegion.o \
	BResources.o \
	BResourceFile.o \
	BSaveThumbFile.o \
	BScriptable.o \
	BSound.o \
	BSoundBuffer.o \
	BSoundMgr.o \
	BSprite.o \
	BSubFrame.o \
	BSurfaceStorage.o \
	BTransitionMgr.o \
	BViewport.o \
	ConvertUTF.o \
	detection.o \
	PathUtil.o \
	StringUtil.o \
	SysClass.o \
	SysInstance.o \
	UIButton.o \
	UIEntity.o \
	UIObject.o \
	UITiledImage.o \
	utils.o \
	wintermute.o
 
MODULE_DIRS += \
	engines/wintermute
 
# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
