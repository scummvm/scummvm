/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/zcode/processor.h"

namespace Glk {
namespace ZCode {

#define MAX_OBJECT 2000

enum O1 {
	O1_PARENT          = 4,
	O1_SIBLING         = 5,
	O1_CHILD           = 6,
	O1_PROPERTY_OFFSET = 7,
	O1_SIZE            = 9
};

enum O4 {
	O4_PARENT          = 6,
	O4_SIBLING         = 8,
	O4_CHILD           = 10,
	O4_PROPERTY_OFFSET = 12,
	O4_SIZE            = 14
};

zword Processor::object_address(zword obj) {
	// Check object number
	if (obj > ((h_version <= V3) ? 255 : MAX_OBJECT)) {
		print_string("@Attempt to address illegal object ");
		print_num(obj);
		print_string(".  This is normally fatal.");
		new_line();
		runtimeError(ERR_ILL_OBJ);
	}

	// Return object address
	if (h_version <= V3)
		return h_objects + ((obj - 1) * O1_SIZE + 62);
	else
		return h_objects + ((obj - 1) * O4_SIZE + 126);
}

zword Processor::object_name(zword object) {
	zword obj_addr;
	zword name_addr;

	obj_addr = object_address(object);

	// The object name address is found at the start of the properties
	if (h_version <= V3)
		obj_addr += O1_PROPERTY_OFFSET;
	else
		obj_addr += O4_PROPERTY_OFFSET;

	LOW_WORD(obj_addr, name_addr);

	return name_addr;
}

zword Processor::first_property(zword obj) {
	zword prop_addr;
	zbyte size;

	// Fetch address of object name
	prop_addr = object_name (obj);

	// Get length of object name
	LOW_BYTE(prop_addr, size);

	// Add name length to pointer
	return prop_addr + 1 + 2 * size;
}

zword Processor::next_property(zword prop_addr) {
	zbyte value;

	// Load the current property id
	LOW_BYTE(prop_addr, value);
	prop_addr++;

	// Calculate the length of this property
	if (h_version <= V3)
		value >>= 5;
	else if (!(value & 0x80))
		value >>= 6;
	else {
		LOW_BYTE(prop_addr, value);
		value &= 0x3f;

		if (value == 0)
			// demanded by Spec 1.0
			value = 64;
	}

	// Add property length to current property pointer
	return prop_addr + value + 1;
}

void Processor::unlink_object(zword object) {
	zword obj_addr;
	zword parent_addr;
	zword sibling_addr;

	if (object == 0) {
		runtimeError(ERR_REMOVE_OBJECT_0);
		return;
	}

	obj_addr = object_address(object);

	if (h_version <= V3) {

		zbyte parent;
		zbyte younger_sibling;
		zbyte older_sibling;
		zbyte zero = 0;

		// Get parent of object, and return if no parent
		obj_addr += O1_PARENT;
		LOW_BYTE(obj_addr, parent);
		if (!parent)
			return;

		// Get (older) sibling of object and set both parent and sibling pointers to 0
		SET_BYTE(obj_addr, zero);
		obj_addr += O1_SIBLING - O1_PARENT;
		LOW_BYTE(obj_addr, older_sibling);
		SET_BYTE(obj_addr, zero);

		// Get first child of parent (the youngest sibling of the object)
		parent_addr = object_address(parent) + O1_CHILD;
		LOW_BYTE(parent_addr, younger_sibling);

		// Remove object from the list of siblings
		if (younger_sibling == object)
			SET_BYTE(parent_addr, older_sibling);
		else {
			do {
				sibling_addr = object_address(younger_sibling) + O1_SIBLING;
				LOW_BYTE(sibling_addr, younger_sibling);
			} while (younger_sibling != object);
			SET_BYTE(sibling_addr, older_sibling);
		}
	} else {
		zword parent;
		zword younger_sibling;
		zword older_sibling;
		zword zero = 0;

		// Get parent of object, and return if no parent
		obj_addr += O4_PARENT;
		LOW_WORD(obj_addr, parent);
		if (!parent)
			return;

		// Get (older) sibling of object and set both parent and sibling pointers to 0
		SET_WORD(obj_addr, zero);
		obj_addr += O4_SIBLING - O4_PARENT;
		LOW_WORD(obj_addr, older_sibling);
		SET_WORD(obj_addr, zero);

		// Get first child of parent (the youngest sibling of the object)
		parent_addr = object_address(parent) + O4_CHILD;
		LOW_WORD(parent_addr, younger_sibling);

		// Remove object from the list of siblings
		if (younger_sibling == object) {
			SET_WORD(parent_addr, older_sibling);
		} else {
			do {
				sibling_addr = object_address(younger_sibling) + O4_SIBLING;
				LOW_WORD(sibling_addr, younger_sibling);
			} while (younger_sibling != object);
			SET_WORD(sibling_addr, older_sibling);
		}
	}
}

void Processor::z_clear_attr() {
	zword obj_addr;
	zbyte value;

	if (_storyId == SHERLOCK)
		if (zargs[1] == 48)
			return;

	if (zargs[1] > ((h_version <= V3) ? 31 : 47))
		runtimeError(ERR_ILL_ATTR);

	// If we are monitoring attribute assignment display a short note
	if (_attribute_assignment) {
		stream_mssg_on();
		print_string("@clear_attr ");
		print_object(zargs[0]);
		print_string(" ");
		print_num(zargs[1]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_CLEAR_ATTR_0);
		return;
	}

	// Get attribute address
	obj_addr = object_address(zargs[0]) + zargs[1] / 8;

	// Clear attribute bit
	LOW_BYTE(obj_addr, value);
	value &= ~(0x80 >> (zargs[1] & 7));
	SET_BYTE(obj_addr, value);
}

void Processor::z_jin() {
	zword obj_addr;

	// If we are monitoring object locating display a short note
	if (_object_locating) {
		stream_mssg_on();
		print_string("@jin ");
		print_object(zargs[0]);
		print_string(" ");
		print_object(zargs[1]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_JIN_0);
		branch(0 == zargs[1]);
		return;
	}

	obj_addr = object_address(zargs[0]);

	if (h_version <= V3) {
		zbyte parent;

		// Get parent id from object
		obj_addr += O1_PARENT;
		LOW_BYTE(obj_addr, parent);

		// Branch if the parent is obj2
		branch(parent == zargs[1]);

	} else {
		zword parent;

		// Get parent id from object
		obj_addr += O4_PARENT;
		LOW_WORD(obj_addr, parent);

		// Branch if the parent is obj2
		branch(parent == zargs[1]);
	}
}

void Processor::z_get_child() {
	zword obj_addr;

	// If we are monitoring object locating display a short note
	if (_object_locating) {
		stream_mssg_on();
		print_string("@get_child ");
		print_object(zargs[0]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_CHILD_0);
		store(0);
		branch(false);
		return;
	}

	obj_addr = object_address(zargs[0]);

	if (h_version <= V3) {
		zbyte child;

		// Get child id from object
		obj_addr += O1_CHILD;
		LOW_BYTE(obj_addr, child);

		// Store child id and branch
		store(child);
		branch(child);
	} else {
		zword child;

		// Get child id from object
		obj_addr += O4_CHILD;
		LOW_WORD(obj_addr, child);

		// Store child id and branch
		store(child);
		branch(child);
	}
}

void Processor::z_get_next_prop() {
	zword prop_addr;
	zbyte value;
	zbyte mask;

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_NEXT_PROP_0);
		store(0);
		return;
	}

	// Property id is in bottom five (six) bits
	mask = (h_version <= V3) ? 0x1f : 0x3f;

	// Load address of first property
	prop_addr = first_property(zargs[0]);

	if (zargs[1] != 0) {
		// Scan down the property list
		do {
			LOW_BYTE(prop_addr, value);
			prop_addr = next_property(prop_addr);
		} while ((value & mask) > zargs[1]);

		// Exit if the property does not exist
		if ((value & mask) != zargs[1])
			runtimeError(ERR_NO_PROP);
	}

	// Return the property id
	LOW_BYTE(prop_addr, value);
	store((zword)(value & mask));
}

void Processor::z_get_parent() {
	zword obj_addr;

	// If we are monitoring object locating display a short note
	if (_object_locating) {
		stream_mssg_on();
		print_string("@get_parent ");
		print_object(zargs[0]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_PARENT_0);
		store(0);
		return;
	}

	obj_addr = object_address(zargs[0]);

	if (h_version <= V3) {
		zbyte parent;

		// Get parent id from object
		obj_addr += O1_PARENT;
		LOW_BYTE(obj_addr, parent);

		// Store parent
		store(parent);

	} else {
		zword parent;

		// Get parent id from object
		obj_addr += O4_PARENT;
		LOW_WORD(obj_addr, parent);

		// Store parent
		store(parent);
	}
}

void Processor::z_get_prop() {
	zword prop_addr;
	zword wprop_val;
	zbyte bprop_val;
	zbyte value;
	zbyte mask;

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_PROP_0);
		store(0);
		return;
	}

	// Property id is in bottom five (six) bits
	mask = (h_version <= V3) ? 0x1f : 0x3f;

	// Load address of first property
	prop_addr = first_property(zargs[0]);

	// Scan down the property list
	for (;;) {
		LOW_BYTE(prop_addr, value);
		if ((value & mask) <= zargs[1])
			break;
		prop_addr = next_property(prop_addr);
	}

	if ((value & mask) == zargs[1]) {
		// property found

		// Load property(byte or word sized)
		prop_addr++;

		if ((h_version <= V3 && !(value & 0xe0)) || (h_version >= V4 && !(value & 0xc0))) {
			LOW_BYTE(prop_addr, bprop_val);
			wprop_val = bprop_val;
		} else {
			LOW_WORD(prop_addr, wprop_val);
		}
	} else {
		// property not found

		// Load default value
		prop_addr = h_objects + 2 * (zargs[1] - 1);
		LOW_WORD(prop_addr, wprop_val);
	}

	// Store the property value
	store(wprop_val);
}

void Processor::z_get_prop_addr() {
	zword prop_addr;
	zbyte value;
	zbyte mask;

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_PROP_ADDR_0);
		store(0);
		return;
	}

	if (_storyId == BEYOND_ZORK)
		if (zargs[0] > MAX_OBJECT)
			{ store(0); return; }

	// Property id is in bottom five (six) bits
	mask = (h_version <= V3) ? 0x1f : 0x3f;

	// Load address of first property
	prop_addr = first_property(zargs[0]);

	// Scan down the property list
	for (;;) {
		LOW_BYTE(prop_addr, value);
		if ((value & mask) <= zargs[1])
			break;
		prop_addr = next_property(prop_addr);
	}

	// Calculate the property address or return zero
	if ((value & mask) == zargs[1]) {

		if (h_version >= V4 && (value & 0x80))
			prop_addr++;
		store((zword)(prop_addr + 1));

	} else {
		store(0);
	}
}

void Processor::z_get_prop_len() {
	zword addr;
	zbyte value;

	// Back up the property pointer to the property id
	addr = zargs[0] - 1;
	LOW_BYTE(addr, value);

	// Calculate length of property
	if (h_version <= V3)
		value = (value >> 5) + 1;
	else if (!(value & 0x80))
		value = (value >> 6) + 1;
	else {
		value &= 0x3f;

		if (value == 0)
			value = 64;        // demanded by Spec 1.0
	}

	// Store length of property
	store(value);
}

void Processor::z_get_sibling() {
	zword obj_addr;

	if (zargs[0] == 0) {
		runtimeError(ERR_GET_SIBLING_0);
		store(0);
		branch(false);
		return;
	}

	obj_addr = object_address(zargs[0]);

	if (h_version <= V3) {
		zbyte sibling;

		// Get sibling id from object
		obj_addr += O1_SIBLING;
		LOW_BYTE(obj_addr, sibling);

		// Store sibling and branch
		store(sibling);
		branch(sibling);

	} else {
		zword sibling;

		// Get sibling id from object
		obj_addr += O4_SIBLING;
		LOW_WORD(obj_addr, sibling);

		// Store sibling and branch
		store(sibling);
		branch(sibling);
	}
}

