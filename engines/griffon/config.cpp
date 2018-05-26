/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>

#include <SDL/SDL.h>

#include "config.h"
 
 CONFIG config = {
	320, 240, 16, 0, 0, 0, 1, 127, 1, 127
};

char config_ini[64] = "config.ini";

#define PRINT(A,B) 			\
do {					\
	char line[256];			\
	sprintf(line, A "\n", B);	\
	fputs(line, fp);		\
} while(0)

#define INPUT(A, B)			\
do {					\
	char line[256];			\
	fgets(line, sizeof(line), fp);	\
	sscanf(line, A, B);		\
} while(0)

void config_load(CONFIG *config)
{
	char line[128];
	char arg[128];
	FILE *fp;

	fp = fopen(config_ini, "r");
	if(fp) {
		while(fgets(line, sizeof(line), fp) != NULL) {
			sscanf(line, "%s", arg); // eliminate eol and eof by this

			if(strcmp(arg, "SCR_WIDTH:") == 0) {
				fgets(line, sizeof(line), fp);
				sscanf(line, "%i", &config->scr_width);
			} else if(strcmp(arg, "SCR_HEIGHT:") == 0) {
				fgets(line, sizeof(line), fp);
				sscanf(line, "%i", &config->scr_height);
			} else if(strcmp(arg, "SCR_BITS:") == 0) {
				fgets(line, sizeof(line), fp);
				sscanf(line, "%i", &config->scr_bpp);
			} else if(strcmp(arg, "HWACCEL:YES") == 0) {
				config->hwaccel = SDL_HWACCEL;
			} else if(strcmp(arg, "HWACCEL:NO") == 0) {
				config->hwaccel = 0;
			} else if(strcmp(arg, "HWSURFACE:YES") == 0) {
				config->hwsurface = SDL_HWSURFACE;
			} else if(strcmp(arg, "HWSURFACE:NO") == 0) {
				config->hwsurface = 0;
			} else if(strcmp(arg, "FULLSCREEN:YES") == 0) {
				config->fullscreen = SDL_FULLSCREEN;
			} else if(strcmp(arg, "FULLSCREEN:NO") == 0) {
				config->fullscreen = 0;
			} else if(strcmp(arg, "MUSIC:YES") == 0) {
				config->music = 1;
			} else if(strcmp(arg, "MUSIC:NO") == 0) {
				config->music = 0;
			} else if(strcmp(arg, "SNDEFFECTS:YES") == 0) {
				config->effects = 1;
			} else if(strcmp(arg, "SNDEFFECTS:NO") == 0) {
				config->effects = 0;
			} else if(strcmp(arg, "opmusicvol:") == 0) {
				fgets(line, sizeof(line), fp);
				sscanf(line, "%i", &config->musicvol);
			} else if(strcmp(arg, "opeffectsvol:") == 0) {
				fgets(line, sizeof(line), fp);
				sscanf(line, "%i", &config->effectsvol);
			}
		}

		fclose(fp);
	}
}

void config_save(CONFIG *config)
{
	FILE *fp = fopen(config_ini, "w");

	if(fp) {
		PRINT("%s", "SCR_WIDTH:");
		PRINT("%i", config->scr_width);
		PRINT("%s", "SCR_HEIGHT:");
		PRINT("%i", config->scr_height);
		PRINT("%s", "SCR_BITS:");
		PRINT("%i", config->scr_bpp);
		PRINT("%s", config->hwaccel ? "HWACCEL:YES" : "HWACCEL:NO");
		PRINT("%s", config->hwsurface ? "HWSURFACE:YES" : "HWSURFACE:NO");
		PRINT("%s", config->fullscreen ? "FULLSCREEN:YES" : "FULLSCREEN:NO");
		PRINT("%s", config->music ? "MUSIC:YES" : "MUSIC:NO");
		PRINT("%s", config->effects ? "SNDEFFECTS:YES" : "SNDEFFECTS:NO");
		PRINT("%s", "opmusicvol:");
		PRINT("%i", config->musicvol);
		PRINT("%s", "opeffectsvol:");
		PRINT("%i", config->effectsvol);
		fclose(fp);
	}
}