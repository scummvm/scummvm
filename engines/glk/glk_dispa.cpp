/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software{} you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation{} either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program{} if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/glk_api.h"

namespace Glk {


#define NUMCLASSES   \
    (sizeof(class_table) / sizeof(gidispatch_intconst_t))

#define NUMINTCONSTANTS   \
    (sizeof(intconstant_table) / sizeof(gidispatch_intconst_t))

/**
 * The constants in this table must be ordered alphabetically.
 */
static const gidispatch_intconst_t class_table[] = {
    { "fileref", (2) },   /* "Qc" */
    { "schannel", (3) },  /* "Qd" */
    { "stream", (1) },    /* "Qb" */
    { "window", (0) },    /* "Qa" */
};

/**
 * The constants in this table must be ordered alphabetically.
 */
static const gidispatch_intconst_t intconstant_table[] = {
    { "evtype_Arrange", (5)  },
    { "evtype_CharInput", (2) },
    { "evtype_Hyperlink", (8) },
    { "evtype_LineInput", (3) },
    { "evtype_MouseInput", (4) },
    { "evtype_None", (0) },
    { "evtype_Redraw", (6) },
    { "evtype_SoundNotify", (7) },
    { "evtype_Timer", (1) },
    { "evtype_VolumeNotify", (9) },

    { "filemode_Read", (0x02) },
    { "filemode_ReadWrite", (0x03) },
    { "filemode_Write", (0x01) },
    { "filemode_WriteAppend", (0x05) },

    { "fileusage_BinaryMode", (0x000) },
    { "fileusage_Data", (0x00) },
    { "fileusage_InputRecord", (0x03) },
    { "fileusage_SavedGame", (0x01) },
    { "fileusage_TextMode",   (0x100) },
    { "fileusage_Transcript", (0x02) },
    { "fileusage_TypeMask", (0x0f) },

    { "gestalt_CharInput", (1) },
    { "gestalt_CharOutput", (3) },
    { "gestalt_CharOutput_ApproxPrint", (1) },
    { "gestalt_CharOutput_CannotPrint", (0) },
    { "gestalt_CharOutput_ExactPrint", (2) },
    { "gestalt_DateTime", (20) },
    { "gestalt_DrawImage", (7) },
    { "gestalt_GarglkText", (0x1100) },
    { "gestalt_Graphics", (6) },
    { "gestalt_GraphicsTransparency", (14) },
    { "gestalt_HyperlinkInput", (12) },
    { "gestalt_Hyperlinks", (11) },
    { "gestalt_LineInput", (2) },
    { "gestalt_LineInputEcho", (17) },
    { "gestalt_LineTerminatorKey", (19) },
    { "gestalt_LineTerminators", (18) },
    { "gestalt_MouseInput", (4) },
    { "gestalt_Sound", (8) },
    { "gestalt_Sound2", (21) },
    { "gestalt_SoundMusic", (13) },
    { "gestalt_SoundNotify", (10) },
    { "gestalt_SoundVolume", (9) },
    { "gestalt_Timer", (5) },
    { "gestalt_Unicode", (15) },
    { "gestalt_UnicodeNorm", (16) },
    { "gestalt_Version", (0) },

    { "imagealign_InlineCenter",  (0x03) },
    { "imagealign_InlineDown",  (0x02) },
    { "imagealign_MarginLeft",  (0x04) },
    { "imagealign_MarginRight",  (0x05) },
    { "imagealign_InlineUp",  (0x01) },

    { "keycode_Delete",   (0xfffffff9) },
    { "keycode_Down",     (0xfffffffb) },
    { "keycode_End",      (0xfffffff3) },
    { "keycode_Escape",   (0xfffffff8) },
    { "keycode_Func1",    (0xffffffef) },
    { "keycode_Func10",   (0xffffffe6) },
    { "keycode_Func11",   (0xffffffe5) },
    { "keycode_Func12",   (0xffffffe4) },
    { "keycode_Func2",    (0xffffffee) },
    { "keycode_Func3",    (0xffffffed) },
    { "keycode_Func4",    (0xffffffec) },
    { "keycode_Func5",    (0xffffffeb) },
    { "keycode_Func6",    (0xffffffea) },
    { "keycode_Func7",    (0xffffffe9) },
    { "keycode_Func8",    (0xffffffe8) },
    { "keycode_Func9",    (0xffffffe7) },
    { "keycode_Home",     (0xfffffff4) },
    { "keycode_Left",     (0xfffffffe) },
    { "keycode_MAXVAL",   (28)  },
    { "keycode_PageDown", (0xfffffff5) },
    { "keycode_PageUp",   (0xfffffff6) },
    { "keycode_Return",   (0xfffffffa) },
    { "keycode_Right",    (0xfffffffd) },
    { "keycode_Tab",      (0xfffffff7) },
    { "keycode_Unknown",  (0xffffffff) },
    { "keycode_Up",       (0xfffffffc) },

    { "seekmode_Current", (1) },
    { "seekmode_End", (2) },
    { "seekmode_Start", (0) },

    { "style_Alert", (5) },
    { "style_BlockQuote", (7) },
    { "style_Emphasized", (1) },
    { "style_Header", (3) },
    { "style_Input", (8) },
    { "style_NUMSTYLES", (11) },
    { "style_Normal", (0) },
    { "style_Note", (6) },
    { "style_Preformatted", (2) },
    { "style_Subheader", (4) },
    { "style_User1", (9) },
    { "style_User2", (10) },
    { "stylehint_BackColor", (8) },
    { "stylehint_Indentation", (0) },
    { "stylehint_Justification", (2)  },
    { "stylehint_NUMHINTS", (10) },
    { "stylehint_Oblique", (5) },
    { "stylehint_ParaIndentation", (1) },
    { "stylehint_Proportional", (6) },
    { "stylehint_ReverseColor", (9) },
    { "stylehint_Size", (3) },
    { "stylehint_TextColor", (7) },
    { "stylehint_Weight", (4) },
    { "stylehint_just_Centered", (2) },
    { "stylehint_just_LeftFlush", (0) },
    { "stylehint_just_LeftRight", (1) },
    { "stylehint_just_RightFlush", (3) },

    { "winmethod_Above", (0x02)  },
    { "winmethod_Below", (0x03)  },
    { "winmethod_Border", (0x000)  },
    { "winmethod_BorderMask", (0x100)  },
    { "winmethod_DirMask", (0x0f) },
    { "winmethod_DivisionMask", (0xf0) },
    { "winmethod_Fixed", (0x10) },
    { "winmethod_Left",  (0x00)  },
    { "winmethod_NoBorder", (0x100)  },
    { "winmethod_Proportional", (0x20) },
    { "winmethod_Right", (0x01)  },

    { "wintype_AllTypes", (0)  },
    { "wintype_Blank", (2)  },
    { "wintype_Graphics", (5)  },
    { "wintype_Pair", (1)  },
    { "wintype_TextBuffer", (3) },
    { "wintype_TextGrid", (4) },
};

void GlkAPI::gidispatch_set_object_registry(gidispatch_rock_t(*regi)(void *obj, uint objclass),
		void(*unregi)(void *obj, uint objclass, gidispatch_rock_t objrock)) {
	Window *win;
	Stream *str;
	frefid_t fref;

	gli_register_obj = regi;
	gli_unregister_obj = unregi;

	if (gli_register_obj)
	{
		/* It's now necessary to go through all existing objects, and register
			them. */
		for (win = glk_window_iterate(nullptr, nullptr);
			win;
			win = glk_window_iterate(win, nullptr))
		{
			win->_dispRock = (*gli_register_obj)(win, gidisp_Class_Window);
		}
		for (str = glk_stream_iterate(nullptr, nullptr);
			str;
			str = glk_stream_iterate(str, nullptr))
		{
			str->_dispRock = (*gli_register_obj)(str, gidisp_Class_Stream);
		}
		for (fref = glk_fileref_iterate(nullptr, nullptr);
			fref;
			fref = glk_fileref_iterate(fref, nullptr))
		{
			fref->_dispRock = (*gli_register_obj)(fref, gidisp_Class_Fileref);
		}
	}
}

void GlkAPI::gidispatch_set_retained_registry(gidispatch_rock_t(*regi)(void *array, uint len, const char *typecode),
		void(*unregi)(void *array, uint len, const char *typecode, gidispatch_rock_t objrock)) {
	gli_register_arr = regi;
	gli_unregister_arr = unregi;
}

uint32 GlkAPI::gidispatch_count_classes() const {
    return NUMCLASSES;
}

const gidispatch_intconst_t *GlkAPI::gidispatch_get_class(uint32 index) const {
    if (index >= NUMCLASSES)
        return nullptr;
    return &(class_table[index]);
}

uint32 GlkAPI::gidispatch_count_intconst() const {
    return NUMINTCONSTANTS;
}

const gidispatch_intconst_t *GlkAPI::gidispatch_get_intconst(uint32 index) const {
    if (index >= NUMINTCONSTANTS)
        return nullptr;
    return &(intconstant_table[index]);
}

const char *GlkAPI::gidispatch_prototype(uint32 funcnum) const {
    switch (funcnum) {
        case 0x0001: /* exit */
            return "0:";
        case 0x0002: /* set_interrupt_handler */
            /* cannot be invoked through dispatch layer */
            return nullptr;
        case 0x0003: /* tick */
            return "0:";
        case 0x0004: /* gestalt */
            return "3IuIu:Iu";
        case 0x0005: /* gestalt_ext */
            return "4IuIu&#Iu:Iu";
        case 0x0020: /* window_iterate */
            return "3Qa<Iu:Qa";
        case 0x0021: /* window_get_rock */
            return "2Qa:Iu";
        case 0x0022: /* window_get_root */
            return "1:Qa";
        case 0x0023: /* window_open */
            return "6QaIuIuIuIu:Qa";
        case 0x0024: /* window_close */
            return "2Qa<[2IuIu]:";
        case 0x0025: /* window_get_size */
            return "3Qa<Iu<Iu:";
        case 0x0026: /* window_set_arrangement */
            return "4QaIuIuQa:";
        case 0x0027: /* window_get_arrangement */
            return "4Qa<Iu<Iu<Qa:";
        case 0x0028: /* window_get_type */
            return "2Qa:Iu";
        case 0x0029: /* window_get_parent */
            return "2Qa:Qa";
        case 0x002A: /* window_clear */
            return "1Qa:";
        case 0x002B: /* window_move_cursor */
            return "3QaIuIu:";
        case 0x002C: /* window_get_stream */
            return "2Qa:Qb";
        case 0x002D: /* window_set_echo_stream */
            return "2QaQb:";
        case 0x002E: /* window_get_echo_stream */
            return "2Qa:Qb";
        case 0x002F: /* set_window */
            return "1Qa:";
        case 0x0030: /* window_get_sibling */
            return "2Qa:Qa";
        case 0x0040: /* stream_iterate */
            return "3Qb<Iu:Qb";
        case 0x0041: /* stream_get_rock */
            return "2Qb:Iu";
        case 0x0042: /* stream_open_file */
            return "4QcIuIu:Qb";
        case 0x0043: /* stream_open_memory */
            return "4&#!CnIuIu:Qb";
        case 0x0044: /* stream_close */
            return "2Qb<[2IuIu]:";
        case 0x0045: /* stream_set_position */
            return "3QbIsIu:";
        case 0x0046: /* stream_get_position */
            return "2Qb:Iu";
        case 0x0047: /* stream_set_current */
            return "1Qb:";
        case 0x0048: /* stream_get_current */
            return "1:Qb";
        case 0x0060: /* fileref_create_temp */
            return "3IuIu:Qc";
        case 0x0061: /* fileref_create_by_name */
            return "4IuSIu:Qc";
        case 0x0062: /* fileref_create_by_prompt */
            return "4IuIuIu:Qc";
        case 0x0063: /* fileref_destroy */
            return "1Qc:";
        case 0x0064: /* fileref_iterate */
            return "3Qc<Iu:Qc";
        case 0x0065: /* fileref_get_rock */
            return "2Qc:Iu";
        case 0x0066: /* fileref_delete_file */
            return "1Qc:";
        case 0x0067: /* fileref_does_file_exist */
            return "2Qc:Iu";
        case 0x0068: /* fileref_create_from_fileref */
            return "4IuQcIu:Qc";
        case 0x0080: /* put_char */
            return "1Cu:";
        case 0x0081: /* put_char_stream */
            return "2QbCu:";
        case 0x0082: /* put_string */
            return "1S:";
        case 0x0083: /* put_string_stream */
            return "2QbS:";
        case 0x0084: /* put_buffer */
            return "1>+#Cn:";
        case 0x0085: /* put_buffer_stream */
            return "2Qb>+#Cn:";
        case 0x0086: /* set_style */
            return "1Iu:";
        case 0x0087: /* set_style_stream */
            return "2QbIu:";
        case 0x0090: /* get_char_stream */
            return "2Qb:Is";
        case 0x0091: /* get_line_stream */
            return "3Qb<+#Cn:Iu";
        case 0x0092: /* get_buffer_stream */
            return "3Qb<+#Cn:Iu";
        case 0x00A0: /* char_to_lower */
            return "2Cu:Cu";
        case 0x00A1: /* char_to_upper */
            return "2Cu:Cu";
        case 0x00B0: /* stylehint_set */
            return "4IuIuIuIs:";
        case 0x00B1: /* stylehint_clear */
            return "3IuIuIu:";
        case 0x00B2: /* style_distinguish */
            return "4QaIuIu:Iu";
        case 0x00B3: /* style_measure */
            return "5QaIuIu<Iu:Iu";
        case 0x00C0: /* select */
            return "1<+[4IuQaIuIu]:";
        case 0x00C1: /* select_poll */
            return "1<+[4IuQaIuIu]:";
        case 0x00D0: /* request_line_event */
            return "3Qa&+#!CnIu:";
        case 0x00D1: /* cancel_line_event */
            return "2Qa<[4IuQaIuIu]:";
        case 0x00D2: /* request_char_event */
            return "1Qa:";
        case 0x00D3: /* cancel_char_event */
            return "1Qa:";
        case 0x00D4: /* request_mouse_event */
            return "1Qa:";
        case 0x00D5: /* cancel_mouse_event */
            return "1Qa:";
        case 0x00D6: /* request_timer_events */
            return "1Iu:";

#ifdef GLK_MODULE_IMAGE
        case 0x00E0: /* image_get_info */
            return "4Iu<Iu<Iu:Iu";
        case 0x00E1: /* image_draw */
            return "5QaIuIsIs:Iu";
        case 0x00E2: /* image_draw_scaled */
            return "7QaIuIsIsIuIu:Iu";
        case 0x00E8: /* window_flow_break */
            return "1Qa:";
        case 0x00E9: /* window_erase_rect */
            return "5QaIsIsIuIu:";
        case 0x00EA: /* window_fill_rect */
            return "6QaIuIsIsIuIu:";
        case 0x00EB: /* window_set_background_color */
            return "2QaIu:";
#endif /* GLK_MODULE_IMAGE */

#ifdef GLK_MODULE_SOUND
        case 0x00F0: /* schannel_iterate */
            return "3Qd<Iu:Qd";
        case 0x00F1: /* schannel_get_rock */
            return "2Qd:Iu";
        case 0x00F2: /* schannel_create */
            return "2Iu:Qd";
        case 0x00F3: /* schannel_destroy */
            return "1Qd:";
        case 0x00F8: /* schannel_play */
            return "3QdIu:Iu";
        case 0x00F9: /* schannel_play_ext */
            return "5QdIuIuIu:Iu";
        case 0x00FA: /* schannel_stop */
            return "1Qd:";
        case 0x00FB: /* schannel_set_volume */
            return "2QdIu:";
        case 0x00FC: /* sound_load_hint */
            return "2IuIu:";

#ifdef GLK_MODULE_SOUND2
        case 0x00F4: /* schannel_create_ext */
            return "3IuIu:Qd";
        case 0x00F7: /* schannel_play_multi */
            return "4>+#Qd>+#IuIu:Iu";
        case 0x00FD: /* schannel_set_volume_ext */
            return "4QdIuIuIu:";
        case 0x00FE: /* schannel_pause */
            return "1Qd:";
        case 0x00FF: /* schannel_unpause */
            return "1Qd:";
#endif /* GLK_MODULE_SOUND2 */
#endif /* GLK_MODULE_SOUND */

#ifdef GLK_MODULE_HYPERLINKS
        case 0x0100: /* set_hyperlink */
            return "1Iu:";
        case 0x0101: /* set_hyperlink_stream */
            return "2QbIu:";
        case 0x0102: /* request_hyperlink_event */
            return "1Qa:";
        case 0x0103: /* cancel_hyperlink_event */
            return "1Qa:";
#endif /* GLK_MODULE_HYPERLINKS */

#ifdef GLK_MODULE_UNICODE
        case 0x0120: /* buffer_to_lower_case_uni */
            return "3&+#IuIu:Iu";
        case 0x0121: /* buffer_to_upper_case_uni */
            return "3&+#IuIu:Iu";
        case 0x0122: /* buffer_to_title_case_uni */
            return "4&+#IuIuIu:Iu";
        case 0x0128: /* put_char_uni */
            return "1Iu:";
        case 0x0129: /* put_string_uni */
            return "1U:";
        case 0x012A: /* put_buffer_uni */
            return "1>+#Iu:";
        case 0x012B: /* put_char_stream_uni */
            return "2QbIu:";
        case 0x012C: /* put_string_stream_uni */
            return "2QbU:";
        case 0x012D: /* put_buffer_stream_uni */
            return "2Qb>+#Iu:";
        case 0x0130: /* get_char_stream_uni */
            return "2Qb:Is";
        case 0x0131: /* get_buffer_stream_uni */
            return "3Qb<+#Iu:Iu";
        case 0x0132: /* get_line_stream_uni */
            return "3Qb<+#Iu:Iu";
        case 0x0138: /* stream_open_file_uni */
            return "4QcIuIu:Qb";
        case 0x0139: /* stream_open_memory_uni */
            return "4&#!IuIuIu:Qb";
        case 0x0140: /* request_char_event_uni */
            return "1Qa:";
        case 0x0141: /* request_line_event_uni */
            return "3Qa&+#!IuIu:";
#endif /* GLK_MODULE_UNICODE */

#ifdef GLK_MODULE_UNICODE_NORM
        case 0x0123: /* buffer_canon_decompose_uni */
            return "3&+#IuIu:Iu";
        case 0x0124: /* buffer_canon_normalize_uni */
            return "3&+#IuIu:Iu";
#endif /* GLK_MODULE_UNICODE_NORM */

#ifdef GLK_MODULE_LINE_ECHO
        case 0x0150: /* set_echo_line_event */
            return "2QaIu:";
#endif /* GLK_MODULE_LINE_ECHO */

#ifdef GLK_MODULE_LINE_TERMINATORS
        case 0x0151: /* set_terminators_line_event */
            return "2Qa>#Iu:";
#endif /* GLK_MODULE_LINE_TERMINATORS */

#ifdef GLK_MODULE_DATETIME
        case 0x0160: /* current_time */
            return "1<+[3IsIuIs]:";
        case 0x0161: /* current_simple_time */
            return "2Iu:Is";
        case 0x0168: /* time_to_date_utc */
            return "2>+[3IsIuIs]<+[8IsIsIsIsIsIsIsIs]:";
        case 0x0169: /* time_to_date_local */
            return "2>+[3IsIuIs]<+[8IsIsIsIsIsIsIsIs]:";
        case 0x016A: /* simple_time_to_date_utc */
            return "3IsIu<+[8IsIsIsIsIsIsIsIs]:";
        case 0x016B: /* simple_time_to_date_local */
            return "3IsIu<+[8IsIsIsIsIsIsIsIs]:";
        case 0x016C: /* date_to_time_utc */
            return "2>+[8IsIsIsIsIsIsIsIs]<+[3IsIuIs]:";
        case 0x016D: /* date_to_time_local */
            return "2>+[8IsIsIsIsIsIsIsIs]<+[3IsIuIs]:";
        case 0x016E: /* date_to_simple_time_utc */
            return "3>+[8IsIsIsIsIsIsIsIs]Iu:Is";
        case 0x016F: /* date_to_simple_time_local */
            return "3>+[8IsIsIsIsIsIsIsIs]Iu:Is";
#endif /* GLK_MODULE_DATETIME */

#ifdef GLK_MODULE_GARGLKTEXT
        case 0x1100: /* garglk_set_zcolors */
            return "2IuIu:";
        case 0x1101: /* garglk_set_zcolors_stream */
            return "3QbIuIu:";
        case 0x1102: /* garglk_set_reversevideo */
            return "1Iu:";
        case 0x1103: /* garglk_set_reversevideo_stream */
            return "2QbIu:";
#endif /* GLK_MODULE_GARGLKTEXT */

        default:
            return nullptr;
    }
}

void GlkAPI::gidispatch_call(uint32 funcnum, uint32 numargs, gluniversal_t *arglist) {
    switch (funcnum) {
        case 0x0001: /* exit */
            glk_exit();
            break;
        case 0x0002: /* set_interrupt_handler */
            /* cannot be invoked through dispatch layer */
            break;
        case 0x0003: /* tick */
            glk_tick();
            break;
        case 0x0004: /* gestalt */
            arglist[3]._uint = glk_gestalt(arglist[0]._uint, arglist[1]._uint);
            break;
        case 0x0005: /* gestalt_ext */
            if (arglist[2]._ptrflag) {
                arglist[6]._uint = glk_gestalt_ext(arglist[0]._uint, arglist[1]._uint,
                    (uint *)arglist[3]._array, arglist[4]._uint);
            } else {
                arglist[4]._uint = glk_gestalt_ext(arglist[0]._uint, arglist[1]._uint,
                    nullptr, 0);
            }
            break;
        case 0x0020: /* window_iterate */
            if (arglist[1]._ptrflag)
                arglist[4]._opaqueref = glk_window_iterate((Window *)arglist[0]._opaqueref, &arglist[2]._uint);
            else
                arglist[3]._opaqueref = glk_window_iterate((Window *)arglist[0]._opaqueref, nullptr);
            break;
        case 0x0021: /* window_get_rock */
            arglist[2]._uint = glk_window_get_rock((Window *)arglist[0]._opaqueref);
            break;
        case 0x0022: /* window_get_root */
            arglist[1]._opaqueref = glk_window_get_root();
            break;
        case 0x0023: /* window_open */
            arglist[6]._opaqueref = glk_window_open((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                arglist[2]._uint, arglist[3]._uint, arglist[4]._uint);
            break;
        case 0x0024: /* window_close */
            if (arglist[1]._ptrflag) {
                stream_result_t dat;
                glk_window_close((Window *)arglist[0]._opaqueref, &dat);
                arglist[2]._uint = dat._readCount;
                arglist[3]._uint = dat._writeCount;
            } else {
                glk_window_close((Window *)arglist[0]._opaqueref, nullptr);
            }
            break;
        case 0x0025: /* window_get_size */
            {
                int ix = 1;
                uint *ptr1, *ptr2;
                if (!arglist[ix]._ptrflag) {
                    ptr1 = nullptr;
                } else {
                    ix++;
                    ptr1 = &(arglist[ix]._uint);
                }
                ix++;
                if (!arglist[ix]._ptrflag) {
                    ptr2 = nullptr;
                } else {
                    ix++;
                    ptr2 = &(arglist[ix]._uint);
                }
                ix++;
                glk_window_get_size((Window *)arglist[0]._opaqueref, ptr1, ptr2);
				break;
            }
        case 0x0026: /* window_set_arrangement */
            glk_window_set_arrangement((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                arglist[2]._uint, (Window *)arglist[3]._opaqueref);
            break;
        case 0x0027: /* window_get_arrangement */
            {
                int ix = 1;
                uint *ptr1, *ptr2;
                winid_t *ptr3;
                if (!arglist[ix]._ptrflag) {
                    ptr1 = nullptr;
                } else {
                    ix++;
                    ptr1 = &(arglist[ix]._uint);
                }
                ix++;
                if (!arglist[ix]._ptrflag) {
                    ptr2 = nullptr;
                } else {
                    ix++;
                    ptr2 = &(arglist[ix]._uint);
                }
                ix++;
                if (!arglist[ix]._ptrflag) {
                    ptr3 = nullptr;
                } else {
                    ix++;
                    ptr3 = (winid_t *)(&(arglist[ix]._opaqueref));
                }
                ix++;
                glk_window_get_arrangement((Window *)arglist[0]._opaqueref, ptr1, ptr2, ptr3);
            }
            break;
        case 0x0028: /* window_get_type */
            arglist[2]._uint = glk_window_get_type((Window *)arglist[0]._opaqueref);
            break;
        case 0x0029: /* window_get_parent */
            arglist[2]._opaqueref = glk_window_get_parent((Window *)arglist[0]._opaqueref);
            break;
        case 0x002A: /* window_clear */
            glk_window_clear((Window *)arglist[0]._opaqueref);
            break;
        case 0x002B: /* window_move_cursor */
            glk_window_move_cursor((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                arglist[2]._uint);
            break;
        case 0x002C: /* window_get_stream */
            arglist[2]._opaqueref = glk_window_get_stream((Window *)arglist[0]._opaqueref);
            break;
        case 0x002D: /* window_set_echo_stream */
            glk_window_set_echo_stream((Window *)arglist[0]._opaqueref, (Stream *)arglist[1]._opaqueref);
            break;
        case 0x002E: /* window_get_echo_stream */
            arglist[2]._opaqueref = glk_window_get_echo_stream((Window *)arglist[0]._opaqueref);
            break;
        case 0x002F: /* set_window */
            glk_set_window((Window *)arglist[0]._opaqueref);
            break;
        case 0x0030: /* window_get_sibling */
            arglist[2]._opaqueref = glk_window_get_sibling((Window *)arglist[0]._opaqueref);
            break;
        case 0x0040: /* stream_iterate */
            if (arglist[1]._ptrflag)
                arglist[4]._opaqueref = glk_stream_iterate((Stream *)arglist[0]._opaqueref, &arglist[2]._uint);
            else
                arglist[3]._opaqueref = glk_stream_iterate((Stream *)arglist[0]._opaqueref, nullptr);
            break;
        case 0x0041: /* stream_get_rock */
            arglist[2]._uint = glk_stream_get_rock((Stream *)arglist[0]._opaqueref);
            break;
        case 0x0042: /* stream_open_file */
            arglist[4]._opaqueref = glk_stream_open_file((frefid_t)arglist[0]._opaqueref, (FileMode)arglist[1]._uint,
                arglist[2]._uint);
            break;
        case 0x0043: /* stream_open_memory */
            if (arglist[0]._ptrflag)
                arglist[6]._opaqueref = glk_stream_open_memory((char *)arglist[1]._array,
                    arglist[2]._uint, (FileMode)arglist[3]._uint, arglist[4]._uint);
            else
                arglist[4]._opaqueref = glk_stream_open_memory(nullptr, 0, (FileMode)arglist[1]._uint, arglist[2]._uint);
            break;
        case 0x0044: /* stream_close */
            if (arglist[1]._ptrflag) {
                stream_result_t dat;
                glk_stream_close((Stream *)arglist[0]._opaqueref, &dat);
                arglist[2]._uint = dat._readCount;
                arglist[3]._uint = dat._writeCount;
            } else {
                glk_stream_close((Stream *)arglist[0]._opaqueref, nullptr);
            }
            break;
        case 0x0045: /* stream_set_position */
            glk_stream_set_position((Stream *)arglist[0]._opaqueref, arglist[1]._sint,
                arglist[2]._uint);
            break;
        case 0x0046: /* stream_get_position */
            arglist[2]._uint = glk_stream_get_position((Stream *)arglist[0]._opaqueref);
            break;
        case 0x0047: /* stream_set_current */
            glk_stream_set_current((Stream *)arglist[0]._opaqueref);
            break;
        case 0x0048: /* stream_get_current */
            arglist[1]._opaqueref = glk_stream_get_current();
            break;
        case 0x0060: /* fileref_create_temp */
            arglist[3]._opaqueref = glk_fileref_create_temp(arglist[0]._uint,
                arglist[1]._uint);
            break;
        case 0x0061: /* fileref_create_by_name */
            arglist[4]._opaqueref = glk_fileref_create_by_name(arglist[0]._uint,
                arglist[1]._charstr, arglist[2]._uint);
            break;
        case 0x0062: /* fileref_create_by_prompt */
            arglist[4]._opaqueref = glk_fileref_create_by_prompt(arglist[0]._uint, (FileMode)arglist[1]._uint, arglist[2]._uint);
            break;
        case 0x0063: /* fileref_destroy */
            glk_fileref_destroy((frefid_t)arglist[0]._opaqueref);
            break;
        case 0x0064: /* fileref_iterate */
            if (arglist[1]._ptrflag)
                arglist[4]._opaqueref = glk_fileref_iterate((frefid_t)arglist[0]._opaqueref, &arglist[2]._uint);
            else
                arglist[3]._opaqueref = glk_fileref_iterate((frefid_t)arglist[0]._opaqueref, nullptr);
            break;
        case 0x0065: /* fileref_get_rock */
            arglist[2]._uint = glk_fileref_get_rock((frefid_t)arglist[0]._opaqueref);
            break;
        case 0x0066: /* fileref_delete_file */
            glk_fileref_delete_file((frefid_t)arglist[0]._opaqueref);
            break;
        case 0x0067: /* fileref_does_file_exist */
            arglist[2]._uint = glk_fileref_does_file_exist((frefid_t)arglist[0]._opaqueref);
            break;
        case 0x0068: /* fileref_create_from_fileref */
            arglist[4]._opaqueref = glk_fileref_create_from_fileref(arglist[0]._uint, (frefid_t)arglist[1]._opaqueref, arglist[2]._uint);
            break;
        case 0x0080: /* put_char */
            glk_put_char(arglist[0]._uch);
            break;
        case 0x0081: /* put_char_stream */
            glk_put_char_stream((Stream *)arglist[0]._opaqueref, arglist[1]._uch);
            break;
        case 0x0082: /* put_string */
            glk_put_string(arglist[0]._charstr);
            break;
        case 0x0083: /* put_string_stream */
            glk_put_string_stream((Stream *)arglist[0]._opaqueref, arglist[1]._charstr);
            break;
        case 0x0084: /* put_buffer */
            if (arglist[0]._ptrflag)
                glk_put_buffer((const char *)arglist[1]._array, arglist[2]._uint);
            else
                glk_put_buffer(nullptr, 0);
            break;
        case 0x0085: /* put_buffer_stream */
            if (arglist[1]._ptrflag)
                glk_put_buffer_stream((Stream *)arglist[0]._opaqueref, (const char *)arglist[2]._array, arglist[3]._uint);
            else
                glk_put_buffer_stream((Stream *)arglist[0]._opaqueref,
                    nullptr, 0);
            break;
        case 0x0086: /* set_style */
            glk_set_style(arglist[0]._uint);
            break;
        case 0x0087: /* set_style_stream */
            glk_set_style_stream((Stream *)arglist[0]._opaqueref, arglist[1]._uint);
            break;
        case 0x0090: /* get_char_stream */
            arglist[2]._sint = glk_get_char_stream((Stream *)arglist[0]._opaqueref);
            break;
        case 0x0091: /* get_line_stream */
            if (arglist[1]._ptrflag)
                arglist[5]._uint = glk_get_line_stream((Stream *)arglist[0]._opaqueref, (char *)arglist[2]._array, arglist[3]._uint);
            else
                arglist[3]._uint = glk_get_line_stream((Stream *)arglist[0]._opaqueref,
                    nullptr, 0);
            break;
        case 0x0092: /* get_buffer_stream */
            if (arglist[1]._ptrflag)
                arglist[5]._uint = glk_get_buffer_stream((Stream *)arglist[0]._opaqueref, (char *)arglist[2]._array, arglist[3]._uint);
            else
                arglist[3]._uint = glk_get_buffer_stream((Stream *)arglist[0]._opaqueref,
                    nullptr, 0);
            break;
        case 0x00A0: /* char_to_lower */
            arglist[2]._uch = glk_char_to_lower(arglist[0]._uch);
            break;
        case 0x00A1: /* char_to_upper */
            arglist[2]._uch = glk_char_to_upper(arglist[0]._uch);
            break;
        case 0x00B0: /* stylehint_set */
            glk_stylehint_set(arglist[0]._uint, arglist[1]._uint,
                arglist[2]._uint, arglist[3]._sint);
            break;
        case 0x00B1: /* stylehint_clear */
            glk_stylehint_clear(arglist[0]._uint, arglist[1]._uint,
                arglist[2]._uint);
            break;
        case 0x00B2: /* style_distinguish */
            arglist[4]._uint = glk_style_distinguish((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                arglist[2]._uint);
            break;
        case 0x00B3: /* style_measure */
            if (arglist[3]._ptrflag)
                arglist[6]._uint = glk_style_measure((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                    arglist[2]._uint, &(arglist[4]._uint));
            else
                arglist[5]._uint = glk_style_measure((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                    arglist[2]._uint, nullptr);
            break;
        case 0x00C0: /* select */
            if (arglist[0]._ptrflag) {
                event_t dat;
                glk_select(&dat);
                arglist[1]._uint = dat.type;
                arglist[2]._opaqueref = dat.window;
                arglist[3]._uint = dat.val1;
                arglist[4]._uint = dat.val2;
            } else {
                glk_select(nullptr);
            }
            break;
        case 0x00C1: /* select_poll */
            if (arglist[0]._ptrflag) {
                event_t dat;
                glk_select_poll(&dat);
                arglist[1]._uint = dat.type;
                arglist[2]._opaqueref = dat.window;
                arglist[3]._uint = dat.val1;
                arglist[4]._uint = dat.val2;
            } else {
                glk_select_poll(nullptr);
            }
            break;
        case 0x00D0: /* request_line_event */
            if (arglist[1]._ptrflag)
                glk_request_line_event((Window *)arglist[0]._opaqueref, (char *)arglist[2]._array,
                    arglist[3]._uint, arglist[4]._uint);
            else
                glk_request_line_event((Window *)arglist[0]._opaqueref, nullptr,
                    0, arglist[2]._uint);
            break;
        case 0x00D1: /* cancel_line_event */
            if (arglist[1]._ptrflag) {
                event_t dat;
                glk_cancel_line_event((Window *)arglist[0]._opaqueref, &dat);
                arglist[2]._uint = dat.type;
                arglist[3]._opaqueref = dat.window;
                arglist[4]._uint = dat.val1;
                arglist[5]._uint = dat.val2;
            } else {
                glk_cancel_line_event((Window *)arglist[0]._opaqueref, nullptr);
            }
            break;
        case 0x00D2: /* request_char_event */
            glk_request_char_event((Window *)arglist[0]._opaqueref);
            break;
        case 0x00D3: /* cancel_char_event */
            glk_cancel_char_event((Window *)arglist[0]._opaqueref);
            break;
        case 0x00D4: /* request_mouse_event */
            glk_request_mouse_event((Window *)arglist[0]._opaqueref);
            break;
        case 0x00D5: /* cancel_mouse_event */
            glk_cancel_mouse_event((Window *)arglist[0]._opaqueref);
            break;
        case 0x00D6: /* request_timer_events */
            glk_request_timer_events(arglist[0]._uint);
            break;

#ifdef GLK_MODULE_IMAGE
        case 0x00E0: /* image_get_info */
            {
                int ix = 1;
                uint *ptr1, *ptr2;
                if (!arglist[ix]._ptrflag) {
                    ptr1 = nullptr;
                } else {
                    ix++;
                    ptr1 = &(arglist[ix]._uint);
                }
                ix++;
                if (!arglist[ix]._ptrflag) {
                    ptr2 = nullptr;
                } else {
                    ix++;
                    ptr2 = &(arglist[ix]._uint);
                }
                ix++;
                ix++;
                arglist[ix]._uint = glk_image_get_info(arglist[0]._uint, ptr1, ptr2);
            }
            break;
        case 0x00E1: /* image_draw */
            arglist[5]._uint = glk_image_draw((Window *)arglist[0]._opaqueref,
                arglist[1]._uint,
                arglist[2]._sint, arglist[3]._sint);
            break;
        case 0x00E2: /* image_draw_scaled */
            arglist[7]._uint = glk_image_draw_scaled((Window *)arglist[0]._opaqueref,
                arglist[1]._uint,
                arglist[2]._sint, arglist[3]._sint,
                arglist[4]._uint, arglist[5]._uint);
            break;
        case 0x00E8: /* window_flow_break */
            glk_window_flow_break((Window *)arglist[0]._opaqueref);
            break;
        case 0x00E9: /* window_erase_rect */
            glk_window_erase_rect((Window *)arglist[0]._opaqueref,
                arglist[1]._sint, arglist[2]._sint,
                arglist[3]._uint, arglist[4]._uint);
            break;
        case 0x00EA: /* window_fill_rect */
            glk_window_fill_rect((Window *)arglist[0]._opaqueref, arglist[1]._uint,
                arglist[2]._sint, arglist[3]._sint,
                arglist[4]._uint, arglist[5]._uint);
            break;
        case 0x00EB: /* window_set_background_color */
            glk_window_set_background_color((Window *)arglist[0]._opaqueref, arglist[1]._uint);
            break;
#endif /* GLK_MODULE_IMAGE */

#ifdef GLK_MODULE_SOUND
        case 0x00F0: /* schannel_iterate */
            if (arglist[1]._ptrflag)
                arglist[4]._opaqueref = glk_schannel_iterate((schanid_t)arglist[0]._opaqueref, &arglist[2]._uint);
            else
                arglist[3]._opaqueref = glk_schannel_iterate((schanid_t)arglist[0]._opaqueref, nullptr);
            break;
        case 0x00F1: /* schannel_get_rock */
            arglist[2]._uint = glk_schannel_get_rock((schanid_t)arglist[0]._opaqueref);
            break;
        case 0x00F2: /* schannel_create */
            arglist[2]._opaqueref = glk_schannel_create(arglist[0]._uint);
            break;
        case 0x00F3: /* schannel_destroy */
            glk_schannel_destroy((schanid_t)arglist[0]._opaqueref);
            break;
        case 0x00F8: /* schannel_play */
            arglist[3]._uint = glk_schannel_play((schanid_t)arglist[0]._opaqueref, arglist[1]._uint);
            break;
        case 0x00F9: /* schannel_play_ext */
            arglist[5]._uint = glk_schannel_play_ext((schanid_t)arglist[0]._opaqueref,
                arglist[1]._uint, arglist[2]._uint, arglist[3]._uint);
            break;
        case 0x00FA: /* schannel_stop */
            glk_schannel_stop((schanid_t)arglist[0]._opaqueref);
            break;
        case 0x00FB: /* schannel_set_volume */
            glk_schannel_set_volume((schanid_t)arglist[0]._opaqueref, arglist[1]._uint);
            break;
        case 0x00FC: /* sound_load_hint */
            glk_sound_load_hint(arglist[0]._uint, arglist[1]._uint);
            break;

#ifdef GLK_MODULE_SOUND2
        case 0x00F4: /* schannel_create_ext */
            arglist[3]._opaqueref = glk_schannel_create_ext(arglist[0]._uint, arglist[1]._uint);
            break;
        case 0x00F7: /* schannel_play_multi */
            if (arglist[0]._ptrflag && arglist[3]._ptrflag)
                arglist[8]._uint = glk_schannel_play_multi((schanid_t *)arglist[1]._array, arglist[2]._uint, (uint *)arglist[4]._array, arglist[5]._uint, arglist[6]._uint);
            else if (arglist[0]._ptrflag)
                arglist[6]._uint = glk_schannel_play_multi((schanid_t *)arglist[1]._array, arglist[2]._uint, nullptr, 0, arglist[4]._uint);
            else if (arglist[1]._ptrflag)
                arglist[6]._uint = glk_schannel_play_multi(nullptr, 0, (uint *)arglist[2]._array, arglist[3]._uint, arglist[4]._uint);
            else
                arglist[4]._uint = glk_schannel_play_multi(nullptr, 0, nullptr, 0, arglist[2]._uint);
            break;
        case 0x00FD: /* schannel_set_volume_ext */
            glk_schannel_set_volume_ext((schanid_t)arglist[0]._opaqueref, arglist[1]._uint, arglist[2]._uint, arglist[3]._uint);
            break;
        case 0x00FE: /* schannel_pause */
            glk_schannel_pause((schanid_t)arglist[0]._opaqueref);
            break;
        case 0x00FF: /* schannel_unpause */
            glk_schannel_unpause((schanid_t)arglist[0]._opaqueref);
            break;
#endif /* GLK_MODULE_SOUND2 */
#endif /* GLK_MODULE_SOUND */

#ifdef GLK_MODULE_HYPERLINKS
        case 0x0100: /* set_hyperlink */
            glk_set_hyperlink(arglist[0]._uint);
            break;
        case 0x0101: /* set_hyperlink_stream */
            glk_set_hyperlink_stream((strid_t)arglist[0]._opaqueref, arglist[1]._uint);
            break;
        case 0x0102: /* request_hyperlink_event */
            glk_request_hyperlink_event((Window *)arglist[0]._opaqueref);
            break;
        case 0x0103: /* cancel_hyperlink_event */
            glk_cancel_hyperlink_event((Window *)arglist[0]._opaqueref);
            break;
#endif /* GLK_MODULE_HYPERLINKS */

#ifdef GLK_MODULE_UNICODE
        case 0x0120: /* buffer_to_lower_case_uni */
            if (arglist[0]._ptrflag)
                arglist[5]._uint = glk_buffer_to_lower_case_uni((uint32 *)arglist[1]._array, arglist[2]._uint, arglist[3]._uint);
            else
                arglist[3]._uint = glk_buffer_to_lower_case_uni(nullptr, 0, arglist[1]._uint);
            break;
        case 0x0121: /* buffer_to_upper_case_uni */
            if (arglist[0]._ptrflag)
                arglist[5]._uint = glk_buffer_to_upper_case_uni((uint32 *)arglist[1]._array, arglist[2]._uint, arglist[3]._uint);
            else
                arglist[3]._uint = glk_buffer_to_upper_case_uni(nullptr, 0, arglist[1]._uint);
            break;
        case 0x0122: /* buffer_to_title_case_uni */
            if (arglist[0]._ptrflag)
                arglist[6]._uint = glk_buffer_to_title_case_uni((uint32 *)arglist[1]._array, arglist[2]._uint, arglist[3]._uint, arglist[4]._uint);
            else
                arglist[4]._uint = glk_buffer_to_title_case_uni(nullptr, 0, arglist[1]._uint, arglist[2]._uint);
            break;
        case 0x0128: /* put_char_uni */
            glk_put_char_uni(arglist[0]._uint);
            break;
        case 0x0129: /* put_string_uni */
            glk_put_string_uni(arglist[0]._unicharstr);
            break;
        case 0x012A: /* put_buffer_uni */
            if (arglist[0]._ptrflag)
                glk_put_buffer_uni((const uint32 *)arglist[1]._array, arglist[2]._uint);
            else
                glk_put_buffer_uni(nullptr, 0);
            break;
        case 0x012B: /* put_char_stream_uni */
            glk_put_char_stream_uni((strid_t)arglist[0]._opaqueref, arglist[1]._uint);
            break;
        case 0x012C: /* put_string_stream_uni */
            glk_put_string_stream_uni((strid_t)arglist[0]._opaqueref, arglist[1]._unicharstr);
            break;
        case 0x012D: /* put_buffer_stream_uni */
            if (arglist[1]._ptrflag)
                glk_put_buffer_stream_uni((strid_t)arglist[0]._opaqueref, (const uint32 *)arglist[2]._array, arglist[3]._uint);
            else
                glk_put_buffer_stream_uni((strid_t)arglist[0]._opaqueref, nullptr, 0);
            break;
        case 0x0130: /* get_char_stream_uni */
            arglist[2]._sint = glk_get_char_stream_uni((strid_t)arglist[0]._opaqueref);
            break;
        case 0x0131: /* get_buffer_stream_uni */
            if (arglist[1]._ptrflag)
                arglist[5]._uint = glk_get_buffer_stream_uni((strid_t)arglist[0]._opaqueref,
                    (uint32 *)arglist[2]._array, arglist[3]._uint);
            else
                arglist[3]._uint = glk_get_buffer_stream_uni((strid_t)arglist[0]._opaqueref, nullptr, 0);
            break;
        case 0x0132: /* get_line_stream_uni */
            if (arglist[1]._ptrflag)
                arglist[5]._uint = glk_get_line_stream_uni((strid_t)arglist[0]._opaqueref,
                    (uint32 *)arglist[2]._array, arglist[3]._uint);
            else
                arglist[3]._uint = glk_get_line_stream_uni((strid_t)arglist[0]._opaqueref, nullptr, 0);
            break;
        case 0x0138: /* stream_open_file_uni */
            arglist[4]._opaqueref = glk_stream_open_file_uni((frefid_t)arglist[0]._opaqueref, (FileMode)arglist[1]._uint,
                arglist[2]._uint);
            break;
        case 0x0139: /* stream_open_memory_uni */
            if (arglist[0]._ptrflag)
                arglist[6]._opaqueref = glk_stream_open_memory_uni((uint32 *)arglist[1]._array,
                    arglist[2]._uint, (FileMode)arglist[3]._uint, arglist[4]._uint);
            else
                arglist[4]._opaqueref = glk_stream_open_memory_uni(nullptr, 0, (FileMode)arglist[1]._uint, arglist[2]._uint);
            break;
        case 0x0140: /* request_char_event_uni */
            glk_request_char_event_uni((Window *)arglist[0]._opaqueref);
            break;
        case 0x0141: /* request_line_event_uni */
            if (arglist[1]._ptrflag)
                glk_request_line_event_uni((Window *)arglist[0]._opaqueref, (uint32 *)arglist[2]._array,
                    arglist[3]._uint, arglist[4]._uint);
            else
                glk_request_line_event_uni((Window *)arglist[0]._opaqueref, nullptr,
                    0, arglist[2]._uint);
            break;
#endif /* GLK_MODULE_UNICODE */

#ifdef GLK_MODULE_UNICODE_NORM
        case 0x0123: /* buffer_canon_decompose_uni */
            if (arglist[0]._ptrflag)
                arglist[5]._uint = glk_buffer_canon_decompose_uni((uint32 *)arglist[1]._array, arglist[2]._uint, arglist[3]._uint);
            else
                arglist[3]._uint = glk_buffer_canon_decompose_uni(nullptr, 0, arglist[1]._uint);
            break;
        case 0x0124: /* buffer_canon_normalize_uni */
            if (arglist[0]._ptrflag)
                arglist[5]._uint = glk_buffer_canon_normalize_uni((uint32 *)arglist[1]._array, arglist[2]._uint, arglist[3]._uint);
            else
                arglist[3]._uint = glk_buffer_canon_normalize_uni(nullptr, 0, arglist[1]._uint);
            break;
#endif /* GLK_MODULE_UNICODE_NORM */

#ifdef GLK_MODULE_LINE_ECHO
        case 0x0150: /* set_echo_line_event */
            glk_set_echo_line_event((Window *)arglist[0]._opaqueref, arglist[1]._uint);
            break;
#endif /* GLK_MODULE_LINE_ECHO */

#ifdef GLK_MODULE_LINE_TERMINATORS
        case 0x0151: /* set_terminators_line_event */
            if (arglist[1]._ptrflag)
                glk_set_terminators_line_event((Window *)arglist[0]._opaqueref, (const uint32 *)arglist[2]._array, arglist[3]._uint);
            else
                glk_set_terminators_line_event((Window *)arglist[0]._opaqueref,
                    nullptr, 0);
            break;
#endif /* GLK_MODULE_LINE_TERMINATORS */

#ifdef GLK_MODULE_DATETIME
        case 0x0160: /* current_time */
            if (arglist[0]._ptrflag) {
                glktimeval_t dat;
                glk_current_time(&dat);
                arglist[1]._sint = dat.high_sec;
                arglist[2]._uint = dat.low_sec;
                arglist[3]._sint = dat.microsec;
            } else {
                glk_current_time(nullptr);
            }
            break;
        case 0x0161: /* current_simple_time */
            arglist[2]._sint = glk_current_simple_time(arglist[0]._uint);
            break;
        case 0x0168: /* time_to_date_utc */ {
            glktimeval_t timeval;
            glktimeval_t *timeptr = nullptr;
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 0;
            if (arglist[ix++]._ptrflag) {
                timeptr = &timeval;
                timeval.high_sec = arglist[ix++]._sint;
                timeval.low_sec = arglist[ix++]._uint;
                timeval.microsec = arglist[ix++]._sint;
            }
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
            }
            glk_time_to_date_utc(timeptr, dateptr);
            if (dateptr) {
                arglist[ix++]._sint = date.year;
                arglist[ix++]._sint = date.month;
                arglist[ix++]._sint = date.day;
                arglist[ix++]._sint = date.weekday;
                arglist[ix++]._sint = date.hour;
                arglist[ix++]._sint = date.minute;
                arglist[ix++]._sint = date.second;
                arglist[ix++]._sint = date.microsec;
            }
            }
            break;
        case 0x0169: /* time_to_date_local */ {
            glktimeval_t timeval;
            glktimeval_t *timeptr = nullptr;
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 0;
            if (arglist[ix++]._ptrflag) {
                timeptr = &timeval;
                timeval.high_sec = arglist[ix++]._sint;
                timeval.low_sec = arglist[ix++]._uint;
                timeval.microsec = arglist[ix++]._sint;
            }
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
            }
            glk_time_to_date_local(timeptr, dateptr);
            if (dateptr) {
                arglist[ix++]._sint = date.year;
                arglist[ix++]._sint = date.month;
                arglist[ix++]._sint = date.day;
                arglist[ix++]._sint = date.weekday;
                arglist[ix++]._sint = date.hour;
                arglist[ix++]._sint = date.minute;
                arglist[ix++]._sint = date.second;
                arglist[ix++]._sint = date.microsec;
            }
            }
            break;
        case 0x016A: /* simple_time_to_date_utc */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 2;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
            }
            glk_simple_time_to_date_utc(arglist[0]._sint, arglist[1]._uint, dateptr);
            if (dateptr) {
                arglist[ix++]._sint = date.year;
                arglist[ix++]._sint = date.month;
                arglist[ix++]._sint = date.day;
                arglist[ix++]._sint = date.weekday;
                arglist[ix++]._sint = date.hour;
                arglist[ix++]._sint = date.minute;
                arglist[ix++]._sint = date.second;
                arglist[ix++]._sint = date.microsec;
            }
            }
            break;
        case 0x016B: /* simple_time_to_date_local */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 2;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
            }
            glk_simple_time_to_date_local(arglist[0]._sint, arglist[1]._uint, dateptr);
            if (dateptr) {
                arglist[ix++]._sint = date.year;
                arglist[ix++]._sint = date.month;
                arglist[ix++]._sint = date.day;
                arglist[ix++]._sint = date.weekday;
                arglist[ix++]._sint = date.hour;
                arglist[ix++]._sint = date.minute;
                arglist[ix++]._sint = date.second;
                arglist[ix++]._sint = date.microsec;
            }
            }
            break;
        case 0x016C: /* date_to_time_utc */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            glktimeval_t timeval;
            glktimeval_t *timeptr = nullptr;
			timeval.high_sec = timeval.low_sec = timeval.microsec = 0;
			
			int ix = 0;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
                date.year = arglist[ix++]._sint;
                date.month = arglist[ix++]._sint;
                date.day = arglist[ix++]._sint;
                date.weekday = arglist[ix++]._sint;
                date.hour = arglist[ix++]._sint;
                date.minute = arglist[ix++]._sint;
                date.second = arglist[ix++]._sint;
                date.microsec = arglist[ix++]._sint;
            }
            if (arglist[ix++]._ptrflag) {
                timeptr = &timeval;
            }
            glk_date_to_time_utc(dateptr, timeptr);
            if (timeptr) {
                arglist[ix++]._sint = timeval.high_sec;
                arglist[ix++]._uint = timeval.low_sec;
                arglist[ix++]._sint = timeval.microsec;
            }
            }
            break;
        case 0x016D: /* date_to_time_local */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            glktimeval_t timeval;
            glktimeval_t *timeptr = nullptr;
			timeval.high_sec = timeval.low_sec = timeval.microsec = 0;

            int ix = 0;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
                date.year = arglist[ix++]._sint;
                date.month = arglist[ix++]._sint;
                date.day = arglist[ix++]._sint;
                date.weekday = arglist[ix++]._sint;
                date.hour = arglist[ix++]._sint;
                date.minute = arglist[ix++]._sint;
                date.second = arglist[ix++]._sint;
                date.microsec = arglist[ix++]._sint;
            }
            if (arglist[ix++]._ptrflag) {
                timeptr = &timeval;
            }
            glk_date_to_time_local(dateptr, timeptr);
            if (timeptr) {
                arglist[ix++]._sint = timeval.high_sec;
                arglist[ix++]._uint = timeval.low_sec;
                arglist[ix++]._sint = timeval.microsec;
            }
            }
            break;
        case 0x016E: /* date_to_simple_time_utc */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 0;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
                date.year = arglist[ix++]._sint;
                date.month = arglist[ix++]._sint;
                date.day = arglist[ix++]._sint;
                date.weekday = arglist[ix++]._sint;
                date.hour = arglist[ix++]._sint;
                date.minute = arglist[ix++]._sint;
                date.second = arglist[ix++]._sint;
                date.microsec = arglist[ix++]._sint;
            }
            arglist[ix+2]._sint = glk_date_to_simple_time_utc(dateptr, arglist[ix]._uint);
            }
            break;
        case 0x016F: /* date_to_simple_time_local */ {
            glkdate_t date;
            glkdate_t *dateptr = nullptr;
            int ix = 0;
            if (arglist[ix++]._ptrflag) {
                dateptr = &date;
                date.year = arglist[ix++]._sint;
                date.month = arglist[ix++]._sint;
                date.day = arglist[ix++]._sint;
                date.weekday = arglist[ix++]._sint;
                date.hour = arglist[ix++]._sint;
                date.minute = arglist[ix++]._sint;
                date.second = arglist[ix++]._sint;
                date.microsec = arglist[ix++]._sint;
            }
            arglist[ix+2]._sint = glk_date_to_simple_time_local(dateptr, arglist[ix]._uint);
            }
            break;
