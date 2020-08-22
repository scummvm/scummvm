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

#include "glk/glulx/glulx.h"

namespace Glk {
namespace Glulx {

/**
 * Git passes along function arguments in reverse order. To make our lives more interesting
 */
#ifdef ARGS_REVERSED
#define ARG(argv, argc, ix) (argv[(argc-1)-ix])
#else
#define ARG(argv, argc, ix) (argv[ix])
#endif

/**
 * Any function can be called with any number of arguments. This macro lets us snarf a given argument,
 * or zero if it wasn't supplied.
 */
#define ARG_IF_GIVEN(argv, argc, ix)  ((argc > ix) ? (ARG(argv, argc, ix)) : 0)

acceleration_func Glulx::accel_find_func(uint index) {
	switch (index) {
	case 0:
		return nullptr;     // 0 always means no acceleration
	case 1:
		return &Glulx::func_1_z__region;
	case 2:
		return &Glulx::func_2_cp__tab;
	case 3:
		return &Glulx::func_3_ra__pr;
	case 4:
		return &Glulx::func_4_rl__pr;
	case 5:
		return &Glulx::func_5_oc__cl;
	case 6:
		return &Glulx::func_6_rv__pr;
	case 7:
		return &Glulx::func_7_op__pr;
	case 8:
		return &Glulx::func_8_cp__tab;
	case 9:
		return &Glulx::func_9_ra__pr;
	case 10:
		return &Glulx::func_10_rl__pr;
	case 11:
		return &Glulx::func_11_oc__cl;
	case 12:
		return &Glulx::func_12_rv__pr;
	case 13:
		return &Glulx::func_13_op__pr;
	}
	return nullptr;
}

acceleration_func Glulx::accel_get_func(uint addr) {
	int bucknum;
	accelentry_t *ptr;

	if (!accelentries)
		return nullptr;

	bucknum = (addr % ACCEL_HASH_SIZE);
	for (ptr = accelentries[bucknum]; ptr; ptr = ptr->next) {
		if (ptr->addr == addr)
			return ptr->func;
	}
	return nullptr;
}

void Glulx::accel_iterate_funcs(void (*func)(uint index, uint addr)) {
	int bucknum;
	accelentry_t *ptr;

	if (!accelentries)
		return;

	for (bucknum = 0; bucknum < ACCEL_HASH_SIZE; bucknum++) {
		for (ptr = accelentries[bucknum]; ptr; ptr = ptr->next) {
			if (ptr->func) {
				func(ptr->index, ptr->addr);
			}
		}
	}
}

void Glulx::accel_set_func(uint index, uint addr) {
	int bucknum;
	accelentry_t *ptr;
	int functype;
	acceleration_func new_func = nullptr;

	/* Check the Glulx type identifier byte. */
	functype = Mem1(addr);
	if (functype != 0xC0 && functype != 0xC1) {
		fatal_error_i("Attempt to accelerate non-function.", addr);
	}

	if (!accelentries) {
		accelentries = (accelentry_t **)glulx_malloc(ACCEL_HASH_SIZE
		               * sizeof(accelentry_t *));
		if (!accelentries)
			fatal_error("Cannot malloc acceleration table.");
		for (bucknum = 0; bucknum < ACCEL_HASH_SIZE; bucknum++)
			accelentries[bucknum] = nullptr;
	}

	new_func = accel_find_func(index);
	/* Might be nullptr, if the index is zero or not recognized. */

	bucknum = (addr % ACCEL_HASH_SIZE);
	for (ptr = accelentries[bucknum]; ptr; ptr = ptr->next) {
		if (ptr->addr == addr)
			break;
	}
	if (!ptr) {
		if (!new_func) {
			return; /* no need for a new entry */
		}
		ptr = (accelentry_t *)glulx_malloc(sizeof(accelentry_t));
		if (!ptr)
			fatal_error("Cannot malloc acceleration entry.");
		ptr->addr = addr;
		ptr->index = 0;
		ptr->func = nullptr;
		ptr->next = accelentries[bucknum];
		accelentries[bucknum] = ptr;
	}

	ptr->index = index;
	ptr->func = new_func;
}

void Glulx::accel_set_param(uint index, uint val) {
	switch (index) {
	case 0:
		classes_table = val;
		break;
	case 1:
		indiv_prop_start = val;
		break;
	case 2:
		class_metaclass = val;
		break;
	case 3:
		object_metaclass = val;
		break;
	case 4:
		routine_metaclass = val;
		break;
	case 5:
		string_metaclass = val;
		break;
	case 6:
		self = val;
		break;
	case 7:
		num_attr_bytes = val;
		break;
	case 8:
		cpv__start = val;
		break;
	}
}

uint Glulx::accel_get_param_count() const {
	return 9;
}

uint Glulx::accel_get_param(uint index) const {
	switch (index) {
	case 0:
		return classes_table;
	case 1:
		return indiv_prop_start;
	case 2:
		return class_metaclass;
	case 3:
		return object_metaclass;
	case 4:
		return routine_metaclass;
	case 5:
		return string_metaclass;
	case 6:
		return self;
	case 7:
		return num_attr_bytes;
	case 8:
		return cpv__start;
	default:
		return 0;
	}
}

void Glulx::accel_error(const char *msg) {
	glk_put_char('\n');
	glk_put_string(msg);
	glk_put_char('\n');
}

int Glulx::obj_in_class(uint obj) {
	// This checks whether obj is contained in Class, not whether it is a member of Class
	return (Mem4(obj + 13 + num_attr_bytes) == class_metaclass);
}

uint Glulx::get_prop(uint obj, uint id) {
	uint cla = 0;
	uint prop;
	uint call_argv[2];

	if (id & 0xFFFF0000) {
		cla = Mem4(classes_table + ((id & 0xFFFF) * 4));
		ARG(call_argv, 2, 0) = obj;
		ARG(call_argv, 2, 1) = cla;
		if (func_5_oc__cl(2, call_argv) == 0)
			return 0;

		id >>= 16;
		obj = cla;
	}

	ARG(call_argv, 2, 0) = obj;
	ARG(call_argv, 2, 1) = id;
	prop = func_2_cp__tab(2, call_argv);
	if (prop == 0)
		return 0;

	if (obj_in_class(obj) && (cla == 0)) {
		if ((id < indiv_prop_start) || (id >= indiv_prop_start + 8))
			return 0;
	}

	if (Mem4(self) != obj) {
		if (Mem1(prop + 9) & 1)
			return 0;
	}
	return prop;
}

uint Glulx::get_prop_new(uint obj, uint id) {
	uint cla = 0;
	uint prop;
	uint call_argv[2];

	if (id & 0xFFFF0000) {
		cla = Mem4(classes_table + ((id & 0xFFFF) * 4));
		ARG(call_argv, 2, 0) = obj;
		ARG(call_argv, 2, 1) = cla;
		if (func_11_oc__cl(2, call_argv) == 0)
			return 0;

		id >>= 16;
		obj = cla;
	}

	ARG(call_argv, 2, 0) = obj;
	ARG(call_argv, 2, 1) = id;
	prop = func_8_cp__tab(2, call_argv);
	if (prop == 0)
		return 0;

	if (obj_in_class(obj) && (cla == 0)) {
		if ((id < indiv_prop_start) || (id >= indiv_prop_start + 8))
			return 0;
	}

	if (Mem4(self) != obj) {
		if (Mem1(prop + 9) & 1)
			return 0;
	}
	return prop;
}

uint Glulx::func_1_z__region(uint argc, uint *argv) {
	uint addr;
	uint tb;

	if (argc < 1)
		return 0;

	addr = ARG(argv, argc, 0);
	if (addr < 36)
		return 0;
	if (addr >= endmem)
		return 0;

	tb = Mem1(addr);
	if (tb >= 0xE0) {
		return 3;
	}
	if (tb >= 0xC0) {
		return 2;
	}
	if (tb >= 0x70 && tb <= 0x7F && addr >= ramstart) {
		return 1;
	}
	return 0;
}

uint Glulx::func_2_cp__tab(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint otab, max;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	if (func_1_z__region(1, &obj) != 1) {
		accel_error("[** Programming error: tried to find the \".\" of (something) **]");
		return 0;
	}

	otab = Mem4(obj + 16);
	if (!otab)
		return 0;

	max = Mem4(otab);
	otab += 4;
	/* @binarysearch id 2 otab 10 max 0 0 res; */
	return binary_search(id, 2, otab, 10, max, 0, 0);
}

uint Glulx::func_3_ra__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint prop;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	prop = get_prop(obj, id);
	if (prop == 0)
		return 0;

