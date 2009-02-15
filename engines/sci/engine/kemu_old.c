/***************************************************************************
 kemu_old.c Copyright (C) 2002 Christoph Reichenbach


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

    Christoph Reichenbach (CR) <jameson@linuxgames.com>

***************************************************************************/
/* Emulation code for old kernel functions */

#include <engine.h>
#include "kernel_compat.h"
#include "kernel_types.h"
#include "heap.h"

#define EMU_HEAP_START_ADDR 1000

#define EMU_TYPE_VERBATIM 0 /* Arithmetic values */
#define EMU_TYPE_REGISTERS 1 /* Must re-interpret afterwards. Also used for objects. */
#define EMU_TYPE_BLOCK 2 /* Bulk data; string or similar. May physically point to stack */

static int funct_nr;

typedef struct {
	heap_ptr start; /* Beginning of the block */
	reg_t pos; /* Needed for resolving conflicts */
	int length; /* Number of bytes occupied on the heap */
	object_t *obj; /* For object types: Pointer to the physical object */

	/* Emulation data part */
	int emudat_type; /* EMU_TYPE_... */
	heap_ptr emudat_location; /* Only for 1 and 2 */
	union {
		reg_t *regs; /* registers, for type 1 */
		byte *block; /* Bulk data location, for type 2 */
	} emu_data;
	int emudat_size; /* Amount of bytes or number of entries */
} emu_param_t;

static inline emu_param_t
identify_value(state_t *s, reg_t reg, heap_ptr firstfree)
{
	emu_param_t retval;
	int type = determine_reg_type(s, reg);

	retval.start = firstfree;
	retval.pos = reg;

	retval.obj = NULL;
	retval.length = 0;
	retval.emudat_type = EMU_TYPE_VERBATIM; /* To allow error aborts */

	if (type & KSIG_NULL)
		type &= KSIG_ARITHMETIC;

	switch (type) {

	case KSIG_ARITHMETIC: /* trivial */
		retval.emudat_size = 0;
		break;

	case KSIG_OBJECT:
		retval.emudat_type = EMU_TYPE_REGISTERS;
		retval.obj = obj_get(s, reg);
		if (!retval.obj) { BREAKPOINT(); }
		retval.length = -SCRIPT_OBJECT_MAGIC_OFFSET
			+ retval.obj->variables_nr * 4 /* values and selectors */
			+ 2; /* Extra magic selector word (is this really needed?) */
		retval.emu_data.regs = retval.obj->variables;
		retval.emudat_size = retval.obj->variables_nr;
		retval.emudat_location = retval.start - SCRIPT_OBJECT_MAGIC_OFFSET;
		break;

	case KSIG_REF: {
		retval.emudat_type = EMU_TYPE_BLOCK;

		retval.emu_data.block =
			s->seg_manager.dereference(&s->seg_manager,
						   reg, &retval.emudat_size);

		if (!retval.emu_data.block) {
			SCIkdebug(SCIkERROR, "Cannot handle references into segment type %02x"
				  " (from "PREG") during emulation\n",
				  type, PRINT_REG(reg));
			retval.emudat_type = EMU_TYPE_VERBATIM;
			retval.length = 0;
		}

		retval.length = retval.emudat_size;
		retval.emudat_location = retval.start;

		break;
	}

	default:
		SCIkdebug(SCIkERROR, "Cannot handle argument type %02x (from "PREG
			  ") during emulation\n",
			  type, PRINT_REG(reg));
	}

	return retval;
}

static inline void
writeout_value(state_t *s, emu_param_t *p)
{
	if (p->obj) /* First copy object; don't need to read back this part later */
		memcpy(s->heap + p->start, p->obj->base_obj + SCRIPT_OBJECT_MAGIC_OFFSET,
		       p->length);

	switch (p->emudat_type) {

	case EMU_TYPE_VERBATIM:
		SCIkdebug(SCIkEMU, "\tVerbatim/Arithmetic\n");
		return;

	case EMU_TYPE_REGISTERS: {
		int i, max = p->emudat_size;

		SCIkdebug(SCIkEMU, "\tObject, %d selectors\n", max);

		for (i = 0; i < max; i++) {
			byte *dest = s->heap + p->emudat_location + (i << 1);

			dest[0] = p->emu_data.regs[i].offset & 0xff;
			dest[1] = (p->emu_data.regs[i].offset >> 8) & 0xff;
			/* Assume they're all numeric values */
		}
		return;
	}

	case EMU_TYPE_BLOCK:
		SCIkdebug(SCIkEMU, "\tBulk\n");
		memcpy(s->heap + p->emudat_location, p->emu_data.block, p->emudat_size);
		return;

	default:
		BREAKPOINT();
	}
}

