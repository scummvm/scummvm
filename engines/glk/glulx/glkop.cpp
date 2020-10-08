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

/* This code is actually very general; it could work for almost any
   32-bit VM which remotely resembles Glulx or the Z-machine in design.

   To be precise, we make the following assumptions:

   - An argument list is an array of 32-bit values, which can represent
     either integers or addresses.
   - We can read or write to a 32-bit integer in VM memory using the macros
     ReadMemory(addr) and WriteMemory(addr), where addr is an address
     taken from the argument list.
   - A character array is a sequence of bytes somewhere in VM memory.
     The array can be turned into a C char array by the macro
     CaptureCArray(addr, len), and released by ReleaseCArray().
     The passin, passout hints may be used to avoid unnecessary copying.
   - An integer array is a sequence of integers somewhere in VM memory.
     The array can be turned into a C integer array by the macro
     CaptureIArray(addr, len), and released by ReleaseIArray().
     These macros are responsible for fixing byte-order and alignment
     (if the C ABI does not match the VM's). The passin, passout hints
     may be used to avoid unnecessary copying.
   - A Glk object array is a sequence of integers in VM memory. It is
     turned into a C pointer array (remember that C pointers may be more
     than 4 bytes!) The pointer array is allocated by
     CapturePtrArray(addr, len, objclass) and released by ReleasePtrArray().
     Again, the macros handle the conversion.
   - A Glk structure (such as event_t) is a set of integers somewhere
     in VM memory, which can be read and written with the macros
     ReadStructField(addr, fieldnum) and WriteStructField(addr, fieldnum).
     The fieldnum is an integer (from 0 to 3, for event_t.)
   - A VM string can be turned into a C-style string with the macro
     ptr = DecodeVMString(addr). After the string is used, this code
     calls ReleaseVMString(ptr), which should free any memory that
     DecodeVMString allocates.
   - A VM Unicode string can be turned into a zero-terminated array
     of 32-bit integers, in the same way, with DecodeVMUstring
     and ReleaseVMUstring.

     To work this code over for a new VM, just diddle the macros.
*/

static gidispatch_rock_t classtable_register(void *obj, uint objclass) {
	return g_vm->glulxe_classtable_register(obj, objclass);
}

static void classtable_unregister(void *obj, uint objclass, gidispatch_rock_t objrock) {
	g_vm->glulxe_classtable_unregister(obj, objclass, objrock);
}

static gidispatch_rock_t retained_register(void *array, uint len, const char *typecode) {
	return g_vm->glulxe_retained_register(array, len, typecode);
}

static void retained_unregister(void *array, uint len, const char *typecode, gidispatch_rock_t objrock) {
	g_vm->glulxe_retained_unregister(array, len, typecode, objrock);
}

void Glulx::glkopInit() {
	library_select_hook = nullptr;
	arrays = nullptr;
	num_classes = 0;
	classes = nullptr;
}

bool Glulx::init_dispatch() {
	int ix;

	/* Set up the game-ID hook. (This is ifdeffed because not all Glk
	   libraries have this call.) */
#ifdef GI_DISPA_GAME_ID_AVAILABLE
	gidispatch_set_game_id_hook(&get_game_id);
#endif /* GI_DISPA_GAME_ID_AVAILABLE */

	/* Allocate the class hash tables. */
	num_classes = gidispatch_count_classes();
	classes = (classtable_t **)glulx_malloc(num_classes  * sizeof(classtable_t *));
	if (!classes)
		return false;

	for (ix = 0; ix < num_classes; ix++) {
		classes[ix] = new_classtable((glulx_random() % (uint)(101)) + 1);
		if (!classes[ix])
			return false;
	}

	/* Set up the two callbacks. */
	gidispatch_set_object_registry(&classtable_register, &classtable_unregister);
	gidispatch_set_retained_registry(&retained_register, &retained_unregister);

	/* If the library supports autorestore callbacks, set those up too.
	   (These are only used in iosglk, currently.) */
#ifdef GIDISPATCH_AUTORESTORE_REGISTRY
	gidispatch_set_autorestore_registry(&glulxe_array_locate, &glulxe_array_restore);
#endif /* GIDISPATCH_AUTORESTORE_REGISTRY */

	return true;
}