	return Mem4(prop + 4);
}

uint Glulx::func_4_rl__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint prop;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	prop = get_prop(obj, id);
	if (prop == 0)
		return 0;

	return 4 * Mem2(prop + 2);
}

uint Glulx::func_5_oc__cl(uint argc, uint *argv) {
	uint obj;
	uint cla;
	uint zr, prop, inlist, inlistlen, jx;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	cla = ARG_IF_GIVEN(argv, argc, 1);

	zr = func_1_z__region(1, &obj);
	if (zr == 3)
		return (cla == string_metaclass) ? 1 : 0;
	if (zr == 2)
		return (cla == routine_metaclass) ? 1 : 0;
	if (zr != 1)
		return 0;

	if (cla == class_metaclass) {
		if (obj_in_class(obj))
			return 1;
		if (obj == class_metaclass)
			return 1;
		if (obj == string_metaclass)
			return 1;
		if (obj == routine_metaclass)
			return 1;
		if (obj == object_metaclass)
			return 1;
		return 0;
	}
	if (cla == object_metaclass) {
		if (obj_in_class(obj))
			return 0;
		if (obj == class_metaclass)
			return 0;
		if (obj == string_metaclass)
			return 0;
		if (obj == routine_metaclass)
			return 0;
		if (obj == object_metaclass)
			return 0;
		return 1;
	}
	if ((cla == string_metaclass) || (cla == routine_metaclass))
		return 0;

	if (!obj_in_class(cla)) {
		accel_error("[** Programming error: tried to apply 'ofclass' with non-class **]");
		return 0;
	}

	prop = get_prop(obj, 2);
	if (prop == 0)
		return 0;

	inlist = Mem4(prop + 4);
	if (inlist == 0)
		return 0;

	inlistlen = Mem2(prop + 2);
	for (jx = 0; jx < inlistlen; jx++) {
		if (Mem4(inlist + (4 * jx)) == cla)
			return 1;
	}
	return 0;
}

