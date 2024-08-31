/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "qdengine/qd_fwd.h"
#include "qdengine/parser/qdscr_parser.h"
#include "qdengine/parser/xml_parser.h"

namespace QDEngine {

xml::parser *parser = nullptr;

void cleanup_XML_Parser() {
	delete parser;
	parser = nullptr;
}

xml::parser &qdscr_XML_Parser() {
	if (!parser)
		parser = new xml::parser;

	if (!parser->num_tag_formats()) {
		parser->register_tag_format("qd_script", xml::tag(QDSCR_ROOT, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("ID", xml::tag(QDSCR_ID, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("x", xml::tag(QDSCR_X, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("y", xml::tag(QDSCR_Y, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("sx", xml::tag(QDSCR_SX, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("sy", xml::tag(QDSCR_SY, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("pos_2d", xml::tag(QDSCR_POS2D, xml::tag::TAG_DATA_FLOAT, 2));
		parser->register_tag_format("pos_3d", xml::tag(QDSCR_POS3D, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("src_pos", xml::tag(QDSCR_SRC_POS, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("dest_pos", xml::tag(QDSCR_DEST_POS, xml::tag::TAG_DATA_FLOAT, 3));

		parser->register_tag_format("file", xml::tag(QDSCR_FILE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("start_time", xml::tag(QDSCR_START_TIME, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("period", xml::tag(QDSCR_PERIOD, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("length", xml::tag(QDSCR_LENGTH, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("speed", xml::tag(QDSCR_SPEED, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("animation_speed", xml::tag(QDSCR_ANIMATION_SPEED, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("interpolation_time", xml::tag(QDSCR_INTERPOLATION_TIME, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("scale", xml::tag(QDSCR_SCALE, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("name", xml::tag(QDSCR_NAME, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("flag", xml::tag(QDSCR_FLAG, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("flags", xml::tag(QDSCR_FLAG, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("rnd", xml::tag(QDSCR_RND, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("mask_size", xml::tag(QDSCR_MASK_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("mask_attributes", xml::tag(QDSCR_MASK_ATTRIBUTES, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("mask_heights", xml::tag(QDSCR_MASK_HEIGHTS, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("bound", xml::tag(QDSCR_BOUND, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("type", xml::tag(QDSCR_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("size", xml::tag(QDSCR_SIZE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("state", xml::tag(QDSCR_STATE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("text", xml::tag(QDSCR_TEXT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("short_text", xml::tag(QDSCR_SHORT_TEXT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("value", xml::tag(QDSCR_VALUE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("time", xml::tag(QDSCR_TIME, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("cd", xml::tag(QDSCR_CD, xml::tag::TAG_DATA_UNSIGNED_INT, 1));
		parser->register_tag_format("align", xml::tag(QDSCR_ALIGN, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("valign", xml::tag(QDSCR_VALIGN, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("orientation", xml::tag(QDSCR_ORIENTATION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("color", xml::tag(QDSCR_COLOR, xml::tag::TAG_DATA_UNSIGNED_INT, 1));
		parser->register_tag_format("comment", xml::tag(QDSCR_COMMENT, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("font_info", xml::tag(QDSCR_FONT_INFO, xml::tag::TAG_DATA_VOID, -1));

		parser->register_tag_format("screen_size", xml::tag(QDSCR_SCREEN_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("text_set", xml::tag(QDSCR_TEXT_SET, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("text_color", xml::tag(QDSCR_TEXT_COLOR, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("text_hover_color", xml::tag(QDSCR_TEXT_HOVER_COLOR, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("text_align", xml::tag(QDSCR_TEXT_ALIGN, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("contour_rect", xml::tag(QDSCR_CONTOUR_RECTANGLE, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("contour_circle", xml::tag(QDSCR_CONTOUR_CIRCLE, xml::tag::TAG_DATA_SHORT, 1));
		parser->register_tag_format("contour_polygon", xml::tag(QDSCR_CONTOUR_POLYGON, xml::tag::TAG_DATA_SHORT, -1));

		parser->register_tag_format("named_object", xml::tag(QDSCR_NAMED_OBJECT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("types", xml::tag(QDSCR_NAMED_OBJECT_TYPES, xml::tag::TAG_DATA_INT, -1));

		parser->register_tag_format("grid_zone", xml::tag(QDSCR_GRID_ZONE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("grid_zone_cells", xml::tag(QDSCR_GRID_ZONE_CELLS, xml::tag::TAG_DATA_SHORT, -1));
		parser->register_tag_format("grid_zone_height", xml::tag(QDSCR_GRID_ZONE_HEIGHT, xml::tag::TAG_DATA_UNSIGNED_INT, 1));
		parser->register_tag_format("grid_zone_contour", xml::tag(QDSCR_GRID_ZONE_CONTOUR, xml::tag::TAG_DATA_SHORT, -1));
		parser->register_tag_format("grid_zone_state", xml::tag(QDSCR_GRID_ZONE_STATE, xml::tag::TAG_DATA_VOID, -1));
		parser->register_tag_format("shadow_color", xml::tag(QDSCR_GRID_ZONE_SHADOW_COLOR, xml::tag::TAG_DATA_UNSIGNED_INT, 1));
		parser->register_tag_format("shadow_alpha", xml::tag(QDSCR_GRID_ZONE_SHADOW_ALPHA, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("trigger_element", xml::tag(QDSCR_TRIGGER_ELEMENT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("trigger_element_link", xml::tag(QDSCR_TRIGGER_ELEMENT_LINK, xml::tag::TAG_DATA_INT, 3));
		parser->register_tag_format("parent_links", xml::tag(QDSCR_TRIGGER_ELEMENT_PARENT_LINKS, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("child_links", xml::tag(QDSCR_TRIGGER_ELEMENT_CHILD_LINKS, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("link", xml::tag(QDSCR_TRIGGER_ELEMENT_LINK, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("auto_restart", xml::tag(QDSCR_TRIGGER_ELEMENT_LINK_AUTO_RESTART, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("start_element", xml::tag(QDSCR_TRIGGER_START_ELEMENT, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("trigger_chain", xml::tag(QDSCR_TRIGGER_CHAIN, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("trigger_chain_root", xml::tag(QDSCR_TRIGGER_CHAIN_ROOT, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("sound", xml::tag(QDSCR_SOUND, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("volume", xml::tag(QDSCR_SOUND_VOLUME, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("music_track", xml::tag(QDSCR_MUSIC_TRACK, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("cycled", xml::tag(QDSCR_MUSIC_TRACK_CYCLED, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("video", xml::tag(QDSCR_VIDEO, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("video_position", xml::tag(QDSCR_VIDEO_POSITION, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("video_background", xml::tag(QDSCR_VIDEO_BACKGROUND, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("minigame", xml::tag(QDSCR_MINIGAME, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("dll_name", xml::tag(QDSCR_MINIGAME_DLL_NAME, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("game_name", xml::tag(QDSCR_MINIGAME_GAME_NAME, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("config_file", xml::tag(QDSCR_MINIGAME_CONFIG_FILE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("minigame_state", xml::tag(QDSCR_MINIGAME_STATE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("score", xml::tag(QDSCR_MINIGAME_SCORE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("minigame_config_prm", xml::tag(QDSCR_MINIGAME_CONFIG_PARAMETER, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("animation_frame", xml::tag(QDSCR_ANIMATION_FRAME, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("animation_info", xml::tag(QDSCR_ANIMATION_INFO, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("animation_file", xml::tag(QDSCR_ANIMATION_FILE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("animation", xml::tag(QDSCR_ANIMATION, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("animation_set", xml::tag(QDSCR_ANIMATION_SET, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("start_angle", xml::tag(QDSCR_ANIMATION_SET_START_ANGLE, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("animation_turn", xml::tag(QDSCR_ANIMATION_SET_TURN, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("walk_sound_frequency", xml::tag(QDSCR_OBJECT_STATE_WALK_SOUND_FREQUENCY, xml::tag::TAG_DATA_FLOAT, -1));

		parser->register_tag_format("center_offsets", xml::tag(QDSCR_STATE_CENTER_OFFSETS, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("static_center_offsets", xml::tag(QDSCR_STATE_STATIC_CENTER_OFFSETS, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("start_center_offsets", xml::tag(QDSCR_STATE_START_CENTER_OFFSETS, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("stop_center_offsets", xml::tag(QDSCR_STATE_STOP_CENTER_OFFSETS, xml::tag::TAG_DATA_INT, -1));

		parser->register_tag_format("coords_animation_point", xml::tag(QDSCR_COORDS_ANIMATION_POINT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("coords_animation", xml::tag(QDSCR_COORDS_ANIMATION, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("animation_phase", xml::tag(QDSCR_ANIMATION_PHASE, xml::tag::TAG_DATA_FLOAT, 1));

		parser->register_tag_format("parallax_offset", xml::tag(QDSCR_PARALLAX_OFFSET, xml::tag::TAG_DATA_INT, 2));

		parser->register_tag_format("static_object", xml::tag(QDSCR_STATIC_OBJECT, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("animated_object", xml::tag(QDSCR_ANIMATED_OBJECT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("object_state", xml::tag(QDSCR_OBJECT_STATE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("movement_states", xml::tag(QDSCR_OBJECT_MOVEMENT_STATES, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("object_movement_state", xml::tag(QDSCR_OBJECT_MOVEMENT_STATE, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("object_direction", xml::tag(QDSCR_OBJECT_DIRECTION, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("default_pos", xml::tag(QDSCR_OBJECT_DEFAULT_POS, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("default_direction", xml::tag(QDSCR_OBJECT_DEFAULT_DIRECTION, xml::tag::TAG_DATA_FLOAT, 1));

		parser->register_tag_format("object_state_static", xml::tag(QDSCR_OBJECT_STATE_STATIC, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("object_state_walk", xml::tag(QDSCR_OBJECT_STATE_WALK, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("object_state_mask", xml::tag(QDSCR_OBJECT_STATE_MASK, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("state_mask_pos", xml::tag(QDSCR_OBJECT_STATE_MASK_POS, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("state_mask_size", xml::tag(QDSCR_OBJECT_STATE_MASK_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("state_mask_data", xml::tag(QDSCR_OBJECT_STATE_MASK_DATA, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("state_mask_parent", xml::tag(QDSCR_OBJECT_STATE_MASK_PARENT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("state_mask_contour", xml::tag(QDSCR_OBJECT_STATE_MASK_CONTOUR, xml::tag::TAG_DATA_SHORT, -1));
		parser->register_tag_format("center_offset", xml::tag(QDSCR_OBJECT_STATE_CENTER_OFFSET, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("state_activation_delay", xml::tag(QDSCR_OBJECT_STATE_DELAY, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("sound_delay", xml::tag(QDSCR_OBJECT_STATE_SOUND_DELAY, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("text_delay", xml::tag(QDSCR_OBJECT_STATE_TEXT_DELAY, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("cursor_id", xml::tag(QDSCR_OBJECT_STATE_CURSOR_ID, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("acceleration", xml::tag(QDSCR_OBJECT_STATE_ACCELERATION, xml::tag::TAG_DATA_FLOAT, 2));
		parser->register_tag_format("rnd_move", xml::tag(QDSCR_OBJECT_STATE_RND_MOVE, xml::tag::TAG_DATA_FLOAT, 2));

		parser->register_tag_format("condition", xml::tag(QDSCR_CONDITION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("condition_data_int", xml::tag(QDSCR_CONDITION_DATA_INT, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("condition_data_float", xml::tag(QDSCR_CONDITION_DATA_FLOAT, xml::tag::TAG_DATA_FLOAT, -1));
		parser->register_tag_format("condition_data_string", xml::tag(QDSCR_CONDITION_DATA_STRING, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("condition_inverse", xml::tag(QDSCR_CONDITION_INVERSE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("conditions_mode", xml::tag(QDSCR_CONDITIONS_MODE, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("condition_group", xml::tag(QDSCR_CONDITION_GROUP, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("condition_object", xml::tag(QDSCR_CONDITION_OBJECT, xml::tag::TAG_DATA_VOID, 1));

		parser->register_tag_format("counter", xml::tag(QDSCR_COUNTER, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("counter_element", xml::tag(QDSCR_COUNTER_ELEMENT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("inc_value", xml::tag(QDSCR_COUNTER_INC_VALUE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("limit", xml::tag(QDSCR_COUNTER_LIMIT, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("digits", xml::tag(QDSCR_COUNTER_DIGITS, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("counter_name", xml::tag(QDSCR_COUNTER_NAME, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("object_scale", xml::tag(QDSCR_OBJ_SCALE_INFO, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("mouse_object", xml::tag(QDSCR_MOUSE_OBJECT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("default_cursors", xml::tag(QDSCR_MOUSE_DEFAULT_CURSORS, xml::tag::TAG_DATA_INT, 6));

		parser->register_tag_format("movement_animation_num", xml::tag(QDSCR_MOVEMENT_ANIMATION_NUMBER, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("movement_animations", xml::tag(QDSCR_MOVEMENT_ANIMATIONS, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("static_animations", xml::tag(QDSCR_STATIC_ANIMATIONS, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("moving_object", xml::tag(QDSCR_MOVING_OBJECT, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("collision_radius", xml::tag(QDSCR_COLLISION_RADIUS, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("collision_delay", xml::tag(QDSCR_COLLISION_DELAY, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("collision_path", xml::tag(QDSCR_COLLISION_PATH, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("follow_min_radius", xml::tag(QDSCR_FOLLOW_MIN_RADIUS, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("follow_max_radius", xml::tag(QDSCR_FOLLOW_MAX_RADIUS, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("attach_shift", xml::tag(QDSCR_ATTACH_SHIFT, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("control", xml::tag(QDSCR_PERSONAGE_CONTROL, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("movement", xml::tag(QDSCR_PERSONAGE_MOVEMENT_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("rotation_angle_per_quant", xml::tag(QDSCR_ROTATION_ANGLE_PER_QUANT, xml::tag::TAG_DATA_FLOAT, 1));

		parser->register_tag_format("inventory_cell_type", xml::tag(QDSCR_INVENTORY_CELL_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("inventory_cell_set", xml::tag(QDSCR_INVENTORY_CELL_SET, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("inventory_cell_set_size", xml::tag(QDSCR_INVENTORY_CELL_SET_SIZE, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("inventory_cell_set_additional_cells", xml::tag(QDSCR_INVENTORY_CELL_SET_ADDITIONAL_CELLS, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("inventory_cell_set_pos", xml::tag(QDSCR_INVENTORY_CELL_SET_POS, xml::tag::TAG_DATA_SHORT, 2));
		parser->register_tag_format("inventory", xml::tag(QDSCR_INVENTORY, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("camera", xml::tag(QDSCR_CAMERA, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("camera_grid_size", xml::tag(QDSCR_CAMERA_GRID_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("camera_grid_height", xml::tag(QDSCR_CAMERA_GRID_HEIGHT, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("camera_grid_attributes", xml::tag(QDSCR_CAMERA_GRID_ATTRIBUTES, xml::tag::TAG_DATA_INT, -1));
		parser->register_tag_format("camera_grid_center", xml::tag(QDSCR_CAMERA_GRID_CENTER, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("camera_cell_size", xml::tag(QDSCR_CAMERA_CELL_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("camera_focus", xml::tag(QDSCR_CAMERA_FOCUS, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("camera_angles", xml::tag(QDSCR_CAMERA_ANGLES, xml::tag::TAG_DATA_FLOAT, 3));
		parser->register_tag_format("camera_screen_size", xml::tag(QDSCR_CAMERA_SCREEN_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("camera_screen_offset", xml::tag(QDSCR_CAMERA_SCREEN_OFFSET, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("camera_screen_center", xml::tag(QDSCR_CAMERA_SCREEN_CENTER, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("camera_mode", xml::tag(QDSCR_CAMERA_MODE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("scrolling_speed", xml::tag(QDSCR_CAMERA_SCROLLING_SPEED, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("scrolling_dist", xml::tag(QDSCR_CAMERA_SCROLLING_DIST, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("smooth_switch", xml::tag(QDSCR_CAMERA_SMOOTH_SWITCH, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("camera_scale_pow", xml::tag(QDSCR_CAMERA_SCALE_POW, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("camera_scale_z_offset", xml::tag(QDSCR_CAMERA_SCALE_Z_OFFSET, xml::tag::TAG_DATA_FLOAT, 1));

		parser->register_tag_format("game_end", xml::tag(QDSCR_GAME_END, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("end_screen", xml::tag(QDSCR_GAME_END_SCREEN, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("interface", xml::tag(QDSCR_INTERFACE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("draw_scene", xml::tag(QDSCR_INTERFACE_SCENE_REDRAW, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("main_menu", xml::tag(QDSCR_INTERFACE_MAIN_MENU, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("ingame_screen0", xml::tag(QDSCR_INTERFACE_INGAME_SCREEN0, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("ingame_screen1", xml::tag(QDSCR_INTERFACE_INGAME_SCREEN1, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("save_prompt_screen", xml::tag(QDSCR_INTERFACE_SAVE_PROMPT_SCREEN, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("save_title_screen", xml::tag(QDSCR_INTERFACE_SAVE_NAME_SCREEN, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("highscore_name_screen", xml::tag(QDSCR_INTERFACE_HIGHSCORE_NAME_SCREEN, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("need_save_screenshot", xml::tag(QDSCR_INTERFACE_NEED_SAVE_SCREENSHOT, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("need_show_save_name", xml::tag(QDSCR_INTERFACE_NEED_SHOW_SAVE_NAME, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("need_show_save_time", xml::tag(QDSCR_INTERFACE_NEED_SHOW_SAVE_TIME, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("save_font_type", xml::tag(QDSCR_INTERFACE_SAVE_FONT_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("save_font_color", xml::tag(QDSCR_INTERFACE_SAVE_FONT_COLOR, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("is_autosave", xml::tag(QDSCR_INTERFACE_SAVE_IS_AUTOSAVE, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("interface_screen", xml::tag(QDSCR_INTERFACE_SCREEN, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("hide_time", xml::tag(QDSCR_INTERFACE_SCREEN_HIDE_TIME, xml::tag::TAG_DATA_FLOAT, 1));
		parser->register_tag_format("hide_offset", xml::tag(QDSCR_INTERFACE_SCREEN_HIDE_OFFSET, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("interface_element", xml::tag(QDSCR_INTERFACE_ELEMENT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("interface_element_state", xml::tag(QDSCR_INTERFACE_ELEMENT_STATE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("state_mode", xml::tag(QDSCR_INTERFACE_ELEMENT_STATE_MODE, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("pos", xml::tag(QDSCR_INTERFACE_ELEMENT_POS, xml::tag::TAG_DATA_INT, 3));

		parser->register_tag_format("option_id", xml::tag(QDSCR_INTERFACE_OPTION_ID, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("event", xml::tag(QDSCR_INTERFACE_EVENT, xml::tag::TAG_DATA_VOID, 1));
		parser->register_tag_format("event_data", xml::tag(QDSCR_INTERFACE_EVENT_DATA, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("before_animation", xml::tag(QDSCR_INTERFACE_EVENT_BEFORE_ANIMATION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("activation_type", xml::tag(QDSCR_INTERFACE_EVENT_ACTIVATION_TYPE, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("action", xml::tag(QDSCR_INTERFACE_ACTION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("action_data", xml::tag(QDSCR_INTERFACE_ACTION_DATA, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("animation_flags", xml::tag(QDSCR_INTERFACE_ANIMATION_FLAGS, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("slider_rect", xml::tag(QDSCR_INTERFACE_SLIDER_RECTANGLE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("slider_orientation", xml::tag(QDSCR_INTERFACE_SLIDER_ORIENTATION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("background_offset", xml::tag(QDSCR_INTERFACE_BACKGROUND_OFFSET, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("thumbnail_size", xml::tag(QDSCR_INTERFACE_THUMBNAIL_SIZE, xml::tag::TAG_DATA_INT, 2));
		parser->register_tag_format("text_shift", xml::tag(QDSCR_INTERFACE_TEXT_SHIFT, xml::tag::TAG_DATA_INT, 2));

		parser->register_tag_format("border_corner", xml::tag(QDSCR_TEXT_WINDOW_BORDER_CORNER, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("border_line", xml::tag(QDSCR_TEXT_WINDOW_BORDER_LINE_H, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("border_vline", xml::tag(QDSCR_TEXT_WINDOW_BORDER_LINE_V, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("border_back", xml::tag(QDSCR_TEXT_WINDOW_BORDER_BACK, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("window_slider", xml::tag(QDSCR_TEXT_WINDOW_SLIDER, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("window_type", xml::tag(QDSCR_TEXT_WINDOW_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("string_length", xml::tag(QDSCR_TEXT_WINDOW_MAX_STRING_LENGTH, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("info_type", xml::tag(QDSCR_TEXT_WINDOW_INFO_TYPE, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("player_id", xml::tag(QDSCR_TEXT_WINDOW_PLAYER_ID, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("scene", xml::tag(QDSCR_SCENE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("save_slot", xml::tag(QDSCR_SCENE_SAVE_SLOT, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("startup_scene", xml::tag(QDSCR_STARTUP_SCENE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("restart_minigame", xml::tag(QDSCR_SCENE_RESTART_MINIGAME, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("location", xml::tag(QDSCR_LOCATION, xml::tag::TAG_DATA_STRING, -1));

		parser->register_tag_format("game_title", xml::tag(QDSCR_GAME_TITLE, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("text_db", xml::tag(QDSCR_TEXT_DB, xml::tag::TAG_DATA_STRING, -1));
		parser->register_tag_format("cd_key", xml::tag(QDSCR_CD_KEY, xml::tag::TAG_DATA_STRING, -1));

		// For qdScreenTextFormat
		parser->register_tag_format("screen_text_format", xml::tag(QDSCR_SCREEN_TEXT_FORMAT, xml::tag::TAG_DATA_VOID, -1));
		parser->register_tag_format("font_type", xml::tag(QDSCR_FONT_TYPE, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("default_font", xml::tag(QDSCR_DEFAULT_FONT, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("global_depend", xml::tag(QDSCR_GLOBAL_DEPEND, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("inverse_direction", xml::tag(QDSCR_INVERSE_DIRECTION, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("alpha", xml::tag(QDSCR_ALPHA, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("enable_background", xml::tag(QDSCR_ENABLE_BACKGROUND, xml::tag::TAG_DATA_INT, 1));
		parser->register_tag_format("fade_time", xml::tag(QDSCR_FADE_TIME, xml::tag::TAG_DATA_FLOAT, 1));

		parser->register_tag_format("hof_size", xml::tag(QDSCR_HALL_OF_FAME_SIZE, xml::tag::TAG_DATA_INT, 1));

		parser->register_tag_format("screen_transform", xml::tag(QDSCR_SCREEN_TRANSFORM, xml::tag::TAG_DATA_FLOAT, 6));

		parser->register_tag_format("compression", xml::tag(QDSCR_RESOURCE_COMPRESSION, xml::tag::TAG_DATA_INT, 1));
	}

	return *parser;
}

const char *qdscr_XML_string(const char *p) {
	static Common::String conv_str;

	conv_str = p;

	uint32 pos = Common::String::npos;
	do {
		pos = conv_str.find("&", pos + 1);
		if (pos != Common::String::npos)
			conv_str.replace(pos, 1, "&amp;");

	} while (pos != Common::String::npos);

	pos = Common::String::npos;
	do {
		pos = conv_str.find("<", pos + 1);
		if (pos != Common::String::npos)
			conv_str.replace(pos, 1, "&lt;");

	} while (pos != Common::String::npos);

	pos = Common::String::npos;
	do {
		pos = conv_str.find(">", pos + 1);
		if (pos != Common::String::npos)
			conv_str.replace(pos, 1, "&gt;");

	} while (pos != Common::String::npos);

	pos = Common::String::npos;
	do {
		pos = conv_str.find("\"", pos + 1);
		if (pos != Common::String::npos)
			conv_str.replace(pos, 1, "&quot;");

	} while (pos != Common::String::npos);

	pos = Common::String::npos;
	do {
		pos = conv_str.find("'", pos + 1);
		if (pos != Common::String::npos)
			conv_str.replace(pos, 1, "&#039;");

	} while (pos != Common::String::npos);

	return (const char *)transCyrillic(conv_str.c_str());
}

const char *qdscr_XML_string(const Common::String s) {
	return qdscr_XML_string(s.c_str());
}


} // namespace QDEngine