uint Glulx::perform_glk(uint funcnum, uint numargs, uint *arglist) {
	uint retval = 0;

	switch (funcnum) {
	/* To speed life up, we implement commonly-used Glk functions
	   directly -- instead of bothering with the whole prototype
	   mess. */

	case 0x0047: /* stream_set_current */
		if (numargs != 1)
			goto WrongArgNum;
		glk_stream_set_current(find_stream_by_id(arglist[0]));
		break;
	case 0x0048: /* stream_get_current */
		if (numargs != 0)
			goto WrongArgNum;
		retval = find_id_for_stream(glk_stream_get_current());
		break;
	case 0x0080: /* put_char */
		if (numargs != 1)
			goto WrongArgNum;
		glk_put_char(arglist[0] & 0xFF);
		break;
	case 0x0081: /* put_char_stream */
		if (numargs != 2)
			goto WrongArgNum;
		glk_put_char_stream(find_stream_by_id(arglist[0]), arglist[1] & 0xFF);
		break;
	case 0x00C0: /* select */
		/* call a library hook on every glk_select() */
		if (library_select_hook)
			library_select_hook(arglist[0]);
		/* but then fall through to full dispatcher, because there's no real
		   need for speed here */
		goto FullDispatcher;
	case 0x00A0: /* char_to_lower */
		if (numargs != 1)
			goto WrongArgNum;
		retval = glk_char_to_lower(arglist[0] & 0xFF);
		break;
	case 0x00A1: /* char_to_upper */
		if (numargs != 1)
			goto WrongArgNum;
		retval = glk_char_to_upper(arglist[0] & 0xFF);
		break;
	case 0x0128: /* put_char_uni */
		if (numargs != 1)
			goto WrongArgNum;
		glk_put_char_uni(arglist[0]);
		break;
	case 0x012B: /* put_char_stream_uni */
		if (numargs != 2)
			goto WrongArgNum;
		glk_put_char_stream_uni(find_stream_by_id(arglist[0]), arglist[1]);
		break;

WrongArgNum:
		error("Wrong number of arguments to Glk function.");
		break;

FullDispatcher:
	default: {
		/* Go through the full dispatcher prototype foo. */
		const char *proto, *cx;
		dispatch_splot_t splot;
		int argnum, argnum2;

		/* Grab the string. */
		proto = gidispatch_prototype(funcnum);
		if (!proto)
			error("Unknown Glk function.");

		splot.varglist = arglist;
		splot.numvargs = numargs;
		splot.retval = &retval;

		/* The work goes in four phases. First, we figure out how many
		   arguments we want, and allocate space for the Glk argument
		   list. Then we go through the Glulx arguments and load them
		   into the Glk list. Then we call. Then we go through the
		   arguments again, unloading the data back into Glulx memory. */

		/* Phase 0. */
		prepare_glk_args(proto, &splot);

		/* Phase 1. */
		argnum = 0;
		cx = proto;
		parse_glk_args(&splot, &cx, 0, &argnum, 0, 0);

		/* Phase 2. */
		gidispatch_call(funcnum, argnum, splot.garglist);

		// WORKAROUND: For stream_open_file calls, for savegame handling
		// we need to store a copy of what the savegame description was,
		// for use when we actually generate the savefile contents
		if (funcnum == 0x42) {
			frefid_t fref = (frefid_t)splot.garglist[0]._opaqueref;
			_savegameDescription = fref->_description;
		}

		/* Phase 3. */
		argnum2 = 0;
		cx = proto;
		unparse_glk_args(&splot, &cx, 0, &argnum2, 0, 0);
		if (argnum != argnum2)
			error("Argument counts did not match.");

		break;
	}
	}

	return retval;
}

const char *Glulx::read_prefix(const char *cx, int *isref, int *isarray, int *passin, int *passout,
                                int *nullok, int *isretained,  int *isreturn) {
	*isref = false;
	*passin = false;
	*passout = false;
	*nullok = true;
	*isarray = false;
	*isretained = false;
	*isreturn = false;
	while (1) {
		if (*cx == '<') {
			*isref = true;
			*passout = true;
		} else if (*cx == '>') {
			*isref = true;
			*passin = true;
		} else if (*cx == '&') {
			*isref = true;
			*passout = true;
			*passin = true;
		} else if (*cx == '+') {
			*nullok = false;
		} else if (*cx == ':') {
			*isref = true;
			*passout = true;
			*nullok = false;
			*isreturn = true;
		} else if (*cx == '#') {
			*isarray = true;
		} else if (*cx == '!') {
			*isretained = true;
		} else {
			break;
		}
		cx++;
	}
	return cx;
}