uint Glulx::func_6_rv__pr(uint argc, uint *argv) {
	uint id;
	uint addr;

	id = ARG_IF_GIVEN(argv, argc, 1);

	addr = func_3_ra__pr(argc, argv);

	if (addr == 0) {
		if ((id > 0) && (id < indiv_prop_start))
			return Mem4(cpv__start + (4 * id));

		accel_error("[** Programming error: tried to read (something) **]");
		return 0;
	}

	return Mem4(addr);
}

uint Glulx::func_7_op__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint zr;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	zr = func_1_z__region(1, &obj);
	if (zr == 3) {
		/* print is INDIV_PROP_START+6 */
		if (id == indiv_prop_start + 6)
			return 1;
		/* print_to_array is INDIV_PROP_START+7 */
		if (id == indiv_prop_start + 7)
			return 1;
		return 0;
	}
	if (zr == 2) {
		/* call is INDIV_PROP_START+5 */
		return ((id == indiv_prop_start + 5) ? 1 : 0);
	}
	if (zr != 1)
		return 0;

	if ((id >= indiv_prop_start) && (id < indiv_prop_start + 8)) {
		if (obj_in_class(obj))
			return 1;
	}

	return ((func_3_ra__pr(argc, argv)) ? 1 : 0);
}

uint Glulx::func_8_cp__tab(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint otab, max;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	if (func_1_z__region(1, &obj) != 1) {
		accel_error("[** Programming error: tried to find the \".\" of (something) **]");
		return 0;
	}

	otab = Mem4(obj + 4 * (3 + (int)(num_attr_bytes / 4)));
	if (!otab)
		return 0;

	max = Mem4(otab);
	otab += 4;
	/* @binarysearch id 2 otab 10 max 0 0 res; */
	return binary_search(id, 2, otab, 10, max, 0, 0);
}