static inline void
recover_value(state_t *s, emu_param_t *p)
{ /* Writes back the value */
	switch (p->emudat_type) {

	case EMU_TYPE_VERBATIM:
		return;

	case EMU_TYPE_REGISTERS: {
		int i, max = p->emudat_size;
		for (i = 0; i < max; i++) {
			int val = GET_HEAP(p->emudat_location + (i << 1));
			if (p->emu_data.regs[i].offset != val) {
				SCIkdebug(SCIkEMU, "	Recovering property #%d from %04x: 0:%04x\n",
					  i, p->emudat_location + (i << 1), val);
				p->emu_data.regs[i] = make_reg(0, val);
			} else {
				SCIkdebug(SCIkEMU, "	Property #%d from %04x is unchanged (%04x vs "PREG")\n",
					  i, p->emudat_location + (i << 1), val, PRINT_REG(p->emu_data.regs[i]));
			}
			/* Don't overwrite unless something changed, to preserve pointers */
		}
		return;
	}

	case EMU_TYPE_BLOCK:
		memcpy(p->emu_data.block, s->heap + p->emudat_location, p->emudat_size);
		SCIkdebug(SCIkEMU, "    Recovering %d raw bytes from %04x\n",
			  p->emudat_size, p->emudat_location);
		return;

	default:
		BREAKPOINT();
	}
}

static void
_restrict_against(state_t *s, emu_param_t *self, emu_param_t *other)
{
	if (self->pos.segment != other->pos.segment)
		return;

	if (self->pos.offset <= other->pos.offset
	    && self->pos.offset + self->emudat_size > other->pos.offset) {
		mem_obj_t *mobj = GET_SEGMENT_ANY(s->seg_manager, self->pos.segment);

		self->emudat_size = other->pos.offset - self->pos.offset;

		if (mobj && mobj->type == MEM_OBJ_STACK)
			self->emudat_size *= sizeof(reg_t); /* Accomodate for size differences */
	}
}

static void
resolve_conflicts(state_t *s, emu_param_t *params, int count)
{
	int i, j;
	for (i = 0; i < count; i++)
		for (j = 0; j < count; j++)
			if (i != j)
				_restrict_against(s, params + i, params + j);
}

reg_t
kFsciEmu(state_t *s, int _funct_nr, int argc, reg_t *argv)
{
	emu_param_t *shadow_args;
	funct_nr = _funct_nr;

	if (!s->kfunct_emu_table[funct_nr]) {
		SCIkwarn(SCIkERROR, "Attempt to emulate unknown kernel function %x\n",
			 funct_nr);
		return NULL_REG;
	} else {
		heap_ptr argp = EMU_HEAP_START_ADDR; /* arguments go here */
		heap_ptr datap = argp + argc * 2; /* copied stuff goes here */
		int i;

		shadow_args = sci_malloc(sizeof(emu_param_t) * (argc + 1
								/* Prevents stupid warning */));
		memset(shadow_args, 0, sizeof(emu_param_t) * (argc + 1));

		SCIkdebug(SCIkEMU, "Emulating kernel call %s[%x] w/ %d arguments at HP:%04x\n",
			  s->kernel_names[funct_nr], funct_nr, argc, argp);

		for (i = 0; i < argc; i++) {
			int emu_value = argv[i].offset; /* Value we'll pass to the function */

			SCIkdebug(SCIkEMU, "   %3d : ["PREG"] ->\n", i, PRINT_REG(argv[i]));

			shadow_args[i] = identify_value(s, argv[i], datap);

			switch (shadow_args[i].emudat_type) {

			case EMU_TYPE_VERBATIM:
				break;

			case EMU_TYPE_REGISTERS:
			case EMU_TYPE_BLOCK:
				emu_value = shadow_args[i].emudat_location;
				break;

			default:
				BREAKPOINT();

			}

			SCIkdebug(SCIkEMU, "\t%04x [%d at %04x]\n",
				  emu_value, shadow_args[i].length, shadow_args[i].start);

			PUT_HEAP(argp, emu_value);
			argp += 2;

			if (0xffff - shadow_args[i].length < datap) {
				SCIkdebug(SCIkERROR, "Out of heap space while emulating!\n");
				return NULL_REG;
			}
			datap += shadow_args[i].length; /* Step over last block we wrote */
		}

		for (i = 0; i < argc; i++)
			writeout_value(s, shadow_args + i);

		resolve_conflicts(s, shadow_args, argc);

		s->kfunct_emu_table[funct_nr](s, funct_nr, argc, EMU_HEAP_START_ADDR);

		for (i = 0; i < argc; i++)
			recover_value(s, shadow_args + i);
		free(shadow_args);
		return make_reg(0, s->acc);
	}
}


int
read_selector16(state_t *s, heap_ptr object, selector_t selector_id, const char *file, int line)
{
	int slc_count = GET_HEAP(object + SCRIPT_SELECTORCTR_OFFSET);
	int i;
	heap_ptr slc_names = object + slc_count * 2;

	for (i = 0; i < slc_count; i++) {
		if (GET_HEAP(slc_names + (i<<1)) == selector_id)
			return GET_HEAP(object + (i<<1));
	}

	SCIkdebug(SCIkWARNING, "[EMU] Could not read selector %d from HP:%04x (%s, L%d)\n",
		  selector_id, object, file, line);
	return 0;
}

void
write_selector16(state_t *s, heap_ptr object, selector_t selector_id,
		 int value, const char *fname, int line)
{
	int slc_count = GET_HEAP(object + SCRIPT_SELECTORCTR_OFFSET);
	int i;
	heap_ptr slc_names = object + slc_count * 2;

	for (i = 0; i < slc_count; i++) {
		if (GET_HEAP(slc_names + (i<<1)) == selector_id) {
			PUT_HEAP(object + (i<<1), value);
			return;
		}
	}

	SCIkdebug(SCIkWARNING, "[EMU] Could not write selector %d from HP:%04x (%s, L%d)\n",
		  selector_id, object, fname, line);
}