void Glulx::prepare_glk_args(const char *proto, dispatch_splot_t *splot) {
	static gluniversal_t *garglist = nullptr;
	static int garglist_size = 0;

	int ix;
	int numwanted, numvargswanted, maxargs;
	const char *cx;

	cx = proto;
	numwanted = 0;
	while (*cx >= '0' && *cx <= '9') {
		numwanted = 10 * numwanted + (*cx - '0');
		cx++;
	}
	splot->numwanted = numwanted;

	maxargs = 0;
	numvargswanted = 0;
	for (ix = 0; ix < numwanted; ix++) {
		int isref, passin, passout, nullok, isarray, isretained, isreturn;
		cx = read_prefix(cx, &isref, &isarray, &passin, &passout, &nullok,
		                 &isretained, &isreturn);
		if (isref) {
			maxargs += 2;
		} else {
			maxargs += 1;
		}
		if (!isreturn) {
			if (isarray) {
				numvargswanted += 2;
			} else {
				numvargswanted += 1;
			}
		}

		if (*cx == 'I' || *cx == 'C') {
			cx += 2;
		} else if (*cx == 'Q') {
			cx += 2;
		} else if (*cx == 'S' || *cx == 'U') {
			cx += 1;
		} else if (*cx == '[') {
			int refdepth, nwx;
			cx++;
			nwx = 0;
			while (*cx >= '0' && *cx <= '9') {
				nwx = 10 * nwx + (*cx - '0');
				cx++;
			}
			maxargs += nwx; /* This is *only* correct because all structs contain
                         plain values. */
			refdepth = 1;
			while (refdepth > 0) {
				if (*cx == '[')
					refdepth++;
				else if (*cx == ']')
					refdepth--;
				cx++;
			}
		} else {
			error("Illegal format string.");
		}
	}

	if (*cx != ':' && *cx != '\0')
		error("Illegal format string.");

	splot->maxargs = maxargs;

	if (splot->numvargs != numvargswanted)
		error("Wrong number of arguments to Glk function.");

	if (garglist && garglist_size < maxargs) {
		glulx_free(garglist);
		garglist = nullptr;
		garglist_size = 0;
	}
	if (!garglist) {
		garglist_size = maxargs + 16;
		garglist = (gluniversal_t *)glulx_malloc(garglist_size
		           * sizeof(gluniversal_t));
	}
	if (!garglist)
		error("Unable to allocate storage for Glk arguments.");

	splot->garglist = garglist;
}

void Glulx::parse_glk_args(dispatch_splot_t *splot, const char **proto, int depth, int *argnumptr,
                            uint subaddress, int subpassin) {
	const char *cx;
	int ix, argx;
	int gargnum, numwanted;
	void *opref;
	gluniversal_t *garglist;
	uint *varglist;

	garglist = splot->garglist;
	varglist = splot->varglist;
	gargnum = *argnumptr;
	cx = *proto;

	numwanted = 0;
	while (*cx >= '0' && *cx <= '9') {
		numwanted = 10 * numwanted + (*cx - '0');
		cx++;
	}

	for (argx = 0, ix = 0; argx < numwanted; argx++, ix++) {
		char typeclass;
		int skipval;
		int isref, passin, passout, nullok, isarray, isretained, isreturn;
		cx = read_prefix(cx, &isref, &isarray, &passin, &passout, &nullok,
		                 &isretained, &isreturn);

		typeclass = *cx;
		cx++;

		skipval = false;
		if (isref) {
			if (!isreturn && varglist[ix] == 0) {
				if (!nullok)
					error("Zero passed invalidly to Glk function.");
				garglist[gargnum]._ptrflag = false;
				gargnum++;
				skipval = true;
			} else {
				garglist[gargnum]._ptrflag = true;
				gargnum++;
			}
		}
		if (!skipval) {
			uint thisval;

			if (typeclass == '[') {

				parse_glk_args(splot, &cx, depth + 1, &gargnum, varglist[ix], passin);

			} else if (isarray) {
				/* definitely isref */

				switch (typeclass) {
				case 'C':
					/* This test checks for a giant array length, which is
					   deprecated. It displays a warning and cuts it down to
					   something reasonable. Future releases of this interpreter
					   may remove this test and go on to verify_array_addresses(),
					   which treats this case as a fatal error. */
					if (varglist[ix + 1] > endmem
					        || varglist[ix] + varglist[ix + 1] > endmem) {
						nonfatal_warning_i("Memory access was much too long -- perhaps a print_to_array call with only one argument", varglist[ix + 1]);
						varglist[ix + 1] = endmem - varglist[ix];
					}
					verify_array_addresses(varglist[ix], varglist[ix + 1], 1);
					garglist[gargnum]._array = CaptureCArray(varglist[ix], varglist[ix + 1], passin);
					gargnum++;
					ix++;
					garglist[gargnum]._uint = varglist[ix];
					gargnum++;
					cx++;
					break;
				case 'I':
					/* See comment above. */
					if (varglist[ix + 1] > endmem / 4
					        || varglist[ix + 1] > (endmem - varglist[ix]) / 4) {
						nonfatal_warning_i("Memory access was much too long -- perhaps a print_to_array call with only one argument", varglist[ix + 1]);
						varglist[ix + 1] = (endmem - varglist[ix]) / 4;
					}
					verify_array_addresses(varglist[ix], varglist[ix + 1], 4);
					garglist[gargnum]._array = CaptureIArray(varglist[ix], varglist[ix + 1], passin);
					gargnum++;
					ix++;
					garglist[gargnum]._uint = varglist[ix];
					gargnum++;
					cx++;
					break;
				case 'Q':
					/* This case was added after the giant arrays were deprecated,
					   so we don't bother to allow for that case. We just verify
					   the length. */
					verify_array_addresses(varglist[ix], varglist[ix + 1], 4);
					garglist[gargnum]._array = CapturePtrArray(varglist[ix], varglist[ix + 1], (*cx - 'a'), passin);
					gargnum++;
					ix++;
					garglist[gargnum]._uint = varglist[ix];
					gargnum++;
					cx++;
					break;
				default:
					error("Illegal format string.");
					break;
				}
			} else {
				/* a plain value or a reference to one. */

				if (isreturn) {
					thisval = 0;
				} else if (depth > 0) {
					/* Definitely not isref or isarray. */
					if (subpassin)
						thisval = ReadStructField(subaddress, ix);
					else
						thisval = 0;
				} else if (isref) {
					if (passin)
						thisval = ReadMemory(varglist[ix]);
					else
						thisval = 0;
				} else {
					thisval = varglist[ix];
				}

				switch (typeclass) {
				case 'I':
					if (*cx == 'u')
						garglist[gargnum]._uint = (uint)(thisval);
					else if (*cx == 's')
						garglist[gargnum]._sint = (int)(thisval);
					else
						error("Illegal format string.");
					gargnum++;
					cx++;
					break;
				case 'Q':
					if (thisval) {
						opref = classes_get(*cx - 'a', thisval);
						if (!opref) {
							error("Reference to nonexistent Glk object.");
						}
					} else {
						opref = nullptr;
					}
					garglist[gargnum]._opaqueref = opref;
					gargnum++;
					cx++;
					break;
				case 'C':
					if (*cx == 'u')
						garglist[gargnum]._uch = (unsigned char)(thisval);
					else if (*cx == 's')
						garglist[gargnum]._sch = (signed char)(thisval);
					else if (*cx == 'n')
						garglist[gargnum]._ch = (char)(thisval);
					else
						error("Illegal format string.");
					gargnum++;
					cx++;
					break;
				case 'S':
					garglist[gargnum]._charstr = DecodeVMString(thisval);
					gargnum++;
					break;
#ifdef GLK_MODULE_UNICODE
				case 'U':
					garglist[gargnum]._unicharstr = DecodeVMUstring(thisval);
					gargnum++;
					break;
#endif /* GLK_MODULE_UNICODE */
				default:
					error("Illegal format string.");
					break;
				}
			}
		} else {
			/* We got a null reference, so we have to skip the format element. */
			if (typeclass == '[') {
				int numsubwanted, refdepth;
				numsubwanted = 0;
				while (*cx >= '0' && *cx <= '9') {
					numsubwanted = 10 * numsubwanted + (*cx - '0');
					cx++;
				}
				refdepth = 1;
				while (refdepth > 0) {
					if (*cx == '[')
						refdepth++;
					else if (*cx == ']')
						refdepth--;
					cx++;
				}
			} else if (typeclass == 'S' || typeclass == 'U') {
				/* leave it */
			} else {
				cx++;
				if (isarray)
					ix++;
			}
		}
	}

	if (depth > 0) {
		if (*cx != ']')
			error("Illegal format string.");
		cx++;
	} else {
		if (*cx != ':' && *cx != '\0')
			error("Illegal format string.");
	}

	*proto = cx;
	*argnumptr = gargnum;
}

