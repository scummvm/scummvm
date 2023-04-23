MODULE := engines/watchmaker

MODULE_OBJS = \
	3d/animation.o \
	3d/dds_header.o \
	3d/geometry.o \
	3d/light.o \
	3d/loader.o \
	3d/material.o \
	3d/math/llmath.o \
	3d/math/Matrix4x4.o \
	3d/mem_management.o \
	3d/movie.o \
	3d/render/opengl.o \
	3d/render/opengl_2d.o \
	3d/render/opengl_3d.o \
	3d/render/opengl_renderer.o \
	3d/render/opengl_texture.o \
	3d/render/render.o \
	3d/render/shadows.o \
	3d/t3d_body.o \
	3d/t3d_face.o \
	3d/t3d_mesh.o \
	3d/t3d_mesh.o \
	3d/texture.o \
	3d/types3d.o \
	3d/vertex.o \
	\
	classes/do_action.o \
	classes/do_anim.o \
	classes/do_camera.o \
	classes/do_dialog.o \
	classes/do_inv.o \
	classes/do_inv_inv.o \
	classes/do_inv_scr.o \
	classes/do_keyboard.o \
	classes/do_mouse.o \
	classes/do_operate.o \
	classes/do_player.o \
	classes/do_scr_scr.o \
	classes/do_sound.o \
	classes/do_string.o \
	classes/do_system.o \
	\
	init/nl_init.o \
	init/nl_parse.o \
	\
	ll/ll_anim.o \
	ll/ll_diary.o \
	ll/ll_ffile.o \
	ll/ll_mesh.o \
	ll/ll_mouse.o \
	ll/ll_regen.o \
	ll/ll_sound.o \
	ll/ll_string.o \
	ll/ll_system.o \
	ll/ll_util.o \
	\
	t2d/expr.o \
	t2d/t2d.o \
	\
	walk/act.o \
	walk/ball.o \
	walk/walk.o \
	walk/walkutil.o \
	\
	2d_stuff.o \
	console.o \
	extraLS.o \
	file_utils.o \
	fonts.o \
	game.o \
	game_options.o \
	globvar.o \
	main.o \
	metaengine.o \
	rect.o \
	render.o \
	renderer.o \
	saveload.o \
	schedule.o \
	sdl_wrapper.o \
	tga_util.o \
	watchmaker.o \
	windows_hacks.o \
	work_dirs.o



# This module can be built as a plugin
ifeq ($(ENABLE_WATCHMAKER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
