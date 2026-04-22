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

#ifndef MADS_CORE_KEYS_H
#define MADS_CORE_KEYS_H

#include "common/keyboard.h"
#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

/* keystroke defines */
#if 0
#define key_status_1            (byte *) 0x00400017 /* key status 1 */
#define key_status_2            (byte *) 0x00400018 /* key status 2 */

#define KS1_INSERT              0x80    /* Insert state      */
#define KS1_CAPSLOCK            0x40    /* Caps Lock state   */
#define KS1_NUMLOCK             0x20    /* Num Lock state    */
#define KS1_SCROLLLOCK          0x10    /* Scroll Lock state */
#define KS1_ALT                 0x08    /* Alt key state     */
#define KS1_CTRL                0x04    /* Ctrl key state    */
#define KS1_LEFTSHIFT           0x02    /* Left shift state  */
#define KS1_RIGHTSHIFT          0x01    /* Right shift state */
#endif

#define left_key                Common::KEYCODE_LEFT
#define right_key               Common::KEYCODE_RIGHT
#define up_key                  Common::KEYCODE_UP
#define down_key                Common::KEYCODE_DOWN

#define ins_key                 Common::KEYCODE_INSERT
#define del_key                 Common::KEYCODE_DELETE
#define delete_key              Common::KEYCODE_DELETE

#define home_key                Common::KEYCODE_HOME
#define end_key                 Common::KEYCODE_END

#define pgup_key                Common::KEYCODE_PAGEUP
#define pgdn_key                Common::KEYCODE_PAGEDOWN

#define enter_key               Common::KEYCODE_RETURN
#define esc_key                 Common::KEYCODE_ESCAPE
#define bksp_key                Common::KEYCODE_BACKSPACE
#define ctrl_del_key            ((Common::KBD_CTRL << 16) | Common::KEYCODE_DELETE)
#define tab_key                 Common::KEYCODE_TAB
#define backtab_key             ((Common::KBD_SHIFT << 16) | Common::KEYCODE_TAB)

#define space_key               Common::KEYCODE_SPACE

#define A_key                   'A'
#define B_key                   'B'
#define C_key                   'C'
#define D_key                   'D'
#define E_key                   'E'
#define F_key                   'F'
#define G_key                   'G'
#define H_key                   'H'
#define I_key                   'I'
#define J_key                   'J'
#define K_key                   'K'
#define L_key                   'L'
#define M_key                   'M'
#define N_key                   'N'
#define O_key                   'O'
#define P_key                   'P'
#define Q_key                   'Q'
#define R_key                   'R'
#define S_key                   'S'
#define T_key                   'T'
#define U_key                   'U'
#define V_key                   'V'
#define W_key                   'W'
#define X_key                   'X'
#define Y_key                   'Y'
#define Z_key                   'Z'

#define a_key                   'a'
#define b_key                   'b'
#define g_key                   'g'
#define i_key                   'i'
#define j_key                   'j'
#define m_key                   'm'
#define z_key                   'z'

#define alt_a_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_a)
#define alt_b_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_b)
#define alt_c_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_c)
#define alt_d_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_d)
#define alt_e_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_e)
#define alt_f_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_f)
#define alt_g_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_g)
#define alt_h_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_h)
#define alt_i_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_i)
#define alt_j_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_j)
#define alt_k_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_k)
#define alt_l_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_l)
#define alt_m_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_m)
#define alt_n_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_n)
#define alt_o_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_o)
#define alt_p_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_p)
#define alt_q_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_q)
#define alt_r_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_r)
#define alt_s_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_s)
#define alt_t_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_t)
#define alt_u_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_u)
#define alt_v_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_v)
#define alt_w_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_w)
#define alt_x_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_x)
#define alt_y_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_y)
#define alt_z_key               ((Common::KBD_ALT << 16) | Common::KEYCODE_z)

#define f1_key                  Common::KEYCODE_F1
#define f2_key                  Common::KEYCODE_F2
#define f3_key                  Common::KEYCODE_F3
#define f4_key                  Common::KEYCODE_F4
#define f5_key                  Common::KEYCODE_F5
#define f6_key                  Common::KEYCODE_F6
#define f7_key                  Common::KEYCODE_F7
#define f8_key                  Common::KEYCODE_F8
#define f9_key                  Common::KEYCODE_F9
#define f10_key                 Common::KEYCODE_F10