void Glulx::unparse_glk_args(dispatch_splot_t *splot, const char **proto, int depth,
                              int *argnumptr, uint subaddress, int subpassout) {
	const char *cx;
	int ix, argx;
	int gargnum, numwanted;
	void *opref;
	gluniversal_t *garglist;
	uint *varglist;

	garglist = splot->garglist;
	varglist = splot->varglist;
	gargnum = *argnumptr;
	cx = *proto;

	numwanted = 0;
	while (*cx >= '0' && *cx <= '9') {
		numwanted = 10 * numwanted + (*cx - '0');
		cx++;
	}

	for (argx = 0, ix = 0; argx < numwanted; argx++, ix++) {
		char typeclass;
		int skipval;
		int isref, passin, passout, nullok, isarray, isretained, isreturn;
		cx = read_prefix(cx, &isref, &isarray, &passin, &passout, &nullok,
		                 &isretained, &isreturn);

		typeclass = *cx;
		cx++;

		skipval = false;
		if (isref) {
			if (!isreturn && varglist[ix] == 0) {
				if (!nullok)
					error("Zero passed invalidly to Glk function.");
				garglist[gargnum]._ptrflag = false;
				gargnum++;
				skipval = true;
			} else {
				garglist[gargnum]._ptrflag = true;
				gargnum++;
			}
		}
		if (!skipval) {
			uint thisval = 0;

			if (typeclass == '[') {

				unparse_glk_args(splot, &cx, depth + 1, &gargnum, varglist[ix], passout);

			} else if (isarray) {
				/* definitely isref */

				switch (typeclass) {
				case 'C':
					ReleaseCArray((char *)garglist[gargnum]._array, varglist[ix], varglist[ix + 1], passout);
					gargnum++;
					ix++;
					gargnum++;
					cx++;
					break;
				case 'I':
					ReleaseIArray((uint *)garglist[gargnum]._array, varglist[ix], varglist[ix + 1], passout);
					gargnum++;
					ix++;
					gargnum++;
					cx++;
					break;
				case 'Q':
					ReleasePtrArray((void **)garglist[gargnum]._array, varglist[ix], varglist[ix + 1], (*cx - 'a'), passout);
					gargnum++;
					ix++;
					gargnum++;
					cx++;
					break;
				default:
					error("Illegal format string.");
					break;
				}
			} else {
				/* a plain value or a reference to one. */

				if (isreturn || (depth > 0 && subpassout) || (isref && passout)) {
					skipval = false;
				} else {
					skipval = true;
				}

				switch (typeclass) {
				case 'I':
					if (!skipval) {
						if (*cx == 'u')
							thisval = (uint)garglist[gargnum]._uint;
						else if (*cx == 's')
							thisval = (uint)garglist[gargnum]._sint;
						else
							error("Illegal format string.");
					}
					gargnum++;
					cx++;
					break;
				case 'Q':
					if (!skipval) {
						opref = garglist[gargnum]._opaqueref;
						if (opref) {
							gidispatch_rock_t objrock = gidispatch_get_objrock(opref, *cx - 'a');
							assert(objrock.ptr);
							thisval = ((classref_t *)objrock.ptr)->id;
						} else {
							thisval = 0;
						}
					}
					gargnum++;
					cx++;
					break;
				case 'C':
					if (!skipval) {
						if (*cx == 'u')
							thisval = (uint)garglist[gargnum]._uch;
						else if (*cx == 's')
							thisval = (uint)garglist[gargnum]._sch;
						else if (*cx == 'n')
							thisval = (uint)garglist[gargnum]._ch;
						else
							error("Illegal format string.");
					}
					gargnum++;
					cx++;
					break;
				case 'S':
					if (garglist[gargnum]._charstr)
						ReleaseVMString(garglist[gargnum]._charstr);
					gargnum++;
					break;
#ifdef GLK_MODULE_UNICODE
				case 'U':
					if (garglist[gargnum]._unicharstr)
						ReleaseVMUstring(garglist[gargnum]._unicharstr);
					gargnum++;
					break;
#endif /* GLK_MODULE_UNICODE */
				default:
					error("Illegal format string.");
					break;
				}

				if (isreturn) {
					*(splot->retval) = thisval;
				} else if (depth > 0) {
					/* Definitely not isref or isarray. */
					if (subpassout)
						WriteStructField(subaddress, ix, thisval);
				} else if (isref) {
					if (passout)
						WriteMemory(varglist[ix], thisval);
				}
			}
		} else {
			/* We got a null reference, so we have to skip the format element. */
			if (typeclass == '[') {
				int numsubwanted, refdepth;
				numsubwanted = 0;
				while (*cx >= '0' && *cx <= '9') {
					numsubwanted = 10 * numsubwanted + (*cx - '0');
					cx++;
				}
				refdepth = 1;
				while (refdepth > 0) {
					if (*cx == '[')
						refdepth++;
					else if (*cx == ']')
						refdepth--;
					cx++;
				}
			} else if (typeclass == 'S' || typeclass == 'U') {
				/* leave it */
			} else {
				cx++;
				if (isarray)
					ix++;
			}
		}
	}

	if (depth > 0) {
		if (*cx != ']')
			error("Illegal format string.");
		cx++;
	} else {
		if (*cx != ':' && *cx != '\0')
			error("Illegal format string.");
	}

	*proto = cx;
	*argnumptr = gargnum;
}

