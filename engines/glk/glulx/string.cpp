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

void Glulx::stream_get_iosys(uint *mode, uint *rock) {
	*mode = iosys_mode;
	*rock = iosys_rock;
}

void Glulx::stream_setup_unichar() {
#ifdef GLK_MODULE_UNICODE

	if (glk_gestalt(gestalt_Unicode, 0))
		glkio_unichar_han_ptr = &Glulx::glk_put_char_uni;
	else
		glkio_unichar_han_ptr = &Glulx::glkio_unichar_nouni_han;

#else /* GLK_MODULE_UNICODE */

	glkio_unichar_han_ptr = glkio_unichar_nouni_han;

#endif /* GLK_MODULE_UNICODE */
}

void Glulx::stream_set_iosys(uint mode, uint rock) {
	switch (mode) {
	default:
		mode = 0;
		// fall through
	case iosys_None:
		rock = 0;
		stream_char_handler = &Glulx::nopio_char_han;
		stream_unichar_handler = &Glulx::nopio_unichar_han;
		break;
	case iosys_Filter:
		stream_char_handler = &Glulx::filio_char_han;
		stream_unichar_handler = &Glulx::filio_unichar_han;
		break;
	case iosys_Glk:
		if (!glkio_unichar_han_ptr)
			stream_setup_unichar();
		rock = 0;
		stream_char_handler = &Glulx::glk_put_char;
		stream_unichar_handler = glkio_unichar_han_ptr;
		break;
	}

	iosys_mode = mode;
	iosys_rock = rock;
}

void Glulx::nopio_char_han(unsigned char ch) {
}

void Glulx::nopio_unichar_han(uint32 ch) {
}

void Glulx::filio_char_han(unsigned char ch) {
	uint val = ch;
	push_callstub(0, 0);
	enter_function(iosys_rock, 1, &val);
}

void Glulx::filio_unichar_han(uint32 val) {
	uint v = val;
	push_callstub(0, 0);
	enter_function(iosys_rock, 1, &v);
}

void Glulx::glkio_unichar_nouni_han(uint32 val) {
	/* Only used if the Glk library has no Unicode functions */
	if (val > 0xFF)
		val = '?';
	glk_put_char(val);
}

void Glulx::stream_num(int val, int inmiddle, int charnum) {
	int ix = 0;
	int res, jx;
	char buf[16];
	uint ival;

	if (val == 0) {
		buf[ix] = '0';
		ix++;
	} else {
		if (val < 0)
			ival = -val;
		else
			ival = val;

		while (ival != 0) {
			buf[ix] = (ival % 10) + '0';
			ix++;
			ival /= 10;
		}

		if (val < 0) {
			buf[ix] = '-';
			ix++;
		}
	}

	switch (iosys_mode) {

	case iosys_Glk:
		ix -= charnum;
		while (ix > 0) {
			ix--;
			glk_put_char(buf[ix]);
		}
		break;

	case iosys_Filter:
		if (!inmiddle) {
			push_callstub(0x11, 0);
			inmiddle = true;
		}
		if (charnum < ix) {
			ival = buf[(ix - 1) - charnum] & 0xFF;
			pc = val;
			push_callstub(0x12, charnum + 1);
			enter_function(iosys_rock, 1, &ival);
			return;
		}
		break;

	default:
		break;

	}

	if (inmiddle) {
		res = pop_callstub_string(&jx);
		if (res)
			fatal_error("String-on-string call stub while printing number.");
	}
}