#define shift_f1_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F1)
#define shift_f2_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F2)
#define shift_f3_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F3)
#define shift_f4_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F4)
#define shift_f5_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F5)
#define shift_f6_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F6)
#define shift_f7_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F7)
#define shift_f8_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F8)
#define shift_f9_key            ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F9)
#define shift_f10_key           ((Common::KBD_SHIFT << 16) | Common::KEYCODE_F10)

#define ctrl_f1_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F1)
#define ctrl_f2_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F2)
#define ctrl_f3_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F3)
#define ctrl_f4_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F4)
#define ctrl_f5_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F5)
#define ctrl_f6_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F6)
#define ctrl_f7_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F7)
#define ctrl_f8_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F8)
#define ctrl_f9_key             ((Common::KBD_CTRL << 16) | Common::KEYCODE_F9)
#define ctrl_f10_key            ((Common::KBD_CTRL << 16) | Common::KEYCODE_F10)

#define alt_f1_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F1)
#define alt_f2_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F2)
#define alt_f3_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F3)
#define alt_f4_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F4)
#define alt_f5_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F5)
#define alt_f6_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F6)
#define alt_f7_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F7)
#define alt_f8_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F8)
#define alt_f9_key              ((Common::KBD_ALT << 16) | Common::KEYCODE_F9)
#define alt_f10_key             ((Common::KBD_ALT << 16) | Common::KEYCODE_F10)

#define ctrl_a_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_a)
#define ctrl_b_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_b)
#define ctrl_c_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_c)
#define ctrl_d_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_d)
#define ctrl_e_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_e)
#define ctrl_f_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_f)
#define ctrl_g_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_g)
#define ctrl_h_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_h)
#define ctrl_i_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_i)
#define ctrl_j_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_j)
#define ctrl_k_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_k)
#define ctrl_l_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_l)
#define ctrl_m_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_m)
#define ctrl_n_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_n)
#define ctrl_o_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_o)
#define ctrl_p_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_p)
#define ctrl_q_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_q)
#define ctrl_r_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_r)
#define ctrl_s_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_s)
#define ctrl_t_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_t)
#define ctrl_u_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_u)
#define ctrl_v_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_v)
#define ctrl_w_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_w)
#define ctrl_x_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_x)
#define ctrl_y_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_y)
#define ctrl_z_key              ((Common::KBD_CTRL << 16) | Common::KEYCODE_z)

#define KEYS_MAX_BUF_CHARS        32

struct KeyBuffer {
	int buf[KEYS_MAX_BUF_CHARS];
	int len;
};

typedef struct KeyBuffer *KeyPtr;

extern word keys_special_button;


extern int keys_any();
extern int keys_get();

/**
 * Flushes the typeahead buffer
 */
extern void keys_flush();

/**
 * Flushes the contents of the specified key buffer
 */
extern void keys_flush_buffer(KeyPtr key_buf);

/**
 * Adds any waiting typeahead input keys to the key buffer
 */
extern void keys_fill_buffer(KeyPtr key_buf);

/**
 * Reads next character from key buffer (return 0 if no key available)
 */
extern int keys_read_buffer(KeyPtr key_buf);

/**
 * Appends the specified character to the key buffer list.  Returns
 * 0 if key list overflows; returns new length of list otherwise.
 */
extern int keys_append_buffer(KeyPtr key_buf, int newchar);

/**
 * Inserts a character into the key buffer list *before* the specified
 * element (first element is 0).  Returns 0 if key list overflows (or
 * if "before" is out of range).  Otherwise, returns "before + 1" for
 * use in adding lists of characters to middle of queue.
 */
extern int keys_insert_buffer(KeyPtr key_buf, int newchar, int before);

/**
 * Wipes the old key buffer and inserts the specified string in its place.
 */
extern void keys_stuff_buffer(KeyPtr key_buf, int *newstuff);

/**
 * If "target" is an ALT-key combination, returns the ascii value
 * of the associated (upper case) letter.
 * If "target" is an ascii letter, returns the UPPER CASE version
 * of that letter.
 *
 * (Used by routines that trap ALT key and need to accept alt-key
 * and plain key as same key value)
 */
extern int keys_fix_alt(int target);

extern void keys_install();
extern void keys_remove();
extern int keys_check_install();
extern void keys_enable();
extern void keys_disable();

} // namespace MADSV2
} // namespace MADS

#endif
