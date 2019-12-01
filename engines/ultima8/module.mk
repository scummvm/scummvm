MODULE := engines/ultima8

MODULE_OBJS := \
	detection.o \
	ultima8.o \
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
	std/string.o

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
