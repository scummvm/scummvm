/*
 * Copyright 2000, 2001, 2002
 *         Dan Potter. All rights reserved.
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of Cryptic Allusion nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/* Modified by Walter van Niftrik <w.f.b.w.v.niftrik@stud.tue.nl> */

#ifndef __GP_H
#define __GP_H

#include <kos.h>
#include <math.h>

/* Floating-point Sin/Cos; 256 angles, -1.0 to 1.0 */
#define msin(angle) fsin((angle) * 2 * M_PI / 256)
#define mcos(angle) fcos((angle) * 2 * M_PI / 256)

/* bkg.c */
void bkg_setup();
void bkg_render();

/* texture.c */
extern pvr_ptr_t		util_texture;
extern pvr_poly_hdr_t		util_txr_hdr;
void setup_util_texture();

/* 3dutils.c */
void rotate(int zang, int xang, int yang, float *x, float *y, float *z);
void draw_poly_mouse(int ptrx, int ptry, float alpha);
void draw_poly_char(float x1, float y1, float z1, float a, float r, float g, float b, int c);
void draw_poly_strf(float x1, float y1, float z1, float a, float r, float g, float b, char *fmt, ...);
void draw_poly_strf_ctr(float y1, float z1, float a, float r, float g, float b, char *fmt, ...);
void draw_poly_box(float x1, float y1, float x2, float y2, float z,
		float a1, float r1, float g1, float b1,
		float a2, float r2, float g2, float b2);

/* gamemenu.c */
/* Renders the interface and handles controller input.
** Parameters: void.
** Returns   : 1 when the user has chosen a game, 0 otherwise.
*/
int game_menu_render();

/* Renders the currently applicable button info.
** Parameters: void.
** Returns   : void.
*/
void render_button_info();

/* Fills the option list with the defaults, and then overwrites
** them (possible partially) when an option file is found.
** Parameters: void.
** Returns   : void.
*/
void load_option_list();

/* Saves the currently set options to a file on the first VMU.
** Parameters: void.
** Returns   : 0 on success, -1 on error.
*/
int dc_write_config_file();

/* Initializes the menu state based on the current cdrom drive status.
** Parameters: void.
** Returns   : void.
*/
void init_menu_state();

#endif	/* __GP_H */
