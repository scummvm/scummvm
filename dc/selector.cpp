/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002  Marcus Comstedt
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "scumm.h"
#include "dc.h"
#include "icon.h"
#include "label.h"

#include <ronin/gddrive.h>


#define MAX_GAMES 100
#define MAX_DIR 100


void draw_solid_quad(float x1, float y1, float x2, float y2,
		     int c0, int c1, int c2, int c3)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_OPAQUE|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
    TA_CMD_POLYGON_GOURAUD_SHADING;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC|TA_POLYMODE2_FOG_DISABLED;
  mypoly.texture = 0;

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.colour = c0;
  myvertex.x = x1;
  myvertex.y = y1;
  ta_commit_list(&myvertex);

  myvertex.colour = c1;
  myvertex.x = x2;
  ta_commit_list(&myvertex);

  myvertex.colour = c2;
  myvertex.x = x1;
  myvertex.y = y2;
  ta_commit_list(&myvertex);

  myvertex.colour = c3;
  myvertex.x = x2;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);  
}

void draw_trans_quad(float x1, float y1, float x2, float y2,
		     int c0, int c1, int c2, int c3)
{
  struct polygon_list mypoly;
  struct packed_colour_vertex_list myvertex;

  mypoly.cmd =
    TA_CMD_POLYGON|TA_CMD_POLYGON_TYPE_TRANSPARENT|TA_CMD_POLYGON_SUBLIST|
    TA_CMD_POLYGON_STRIPLENGTH_2|TA_CMD_POLYGON_PACKED_COLOUR|
    TA_CMD_POLYGON_GOURAUD_SHADING;
  mypoly.mode1 = TA_POLYMODE1_Z_ALWAYS|TA_POLYMODE1_NO_Z_UPDATE;
  mypoly.mode2 =
    TA_POLYMODE2_BLEND_SRC_ALPHA|TA_POLYMODE2_BLEND_DST_INVALPHA|
    TA_POLYMODE2_FOG_DISABLED|TA_POLYMODE2_ENABLE_ALPHA;
  mypoly.texture = 0;

  mypoly.red = mypoly.green = mypoly.blue = mypoly.alpha = 0;

  ta_commit_list(&mypoly);

  myvertex.cmd = TA_CMD_VERTEX;
  myvertex.ocolour = 0;
  myvertex.z = 0.5;
  myvertex.u = 0.0;
  myvertex.v = 0.0;

  myvertex.colour = c0;
  myvertex.x = x1;
  myvertex.y = y1;
  ta_commit_list(&myvertex);

  myvertex.colour = c1;
  myvertex.x = x2;
  ta_commit_list(&myvertex);

  myvertex.colour = c2;
  myvertex.x = x1;
  myvertex.y = y2;
  ta_commit_list(&myvertex);

  myvertex.colour = c3;
  myvertex.x = x2;
  myvertex.cmd |= TA_CMD_VERTEX_EOS;
  ta_commit_list(&myvertex);  
}


struct Game
{
  char dir[256];
  char filename_base[256];
  char text[256];
  Icon icon;
  Label label;
};

struct Dir
{
  char name[256];
  char deficon[256];
};

static Game the_game;

static bool isGame(const char *fn, char *base)
{
  int l = strlen(fn);
  if(l>4 && (!strcasecmp(fn+l-4, ".000") ||
	     !strcasecmp(fn+l-4, ".SM0"))) {
    strcpy(base, fn);
    base[l-4]='\0';
    return true;
  }
  return false;
}

static void checkName(Scumm *s, Game &game)
{
  s->_exe_name = game.filename_base;
  if(s->detectGame()) {
    char *n = s->getGameName();
    strcpy(game.text, n);
    free(n);
  } else
    strcpy(game.text, game.filename_base);
  s->_exe_name = NULL;
}

static bool isIcon(const char *fn)
{
  int l = strlen(fn);
  if(l>4 && !strcasecmp(fn+l-4, ".ICO"))
    return true;
  else
    return false;
}

static bool loadIcon(Game &game, Dir *dirs, int num_dirs)
{
  char icofn[520];
  sprintf(icofn, "%s%s.ICO", game.dir, game.filename_base);
  if(game.icon.load(icofn))
    return true;
  for(int i=0; i<num_dirs; i++)
    if(!strcmp(dirs[i].name, game.dir) &&
       dirs[i].deficon[0]) {
      sprintf(icofn, "%s%s", game.dir, dirs[i].deficon);      
      if(game.icon.load(icofn))
	return true;
      break;
    }
  return false;
}

static void makeDefIcon(Icon &icon)
{
  icon.load(NULL, 0);
}

static int findGames(Scumm *s, Game *games, int max)
{
  Dir *dirs = new Dir[MAX_DIR];
  int curr_game = 0, curr_dir = 0, num_dirs = 1;
  strcpy(dirs[0].name, "/");
  while(curr_game < max && curr_dir < num_dirs) {
    dirs[curr_dir].deficon[0] = '\0';
    DIR *dirp = opendir(dirs[curr_dir++].name);
    if(dirp) {
      struct dirent *entry;
      while((entry = readdir(dirp)))
	if(entry->d_size < 0) {
	  if(num_dirs < MAX_DIR) {
	    strcpy(dirs[num_dirs].name, dirs[curr_dir-1].name);
	    if(strlen(dirs[num_dirs].name)+strlen(entry->d_name)<255) {
	      strcat(dirs[num_dirs].name, entry->d_name);
	      strcat(dirs[num_dirs].name, "/");
	      num_dirs++;
	    }
	  }
	} else 
	  if(isIcon(entry->d_name))
	    strcpy(dirs[curr_dir-1].deficon, entry->d_name);
	  else if(curr_game < max &&
	     isGame(entry->d_name, games[curr_game].filename_base)) {
	    strcpy(games[curr_game].dir, dirs[curr_dir-1].name);
	    checkName(s, games[curr_game]);
	    curr_game++;
	  }
      closedir(dirp);
    }
  }
  for(int i=0; i<curr_game; i++)
    if(!loadIcon(games[i], dirs, num_dirs))
      makeDefIcon(games[i].icon);
  delete dirs;
  return curr_game;
}

