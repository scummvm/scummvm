MODULE := engines/pink

MODULE_OBJS = \
	pink.o \
	console.o \
	detection.o \
	director.o \
	sound.o \
	file.o \
	archive.o \
	cursor_mgr.o \
    objects/object.o \
    objects/module.o \
    objects/pages/page.o \
    objects/pages/game_page.o \
    objects/inventory.o \
    objects/side_effect.o \
    objects/condition.o \
    resource_mgr.o \
    objects/actions/action.o \
    objects/actions/action_cel.o \
    objects/actions/action_hide.o \
    objects/actions/action_play.o \
    objects/actions/action_sound.o \
    objects/actions/action_still.o \
    objects/actors/actor.o \
    objects/actors/lead_actor.o \
    objects/walk/walk_mgr.o \
    objects/walk/walk_location.o \
    objects/sequences/sequence.o \
    objects/sequences/sequence_item.o \
    objects/sequences/sequencer.o \
    objects/handlers/handler.o \
    objects/handlers/handler_timer.o \


# This module can be built as a plugin
ifeq ($(ENABLE_PLUMBERS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk