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

#include <string.h>
#include <resource.h>
#include <sci_memory.h>
#include "gp.h"
#include "options.h"
#include "dc.h"

#define info_y 120.0f

/* Game entries */
typedef struct {
	char	fn[256];	/* Game name */
	char	dir[256];	/* Full directory path */
} game;

char *freq_select[] = {"50Hz", "60Hz", "Test"};

/* Used to count the number of queued scenes. This is done to make sure that the
** image on the screen is current before calling a function which will block
** further scene rendering for a significant amount of time.
*/
static int load_queued = 0;
/* Holds pointers to text strings describing the selected value of each option */
static char* options_str[NUM_DC_OPTIONS];
/* The index of the currently selected value of each option */
static char options_nr[NUM_DC_OPTIONS];
/* Array of games that were found */
static game games[200];
/* Number of games that were found */
static int num_games = 0;
/* games array index of the currently selected game */
static int sel_game = 0;
/* games array index of the game which is currently displayed first on the
** screen.
*/
static int top_game = 0;
/* Number of options */
static int num_options;
/* options_nr array index of the currently selected option */
static int sel_option = 0;
/* options_nr array index of the game which is currently displayed first on the
** screen.
*/
static int top_option = 0;
/* frequency selector index of the currently selected option */
static int sel_freq = 1;
/* Number of frequency selector options. Always 3. */
static int num_freq = 3;
/* freq_select array index of the option which is currently displayed first on
** the screen. Always 0.
*/
static int top_freq = 0;

/* Pointers to properties of the currently displayed data, either games list or
** options list.
*/
static int *num_entries;
static int *selected;
static int *top;

/* Counts frames. Used for delay purposes in the controller code */
static int framecnt = 0;
/* Controls the color changes of the highlighting bar */
static float throb = 0.2f, dthrob = 0.01f;

/* Current state of menu:
**  0: Drive lid open.
**  1: Searching the CD for SCI games
**  2: Displaying game list
**  3: Running selected game
**  4: Displaying option list
**  5: Saving options to VMU
**  6: Waiting for CD lid to open
**  7: Waiting for Dreamcast to finish scanning the disc
**  8: Drive empty.
**  9: 50Hz/60Hz selector for PAL console
** 10: Testing 60Hz mode for 5 seconds
**
** State changes: 0->{7}, 1->{2,6}, 2->{0,3,4}, 4->{2,5}, 5->{2}, 6->{0}
**                7->{0,1,8}, 8->{0}, 9->{0,1,10}, 10->{9}
*/
static int menu_state;

/* Flag which indicates whether the options have been altered */
static int save_flag = 0;

static void menu_restart()
{
	int status;

	cdrom_get_status(&status, NULL);

	if ((status == CD_STATUS_PAUSED) || (status == CD_STATUS_STANDBY))
		menu_state = 1;
	else
		menu_state = 0;

	num_entries = &num_games;
	top = &top_game;
	selected = &sel_game;
}

void init_menu_state()
{
	/* On PAL consoles without a VGA box, display the 50Hz/60Hz selector */
	if ((flashrom_get_region() == FLASHROM_REGION_EUROPE) &&
	  vid_check_cable()) {
		num_entries = &num_freq;
		top = &top_freq;
		selected = &sel_freq;
		menu_state = 9;
		return;
	}
	else menu_restart();
}

