/* ScummVM - Scumm Interpreter
 * Dreamcast port
 * Copyright (C) 2002-2003  Marcus Comstedt
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

#include <common/stdafx.h>
#include <common/scummsys.h>
#include <base/engine.h>
#include <base/gameDetector.h>
#include <base/plugins.h>
#include <backends/fs/fs.h>
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
  char name[252];
  char deficon[256];
  FilesystemNode *node;
};

static Game the_game;

static bool checkName(const char *base, char *text = 0)
{
  GameDetector g;
  GameSettings gs = g.findGame(base);

  if (gs.gameName) {
    if(text != NULL)
      strcpy(text, gs.description);
    return true;
  }
  return false;
}

static const char *checkDetect(const FilesystemNode *entry)
{
  FSList files;
  files.push_back(*entry);
  GameList candidates;

  const PluginList &plugins = PluginManager::instance().getPlugins();
  PluginList::ConstIterator iter = plugins.begin();
  for (iter = plugins.begin(); iter != plugins.end(); ++iter) {
    candidates.push_back((*iter)->detectGames(files));
  }
  if (candidates.isEmpty())
    return NULL;
  return candidates[0].gameName;
}

static bool isGame(const FilesystemNode *entry, char *base)
{
  const char *fn = entry->displayName().c_str();
  if(!strcasecmp(fn, "00.LFL") ||
     !strcasecmp(fn, "000.LFL")) {
    *base = '\0';
    return true;
  }
  if(const char *dtct = checkDetect(entry)) {
    strcpy(base, dtct);
    return true;
  }
#if 0
  int l = strlen(fn);
  if(l>4 && (!strcasecmp(fn+l-4, ".000") ||
	     !strcasecmp(fn+l-4, ".SM0") ||
	     !strcasecmp(fn+l-4, ".HE0") ||
	     !strcasecmp(fn+l-4, ".LA0"))) {
    strcpy(base, fn);
    base[l-4]='\0';
    return true;
  }
#else
  char *dot;
  if((dot = strchr(fn, '.'))!=NULL) {
    if(!strcasecmp(dot, ".SAN"))
      return false;
    strcpy(base, fn);
    base[dot-fn]='\0';
    if(checkName(base))
      return true;
  }
#endif  
  return false;
}

static bool checkExe(const char *dir, const char *f)
{
  char fn[520];
  int fd;
  sprintf(fn, "%s%s.EXE", dir, f);
  if((fd = open(fn, O_RDONLY))<0)
    return false;
  close(fd);
  return true;
}

static bool isIcon(const FilesystemNode *entry)
{
  int l = entry->displayName().size();
  if(l>4 && !strcasecmp(entry->displayName().c_str()+l-4, ".ICO"))
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

static bool uniqueGame(const char *base, const char *dir, Game *games, int cnt)
{
  while(cnt--)
    if(!strcmp(dir, games->dir) &&
       !strcmp(base, games->filename_base))
      return false;
    else
      games++;
  return true;
}

static int findGames(Game *games, int max)
{
  Dir *dirs = new Dir[MAX_DIR];
  int curr_game = 0, curr_dir = 0, num_dirs = 1;
  dirs[0].node = FilesystemNode::getRoot();
  while(curr_game < max && curr_dir < num_dirs) {
    strncpy(dirs[curr_dir].name, dirs[curr_dir].node->path().c_str(), 252);
    dirs[curr_dir].name[251] = '\0';
    dirs[curr_dir].deficon[0] = '\0';
    FSList *fslist = dirs[curr_dir++].node->listDir(FilesystemNode::kListAll);
    if (fslist != NULL) {
      for (FSList::ConstIterator entry = fslist->begin(); entry != fslist->end();
	   ++entry) {
	if (entry->isDirectory()) {
	  if(num_dirs < MAX_DIR && strcasecmp(entry->displayName().c_str(),
					      "install")) {
	    if ((dirs[num_dirs].node = entry->clone()) != NULL)
	      num_dirs ++;
	  }
	} else 
	  if(isIcon(&*entry))
	    strcpy(dirs[curr_dir-1].deficon, entry->displayName().c_str());
	  else if(curr_game < max &&
		  isGame(&*entry, games[curr_game].filename_base)) {
	    strcpy(games[curr_game].dir, dirs[curr_dir-1].name);
	    if(!*games[curr_game].filename_base) {
	      int i;
	      for(i=strlen(games[curr_game].dir)-1; --i>=0; )
		if(games[curr_game].dir[i]=='/')
		  break;
	      if(i>=0) {
		strcpy(games[curr_game].filename_base,
		       games[curr_game].dir+i+1);
		games[curr_game].filename_base[strlen(games[curr_game].
						      filename_base)-1]='\0';
#if 0
		games[curr_game].dir[i+1]='\0';
#endif
	      }
	      if(checkExe(games[curr_game].dir, "loom"))
		strcpy(games[curr_game].filename_base, "loomcd");
	    }
	    if(uniqueGame(games[curr_game].filename_base,
			  games[curr_game].dir, games, curr_game)) {

	      if(!checkName(games[curr_game].filename_base,
			    games[curr_game].text))
		strcpy(games[curr_game].text, games[curr_game].filename_base);
#if 0
	      printf("Registered game <%s> in <%s> <%s> because of <%s> <%s>\n",
		     games[curr_game].text, games[curr_game].dir,
		     games[curr_game].filename_base,
		     dirs[curr_dir-1].name, entry->displayName().c_str());
#endif
	      curr_game++;
	    }
	  }
      }
      delete fslist;
    }
    delete dirs[curr_dir-1].node;
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
  //printf("waitForDisk, cdstate = %d\n", getCdState());
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

    int mousex = 0, mousey = 0;
    byte shiftFlags;

    int mask = getimask();
    setimask(15);
    handleInput(locked_get_pads(), mousex, mousey, shiftFlags);
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
  int mousex = 0, mousey = 0;

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

    byte shiftFlags;
    int event;

    int mask = getimask();
    setimask(15);
    event = handleInput(locked_get_pads(), mousex, mousey, shiftFlags);
    setimask(mask);
    
    if(event==-OSystem::EVENT_LBUTTONDOWN || event==13 || event==319) {
      int selected_game = top_game + selector_pos;

      for(int fade=0; fade<=256; fade+=4) {

	unsigned int fade_colour = 0x00ffffff | ((255-fade)<<24);

	ta_begin_frame();
    
	drawBackground();
    
	ta_commit_end();
    
	float y = 40.0;

	if(fade < 256)
	  for(int i=top_game, cnt=0; cnt<10 && i<num_games; 
	      i++, cnt++, y += 40.0)
	    if(cnt != selector_pos)
	      drawGameLabel(games[i], 48+(i&15), 50.0, y, 0xffffff, fade);

	y = (40.0/256.0 * (selector_pos + 1))*(256-fade) + 80.0/256.0*fade;
	float x = 50.0/256.0*(256-fade) + 160.0/256.0*fade;
	float scale = 1.0+9.0/256.0*fade;

	drawGameLabel(games[selected_game], 48+(selected_game&15), x, y,
		      0xffff00, 0, scale);

	ta_commit_frame();
      }
      return selected_game;
    }

    if(mousey>=16) {
      if(selector_pos + top_game + 1 < num_games)
	if(++selector_pos >= 10) {
	  --selector_pos;
	  ++top_game;
	}
      mousey -= 16;
    } else if(mousey<=-16) {
      if(selector_pos + top_game > 0)
	if(--selector_pos < 0) {
	  ++selector_pos;
	  --top_game;
	}
      mousey += 16;
    }
  }
}

bool selectGame(char *&ret, char *&dir_ret, Icon &icon)
{
  Game *games = new Game[MAX_GAMES];
  int selected, num_games;

  ta_sync();
  void *mark = ta_txmark();

  for(;;) {
    num_games = findGames(games, MAX_GAMES);

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
#if 0
    chdir(the_game.dir);
#else
    chdir("/");
    static char dirarg[258];
    sprintf(dirarg, "-p%s", the_game.dir);
    dir_ret = dirarg;
#endif
    ret = the_game.filename_base;
    icon = the_game.icon;
    return true;
  } else
    return false;
}