strid_t Glulx::find_stream_by_id(uint objid) {
	if (!objid)
		return nullptr;

	// Recall that class 1 ("b") is streams
	return (strid_t)classes_get(gidisp_Class_Stream, objid);
}

uint Glulx::find_id_for_window(winid_t win) {
	gidispatch_rock_t objrock;

	if (!win)
		return 0;

	objrock = gidispatch_get_objrock(win, gidisp_Class_Window);
	if (!objrock.ptr)
		return 0;
	return ((classref_t *)objrock.ptr)->id;
}

uint Glulx::find_id_for_stream(strid_t str) {
	gidispatch_rock_t objrock;

	if (!str)
		return 0;

	objrock = gidispatch_get_objrock(str, gidisp_Class_Stream);
	if (!objrock.ptr)
		return 0;
	return ((classref_t *)objrock.ptr)->id;
}

uint Glulx::find_id_for_fileref(frefid_t fref) {
	gidispatch_rock_t objrock;

	if (!fref)
		return 0;

	objrock = gidispatch_get_objrock(fref, gidisp_Class_Fileref);
	if (!objrock.ptr)
		return 0;
	return ((classref_t *)objrock.ptr)->id;
}

uint Glulx::find_id_for_schannel(schanid_t schan) {
	gidispatch_rock_t objrock;

	if (!schan)
		return 0;

	objrock = gidispatch_get_objrock(schan, gidisp_Class_Schannel);
	if (!objrock.ptr)
		return 0;
	return ((classref_t *)objrock.ptr)->id;
}

classtable_t *Glulx::new_classtable(uint firstid) {
	int ix;
	classtable_t *ctab = (classtable_t *)glulx_malloc(sizeof(classtable_t));
	if (!ctab)
		return nullptr;

	for (ix = 0; ix < CLASSHASH_SIZE; ix++)
		ctab->bucket[ix] = nullptr;

	ctab->lastid = firstid;

	return ctab;
}

void *Glulx::classes_get(int classid, uint objid) {
	classtable_t *ctab;
	classref_t *cref;
	if (classid < 0 || classid >= num_classes)
		return nullptr;
	ctab = classes[classid];
	cref = ctab->bucket[objid % CLASSHASH_SIZE];
	for (; cref; cref = cref->next) {
		if (cref->id == objid)
			return cref->obj;
	}
	return nullptr;
}

classref_t *Glulx::classes_put(int classid, void *obj, uint origid) {
	int bucknum;
	classtable_t *ctab;
	classref_t *cref;
	if (classid < 0 || classid >= num_classes)
		return nullptr;
	ctab = classes[classid];
	cref = (classref_t *)glulx_malloc(sizeof(classref_t));
	if (!cref)
		return nullptr;
	cref->obj = obj;
	if (!origid) {
		cref->id = ctab->lastid;
		ctab->lastid++;
	} else {
		cref->id = origid;
		if (ctab->lastid <= origid)
			ctab->lastid = origid + 1;
	}
	bucknum = cref->id % CLASSHASH_SIZE;
	cref->bucknum = bucknum;
	cref->next = ctab->bucket[bucknum];
	ctab->bucket[bucknum] = cref;
	return cref;
}

void Glulx::classes_remove(int classid, void *obj) {
	classtable_t *ctab;
	classref_t *cref;
	classref_t **crefp;
	gidispatch_rock_t objrock;
	if (classid < 0 || classid >= num_classes)
		return;
	ctab = classes[classid];
	objrock = gidispatch_get_objrock(obj, classid);
	cref = (classref_t *)objrock.ptr;
	if (!cref)
		return;
	crefp = &(ctab->bucket[cref->bucknum]);
	for (; *crefp; crefp = &((*crefp)->next)) {
		if ((*crefp) == cref) {
			*crefp = cref->next;
			if (!cref->obj) {
				nonfatal_warning("attempt to free nullptr object!");
			}
			cref->obj = nullptr;
			cref->id = 0;
			cref->next = nullptr;
			glulx_free(cref);
			return;
		}
	}
	return;
}

gidispatch_rock_t Glulx::glulxe_classtable_register(void *obj,  uint objclass) {
	classref_t *cref;
	gidispatch_rock_t objrock;
	cref = classes_put(objclass, obj, 0);
	objrock.ptr = cref;
	return objrock;
}

void Glulx::glulxe_classtable_unregister(void *obj, uint objclass,
        gidispatch_rock_t objrock) {
	classes_remove(objclass, obj);
}

gidispatch_rock_t Glulx::glulxe_classtable_register_existing(void *obj, uint objclass, uint dispid) {
	classref_t *cref;
	gidispatch_rock_t objrock;
	cref = classes_put(objclass, obj, dispid);
	objrock.ptr = cref;
	return objrock;
}

char *Glulx::grab_temp_c_array(uint addr, uint len, int passin) {
	arrayref_t *arref = nullptr;
	char *arr = nullptr;
	uint ix, addr2;

	if (len) {
		arr = (char *)glulx_malloc(len * sizeof(char));
		arref = (arrayref_t *)glulx_malloc(sizeof(arrayref_t));
		if (!arr || !arref)
			error("Unable to allocate space for array argument to Glk call.");

		arref->array = arr;
		arref->addr = addr;
		arref->elemsize = 1;
		arref->retained = false;
		arref->len = len;
		arref->next = arrays;
		arrays = arref;

		if (passin) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 1) {
				arr[ix] = Mem1(addr2);
			}
		}
	}

	return arr;
}

void Glulx::release_temp_c_array(char *arr, uint addr, uint len, int passout) {
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint ix, val, addr2;

	if (arr) {
		for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
			if ((*aptr)->array == arr)
				break;
		}
		arref = *aptr;
		if (!arref)
			error("Unable to re-find array argument in Glk call.");
		if (arref->addr != addr || arref->len != len)
			error("Mismatched array argument in Glk call.");

		if (arref->retained) {
			return;
		}

		*aptr = arref->next;
		arref->next = nullptr;

		if (passout) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 1) {
				val = arr[ix];
				MemW1(addr2, val);
			}
		}
		glulx_free(arr);
		glulx_free(arref);
	}
}

uint *Glulx::grab_temp_i_array(uint addr, uint len, int passin) {
	arrayref_t *arref = nullptr;
	uint *arr = nullptr;
	uint ix, addr2;

	if (len) {
		arr = (uint *)glulx_malloc(len * sizeof(uint));
		arref = (arrayref_t *)glulx_malloc(sizeof(arrayref_t));
		if (!arr || !arref)
			error("Unable to allocate space for array argument to Glk call.");

		arref->array = arr;
		arref->addr = addr;
		arref->elemsize = 4;
		arref->retained = false;
		arref->len = len;
		arref->next = arrays;
		arrays = arref;

		if (passin) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 4) {
				arr[ix] = Mem4(addr2);
			}
		}
	}

	return arr;
}

