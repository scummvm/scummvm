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

#include "mads/madsv2/core/keys.h"
#include "mads/madsv2/core/general.h"
#include "mads/madsv2/core/mem.h"
#include "mads/madsv2/engine.h"

namespace MADS {
namespace MADSV2 {

word keys_special_button;

void keys_install() {
	// No implementation in ScummVM
}

void keys_remove() {
	// No implementation in ScummVM
}

int keys_check_install() {
	error("TODO: keys_check_install");
}

void keys_enable() {
	error("TODO: keys_enable");
}

void keys_disable() {
	error("TODO: keys_disable");
}

int keys_any() {
	return g_engine->hasPendingKey();
}

int keys_get() {
	return g_engine->getKey();
}

void keys_flush(void) {
	g_engine->flushKeys();
}

void keys_flush_buffer(KeyPtr key_buf) {
	key_buf->len = 0;
}

void keys_fill_buffer(KeyPtr key_buf) {
	while (keys_any()) {
		keys_append_buffer(key_buf, keys_get());
	}
}

int keys_read_buffer(KeyPtr key_buf) {
	int myval;

	if (key_buf->len > 0) {
		myval = key_buf->buf[0];
		key_buf->len--;
		memcpy(key_buf->buf, key_buf->buf + 1, (key_buf->len * sizeof(int)));

		return myval;
	} else {
		return 0;
	}
}

int keys_append_buffer(KeyPtr key_buf, int newchar) {
	if (key_buf->len < KEYS_MAX_BUF_CHARS) {
		key_buf->buf[key_buf->len++] = newchar;
		return key_buf->len;
	} else {
		return false;
	}
}

int keys_insert_buffer(KeyPtr key_buf, int newchar, int before) {
	if ((key_buf->len < KEYS_MAX_BUF_CHARS) && (before < key_buf->len) && (before > 0)) {
		memcpy(key_buf->buf + before + 1, key_buf->buf + before, (key_buf->len - before) * sizeof(int));
		key_buf->buf[before] = newchar;
		return (before + 1);
	} else {
		return (false);
	}
}

void keys_stuff_buffer(KeyPtr key_buf, int *newstuff) {
	key_buf->len = 0;

	while ((*newstuff != 0) && (key_buf->len < KEYS_MAX_BUF_CHARS)) {
		key_buf->buf[key_buf->len++] = *(newstuff++);
	}
}

int keys_fix_alt(int target) {
	switch (target) {
	case alt_a_key: return A_key;
	case alt_b_key: return B_key;
	case alt_c_key: return C_key;
	case alt_d_key: return D_key;
	case alt_e_key: return E_key;
	case alt_f_key: return F_key;
	case alt_g_key: return G_key;
	case alt_h_key: return H_key;
	case alt_i_key: return I_key;
	case alt_j_key: return J_key;
	case alt_k_key: return K_key;
	case alt_l_key: return L_key;
	case alt_m_key: return M_key;
	case alt_n_key: return N_key;
	case alt_o_key: return O_key;
	case alt_p_key: return P_key;
	case alt_q_key: return Q_key;
	case alt_r_key: return R_key;
	case alt_s_key: return S_key;
	case alt_t_key: return T_key;
	case alt_u_key: return U_key;
	case alt_v_key: return V_key;
	case alt_w_key: return W_key;
	case alt_x_key: return X_key;
	case alt_y_key: return Y_key;
	case alt_z_key: return Z_key;
	default:
		if ((target >= a_key) && (target <= z_key)) {
			return (target + A_key - a_key);
		} else {
			return (target);
		}
	}
}

} // namespace MADSV2
} // namespace MADS
