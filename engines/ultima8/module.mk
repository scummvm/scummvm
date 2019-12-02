MODULE := engines/ultima8

MODULE_OBJS := \
	detection.o \
	ultima8.o \
	conf/convert_shape.o \
	conf/convert_shape_u8.o \
	conf/convert_shape_crusader.o \
	conf/config_file_manager.o \
	conf/ini_file.o \
	conf/setting_manager.o \
	convert/convert_shape.o \
	convert/u8/convert_shape_u8.o \
	convert/crusader/convert_shape_crusader.o \
	filesys/archive.o \
	filesys/archive_file.o \
	filesys/data.o \
	filesys/dir_file.o \
	filesys/file_system.o \
	filesys/flex_file.o \
	filesys/list_files.o \
	filesys/output_logger.o \
	filesys/raw_archive.o \
	filesys/savegame.o \
	filesys/savegame_writer.o \
	filesys/u8_save_file.o \
	filesys/zip_file.o \
	kernel/allocator.o \
	kernel/core_app.o \
	kernel/delay_process.o \
	kernel/gui_app.o \
	kernel/hid_keys.o \
	kernel/hid_manager.o \
	kernel/joystick.o \
	kernel/kernel.o \
	kernel/memory_manager.o \
	kernel/mouse.o \
	kernel/object.o \
	kernel/object_manager.o \
	kernel/pool.o \
	kernel/process.o \
	kernel/segmented_allocator.o \
	kernel/segmented_pool.o \
	misc/args.o \
	misc/console.o \
	misc/encoding.o \
	misc/id_man.o \
	misc/istring.o \
	misc/md5.o \
	misc/pent_include.o \
	misc/util.o \
	misc/version.o \
	std/misc.o \
	std/string.o \
	usecode/bit_set.o \
	usecode/uc_list.o \
	usecode/uc_machine.o \
	usecode/uc_process.o \
	usecode/uc_stack.o \
	usecode/usecode.o \
	usecode/usecode_flex.o \
	world/camera_process.o \
	world/container.o \
	world/create_item_process.o \
	world/current_map.o \
	world/destroy_item_process.o \
	world/egg.o \
	world/egg_hatcher_process.o \
	world/fireball_process.o \
	world/get_object.o \
	world/glob_egg.o \
	world/gravity_process.o \
	world/item.o \
	world/item_factory.o \
	world/item_sorter.o \
	world/map.o \
	world/map_glob.o \
	world/missile_process.o \
	world/missile_tracker.o \
	world/monster_egg.o \
	world/split_item_process.o \
	world/sprite_process.o \
	world/teleport_egg.o \
	world/world.o \
	world/actors/actor.o \
	world/actors/actor_anim_process.o \
	world/actors/actor_bark_notify_process.o \
	world/actors/ambush_process.o \
	world/actors/animation.o \
	world/actors/animation_tracker.o \
	world/actors/anim_action.o \
	world/actors/avatar_death_process.o \
	world/actors/avatar_gravity_process.o \
	world/actors/avatar_mover_process.o \
	world/actors/clear_feign_death_process.o \
	world/actors/combat_process.o \
	world/actors/grant_peace_process.o \
	world/actors/heal_process.o \
	world/actors/loiter_process.o \
	world/actors/main_actor.o \
	world/actors/pathfinder.o \
	world/actors/pathfinder_process.o \
	world/actors/quick_avatar_mover_process.o \
	world/actors/resurrection_process.o \
	world/actors/scheduler_process.o \
	world/actors/targeted_anim_process.o \
	world/actors/teleport_to_egg_process.o \


# This module can be built as a plugin
ifeq ($(ENABLE_ULTIMA8), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# graphics/AnimDat.o \
# graphics/BaseSoftRenderSurface.o \
# graphics/FrameID.o \
# graphics/GumpShapeArchive.o \
# graphics/InverterProcess.o \
# graphics/MainShapeArchive.o \
# graphics/Palette.o \
# graphics/PaletteFaderProcess.o \
# graphics/PaletteManager.o \
# graphics/PNGWriter.o \
# graphics/RenderSurface.o \
# graphics/ScalerManager.o \
# graphics/Shape.o \
# graphics/ShapeArchive.o \
# graphics/ShapeFrame.o \
# graphics/ShapeInfo.o \
# graphics/SKFPlayer.o \
# graphics/SoftRenderSurface.o \
# graphics/Texture.o \
# graphics/TextureBitmap.o \
# graphics/TexturePNG.o \
# graphics/TextureTarga.o \
# graphics/TypeFlags.o \
# graphics/WpnOvlayDat.o \
# graphics/XFormBlend.o \
# graphics/fonts/FixedWidthFont.o \
# graphics/fonts/Font.o \
# graphics/fonts/FontManager.o \
# graphics/fonts/FontShapeArchive.o \
# graphics/fonts/JPFont.o \
# graphics/fonts/JPRenderedText.o \
# graphics/fonts/RenderedText.o \
# graphics/fonts/ShapeFont.o \
# graphics/fonts/ShapeRenderedText.o \
# graphics/fonts/TTFont.o \
# graphics/fonts/TTFRenderedText.o \