uint Glulx::func_9_ra__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint prop;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	prop = get_prop_new(obj, id);
	if (prop == 0)
		return 0;

	return Mem4(prop + 4);
}

uint Glulx::func_10_rl__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint prop;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	prop = get_prop_new(obj, id);
	if (prop == 0)
		return 0;

	return 4 * Mem2(prop + 2);
}

uint Glulx::func_11_oc__cl(uint argc, uint *argv) {
	uint obj;
	uint cla;
	uint zr, prop, inlist, inlistlen, jx;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	cla = ARG_IF_GIVEN(argv, argc, 1);

	zr = func_1_z__region(1, &obj);
	if (zr == 3)
		return (cla == string_metaclass) ? 1 : 0;
	if (zr == 2)
		return (cla == routine_metaclass) ? 1 : 0;
	if (zr != 1)
		return 0;

	if (cla == class_metaclass) {
		if (obj_in_class(obj))
			return 1;
		if (obj == class_metaclass)
			return 1;
		if (obj == string_metaclass)
			return 1;
		if (obj == routine_metaclass)
			return 1;
		if (obj == object_metaclass)
			return 1;
		return 0;
	}
	if (cla == object_metaclass) {
		if (obj_in_class(obj))
			return 0;
		if (obj == class_metaclass)
			return 0;
		if (obj == string_metaclass)
			return 0;
		if (obj == routine_metaclass)
			return 0;
		if (obj == object_metaclass)
			return 0;
		return 1;
	}
	if ((cla == string_metaclass) || (cla == routine_metaclass))
		return 0;

	if (!obj_in_class(cla)) {
		accel_error("[** Programming error: tried to apply 'ofclass' with non-class **]");
		return 0;
	}

	prop = get_prop_new(obj, 2);
	if (prop == 0)
		return 0;

	inlist = Mem4(prop + 4);
	if (inlist == 0)
		return 0;

	inlistlen = Mem2(prop + 2);
	for (jx = 0; jx < inlistlen; jx++) {
		if (Mem4(inlist + (4 * jx)) == cla)
			return 1;
	}
	return 0;
}

uint Glulx::func_12_rv__pr(uint argc, uint *argv) {
	uint id;
	uint addr;

	id = ARG_IF_GIVEN(argv, argc, 1);

	addr = func_9_ra__pr(argc, argv);

	if (addr == 0) {
		if ((id > 0) && (id < indiv_prop_start))
			return Mem4(cpv__start + (4 * id));

		accel_error("[** Programming error: tried to read (something) **]");
		return 0;
	}

	return Mem4(addr);
}

uint Glulx::func_13_op__pr(uint argc, uint *argv) {
	uint obj;
	uint id;
	uint zr;

	obj = ARG_IF_GIVEN(argv, argc, 0);
	id = ARG_IF_GIVEN(argv, argc, 1);

	zr = func_1_z__region(1, &obj);
	if (zr == 3) {
		/* print is INDIV_PROP_START+6 */
		if (id == indiv_prop_start + 6)
			return 1;
		/* print_to_array is INDIV_PROP_START+7 */
		if (id == indiv_prop_start + 7)
			return 1;
		return 0;
	}
	if (zr == 2) {
		/* call is INDIV_PROP_START+5 */
		return ((id == indiv_prop_start + 5) ? 1 : 0);
	}
	if (zr != 1)
		return 0;

	if ((id >= indiv_prop_start) && (id < indiv_prop_start + 8)) {
		if (obj_in_class(obj))
			return 1;
	}

	return ((func_9_ra__pr(argc, argv)) ? 1 : 0);
}

} // End of namespace Glulx
} // End of namespace Glk