#endif /* GLK_MODULE_DATETIME */

#ifdef GLK_MODULE_GARGLKTEXT
        case 0x1100: /* garglk_set_zcolors */
            garglk_set_zcolors( arglist[0]._uint, arglist[1]._uint );
            break;
        case 0x1101: /* garglk_set_zcolors_stream */
            garglk_set_zcolors_stream((strid_t)arglist[0]._opaqueref, arglist[1]._uint, arglist[2]._uint );
            break;
        case 0x1102: /* garglk_set_reversevideo */
            garglk_set_reversevideo( arglist[0]._uint );
            break;
        case 0x1103: /* garglk_set_reversevideo_stream */
            garglk_set_reversevideo_stream((strid_t)arglist[0]._opaqueref, arglist[1]._uint );
            break;
#endif /* GLK_MODULE_GARGLKTEXT */

        default:
            /* do nothing */
            break;
    }
}

gidispatch_rock_t GlkAPI::gidispatch_get_objrock(void *obj, uint objclass) {
	switch (objclass) {
	case gidisp_Class_Window:
		return ((Window *)obj)->_dispRock;
	case gidisp_Class_Stream:
		return ((Stream *)obj)->_dispRock;
	case gidisp_Class_Fileref:
		return ((FileReference *)obj)->_dispRock;
	case gidisp_Class_Schannel:
		return ((SoundChannel *)obj)->_dispRock;
	default: {
		gidispatch_rock_t dummy;
		dummy.num = 0;
		return dummy;
	}
	}
}

} // End of namespace Glk
