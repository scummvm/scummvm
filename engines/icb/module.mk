MODULE := engines/icb

MODULE_OBJS := \
	actor.o \
	actor_fx_pc.o \
	actor_pc.o \
	actor_view_pc.o \
	animation.o \
	animation_mega_set.o \
	async_generic.o \
	barriers.o \
	bink_stub.o \
	bone.o \
	breath.o \
	camera.o \
	chi.o \
	cluster_manager_pc.o \
	configfile.o \
	console_pc.o \
	custom_logics.o \
	debug.o \
	debug_pc.o \
	detection.o \
	direct_input.o \
	drawpoly_pc.o \
	event_list.o \
	event_manager.o \
	floors.o \
	fn_animation.o \
	fn_event_functions.o \
	fn_fx.o \
	fn_fx_pc.o \
	fn_icon_functions.o \
	fn_interact.o \
	fn_movie_pc.o \
	fn_remora_functions.o \
	fn_routines.o \
	fn_sound.o \
	fn_sting_pc.o \
	footstep.o \
	function.o \
	game_script.o \
	game_volume.o \
	general_interaction.o \
	general_npc_animation.o \
	global_objects.o \
	global_objects_pc.o \
	global_objects_psx.o \
	global_switches.o \
	global_vars.o \
	graphic_prims.o \
	graphic_prims_pc.o \
	health_bar_pc.o \
	icb.o \
	icon_list.o \
	icon_list_manager.o \
	icon_menu.o \
	icon_menu_pc.o \
	jpeg.o \
	keyboard.o \
	light_pc.o \
	line_of_sight.o \
	loadscrn_pc.o \
	logic.o \
	main_menu_pc.o \
	map_marker.o \
	map_marker_pc.o \
	mega_interaction.o \
	metaengine.o \
	mission.o \
	mission_functions.o \
	mouse.o \
	mouse_pc.o \
	movie_pc.o \
	non_ad_module.o \
	options_manager_pc.o \
	p4.o \
	p4_pc.o \
	player.o \
	player_interaction.o \
	player_pc.o \
	prim_route_builder.o \
	protocol.o \
	remora.o \
	remora_pc.o \
	remora_sprite.o \
	remora_sprite_pc.o \
	res_man.o \
	res_man_pc.o \
	route_manager.o \
	session.o \
	set.o \
	set_pc.o \
	shade_pc.o \
	shadow_pc.o \
	shake.o \
	softskin_pc.o \
	sound.o \
	sound_logic.o \
	sound_logic_entry.o \
	sound_lowlevel_pc.o \
	speech.o \
	speech_pc.o \
	stagedraw_pc_poly.o \
	stage_poly.o \
	string_vest.o \
	surface_manager.o \
	text_pc.o \
	text_sprites.o \
	text_sprites_pc.o \
	timer_func.o \
	tracer.o \
	zsupport.o \
	common/datapacker.o \
	common/ptr_util.o \
	common/px_2drealline.o \
	common/px_bones.o \
	common/px_capri_maths.o \
	common/px_capri_maths_pc.o \
	common/px_clu_api.o \
	common/px_globalvariables.o \
	common/px_linkeddatafile.o \
	common/px_scriptengine.o \
	common/px_string.o \
	gfx/gfxstub.o \
	gfx/gfxstub_dutch.o \
	gfx/gfxstub_ogl.o \
	gfx/gfxstub_rev.o \
	gfx/psx_camera.o \
	gfx/psx_pcgpu.o \
	gfx/psx_tman.o \
	gfx/rab_api.o \
	sound/direct_sound.o \
	sound/fx_manager.o \
	sound/music_manager.o \
	sound/sound_common.o \
	sound/speech_manager.o

# This module can be built as a plugin
ifeq ($(ENABLE_ICB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