void Glulx::release_temp_i_array(uint *arr, uint addr, uint len, int passout) {
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint ix, val, addr2;

	if (arr) {
		for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
			if ((*aptr)->array == arr)
				break;
		}
		arref = *aptr;
		if (!arref)
			error("Unable to re-find array argument in Glk call.");
		if (arref->addr != addr || arref->len != len)
			error("Mismatched array argument in Glk call.");

		if (arref->retained) {
			return;
		}

		*aptr = arref->next;
		arref->next = nullptr;

		if (passout) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 4) {
				val = arr[ix];
				MemW4(addr2, val);
			}
		}
		glulx_free(arr);
		glulx_free(arref);
	}
}

void **Glulx::grab_temp_ptr_array(uint addr, uint len, int objclass, int passin) {
	arrayref_t *arref = nullptr;
	void **arr = nullptr;
	uint ix, addr2;

	if (len) {
		arr = (void **)glulx_malloc(len * sizeof(void *));
		arref = (arrayref_t *)glulx_malloc(sizeof(arrayref_t));
		if (!arr || !arref)
			error("Unable to allocate space for array argument to Glk call.");

		arref->array = arr;
		arref->addr = addr;
		arref->elemsize = sizeof(void *);
		arref->retained = false;
		arref->len = len;
		arref->next = arrays;
		arrays = arref;

		if (passin) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 4) {
				uint thisval = Mem4(addr2);
				if (thisval)
					arr[ix] = classes_get(objclass, thisval);
				else
					arr[ix] = nullptr;
			}
		}
	}

	return arr;
}

void Glulx::release_temp_ptr_array(void **arr, uint addr, uint len, int objclass, int passout) {
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint ix, val, addr2;

	if (arr) {
		for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
			if ((*aptr)->array == arr)
				break;
		}
		arref = *aptr;
		if (!arref)
			error("Unable to re-find array argument in Glk call.");
		if (arref->addr != addr || arref->len != len)
			error("Mismatched array argument in Glk call.");

		if (arref->retained) {
			return;
		}

		*aptr = arref->next;
		arref->next = nullptr;

		if (passout) {
			for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 4) {
				void *opref = arr[ix];
				if (opref) {
					gidispatch_rock_t objrock =
					    gidispatch_get_objrock(opref, objclass);
					val = ((classref_t *)objrock.ptr)->id;
				} else {
					val = 0;
				}
				MemW4(addr2, val);
			}
		}
		glulx_free(arr);
		glulx_free(arref);
	}
}

gidispatch_rock_t Glulx::glulxe_retained_register(void *array, uint len, const char *typecode) {
	gidispatch_rock_t rock;
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint elemsize = 0;

	if (typecode[4] == 'C')
		elemsize = 1;
	else if (typecode[4] == 'I')
		elemsize = 4;

	if (!elemsize || array == nullptr) {
		rock.ptr = nullptr;
		return rock;
	}

	for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
		if ((*aptr)->array == array)
			break;
	}
	arref = *aptr;
	if (!arref)
		error("Unable to re-find array argument in Glk call.");
	if (arref->elemsize != elemsize || arref->len != len)
		error("Mismatched array argument in Glk call.");

	arref->retained = true;

	rock.ptr = arref;
	return rock;
}

void Glulx::glulxe_retained_unregister(void *array, uint len, const  char *typecode, gidispatch_rock_t objrock) {
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint ix, addr2, val;
	uint elemsize = 0;

	// TODO: See if original GLULXE has code I'm overlooking to cleanly close everything before freeing memmap
	if (!memmap)
		return;

	if (typecode[4] == 'C')
		elemsize = 1;
	else if (typecode[4] == 'I')
		elemsize = 4;

	if (!elemsize || array == nullptr) {
		return;
	}

	for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
		if ((*aptr)->array == array)
			break;
	}
	arref = *aptr;
	if (!arref)
		error("Unable to re-find array argument in Glk call.");
	if (arref != objrock.ptr)
		error("Mismatched array reference in Glk call.");
	if (!arref->retained)
		error("Unretained array reference in Glk call.");
	if (arref->elemsize != elemsize || arref->len != len)
		error("Mismatched array argument in Glk call.");

	*aptr = arref->next;
	arref->next = nullptr;

	if (elemsize == 1) {
		for (ix = 0, addr2 = arref->addr; ix < arref->len; ix++, addr2 += 1) {
			val = ((char *)array)[ix];
			MemW1(addr2, val);
		}
	} else if (elemsize == 4) {
		for (ix = 0, addr2 = arref->addr; ix < arref->len; ix++, addr2 += 4) {
			val = ((uint *)array)[ix];
			MemW4(addr2, val);
		}
	}

	glulx_free(array);
	glulx_free(arref);
}

