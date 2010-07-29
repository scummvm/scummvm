MODULE := backends/platform/wince

MODULE_OBJS := \
	CEActionsPocket.o \
	CEDevice.o \
	CEScaler.o \
	CEActionsSmartphone.o \
	CELauncherDialog.o \
	wince-sdl.o \
	CEgui/GUIElement.o \
	CEgui/Panel.o \
	CEgui/SDL_ImageResource.o \
	CEgui/ItemAction.o \
	CEgui/PanelItem.o \
	CEgui/Toolbar.o \
	CEgui/ItemSwitch.o \
	CEgui/PanelKeyboard.o \
	CEgui/ToolbarHandler.o \
	CEkeys/EventsBuffer.o \
	../../../gui/Actions.o \
	../../../gui/Key.o \
	../../../gui/KeysDialog.o \
	../sdl/sdl.o \
	../sdl/graphics.o \
	../sdl/events.o \
	../sdl/hardwarekeys.o \
	missing/missing.o \
	PocketSCUMM.o \
	smartLandScale.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