static int load_options(char *infname, char *options)
/* Loads the options from an option file and stores them in an array.
** Parameters: (char *) infname: Full path and filename of the option file.
**             (char *) options: Pointer to the option array.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t inf;
	uint8 *data;
	vmu_pkg_t pkg;
	int j;
	if (!(inf = fs_open(infname, O_RDONLY))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_RDONLY) failed!\n", __FILE__, __LINE__, infname);
		return -1;
	}
	if (!(data = fs_mmap(inf))) {
		sciprintf("%s, L%d: fs_mmap() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	if (vmu_pkg_parse(data, &pkg) == -1) {
		sciprintf("%s, L%d: vmu_pkg_parse() failed!\n", __FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	if ((pkg.data_len != NUM_DC_OPTIONS+2) ||
			(pkg.data[0] != DC_OPTIONS_TAG_MAJOR) ||
			(pkg.data[1] != DC_OPTIONS_TAG_MINOR)) {
		sciprintf("%s, L%d: Option file version doesn't match!\n",
			__FILE__, __LINE__);
		fs_close(inf);
		return -1;
	}
	for (j = 0; j < NUM_DC_OPTIONS; j++)
		options[j] = pkg.data[j+2];

	fs_close(inf);
	return 0;
}

static int save_options(char *outfile, char *options)
/* Saves the options from an array to an option file.
** Parameters: (char *) outfname: Full path and filename of the option file.
**             (char *) options: Pointer to the option array.
** Returns   : 0 on success, -1 on error.
*/
{
	file_t outf;
	uint8 *pkg_out;
	vmu_pkg_t pkg;
	int pkg_size;
	uint8 data[NUM_DC_OPTIONS+2];
	
	strcpy(pkg.desc_short, "FreeSCI");
	strcpy(pkg.desc_long, "Configuration");
	strcpy(pkg.app_id, "FreeSCI");
	data[0] = DC_OPTIONS_TAG_MAJOR;
	data[1] = DC_OPTIONS_TAG_MINOR;
	memcpy(data+2, options_nr, NUM_DC_OPTIONS);
	pkg.icon_cnt = 0;
	pkg.icon_anim_speed = 0;
	pkg.eyecatch_type = VMUPKG_EC_NONE;
	pkg.data = data;
	pkg.data_len = NUM_DC_OPTIONS+2;
	if (vmu_pkg_build(&pkg, &pkg_out, &pkg_size) < 0) {
		sciprintf("%s, L%d: vmu_pkg_build() failed!\n", __FILE__, __LINE__);
		return -1;
	}
	if (!(outf = fs_open(outfile, O_WRONLY | O_TRUNC))) {
		sciprintf("%s, L%d: fs_open(\"%s\", O_WRONLY | O_TRUNC) failed!\n", __FILE__, __LINE__, outfile);
		return -1;
	}
	if (fs_write(outf, pkg_out, pkg_size) < pkg_size) {
		sciprintf("%s, L%d: fs_write() failed!\n", __FILE__, __LINE__);
		fs_close(outf);
		return -1;
	}
	fs_close(outf);
	return 0;
}

void load_option_list() {
	int i;
	char *vmu;
                         
	/* Load defaults */
	for (i = 0; i < NUM_DC_OPTIONS; i++)
		options_nr[i] = 0;

	/* Overwrite with data from option file */
	if ((vmu = dc_get_first_vmu())) {
		char *fn = sci_malloc(strlen(vmu) + 9);
		strcpy(fn, vmu);
		strcat(fn, "/freesci");
		if (load_options(fn, &options_nr[0]))
			sciprintf("%s, L%d: Loading options from VMU failed!\n", __FILE__, __LINE__);
		sci_free(fn);
		sci_free(vmu);
	}
	else sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);

	/* Calculate strings */
	for (i = 0; i < NUM_DC_OPTIONS; i++) {
		int j;
		options_str[i] = dc_options[i].values;
		for (j = 0; j < options_nr[i]; j++)
			options_str[i] += strlen(options_str[i]) + 1;
	}

	num_options = NUM_DC_OPTIONS;
}

static void load_game_list(char *dir)
/* Scans a directory and it's subdirectories for SCI games and stores the name
** and directory of each game.
** Parameters: (char *) dir: Path of the directory tree to scan.
** Returns   : void.
*/
{
	file_t d;

	d = fs_open(dir, O_RDONLY | O_DIR);
	if (!d) return;
	{
		dirent_t *de;
		while ((de = fs_readdir(d)) && num_games < 200) {
			if (!stricmp(de->name, "resource.map")) {
				strncpy(games[num_games].fn, dir, 256);
				strncpy(games[num_games].dir, dir, 256);
				num_games++;
			}
			else if (de->size < 0) {
				char *new_dir;
				new_dir = malloc(strlen(dir)+strlen(de->name)+2);
				strcpy(new_dir, dir);
				strcat(new_dir, "/");
				strcat(new_dir, de->name);
				load_game_list(new_dir);
				free(new_dir);
			}
		}
	}
	fs_close(d);
}

