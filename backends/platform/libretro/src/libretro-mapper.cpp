/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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
#define FORBIDDEN_SYMBOL_EXCEPTION_strcpy
#define FORBIDDEN_SYMBOL_EXCEPTION_strcat

#include "backends/platform/libretro/include/libretro-mapper.h"

struct retro_keymap mapper_keys[RETRO_DEVICE_ID_JOYPAD_LAST] = {0};

static int16_t mapper_digital_buttons_status = 0;
static uint32_t mapper_digital_buttons_prev_status = 0;
static int16_t mapper_analog_stick_status [2][2] = {0};

void mapper_poll_device(void) {
	//Store previous on/off status
	mapper_digital_buttons_prev_status = mapper_digital_buttons_status;
	for (int8_t i = RETRO_DEVICE_ID_JOYPAD_ANALOG; i < RETRO_DEVICE_ID_JOYPAD_LAST; i++)
		mapper_digital_buttons_prev_status |= mapper_get_device_key_value(i) ? 1 << i : 0;

	//Get current status
	mapper_digital_buttons_status = 0;
	bool bitmasks = false;
	// Store real digital buttons status
	if (retro_get_input_bitmask_supported())
		mapper_digital_buttons_status = retro_input_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_MASK);
	else
		for (int8_t i = 0; i < RETRO_DEVICE_ID_JOYPAD_ANALOG; i++)
			mapper_digital_buttons_status |= (retro_input_cb(0, RETRO_DEVICE_JOYPAD, 0, i) << i);

	//Store analog sticks (analog) status
	for (int8_t i = 0; i < 2; i++)
		for (int8_t j = 0; j < 2; j++)
			mapper_analog_stick_status[i][j] = retro_input_cb(0, RETRO_DEVICE_ANALOG, i, j);
}

static int16_t mapper_get_retro_key_index(const char *retro_key_value) {
	uint16_t i = 0;
	while (retro_keys[i].retro_id != RETROK_LAST) {
		if (strcmp(retro_keys[i].value, retro_key_value) == 0)
			return i;
		++i;
	}
	return -1;
}

int8_t mapper_get_mapper_key_index(int16_t key_retro_id, uint8_t start_index) {
	uint8_t i = start_index;
	while (i < RETRO_DEVICE_ID_JOYPAD_LAST - 1) {
		if (mapper_keys[i].retro_id == key_retro_id)
			return i;
		++i;
	}
	return -1;
}

bool mapper_set_device_keys(unsigned int retro_device_id, const char *retro_key_value) {
	int16_t retro_key_index = mapper_get_retro_key_index(retro_key_value);
	if (retro_key_index > -1 && retro_device_id < RETRO_DEVICE_ID_JOYPAD_LAST) {
		mapper_keys[retro_device_id] = retro_keys[retro_key_index];
		return true;
	}
	return false;
}

int16_t mapper_get_device_key_value(unsigned int retro_device_id) {
	if (retro_device_id < RETRO_DEVICE_ID_JOYPAD_ANALOG) {
		return (mapper_digital_buttons_status & (1 << retro_device_id)) > 0;
	} else if (retro_device_id < RETRO_DEVICE_ID_JOYPAD_LAST) {
		int16_t res;
		int16_t sign;
		switch (retro_device_id) {
		case RETRO_DEVICE_ID_JOYPAD_LU:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y];
			sign = -1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_LD:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_Y];
			sign = 1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_LL:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X];
			sign = -1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_LR:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_LEFT][RETRO_DEVICE_ID_ANALOG_X];
			sign = 1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_RU:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y];
			sign = -1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_RD:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_Y];
			sign = 1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_RL:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X];
			sign = -1;
			break;
		case RETRO_DEVICE_ID_JOYPAD_RR:
			res =    mapper_analog_stick_status[RETRO_DEVICE_INDEX_ANALOG_RIGHT][RETRO_DEVICE_ID_ANALOG_X];
			sign = 1;
		}

		bool check_sign = (res > 0) - (res < 0) == sign;
		res = abs(res);
		/* discard analog values of 1 to discriminate from digital inputs */
		return check_sign && res > 1 && res > retro_setting_get_analog_deadzone() ? res : 0;
	}
	return 0;
}

uint8_t mapper_get_device_key_status(unsigned int retro_device_id) {
	uint8_t status = mapper_get_device_key_value(retro_device_id) ? 1 << RETRO_DEVICE_KEY_STATUS : 0;
	status |= (mapper_digital_buttons_prev_status & (1 << retro_device_id)) ? 1 << RETRO_DEVICE_KEY_PREV_STATUS : 0;
	status |= ((status & (1 << RETRO_DEVICE_KEY_STATUS)) > 0) == ((status & (1 << RETRO_DEVICE_KEY_PREV_STATUS)) > 0) ? 0 : 1 << RETRO_DEVICE_KEY_CHANGED;
	return status;
}

int16_t mapper_get_mapper_key_value(int16_t retro_key_retro_id) {
	int16_t result = 0;
	int8_t retro_key_index = 0;
	while (retro_key_index > -1) {
		retro_key_index = mapper_get_mapper_key_index(retro_key_retro_id, retro_key_index);
		if (retro_key_index > -1) {
			if (mapper_get_device_key_value(retro_key_index) > result)
				result = mapper_get_device_key_value(retro_key_index);
			retro_key_index++;
		}
	}
	return result;
}

uint8_t mapper_get_mapper_key_status(int16_t retro_key_retro_id) {
	uint8_t status = 0;
	int8_t retro_key_index = 0;
	while (retro_key_index > -1) {
		retro_key_index = mapper_get_mapper_key_index(retro_key_retro_id, retro_key_index);
		if (retro_key_index > -1) {
			status |= mapper_get_device_key_status(retro_key_index);
			retro_key_index++;
		}
	}
	return status;
}

int16_t mapper_get_device_key_retro_id(unsigned int retro_device_id) {
	return mapper_keys[retro_device_id].retro_id;
}

int16_t mapper_get_device_key_scummvm_id(unsigned int retro_device_id) {
	return mapper_keys[retro_device_id].scummvm_id;
}
