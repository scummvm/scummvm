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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/system.h"
#include "common/file.h"

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/kernel_types.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxVisual
#include "sci/gfx/menubar.h"

namespace Sci {

int _reset_graphics_input(EngineState *s) {
	Resource *resource;
	int font_nr;
	gfx_color_t transparent = { PaletteEntry(), 0, -1, -1, 0 };
	debug(2, "Initializing graphics");

	if (!s->resmgr->isVGA()) {
		for (int i = 0; i < 16; i++) {
			if (gfxop_set_color(s->gfx_state, &(s->ega_colors[i]), gfx_sci0_image_colors[sci0_palette][i].r,
					gfx_sci0_image_colors[sci0_palette][i].g, gfx_sci0_image_colors[sci0_palette][i].b, 0, -1, -1)) {
				return 1;
			}
			gfxop_set_system_color(s->gfx_state, i, &(s->ega_colors[i]));
		}
	} else {
		// Allocate SCI1 system colors
		gfx_color_t black = { PaletteEntry(0, 0, 0), 0, 0, 0, GFX_MASK_VISUAL };
		gfxop_set_system_color(s->gfx_state, 0, &black);

		// Check for Amiga palette file.
		Common::File file;
		if (file.open("spal")) {
			s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal1_amiga(file));
			file.close();
		} else {
			resource = s->resmgr->findResource(ResourceId(kResourceTypePalette, 999), 1);
			if (resource) {
				if (s->_version < SCI_VERSION_1_1)
					s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal1(999, resource->data, resource->size));
				else
					s->gfx_state->gfxResMan->setStaticPalette(gfxr_read_pal11(999, resource->data, resource->size));
				s->resmgr->unlockResource(resource);
			} else {
				debug(2, "Couldn't find the default palette!");
			}
		}
	}

	gfxop_fill_box(s->gfx_state, gfx_rect(0, 0, 320, 200), s->ega_colors[0]); // Fill screen black
	gfxop_update(s->gfx_state);

	gfxop_set_pointer_position(s->gfx_state, Common::Point(160, 150));

	s->pic_is_new = 0;
	s->pic_visible_map = GFX_MASK_NONE; // Other values only make sense for debugging
	s->dyn_views = NULL; // no DynViews
	s->drop_views = NULL; // And, consequently, no list for dropped views

	s->priority_first = 42; // Priority zone 0 ends here

	if (((SciEngine*)g_engine)->getKernel()->usesOldGfxFunctions())
		s->priority_last = 200;
	else
		s->priority_last = 190;

	font_nr = -1;
	do {
		resource = s->resmgr->testResource(ResourceId(kResourceTypeFont, ++font_nr));
	} while ((!resource) && (font_nr < 65536));

	if (!resource) {
		debug(2, "No text font was found.");
		return 1;
	}

	s->visual = new GfxVisual(s->gfx_state, font_nr);

	s->wm_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);

	s->iconbar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 200), s->ega_colors[0], transparent);
	s->iconbar_port->_flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	if (s->resmgr->isVGA()) {
		// This bit sets the foreground and background colors in VGA SCI games
		gfx_color_t fgcolor;
		gfx_color_t bgcolor;
		memset(&fgcolor, 0, sizeof(gfx_color_t));
		memset(&bgcolor, 0, sizeof(gfx_color_t));

#if 0
		fgcolor.visual = s->gfx_state->resstate->static_palette[0];
		fgcolor.mask = GFX_MASK_VISUAL;
		bgcolor.visual = s->gfx_state->resstate->static_palette[255];
		bgcolor.mask = GFX_MASK_VISUAL;
#endif
		s->titlebar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 10), fgcolor, bgcolor);
	} else {
		s->titlebar_port = new GfxPort(s->visual, gfx_rect(0, 0, 320, 10), s->ega_colors[0], s->ega_colors[15]);
	}
	s->titlebar_port->_color.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_color.priority = 11;
	s->titlebar_port->_bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_bgcolor.priority = 11;
	s->titlebar_port->_flags |= GFXW_FLAG_NO_IMPLICIT_SWITCH;

	// but this is correct
	s->picture_port = new GfxPort(s->visual, s->gfx_state->pic_port_bounds, s->ega_colors[0], transparent);

	s->_pics.clear();

	s->visual->add((GfxContainer *)s->visual, s->wm_port);
	s->visual->add((GfxContainer *)s->visual, s->titlebar_port);
	s->visual->add((GfxContainer *)s->visual, s->picture_port);
	s->visual->add((GfxContainer *)s->visual, s->iconbar_port);
	// Add ports to visual

	s->port = s->picture_port; // Currently using the picture port