static void draw_options()
/* Draws the options and their current values on the screen.
** Parameters: void.
** Returns   : void.
*/
{
	float y = 92.0f + 28.0f;
	int i, esel;

	draw_poly_strf_ctr(y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, "Options");

	y += 2*24.0f;

	/* Draw all the options */	
	for (i=0; i<7 && (top_option+i)<NUM_DC_OPTIONS; i++) {
		draw_poly_strf(32.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		  "%-20s %s", dc_options[top_option+i].name,
		  options_str[top_option+i]);
		y += 24.0f;
	}
	
	/* Put a highlight bar under one of them */
	esel = (sel_option - top_option);
	draw_poly_box(31.0f, 92.0f+28.0f+2*24.0f+esel*24.0f - 1.0f,
		609.0f, 92.0f+28.0f+2*24.0f+esel*24.0f + 25.0f, 95.0f,
		throb, throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f);
}

static void draw_listing()
/* Draws the game list on the screen.
** Parameters: void.
** Returns   : void.
*/
{
	float y = 92.0 + 28.0f;
	int i, esel;

	draw_poly_strf_ctr(y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, "Games");

	y += 2*24.0f;

	/* Draw all the game titles */	
	for (i=0; i<7 && (top_game+i)<num_games; i++) {
		draw_poly_strf(32.0f, y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			games[top_game+i].fn);
		y += 24.0f;
	}
	
	/* Put a highlight bar under one of them */
	esel = (sel_game - top_game);
	draw_poly_box(31.0f, 92.0f+28.0f+2*24.0f+esel*24.0f - 1.0f,
		609.0f, 92.0f+28.0f+2*24.0f+esel*24.0f + 25.0f, 95.0f,
		throb, throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f);
}

static void draw_selector()
/* Draws the 50Hz/60Hz selector on the screen.
** Parameters: void.
** Returns   : void.
*/
{
	float y = 92.0 + 28.0f;
	int i;

	draw_poly_strf_ctr(y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f, "Display Mode");

	y += 48.0f;

	/* Draw all frequency options */	
	for (i=0; i<num_freq; i++) {
		draw_poly_strf_ctr(y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			freq_select[i]);
		y += 24.0f;
	}
	
	/* Put a highlight bar under one of them */
	draw_poly_box(31.0f, 92.0f+28.0f+48.0f+sel_freq*24.0f - 1.0f,
		609.0f, 92.0f+28.0f+48.0f+sel_freq*24.0f + 25.0f, 95.0f,
		throb, throb, 0.2f, 0.2f, throb, throb, 0.2f, 0.2f);
}

static void check_controller()
/* Checks controller input and changes menu state accordingly.
** Parameters: void.
** Returns   : void.
*/
{
	static int up_moved = 0, down_moved = 0, a_pressed = 0, b_pressed = 0;
	static uint8 mcont = 0;
	cont_cond_t cond;

	if (!mcont) {
		mcont = maple_first_controller();
		if (!mcont) { return; }
	}
	if (cont_get_cond(mcont, &cond)) { return; }

	if (!(cond.buttons & CONT_DPAD_UP)) {
		if ((framecnt - up_moved) > 10) {
			if (*selected > 0) {
				(*selected)--;
				if (*selected < *top) {
					*top = *selected;
				}
			}
			up_moved = framecnt;
		}
	}
	if (!(cond.buttons & CONT_DPAD_DOWN)) {
		if ((framecnt - down_moved) > 10) {
			if (*selected < (*num_entries - 1)) {
				(*selected)++;
				if (*selected >= (*top+7)) {
					(*top)++;
				}
			}
			down_moved = framecnt;
		}
	}
	if (cond.ltrig > 0) {
		if ((framecnt - up_moved) > 10) {
			*selected -= 7;

			if (*selected < 0) *selected = 0;
			if (*selected < *top) *top = *selected;
			up_moved = framecnt;
		}
	}
	if (cond.rtrig > 0) {
		if ((framecnt - down_moved) > 10) {
			*selected += 7;
			if (*selected > (*num_entries - 1))
				*selected = *num_entries - 1;
			if (*selected >= (*top+7))
				*top = *selected;
			down_moved = framecnt;
		}
	}

	if (!(cond.buttons & CONT_A)) {
		if ((framecnt - a_pressed) > 5)
		{
			if (menu_state == 2)
			{
				fs_chdir(games[*selected].dir);
				menu_state = 3;
			}
			else if (menu_state == 4)
			{
				options_str[sel_option] += strlen(options_str[sel_option]) + 1;
				if (*options_str[sel_option] == '\0') {
					options_str[sel_option] = dc_options[sel_option].values;
					options_nr[sel_option] = 0;
				}
				else options_nr[sel_option]++;
				save_flag = 1;
			}
			else if (menu_state == 9)
			{
				if (sel_freq == 0) menu_restart();
				else if (sel_freq == 1) {
					vid_set_mode(DM_640x480, PM_RGB565);
					menu_restart();
				}
				else if (sel_freq == 2) {
					menu_state = 10;
				}
			}
		}
		a_pressed = framecnt;
	}

	if (!(cond.buttons & CONT_B)) {
		if ((framecnt - b_pressed) > 5) {
			if (menu_state == 4) {
				num_entries = &num_games;
				top = &top_game;
				selected = &sel_game;
				if (save_flag)
					menu_state = 5;
				else menu_state = 2;
			}
			else if (menu_state != 9) {
				num_entries = &num_options;
				top = &top_option;
				selected = &sel_option;
				menu_state = 4;
			}
		}
		b_pressed = framecnt;
	}
	return;
}