void Glulx::stream_string(uint addr, int inmiddle, int bitnum) {
	int ch;
	int type;
	int alldone = false;
	int substring = (inmiddle != 0);
	uint ival;

	if (!addr)
		fatal_error("Called stream_string with null address.");

	while (!alldone) {

		if (inmiddle == 0) {
			type = Mem1(addr);
			if (type == 0xE2)
				addr += 4;
			else
				addr++;
			bitnum = 0;
		} else {
			type = inmiddle;
		}

		if (type == 0xE1) {
			if (tablecache_valid) {
				int bits, numbits;
				int readahead;
				uint tmpaddr;
				cacheblock_t *cablist;
				int done = 0;

				/* bitnum is already set right */
				bits = Mem1(addr);
				if (bitnum)
					bits >>= bitnum;
				numbits = (8 - bitnum);
				readahead = false;

				if (tablecache.type != 0) {
					/* This is a bit of a cheat. If the top-level block is not
					   a branch, then it must be a string-terminator -- otherwise
					   the string would be an infinite repetition of that block.
					   We check for this case and bail immediately. */
					done = 1;
				}

				cablist = tablecache.u.branches;
				while (!done) {
					cacheblock_t *cab;

					if (numbits < CACHEBITS) {
						/* readahead is certainly false */
						int newbyte = Mem1(addr + 1);
						bits |= (newbyte << numbits);
						numbits += 8;
						readahead = true;
					}

					cab = &(cablist[bits & CACHEMASK]);
					numbits -= cab->depth;
					bits >>= cab->depth;
					bitnum += cab->depth;
					if (bitnum >= 8) {
						addr += 1;
						bitnum -= 8;
						if (readahead) {
							readahead = false;
						} else {
							int newbyte = Mem1(addr);
							bits |= (newbyte << numbits);
							numbits += 8;
						}
					}

					switch (cab->type) {
					case 0x00: /* non-leaf node */
						cablist = cab->u.branches;
						break;
					case 0x01: /* string terminator */
						done = 1;
						break;
					case 0x02: /* single character */
						switch (iosys_mode) {
						case iosys_Glk:
							glk_put_char(cab->u.ch);
							break;
						case iosys_Filter:
							ival = cab->u.ch & 0xFF;
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(iosys_rock, 1, &ival);
							return;
						}
						cablist = tablecache.u.branches;
						break;
					case 0x04: /* single Unicode character */
						switch (iosys_mode) {
						case iosys_Glk:
							(this->*glkio_unichar_han_ptr)(cab->u.uch);
							break;
						case iosys_Filter:
							ival = cab->u.uch;
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(iosys_rock, 1, &ival);
							return;
						}
						cablist = tablecache.u.branches;
						break;
					case 0x03: /* C string */
						switch (iosys_mode) {
						case iosys_Glk:
							for (tmpaddr = cab->u.addr; (ch = Mem1(tmpaddr)) != '\0'; tmpaddr++)
								glk_put_char(ch);
							cablist = tablecache.u.branches;
							break;
						case iosys_Filter:
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0xE0;
							addr = cab->u.addr;
							done = 2;
							break;
						default:
							cablist = tablecache.u.branches;
							break;
						}
						break;
					case 0x05: /* C Unicode string */
						switch (iosys_mode) {
						case iosys_Glk:
							for (tmpaddr = cab->u.addr; (ival = Mem4(tmpaddr)) != 0; tmpaddr += 4)
								(this->*glkio_unichar_han_ptr)(ival);
							cablist = tablecache.u.branches;
							break;
						case iosys_Filter:
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0xE2;
							addr = cab->u.addr;
							done = 2;
							break;
						default:
							cablist = tablecache.u.branches;
							break;
						}
						break;
					case 0x08:
					case 0x09:
					case 0x0A:
					case 0x0B: {
						uint oaddr;
						int otype;
						oaddr = cab->u.addr;
						if (cab->type >= 0x09)
							oaddr = Mem4(oaddr);
						if (cab->type == 0x0B)
							oaddr = Mem4(oaddr);
						otype = Mem1(oaddr);
						if (!substring) {
							push_callstub(0x11, 0);
							substring = true;
						}
						if (otype >= 0xE0 && otype <= 0xFF) {
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0;
							addr = oaddr;
							done = 2;
						} else if (otype >= 0xC0 && otype <= 0xDF) {
							uint argc;
							uint *argv;
							if (cab->type == 0x0A || cab->type == 0x0B) {
								argc = Mem4(cab->u.addr + 4);
								argv = pop_arguments(argc, cab->u.addr + 8);
							} else {
								argc = 0;
								argv = nullptr;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(oaddr, argc, argv);
							return;
						} else {
							fatal_error("Unknown object while decoding string indirect reference.");
						}
					}
					break;
					default:
						fatal_error("Unknown entity in string decoding (cached).");
						break;
					}
				}
				if (done > 1) {
					continue; /* restart the top-level loop */
				}
			} else { /* tablecache not valid */
				uint node;
				int byte1;
				int nodetype;
				int done = 0;

				if (!stringtable)
					fatal_error("Attempted to print a compressed string with no table set.");
				/* bitnum is already set right */
				byte1 = Mem1(addr);
				if (bitnum)
					byte1 >>= bitnum;
				node = Mem4(stringtable + 8);
				while (!done) {
					nodetype = Mem1(node);
					node++;
					switch (nodetype) {
					case 0x00: /* non-leaf node */
						if (byte1 & 1)
							node = Mem4(node + 4);
						else
							node = Mem4(node + 0);
						if (bitnum == 7) {
							bitnum = 0;
							addr++;
							byte1 = Mem1(addr);
						} else {
							bitnum++;
							byte1 >>= 1;
						}
						break;
					case 0x01: /* string terminator */
						done = 1;
						break;
					case 0x02: /* single character */
						ch = Mem1(node);
						switch (iosys_mode) {
						case iosys_Glk:
							glk_put_char(ch);
							break;
						case iosys_Filter:
							ival = ch & 0xFF;
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(iosys_rock, 1, &ival);
							return;
						}
						node = Mem4(stringtable + 8);
						break;
					case 0x04: /* single Unicode character */
						ival = Mem4(node);
						switch (iosys_mode) {
						case iosys_Glk:
							(this->*glkio_unichar_han_ptr)(ival);
							break;
						case iosys_Filter:
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(iosys_rock, 1, &ival);
							return;
						}
						node = Mem4(stringtable + 8);
						break;
					case 0x03: /* C string */
						switch (iosys_mode) {
						case iosys_Glk:
							for (; (ch = Mem1(node)) != '\0'; node++)
								glk_put_char(ch);
							node = Mem4(stringtable + 8);
							break;
						case iosys_Filter:
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0xE0;
							addr = node;
							done = 2;
							break;
						default:
							node = Mem4(stringtable + 8);
							break;
						}
						break;
					case 0x05: /* C Unicode string */
						switch (iosys_mode) {
						case iosys_Glk:
							for (; (ival = Mem4(node)) != 0; node += 4)
								(this->*glkio_unichar_han_ptr)(ival);
							node = Mem4(stringtable + 8);
							break;
						case iosys_Filter:
							if (!substring) {
								push_callstub(0x11, 0);
								substring = true;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0xE2;
							addr = node;
							done = 2;
							break;
						default:
							node = Mem4(stringtable + 8);
							break;
						}
						break;
					case 0x08:
					case 0x09:
					case 0x0A:
					case 0x0B: {
						uint oaddr;
						int otype;
						oaddr = Mem4(node);
						if (nodetype == 0x09 || nodetype == 0x0B)
							oaddr = Mem4(oaddr);
						otype = Mem1(oaddr);
						if (!substring) {
							push_callstub(0x11, 0);
							substring = true;
						}
						if (otype >= 0xE0 && otype <= 0xFF) {
							pc = addr;
							push_callstub(0x10, bitnum);
							inmiddle = 0;
							addr = oaddr;
							done = 2;
						} else if (otype >= 0xC0 && otype <= 0xDF) {
							uint argc;
							uint *argv;
							if (nodetype == 0x0A || nodetype == 0x0B) {
								argc = Mem4(node + 4);
								argv = pop_arguments(argc, node + 8);
							} else {
								argc = 0;
								argv = nullptr;
							}
							pc = addr;
							push_callstub(0x10, bitnum);
							enter_function(oaddr, argc, argv);
							return;
						} else {
							fatal_error("Unknown object while decoding string indirect reference.");
						}
					}
					break;
					default:
						fatal_error("Unknown entity in string decoding.");
						break;
					}
				}
				if (done > 1) {
					continue; /* restart the top-level loop */
				}
			}
		} else if (type == 0xE0) {
			switch (iosys_mode) {
			case iosys_Glk:
				while (1) {
					ch = Mem1(addr);
					addr++;
					if (ch == '\0')
						break;
					glk_put_char(ch);
				}
				break;
			case iosys_Filter:
				if (!substring) {
					push_callstub(0x11, 0);
					substring = true;
				}
				ch = Mem1(addr);
				addr++;
				if (ch != '\0') {
					ival = ch & 0xFF;
					pc = addr;
					push_callstub(0x13, 0);
					enter_function(iosys_rock, 1, &ival);
					return;
				}
				break;
			}
		} else if (type == 0xE2) {
			switch (iosys_mode) {
			case iosys_Glk:
				while (1) {
					ival = Mem4(addr);
					addr += 4;
					if (ival == 0)
						break;
					(this->*glkio_unichar_han_ptr)(ival);
				}
				break;
			case iosys_Filter:
				if (!substring) {
					push_callstub(0x11, 0);
					substring = true;
				}
				ival = Mem4(addr);
				addr += 4;
				if (ival != 0) {
					pc = addr;
					push_callstub(0x14, 0);
					enter_function(iosys_rock, 1, &ival);
					return;
				}
				break;
			}
		} else if (type >= 0xE0 && type <= 0xFF) {
			fatal_error("Attempt to print unknown type of string.");
		} else {
			fatal_error("Attempt to print non-string.");
		}

		if (!substring) {
			/* Just get straight out. */
			alldone = true;
		} else {
			/* Pop a stub and see what's to be done. */
			addr = pop_callstub_string(&bitnum);
			if (addr == 0) {
				alldone = true;
			} else {
				inmiddle = 0xE1;
			}
		}
	}
}

uint Glulx::stream_get_table() {
	return stringtable;
}

void Glulx::stream_set_table(uint addr) {
	if (stringtable == addr)
		return;

	/* Drop cache. */
	if (tablecache_valid) {
		if (tablecache.type == 0)
			dropcache(tablecache.u.branches);
		tablecache.u.branches = nullptr;
		tablecache_valid = false;
	}

	stringtable = addr;

	if (stringtable) {
		/* Build cache. We can only do this if the table is entirely in ROM. */
		uint tablelen = Mem4(stringtable);
		uint rootaddr = Mem4(stringtable + 8);
		int cache_stringtable = (stringtable + tablelen <= ramstart);
		/* cache_stringtable = true; ...for testing only */
		/* cache_stringtable = false; ...for testing only */
		if (cache_stringtable) {
			buildcache(&tablecache, rootaddr, CACHEBITS, 0);
			/* dumpcache(&tablecache, 1, 0); */
			tablecache_valid = true;
		}
	}
}

void Glulx::buildcache(cacheblock_t *cablist, uint nodeaddr, int depth, int mask) {
	int ix, type;

	type = Mem1(nodeaddr);

	if (type == 0 && depth == CACHEBITS) {
		cacheblock_t *list, *cab;
		list = (cacheblock_t *)glulx_malloc(sizeof(cacheblock_t) * CACHESIZE);
		buildcache(list, nodeaddr, 0, 0);
		cab = &(cablist[mask]);
		cab->type = 0;
		cab->depth = CACHEBITS;
		cab->u.branches = list;
		return;
	}

	if (type == 0) {
		uint leftaddr  = Mem4(nodeaddr + 1);
		uint rightaddr = Mem4(nodeaddr + 5);
		buildcache(cablist, leftaddr, depth + 1, mask);
		buildcache(cablist, rightaddr, depth + 1, (mask | (1 << depth)));
		return;
	}

	/* Leaf node. */
	nodeaddr++;
	for (ix = mask; ix < CACHESIZE; ix += (1 << depth)) {
		cacheblock_t *cab = &(cablist[ix]);
		cab->type = type;
		cab->depth = depth;
		switch (type) {
		case 0x02:
			cab->u.ch = Mem1(nodeaddr);
			break;
		case 0x04:
			cab->u.uch = Mem4(nodeaddr);
			break;
		case 0x03:
		case 0x05:
		case 0x0A:
		case 0x0B:
			cab->u.addr = nodeaddr;
			break;
		case 0x08:
		case 0x09:
			cab->u.addr = Mem4(nodeaddr);
			break;
		}
	}
}

#if 0
#include <stdio.h>
void Glulx::dumpcache(cacheblock_t *cablist, int count, int indent) {
	int ix, jx;

	for (ix = 0; ix < count; ix++) {
		cacheblock_t *cab = &(cablist[ix]);
		for (jx = 0; jx < indent; jx++)
			printf("  ");
		printf("%X: ", ix);
		switch (cab->type) {
		case 0:
			printf("...\n");
			dumpcache(cab->u.branches, CACHESIZE, indent + 1);
			break;
		case 1:
			printf("<EOS>\n");
			break;
		case 2:
			printf("0x%02X", cab->u.ch);
			if (cab->u.ch < 32)
				printf(" ''\n");
			else
				printf(" '%c'\n", cab->u.ch);
			break;
		default:
			printf("type %02X, address %06lX\n", cab->type, cab->u.addr);
			break;
		}
	}
}
#endif /* 0 */

void Glulx::dropcache(cacheblock_t *cablist) {
	int ix;
	for (ix = 0; ix < CACHESIZE; ix++) {
		cacheblock_t *cab = &(cablist[ix]);
		if (cab->type == 0) {
			dropcache(cab->u.branches);
			cab->u.branches = nullptr;
		}
	}
	glulx_free(cablist);
}

char *Glulx::make_temp_string(uint addr) {
	int ix, len;
	uint addr2;
	char *res;

	if (Mem1(addr) != 0xE0)
		fatal_error("String argument to a Glk call must be unencoded.");
	addr++;

	for (addr2 = addr; Mem1(addr2); addr2++) { };
	len = (addr2 - addr);
	if (len < STATIC_TEMP_BUFSIZE) {
		res = temp_buf;
	} else {
		res = (char *)glulx_malloc(len + 1);
		if (!res)
			fatal_error("Unable to allocate space for string argument to Glk call.");
	}

	for (ix = 0, addr2 = addr; ix < len; ix++, addr2++) {
		res[ix] = Mem1(addr2);
	}
	res[len] = '\0';

	return res;
}

uint32 *Glulx::make_temp_ustring(uint addr) {
	int ix, len;
	uint addr2;
	uint32 *res;

	if (Mem1(addr) != 0xE2)
		fatal_error("Ustring argument to a Glk call must be unencoded.");
	addr += 4;

	for (addr2 = addr; Mem4(addr2); addr2 += 4) { };
	len = (addr2 - addr) / 4;
	if ((len + 1) * 4 < STATIC_TEMP_BUFSIZE) {
		res = (uint32 *)temp_buf;
	} else {
		res = (uint32 *)glulx_malloc((len + 1) * 4);
		if (!res)
			fatal_error("Unable to allocate space for ustring argument to Glk call.");
	}

	for (ix = 0, addr2 = addr; ix < len; ix++, addr2 += 4) {
		res[ix] = Mem4(addr2);
	}
	res[len] = 0;

	return res;
}

void Glulx::free_temp_string(char *str) {
	if (str && str != temp_buf)
		glulx_free(str);
}

void Glulx::free_temp_ustring(uint32 *str) {
	if (str && str != (uint32 *)temp_buf)
		glulx_free(str);
}

} // End of namespace Glulx
} // End of namespace Glk