#if 0
	s->titlebar_port->_bgcolor.mask |= GFX_MASK_PRIORITY;
	s->titlebar_port->_bgcolor.priority = 11; // Standard priority for the titlebar port
#endif

	return 0;
}

int game_init_graphics(EngineState *s) {
#ifdef CUSTOM_GRAPHICS_OPTIONS
#ifndef WITH_PIC_SCALING
	if (s->gfx_state->options->pic0_unscaled == 0)
		warning("Pic scaling was disabled; your version of ScummVM has no support for scaled pic drawing built in.");

	s->gfx_state->options->pic0_unscaled = 1;
#endif
#endif
	return _reset_graphics_input(s);
}

static void _free_graphics_input(EngineState *s) {
	debug(2, "Freeing graphics");

	delete s->visual;

	s->wm_port = s->titlebar_port = s->picture_port = NULL;
	s->visual = NULL;
	s->dyn_views = NULL;
	s->port = NULL;

	s->_pics.clear();
}

int game_init_sound(EngineState *s, int sound_flags) {
	if (s->resmgr->sciVersion() > SCI_VERSION_0_LATE)
		sound_flags |= SFX_STATE_FLAG_MULTIPLAY;

	s->sfx_init_flags = sound_flags;
	s->_sound.sfx_init(s->resmgr, sound_flags);

	return 0;
}

int create_class_table_sci11(EngineState *s) {
	int scriptnr;
	unsigned int seeker_offset;
	char *seeker_ptr;
	int classnr;

	Resource *vocab996 = s->resmgr->findResource(ResourceId(kResourceTypeVocab, 996), 1);

	if (!vocab996)
		s->seg_manager->_classtable.resize(20);
	else
		s->seg_manager->_classtable.resize(vocab996->size >> 2);

	for (scriptnr = 0; scriptnr < 1000; scriptnr++) {
		Resource *heap = s->resmgr->findResource(ResourceId(kResourceTypeHeap, scriptnr), 0);

		if (heap) {
			int global_vars = READ_LE_UINT16(heap->data + 2);

			seeker_ptr = (char*)heap->data + 4 + global_vars * 2;
			seeker_offset = 4 + global_vars * 2;

			while (READ_LE_UINT16((byte*)seeker_ptr) == SCRIPT_OBJECT_MAGIC_NUMBER) {
				if (READ_LE_UINT16((byte*)seeker_ptr + 14) & SCRIPT_INFO_CLASS) {
					classnr = READ_LE_UINT16((byte*)seeker_ptr + 10);
					if (classnr >= (int)s->seg_manager->_classtable.size()) {
						if (classnr >= SCRIPT_MAX_CLASSTABLE_SIZE) {
							warning("Invalid class number 0x%x in script.%d(0x%x), offset %04x",
							        classnr, scriptnr, scriptnr, seeker_offset);
							return 1;
						}

						s->seg_manager->_classtable.resize(classnr + 1); // Adjust maximum number of entries
					}

					s->seg_manager->_classtable[classnr].reg.offset = seeker_offset;
					s->seg_manager->_classtable[classnr].reg.segment = 0;
					s->seg_manager->_classtable[classnr].script = scriptnr;
				}

				seeker_ptr += READ_LE_UINT16((byte*)seeker_ptr + 2) * 2;
				seeker_offset += READ_LE_UINT16((byte*)seeker_ptr + 2) * 2;
			}
		}
	}

	s->resmgr->unlockResource(vocab996);
	vocab996 = NULL;
	return 0;
}