int getCdState()
{
  unsigned int param[4];
  gdGdcGetDrvStat(param);
  return param[0];
}

static void drawBackground()
{
  draw_solid_quad(20.0, 20.0, 620.0, 460.0,
		  0xff0000, 0x00ff00, 0x0000ff, 0xffffff);
}

void waitForDisk()
{
  Label lab;
  int wasopen = 0;
  ta_sync();
  void *mark = ta_txmark();
  lab.create_texture("Please insert game CD.");
  printf("waitForDisk, cdstate = %d\n", getCdState());
  for(;;) {
    int s = getCdState();
    if(s >= 6)
      wasopen = 1;
    if(s > 0 && s < 6 && wasopen) {
      cdfs_reinit();
      chdir("/");
      chdir("/");
      ta_sync();
      ta_txrelease(mark);
      return;
    }
    
    ta_begin_frame();

    drawBackground();

    ta_commit_end();

    lab.draw(166.0, 200.0, 0xffff2020);

    ta_commit_frame();

    int16 mousex = 0, mousey = 0;
    byte lmb=0, rmb=0;
    int key=0;

    int mask = getimask();
    setimask(15);
    handleInput(locked_get_pads(), mousex, mousey, lmb, rmb, key);
    setimask(mask);
  }
}

static void drawGameLabel(Game &game, int pal, float x, float y,
			  unsigned int argb, int fade = 0, float scale = 1.0)
{
  unsigned int fade_alpha = (255-fade)<<24;

  game.icon.draw(x, y, x+32.0*scale, y+32.0*scale, pal, 0xffffff|fade_alpha);
  game.label.draw(x+54.0*scale, y+4.0*scale, argb|fade_alpha, scale);
}

int gameMenu(Game *games, int num_games)
{
  int top_game = 0, selector_pos = 0;
  int16 mousex = 0, mousey = 64;

  if(!num_games)
    return -1;

  for(;;) {

    if(getCdState()>=6)
      return -1;

    ta_begin_frame();
    
    drawBackground();
    
    ta_commit_end();
    
    float y = 40.0;
    for(int i=top_game, cnt=0; cnt<10 && i<num_games; i++, cnt++) {
      int pal = 48+(i&15);

      if(cnt == selector_pos)
	draw_trans_quad(100.0, y, 590.0, y+32.0,
			0x7000ff00, 0x7000ff00, 0x7000ff00, 0x7000ff00);

      games[i].icon.set_palette(pal);
      drawGameLabel(games[i], pal, 50.0, y, (cnt == selector_pos?
					     0xffff00 : 0xffffff));
      y += 40.0;
    }

    ta_commit_frame();

    byte lmb=0, rmb=0;
    int key=0;

    int mask = getimask();
    setimask(15);
    handleInput(locked_get_pads(), mousex, mousey, lmb, rmb, key);
    setimask(mask);
    
    if(lmb || key==13 || key==319) {
      int selected_game = top_game + selector_pos;

      for(int fade=0; fade<=256; fade+=4) {

	unsigned int fade_colour = 0x00ffffff | ((255-fade)<<24);

	ta_begin_frame();
    
	drawBackground();
    
	ta_commit_end();
    
	float y = 40.0;

	if(fade < 256)
	  for(int i=top_game, cnt=0; cnt<10 && i<num_games; i++, cnt++)
	    if(cnt != selector_pos) {
	      drawGameLabel(games[i], 48+(i&15), 50.0, y, 0xffffff, fade);
	      y += 40.0;
	    }

	y = (40.0/256.0 * (selector_pos + 1))*(256-fade) + 80.0/256.0*fade;
	float x = 50.0/256.0*(256-fade) + 160.0/256.0*fade;
	float scale = 1.0+9.0/256.0*fade;

	drawGameLabel(games[selector_pos], 48+(selected_game&15), x, y,
		      0xffff00, 0, scale);

	ta_commit_frame();
      }
      return selected_game;
    }

    if(mousey>=64+16) {
      if(selector_pos + top_game + 1 < num_games)
	if(++selector_pos >= 10) {
	  --selector_pos;
	  ++top_game;
	}
      mousey -= 16;
    } else if(mousey<=64-16) {
      if(selector_pos + top_game > 0)
	if(--selector_pos < 0) {
	  ++selector_pos;
	  --top_game;
	}
      mousey += 16;
    }
  }
}

bool selectGame(Scumm *s, char *&ret, Icon &icon)
{
  Game *games = new Game[MAX_GAMES];
  int selected, num_games;

  ta_sync();
  void *mark = ta_txmark();

  for(;;) {

    num_games = findGames(s, games, MAX_GAMES);

    for(int i=0; i<num_games; i++) {
      games[i].icon.create_texture();
      games[i].label.create_texture(games[i].text);
    }

    selected = gameMenu(games, num_games);

    ta_sync();
    ta_txrelease(mark);

    if(selected == -1)
      waitForDisk();
    else
      break;

  }

  if(selected >= num_games)
    selected = -1;

  if(selected >= 0)
    the_game = games[selected];

  delete games;

  if(selected>=0) {
    chdir(the_game.dir);
    ret = the_game.filename_base;
    icon = the_game.icon;
    return true;
  } else
    return false;
}