void Processor::z_insert_obj() {
	zword obj1 = zargs[0];
	zword obj2 = zargs[1];
	zword obj1_addr;
	zword obj2_addr;

	// If we are monitoring object movements display a short note
	if (_object_movement) {
		stream_mssg_on();
		print_string("@move_obj ");
		print_object(obj1);
		print_string(" ");
		print_object(obj2);
		stream_mssg_off();
	}

	if (obj1 == 0) {
		runtimeError(ERR_MOVE_OBJECT_0);
		return;
	}

	if (obj2 == 0) {
		runtimeError(ERR_MOVE_OBJECT_TO_0);
		return;
	}

	// Get addresses of both objects
	obj1_addr = object_address(obj1);
	obj2_addr = object_address(obj2);

	// Remove object 1 from current parent
	unlink_object(obj1);

	// Make object 1 first child of object 2
	if (h_version <= V3) {
		zbyte child;

		obj1_addr += O1_PARENT;
		SET_BYTE(obj1_addr, obj2);
		obj2_addr += O1_CHILD;
		LOW_BYTE(obj2_addr, child);
		SET_BYTE(obj2_addr, obj1);
		obj1_addr += O1_SIBLING - O1_PARENT;
		SET_BYTE(obj1_addr, child);

	} else {
		zword child;

		obj1_addr += O4_PARENT;
		SET_WORD(obj1_addr, obj2);
		obj2_addr += O4_CHILD;
		LOW_WORD(obj2_addr, child);
		SET_WORD(obj2_addr, obj1);
		obj1_addr += O4_SIBLING - O4_PARENT;
		SET_WORD(obj1_addr, child);
	}
}

void Processor::z_put_prop() {
	zword prop_addr;
	zword value;
	zbyte mask;

	if (zargs[0] == 0) {
		runtimeError(ERR_PUT_PROP_0);
		return;
	}

	// Property id is in bottom five or six bits
	mask = (h_version <= V3) ? 0x1f : 0x3f;

	// Load address of first property
	prop_addr = first_property(zargs[0]);

	// Scan down the property list
	for (;;) {
		LOW_BYTE(prop_addr, value);
		if ((value & mask) <= zargs[1])
			break;

		prop_addr = next_property(prop_addr);
	}

	// Exit if the property does not exist
	if ((value & mask) != zargs[1])
		runtimeError(ERR_NO_PROP);

	// Store the new property value (byte or word sized)
	prop_addr++;

	if ((h_version <= V3 && !(value & 0xe0)) || (h_version >= V4 && !(value & 0xc0))) {
		zbyte v = zargs[2];
		SET_BYTE(prop_addr, v);
	} else {
		zword v = zargs[2];
		SET_WORD(prop_addr, v);
	}
}

void Processor::z_remove_obj() {
	// If we are monitoring object movements display a short note
	if (_object_movement) {
		stream_mssg_on();
		print_string("@remove_obj ");
		print_object(zargs[0]);
		stream_mssg_off();
	}

	// Call unlink_object to do the job
	unlink_object(zargs[0]);
}

void Processor::z_set_attr() {
	zword obj_addr;
	zbyte value;

	if (_storyId == SHERLOCK)
		if (zargs[1] == 48)
			return;

	if (zargs[1] > ((h_version <= V3) ? 31 : 47))
		runtimeError(ERR_ILL_ATTR);

	// If we are monitoring attribute assignment display a short note
	if (_attribute_assignment) {
		stream_mssg_on();
		print_string("@set_attr ");
		print_object(zargs[0]);
		print_string(" ");
		print_num(zargs[1]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_SET_ATTR_0);
		return;
	}

	// Get attribute address
	obj_addr = object_address(zargs[0]) + zargs[1] / 8;

	// Load attribute byte
	LOW_BYTE(obj_addr, value);

	// Set attribute bit
	value |= 0x80 >> (zargs[1] & 7);

	// Store attribute byte
	SET_BYTE(obj_addr, value);
}

void Processor::z_test_attr() {
	zword obj_addr;
	zbyte value;

	if (zargs[1] > ((h_version <= V3) ? 31 : 47))
		runtimeError(ERR_ILL_ATTR);

	// If we are monitoring attribute testing display a short note
	if (_attribute_testing) {
		stream_mssg_on();
		print_string("@test_attr ");
		print_object(zargs[0]);
		print_string(" ");
		print_num(zargs[1]);
		stream_mssg_off();
	}

	if (zargs[0] == 0) {
		runtimeError(ERR_TEST_ATTR_0);
		branch(false);
		return;
	}

	// Get attribute address
	obj_addr = object_address(zargs[0]) + zargs[1] / 8;

	// Load attribute byte
	LOW_BYTE(obj_addr, value);

	// Test attribute
	branch(value & (0x80 >> (zargs[1] & 7)));
}

} // End of namespace ZCode
} // End of namespace Glk