static int create_class_table_sci0(EngineState *s) {
	int scriptnr;
	unsigned int seeker;
	int classnr;
	int magic_offset; // For strange scripts in older SCI versions

	Resource *vocab996 = s->resmgr->findResource(ResourceId(kResourceTypeVocab, 996), 1);
	SciVersion version = s->_version;	// for the offset defines

	if (!vocab996)
		s->seg_manager->_classtable.resize(20);
	else
		s->seg_manager->_classtable.resize(vocab996->size >> 2);

	for (scriptnr = 0; scriptnr < 1000; scriptnr++) {
		int objtype = 0;
		Resource *script = s->resmgr->findResource(ResourceId(kResourceTypeScript, scriptnr), 0);

		if (script) {
			if (((SciEngine*)g_engine)->getKernel()->hasOldScriptHeader())
				magic_offset = seeker = 2;
			else
				magic_offset = seeker = 0;

			do {
				while (seeker < script->size)	{
					unsigned int lastseeker = seeker;
					objtype = (int16)READ_LE_UINT16(script->data + seeker);
					if (objtype == SCI_OBJ_CLASS || objtype == SCI_OBJ_TERMINATOR)
						break;
					seeker += (int16)READ_LE_UINT16(script->data + seeker + 2);
					if (seeker <= lastseeker) {
						s->seg_manager->_classtable.clear();
						error("Script version is invalid");
					}
				}

				if (objtype == SCI_OBJ_CLASS) {
					int sugg_script;

					seeker -= SCRIPT_OBJECT_MAGIC_OFFSET; // Adjust position; script home is base +8 bytes

					classnr = (int16)READ_LE_UINT16(script->data + seeker + 4 + SCRIPT_SPECIES_OFFSET);
					if (classnr >= (int)s->seg_manager->_classtable.size()) {

						if (classnr >= SCRIPT_MAX_CLASSTABLE_SIZE) {
							warning("Invalid class number 0x%x in script.%d(0x%x), offset %04x",
							        classnr, scriptnr, scriptnr, seeker);
							return 1;
						}

						s->seg_manager->_classtable.resize(classnr + 1); // Adjust maximum number of entries
					}

					// Map the class ID to the script the corresponding class is contained in
					// The script number is found in vocab.996, if it exists
					if (!vocab996 || (uint32)classnr >= vocab996->size >> 2)
						sugg_script = -1;
					else
						sugg_script = (int16)READ_LE_UINT16(vocab996->data + 2 + (classnr << 2));

					// First, test whether the script hasn't been claimed, or if it's been claimed by the wrong script

					if (sugg_script == -1 || scriptnr == sugg_script /*|| !s->_classtable[classnr].reg.segment*/)  {
						// Now set the home script of the class
						s->seg_manager->_classtable[classnr].reg.offset = seeker + 4 - magic_offset;
						s->seg_manager->_classtable[classnr].reg.segment = 0;
						s->seg_manager->_classtable[classnr].script = scriptnr;
					}

					seeker += SCRIPT_OBJECT_MAGIC_OFFSET; // Re-adjust position
					seeker += (int16)READ_LE_UINT16(script->data + seeker + 2); // Move to next
				}

			} while (objtype != SCI_OBJ_TERMINATOR && seeker <= script->size);

		}
	}
	s->resmgr->unlockResource(vocab996);
	vocab996 = NULL;
	return 0;
}

// Architectural stuff: Init/Unintialize engine
int script_init_engine(EngineState *s) {
	int result;

	s->kernel_opt_flags = 0;
	s->seg_manager = new SegManager(s->resmgr, s->_version);

	if (s->_version >= SCI_VERSION_1_1)
		result = create_class_table_sci11(s);
	else
		result = create_class_table_sci0(s);

	if (result) {
		debug(2, "Failed to initialize class table");
		return 1;
	}

	s->gc_countdown = GC_INTERVAL - 1;

	SegmentId script_000_segment = s->seg_manager->getSegment(0, SCRIPT_GET_LOCK);

	if (script_000_segment <= 0) {
		debug(2, "Failed to instantiate script.000");
		return 1;
	}

	s->script_000 = s->seg_manager->getScript(script_000_segment);

	s->sys_strings = s->seg_manager->allocateSysStrings(&s->sys_strings_segment);
	s->string_frag_segment = s->seg_manager->allocateStringFrags();

	// Allocate static buffer for savegame and CWD directories
	SystemString *str = &s->sys_strings->strings[SYS_STRING_SAVEDIR];
	str->name = strdup("savedir");
	str->max_size = MAX_SAVE_DIR_SIZE;
	str->value = (reg_t *)calloc(MAX_SAVE_DIR_SIZE, sizeof(reg_t));	// FIXME -- sizeof(char) or sizeof(reg_t) ??
	str->value[0].segment = s->string_frag_segment; // Set to empty string
	str->value[0].offset = 0;


	s->r_acc = s->r_prev = NULL_REG;
	s->restAdjust = 0;

	s->_executionStack.clear();    // Start without any execution stack
	s->execution_stack_base = -1; // No vm is running yet

	s->restarting_flags = SCI_GAME_IS_NOT_RESTARTING;

	s->bp_list = NULL; // No breakpoints defined
	s->have_bp = 0;

	if (((SciEngine*)g_engine)->getKernel()->hasLofsAbsolute())
		s->seg_manager->setExportWidth(1);
	else
		s->seg_manager->setExportWidth(0);

	debug(2, "Engine initialized");

	s->pic_priority_table = NULL;
	s->_pics.clear();

	return 0;
}

