/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qd_precomp.h"

#include <string.h>

#include "qdscr_keywords.h"

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

qdscrKeyword qdscr_Keywords[QDSCR_MAX_KEYWORD_ID] = 
{
	{ QDSCR_BLOCK_BEG, "{", QDSCR_DATA_VOID, 1 },
	{ QDSCR_BLOCK_END, "}", QDSCR_DATA_VOID, 1 },

	{ QDSCR_ID, "ID", QDSCR_DATA_INT, 1 },
	{ QDSCR_X, "x", QDSCR_DATA_INT, 1 },
	{ QDSCR_Y, "y", QDSCR_DATA_INT, 1 },
	{ QDSCR_SX, "sx", QDSCR_DATA_INT, 1 },
	{ QDSCR_SY, "sy", QDSCR_DATA_INT, 1 },
	{ QDSCR_POS2D, "pos_2d", QDSCR_DATA_DOUBLE, 2 },
	{ QDSCR_POS3D, "pos_3d", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_SRC_POS, "src_pos", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_DEST_POS, "dest_pos", QDSCR_DATA_DOUBLE, 3 },

	{ QDSCR_FILE, "file", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_START_TIME, "start_time", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_PERIOD, "period", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_LENGTH, "length", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_SPEED, "speed", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_INTERPOLATION_TIME, "interpolation_time", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_SCALE, "scale", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_NAME, "name", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_FLAG, "flag", QDSCR_DATA_INT, 1 },
	{ QDSCR_RND, "rnd", QDSCR_DATA_INT, 1 },
	{ QDSCR_MASK_SIZE, "mask_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_MASK_ATTRIBUTES, "mask_attributes", QDSCR_DATA_INT, -1 },
	{ QDSCR_MASK_HEIGHTS, "mask_heights", QDSCR_DATA_INT, -1 },
	{ QDSCR_BOUND, "bound", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_TYPE, "type", QDSCR_DATA_INT, 1 },
	{ QDSCR_SIZE, "size", QDSCR_DATA_INT, 1 },
	{ QDSCR_STATE, "state", QDSCR_DATA_INT, 1 },
	{ QDSCR_TEXT, "text", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_SHORT_TEXT, "short_text", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_SCREEN_SIZE, "screen_size", QDSCR_DATA_INT, 2 },

	{ QDSCR_CONTOUR_RECTANGLE, "contour_rect", QDSCR_DATA_INT, 2 },
	{ QDSCR_CONTOUR_CIRCLE, "contour_circle", QDSCR_DATA_INT, 1 },
	{ QDSCR_CONTOUR_POLYGON, "contour_polygon", QDSCR_DATA_INT, -1 },

	{ QDSCR_NAMED_OBJECT, "named_object", QDSCR_DATA_VOID, 1 },

	{ QDSCR_GRID_ZONE, "grid_zone", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_GRID_ZONE_CELLS, "grid_zone_cells", QDSCR_DATA_INT, -1 },
	{ QDSCR_GRID_ZONE_HEIGHT, "grid_zone_height", QDSCR_DATA_INT, 1 },
	{ QDSCR_GRID_ZONE_CONTOUR, "grid_zone_contour", QDSCR_DATA_INT, -1 },

	{ QDSCR_TRIGGER_ELEMENT, "trigger_element", QDSCR_DATA_VOID, 1 },
	{ QDSCR_TRIGGER_ELEMENT_LINK, "trigger_element_link", QDSCR_DATA_INT, 3 },
	{ QDSCR_TRIGGER_ELEMENT_TITLE, "trigger_element_title", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_TRIGGER_ELEMENT_CELL_NUMBER, "trigger_element_cell_number", QDSCR_DATA_INT, 2 },
	{ QDSCR_TRIGGER_PARENT_LINK_OWNER_OFFSET, "parent_link_owner_offset", QDSCR_DATA_INT, 4 },
	{ QDSCR_TRIGGER_PARENT_LINK_CHILD_OFFSET, "parent_link_child_offset", QDSCR_DATA_INT, 4 },
	{ QDSCR_TRIGGER_CHILD_LINK_OWNER_OFFSET, "child_link_owner_offset", QDSCR_DATA_INT, 4 },
	{ QDSCR_TRIGGER_CHILD_LINK_CHILD_OFFSET, "child_link_child_offset", QDSCR_DATA_INT, 4 },

	{ QDSCR_TRIGGER_CHAIN, "trigger_chain", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_TRIGGER_CHAIN_ROOT, "trigger_chain_root", QDSCR_DATA_VOID, 1 },
	{ QDSCR_TRIGGER_CHAIN_WORK_AREA, "trigger_chain_work_area", QDSCR_DATA_INT, 4 },
	{ QDSCR_TRIGGER_CHAIN_LAYOUT, "trigger_chain_layout", QDSCR_DATA_INT, 2 },

	{ QDSCR_TRIGGER_BOUND, "trigger_bound", QDSCR_DATA_INT, 4 },

	{ QDSCR_SOUND, "sound", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_VIDEO, "video", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_VIDEO_POSITION, "video_position", QDSCR_DATA_INT, 2 },
	{ QDSCR_VIDEO_BACKGROUND, "video_background", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_ANIMATION_FRAME, "animation_frame", QDSCR_DATA_VOID, 1 },
	{ QDSCR_ANIMATION_INFO, "animation_info", QDSCR_DATA_VOID, 1 },
	{ QDSCR_ANIMATION_FILE, "animation_file", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_ANIMATION, "animation", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_ANIMATION_SET, "animation_set", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_COORDS_ANIMATION_POINT, "coords_animation_point", QDSCR_DATA_VOID, 1 },
	{ QDSCR_COORDS_ANIMATION, "coords_animation", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_PARALLAX_OFFSET, "parallax_offset", QDSCR_DATA_INT, 2 },

	{ QDSCR_STATIC_OBJECT, "static_object", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_ANIMATED_OBJECT, "animated_object", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_OBJECT_STATE, "object_state", QDSCR_DATA_VOID, 1 },
	{ QDSCR_OBJECT_MOVEMENT_STATES, "movement_states", QDSCR_DATA_VOID, 1 },
	{ QDSCR_OBJECT_MOVEMENT_STATE, "object_movement_state", QDSCR_DATA_VOID, 1 },

	{ QDSCR_OBJECT_DIRECTION, "object_direction", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_OBJECT_DEFAULT_POS, "default_pos", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_OBJECT_DEFAULT_DIRECTION, "default_direction", QDSCR_DATA_DOUBLE, 1 },

	{ QDSCR_OBJECT_STATE_STATIC, "object_state_static", QDSCR_DATA_VOID, 1 },
	{ QDSCR_OBJECT_STATE_WALK, "object_state_walk", QDSCR_DATA_VOID, 1 },
	{ QDSCR_OBJECT_STATE_MASK, "object_state_mask", QDSCR_DATA_VOID, 1 },
	{ QDSCR_OBJECT_STATE_MASK_POS,	"state_mask_pos", QDSCR_DATA_INT, 2 },
	{ QDSCR_OBJECT_STATE_MASK_SIZE,	"state_mask_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_OBJECT_STATE_MASK_DATA,	"state_mask_data", QDSCR_DATA_INT, -1 },
	{ QDSCR_OBJECT_STATE_MASK_PARENT, "state_mask_parent", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_OBJECT_STATE_MASK_CONTOUR, "state_mask_contour", QDSCR_DATA_INT, -1 },
	{ QDSCR_OBJECT_STATE_CENTER_OFFSET, "center_offset", QDSCR_DATA_INT, 2 },
	{ QDSCR_OBJECT_STATE_DELAY, "state_activation_delay", QDSCR_DATA_DOUBLE, 1 },

	{ QDSCR_CONDITION, "condition", QDSCR_DATA_INT, 1 },
	{ QDSCR_CONDITION_DATA_INT, "condition_data_int", QDSCR_DATA_INT, -1 },
	{ QDSCR_CONDITION_DATA_FLOAT, "condition_data_float", QDSCR_DATA_DOUBLE, -1 },
	{ QDSCR_CONDITION_DATA_STRING, "condition_data_string", QDSCR_DATA_CHAR, -1 },
	{ QDSCR_CONDITIONS_MODE, "conditions_mode", QDSCR_DATA_INT, 1 },

	{ QDSCR_OBJ_SCALE_INFO, "object_scale", QDSCR_DATA_CHAR, -1 },
	
	{ QDSCR_MOUSE_OBJECT, "mouse_object", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_MOVEMENT_ANIMATION_NUMBER, "movement_animation_num", QDSCR_DATA_INT, 1 },
	{ QDSCR_MOVEMENT_ANIMATIONS, "movement_animations", QDSCR_DATA_VOID, 1 },
	{ QDSCR_STATIC_ANIMATIONS, "static_animations", QDSCR_DATA_VOID, 1 },
	{ QDSCR_MOVING_OBJECT, "moving_object", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_INVENTORY_CELL_TYPE, "inventory_cell_type", QDSCR_DATA_INT, 1 },
	{ QDSCR_INVENTORY_CELL_SET, "inventory_cell_set", QDSCR_DATA_INT, 1 },
	{ QDSCR_INVENTORY_CELL_SET_SIZE, "inventory_cell_set_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_INVENTORY_CELL_SET_POS, "inventory_cell_set_pos", QDSCR_DATA_INT, 2 },
	{ QDSCR_INVENTORY, "inventory", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_CAMERA, "camera", QDSCR_DATA_VOID, 1 },
	{ QDSCR_CAMERA_GRID_SIZE, "camera_grid_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_CAMERA_GRID_HEIGHT, "camera_grid_height", QDSCR_DATA_INT, -1 },
	{ QDSCR_CAMERA_GRID_ATTRIBUTES, "camera_grid_attributes", QDSCR_DATA_INT, -1 },
	{ QDSCR_CAMERA_GRID_CENTER, "camera_grid_center", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_CAMERA_CELL_SIZE, "camera_cell_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_CAMERA_FOCUS, "camera_focus", QDSCR_DATA_DOUBLE, 1 },
	{ QDSCR_CAMERA_ANGLES, "camera_angles", QDSCR_DATA_DOUBLE, 3 },
	{ QDSCR_CAMERA_SCREEN_SIZE, "camera_screen_size", QDSCR_DATA_INT, 2 },
	{ QDSCR_CAMERA_SCREEN_OFFSET, "camera_screen_offset", QDSCR_DATA_INT, 2 },
	{ QDSCR_CAMERA_SCREEN_CENTER, "camera_screen_center", QDSCR_DATA_INT, 2 },

	{ QDSCR_SCENE, "scene", QDSCR_DATA_CHAR, -1 },

	{ QDSCR_LOCATION, "location", QDSCR_DATA_CHAR, -1 }
};

qdscrKeyword* qdscrGetKeyword(char* ptr)
{
	int i;
	for(i = 0; i < QDSCR_MAX_KEYWORD_ID - 1; i ++)
		if(!strcmp(qdscr_Keywords[i].name,ptr)) return &qdscr_Keywords[i];

	return 0;
}