void render_button_info() {
	draw_poly_box(20.0f, 440.0f-96.0f+4, 640.0f-20.0f, 440.0f, 90.0,
		0.3f, 0.2f, 0.5f, 0.0f, 0.5f, 0.1f, 0.8f, 0.2f);
	if ((menu_state != 4) && (menu_state != 5)) {       
		draw_poly_strf(30.0f,440.0f-96.0f+6+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"D-PAD : Select game              L : Page up");
		draw_poly_strf(30.0f,440.0f-96.0f+6+24.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"    A : Start game               R : Page down");
		draw_poly_strf(30.0f,440.0f-96.0f+6+48.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"                                 B : Options");
	}
	else {
		draw_poly_strf(30.0f,440.0f-96.0f+6+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"D-PAD : Select option            L : Page up");
		draw_poly_strf(30.0f,440.0f-96.0f+6+24.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"    A : Change option            R : Page down");
		draw_poly_strf(30.0f,440.0f-96.0f+6+48.0f+10.0f,100.0f,1.0f,1.0f,1.0f,1.0f,"                                 B : Back");
	}
}

void render_scroll(float speed, float y, float z, float a, float r, float g, float b, char *s)
/* Renders a scrolling text.
** Parameters: void.
** Returns   : void.
*/
{
	static float coord = 640.0f;
	draw_poly_strf(coord, y, z, a , r, g ,b, s);
	coord -= speed;
	if (coord < strlen(s)*-12.0f)
		coord = 640.0f;
}