void script_set_gamestate_save_dir(EngineState *s, const char *path) {
	SystemString *str = &s->sys_strings->strings[SYS_STRING_SAVEDIR];

	strncpy((char *)str->value, path, str->max_size);		// FIXME -- strncpy or internal_stringfrag_strncpy ?
	str->value[str->max_size - 1].segment = s->string_frag_segment; // Make sure to terminate
	str->value[str->max_size - 1].offset &= 0xff00; // Make sure to terminate
}

void internal_stringfrag_strncpy(EngineState *s, reg_t *dest, reg_t *src, int len);

void script_free_vm_memory(EngineState *s) {
	debug(2, "Freeing VM memory");

	if (s->seg_manager)
		s->seg_manager->_classtable.clear();

	// Close all opened file handles
	s->_fileHandles.clear();
	s->_fileHandles.resize(5);
}

void script_free_engine(EngineState *s) {
	script_free_vm_memory(s);

	debug(2, "Freeing state-dependant data");
}

void script_free_breakpoints(EngineState *s) {
	Breakpoint *bp, *bp_next;

	// Free breakpoint list
	bp = s->bp_list;
	while (bp) {
		bp_next = bp->next;
		if (bp->type == BREAK_SELECTOR)
			free(bp->data.name);
		free(bp);
		bp = bp_next;
	}

	s->bp_list = NULL;
}

/*************************************************************/
/* Game instance stuff: Init/Unitialize state-dependant data */
/*************************************************************/

int game_init(EngineState *s) {
	// FIXME Use new VM instantiation code all over the place"
	DataStack *stack;

	stack = s->seg_manager->allocateStack(VM_STACK_SIZE, &s->stack_segment);
	s->stack_base = stack->entries;
	s->stack_top = s->stack_base + VM_STACK_SIZE;

	if (!script_instantiate(s->resmgr, s->seg_manager, s->_version, 0)) {
		warning("game_init(): Could not instantiate script 0");
		return 1;
	}

	s->parser_valid = 0; // Invalidate parser
	s->parser_event = NULL_REG; // Invalidate parser event

	s->_synonyms.clear(); // No synonyms

	if (s->gfx_state && _reset_graphics_input(s))
		return 1;

	s->successor = NULL; // No successor
	s->_statusBarText.clear(); // Status bar is blank
	s->status_bar_foreground = 0;
	s->status_bar_background = !s->resmgr->isVGA() ? 15 : 255;

	SystemString *str = &s->sys_strings->strings[SYS_STRING_PARSER_BASE];
	str->name = strdup("parser-base");
	str->max_size = MAX_PARSER_BASE;
	str->value = (reg_t *)calloc(MAX_PARSER_BASE + 1, sizeof(char));	// FIXME -- sizeof(char) or sizeof(reg_t) ??
	str->value[0].segment = s->string_frag_segment; // Set to empty string
	str->value[0].offset = 0; // Set to empty string

	s->parser_base = make_reg(s->sys_strings_segment, SYS_STRING_PARSER_BASE);

	s->game_start_time = g_system->getMillis();
	s->last_wait_time = s->game_start_time;

	srand(g_system->getMillis()); // Initialize random number generator

//	script_dissect(0, s->_selectorNames);
	// The first entry in the export table of script 0 points to the game object
	s->game_obj = script_lookup_export(s->seg_manager, 0, 0);
	s->_gameName = obj_get_name(s->seg_manager, s->_version, s->game_obj);

	debug(2, " \"%s\" at %04x:%04x", s->_gameName.c_str(), PRINT_REG(s->game_obj));

	// Mark parse tree as unused
	s->parser_nodes[0].type = kParseTreeLeafNode;
	s->parser_nodes[0].content.value = 0;

	s->_menubar = new Menubar(); // Create menu bar

	if (s->sfx_init_flags & SFX_STATE_FLAG_NOSOUND)
		game_init_sound(s, 0);

	// Load game language into printLang property of game object
	s->getLanguage();

	return 0;
}

int game_exit(EngineState *s) {
	s->_executionStack.clear();

	if (!s->successor) {
		s->_sound.sfx_exit();
		// Reinit because some other code depends on having a valid state
		game_init_sound(s, SFX_STATE_FLAG_NOSOUND);
	}

	s->seg_manager->_classtable.clear();
	delete s->seg_manager;
	s->seg_manager = 0;

	s->_synonyms.clear();

	debug(2, "Freeing miscellaneous data...");

	// TODO Free parser segment here

	// TODO Free scripts here

	delete s->_menubar;

	_free_graphics_input(s);

	return 0;
}

} // End of namespace Sci