long Glulx::glulxe_array_locate(void *array, uint len, char *typecode, gidispatch_rock_t objrock, int *elemsizeref) {
	arrayref_t *arref = nullptr;
	arrayref_t **aptr;
	uint elemsize = 0;

	if (typecode[4] == 'C')
		elemsize = 1;
	else if (typecode[4] == 'I')
		elemsize = 4;

	if (!elemsize || array == nullptr) {
		*elemsizeref = 0; /* No need to save the array separately */
		return (unsigned char *)array - memmap;
	}

	for (aptr = (&arrays); (*aptr); aptr = (&((*aptr)->next))) {
		if ((*aptr)->array == array)
			break;
	}
	arref = *aptr;
	if (!arref)
		error("Unable to re-find array argument in array_locate.");
	if (arref != objrock.ptr)
		error("Mismatched array reference in array_locate.");
	if (!arref->retained)
		error("Unretained array reference in array_locate.");
	if (arref->elemsize != elemsize || arref->len != len)
		error("Mismatched array argument in array_locate.");

	*elemsizeref = arref->elemsize;
	return arref->addr;
}

gidispatch_rock_t Glulx::glulxe_array_restore(long bufkey, uint len, char *typecode, void **arrayref) {
	gidispatch_rock_t rock;
	int elemsize = 0;

	if (typecode[4] == 'C')
		elemsize = 1;
	else if (typecode[4] == 'I')
		elemsize = 4;

	if (!elemsize) {
		unsigned char *buf = memmap + bufkey;
		*arrayref = buf;
		rock.ptr = nullptr;
		return rock;
	}

	if (elemsize == 1) {
		char *cbuf = grab_temp_c_array(bufkey, len, false);
		rock = glulxe_retained_register(cbuf, len, typecode);
		*arrayref = cbuf;
	} else {
		uint *ubuf = grab_temp_i_array(bufkey, len, false);
		rock = glulxe_retained_register(ubuf, len, typecode);
		*arrayref = ubuf;
	}
	return rock;
}

void Glulx::set_library_select_hook(void (*func)(uint)) {
	library_select_hook = func;
}

char *Glulx::get_game_id() {
	/* This buffer gets rewritten on every call, but that's okay -- the caller
	   is supposed to copy out the result. */
	static char buf[2 * 64 + 2];
	int ix, jx;

	if (!memmap)
		return nullptr;

	for (ix = 0, jx = 0; ix < 64; ix++) {
		char ch = memmap[ix];
		int val = ((ch >> 4) & 0x0F);
		buf[jx++] = ((val < 10) ? (val + '0') : (val + 'A' - 10));
		val = (ch & 0x0F);
		buf[jx++] = ((val < 10) ? (val + '0') : (val + 'A' - 10));
	}
	buf[jx++] = '\0';

	return buf;
}

uint Glulx::ReadMemory(uint addr) {
	if (addr == 0xffffffff) {
		stackptr -= 4;
		return Stk4(stackptr);
	} else {
		return Mem4(addr);
	}
}

void Glulx::WriteMemory(uint addr, uint val) {
	if (addr == 0xffffffff) {
		StkW4(stackptr, (val));
		stackptr += 4;
	} else {
		MemW4(addr, val);
	}
}

char *Glulx::CaptureCArray(uint addr, uint len, int passin) {
	return grab_temp_c_array(addr, len, passin);
}

void Glulx::ReleaseCArray(char *ptr, uint addr, uint len, int passout) {
	release_temp_c_array(ptr, addr, len, passout);
}

uint *Glulx::CaptureIArray(uint addr, uint len, int passin) {
	return grab_temp_i_array(addr, len, passin);
}

void Glulx::ReleaseIArray(uint *ptr, uint addr, uint len, int passout) {
	release_temp_i_array(ptr, addr, len, passout);
}

void **Glulx::CapturePtrArray(uint addr, uint len, int objclass, int passin) {
	return grab_temp_ptr_array(addr, len, objclass, passin);
}

void Glulx::ReleasePtrArray(void **ptr, uint addr, uint len, int objclass, int passout) {
	return release_temp_ptr_array(ptr, addr, len, objclass, passout);
}

uint Glulx::ReadStructField(uint addr, uint fieldnum) {
	if (addr == 0xffffffff) {
		stackptr -= 4;
		return Stk4(stackptr);
	} else {
		return Mem4(addr + (fieldnum * 4));
	}
}

void Glulx::WriteStructField(uint addr, uint fieldnum, uint val) {
	if (addr == 0xffffffff) {
		StkW4(stackptr, val);
		stackptr += 4;
	} else {
		MemW4(addr + (fieldnum * 4), val);
	}
}

char *Glulx::DecodeVMString(uint addr) {
	return make_temp_string(addr);
}

void Glulx::ReleaseVMString(char *ptr) {
	free_temp_string(ptr);
}

uint32 *Glulx::DecodeVMUstring(uint addr) {
	return make_temp_ustring(addr);
}

void Glulx::ReleaseVMUstring(uint32 *ptr) {
	free_temp_ustring(ptr);
}

} // End of namespace Glulx
} // End of namespace Glk