int game_menu_render() {
	/* Draw a background box */
	draw_poly_box(30.0f, 80.0f+28.0f, 610.0f, 440.0f-96.0f, 90.0f, 
		0.2f, 0.8f, 0.5f, 0.0f, 0.2f, 0.8f, 0.8f, 0.2f);
		
	if (menu_state == 0) {
		int status;
		draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"The drive lid is open.");
		draw_poly_strf(32.0f, info_y+24.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Please insert a game cd.");
		cdrom_get_status(&status, NULL);
		if (status == CD_STATUS_BUSY) menu_state = 7;
		return 0;
	}

	else if (menu_state == 1) {
		if (load_queued < 4) {
			draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"Searching cd for SCI games...");
			load_queued++;
			return 0;
		} else {
			load_game_list("/cd");
			load_queued = 0;
			if (num_games == 0) menu_state = 6;
			else menu_state = 2;
			return 0;
		}
	}
	
	else if (menu_state == 3) {
		draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Starting game, please wait...");
		if (load_queued < 4) {
			load_queued++;
			return 0;
		} else {
			return 1;
		}
	}

	else if (menu_state == 4) {
		/* Draw option menu */
		draw_options();
	}

	else if (menu_state == 2) {
		int status;
		
		cdrom_get_status(&status, NULL);
		if (status == CD_STATUS_OPEN) {
			*num_entries = 0;
			*selected = 0;
			menu_state = 0;
			return 0;
		}

		/* Draw the game listing */
		draw_listing();
	}
	
	else if (menu_state == 5) {
		if (load_queued < 4) {
			draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"Saving options, please wait...");
			load_queued++;
		}
		else {
			char *vmu;
			if ((vmu = dc_get_first_vmu())) {
				char *fn = sci_malloc(strlen(vmu) + 9);
				strcpy(fn, vmu);
				strcat(fn, "/freesci");
				if (save_options(fn, &options_nr[0]))
					sciprintf("%s, L%d: Saving options to VMU failed!\n", __FILE__, __LINE__);
				sci_free(fn);
				sci_free(vmu);
			}
			else sciprintf("%s, L%d: No VMU found!\n", __FILE__, __LINE__);
			save_flag = 0;
			load_queued = 0;
			menu_state = 2;
		}
		return 0;
	}
	
	else if (menu_state == 6) {
		int status;
		draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"No SCI games found!");
		draw_poly_strf(32.0f, info_y + 24.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Please insert a game cd.");
		cdrom_get_status(&status, NULL);
		if (status == CD_STATUS_OPEN) menu_state = 0;
		return 0;
	}

	else if (menu_state == 7) {
		int status;
		draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Scanning cd...");
		cdrom_get_status(&status, NULL);
		if (status == CD_STATUS_PAUSED) menu_state = 1;
		else if (status == CD_STATUS_NO_DISC) menu_state = 8;
		else if (status == CD_STATUS_OPEN) menu_state = 0;
		return 0;
	}

	else if (menu_state == 8) {
		int status;
		draw_poly_strf(32.0f, info_y, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"The drive is empty!");
		draw_poly_strf(32.0f, info_y + 24.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
			"Please insert a game cd.");
		cdrom_get_status(&status, NULL);
		if (status == CD_STATUS_OPEN) menu_state = 0;
		return 0;
	}
	else if (menu_state == 9) {
		/* Draw frequency selector */
		draw_selector();
	}
	else if (menu_state == 10) {
		static int cnt = 60*5;
		if (cnt == 60*5) vid_set_mode(DM_640x480, PM_RGB565);
		if (--cnt) {
			draw_poly_strf_ctr(info_y+3*24.0, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"Testing 60Hz Display Mode");
			draw_poly_strf_ctr(info_y+5*24.0f, 100.0f, 1.0f, 1.0f, 1.0f, 1.0f,
				"%i", cnt/60+1);
		}
		else {
			vid_set_mode(DM_640x480_PAL_IL, PM_RGB565);
			cnt = 60*5;
			menu_state = 9;
		}
	}

	/* Adjust the throbber */
	throb += dthrob;
	if (throb < 0.2f || throb > 0.8f) {
		dthrob = -dthrob;
		throb += dthrob;
	}
	
	/* Check controller input */
	check_controller();

	framecnt++;

	return 0;
}

int dc_write_config_file(char *fn) {
	FILE *cfile;
	if ((cfile = fopen(fn, "w"))) {
		fprintf(cfile, "[game]\n");
		fprintf(cfile, "resource_dir = %s\n", games[*selected].dir);
		fprintf(cfile, "gfx.dc.render_mode = %s\n", options_nr[0]? "pvr" : "vram");
		fprintf(cfile, "gfx.dc.refresh_rate = %s\n", sel_freq? "60Hz" : "50Hz");
		fprintf(cfile, "pic0_dither_mode = ");		
		switch (options_nr[1]) {
		case 0:
			fprintf(cfile, "dither256\n");
			break;
		case 1:
			fprintf(cfile, "flat\n");
			break;
		case 2:
			fprintf(cfile, "dither\n");
		}
					
		if (options_nr[2])
			fprintf(cfile, "pic_antialiasing = simple");

		if (options_nr[3])
			fprintf(cfile, "version = %s\n", options_str[3]);

		if (options_nr[4])
			fprintf(cfile, "resource_version = %s\n", options_str[4]);

		if (options_nr[5])
			fprintf(cfile, "pic_port_bounds = \"0, 0, 320, 200\"\n");

		fclose(cfile);
		return 0;
	}
	sciprintf("%s, L%d: fopen(\"%s\", \"w\") failed!\n", __FILE__, __LINE__, fn);
	return -1;
}
