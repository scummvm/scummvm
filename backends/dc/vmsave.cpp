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
#include "base/engine.h"
#include "dc.h"
#include "icon.h"
#include <scumm/saveload.h>
#include <gui/newgui.h>
#include <gui/message.h>

#include <zlib.h>


// Savegame can not be bigger than this, even before compression
#define MAX_SAVE_SIZE (128*1024)


enum vmsaveResult {
  VMSAVE_OK,
  VMSAVE_NOVM,
  VMSAVE_NOSPACE,
  VMSAVE_WRITEERROR,  
};


static int lastvm=-1;

static void displaySaveResult(vmsaveResult res)
{
  char buf[1024];

  switch(res) {
  case VMSAVE_OK:
    sprintf(buf, "Game saved on unit %c%d", 'A'+(lastvm/6), lastvm%6);
    break;
  case VMSAVE_NOVM:
    strcpy(buf, "No memory card present!");
    break;
  case VMSAVE_NOSPACE:
    strcpy(buf, "Not enough space available!");
    break;
  case VMSAVE_WRITEERROR:
    strcpy(buf, "Write error!!!");
    break;
  default:
    strcpy(buf, "Unknown error!!!");
    break;
  }

  GUI::MessageDialog dialog(buf);
  dialog.runModal();
}

static vmsaveResult trySave(const char *gamename, const char *data, int size,
			    const char *filename, class Icon &icon, int vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  struct vms_file_header header;
  struct timestamp tstamp;
  struct tm tm;
  time_t t;
  unsigned char iconbuffer[512+32];

  if(!vmsfs_check_unit(vm, 0, &info))
    return VMSAVE_NOVM;
  if(!vmsfs_get_superblock(&info, &super))
    return VMSAVE_NOVM;
  int free_cnt = vmsfs_count_free(&super);
  if(vmsfs_open_file(&super, filename, &file))
    free_cnt += file.blks;
  if(((128+512+size+511)>>9) > free_cnt)
    return VMSAVE_NOSPACE;

  memset(&header, 0, sizeof(header));
  strncpy(header.shortdesc, "ScummVM savegame", 16);
  strncpy(header.longdesc, gamename, 32);
  strncpy(header.id, "ScummVM", 16);
  icon.create_vmicon(iconbuffer);
  header.numicons = 1;
  memcpy(header.palette, iconbuffer, sizeof(header.palette));
  time(&t);
  tm = *localtime(&t);
  tstamp.year = tm.tm_year+1900;
  tstamp.month = tm.tm_mon+1;
  tstamp.day = tm.tm_mday;
  tstamp.hour = tm.tm_hour;
  tstamp.minute = tm.tm_min;
  tstamp.second = tm.tm_sec;
  tstamp.wkday = (tm.tm_wday+6)%7;

  vmsfs_beep(&info, 1);

  vmsfs_errno = 0;
  if(!vmsfs_create_file(&super, filename, &header,
			iconbuffer+sizeof(header.palette), NULL,
			data, size, &tstamp)) {
    fprintf(stderr, "%s\n", vmsfs_describe_error());
    vmsfs_beep(&info, 0);
    return VMSAVE_WRITEERROR;
  }

  vmsfs_beep(&info, 0);
  return VMSAVE_OK;
}

static bool tryLoad(char *&buffer, int &size, const char *filename, int vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  struct vms_file_header header;
  struct timestamp tstamp;
  struct tm tm;
  time_t t;
  unsigned char iconbuffer[512+32];

  if(!vmsfs_check_unit(vm, 0, &info))
    return false;
  if(!vmsfs_get_superblock(&info, &super))
    return false;
  if(!vmsfs_open_file(&super, filename, &file))
    return false;
  
  buffer = new char[size = file.size];

  if(vmsfs_read_file(&file, (unsigned char *)buffer, size))
    return true;

  delete buffer;
  return false;
}

static void tryList(const char *prefix, bool *marks, int num, int vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct dir_iterator iter;
  struct dir_entry de;
  int pl = strlen(prefix);

  if(!vmsfs_check_unit(vm, 0, &info))
    return;
  if(!vmsfs_get_superblock(&info, &super))
    return;
  vmsfs_open_dir(&super, &iter);
  while(vmsfs_next_dir_entry(&iter, &de))
    if(de.entry[0]) {
      char buf[16], *endp = NULL;
      strncpy(buf, (char *)de.entry+4, 12);
      buf[12] = 0;
      int l = strlen(buf);
      long i = 42;
      if(l > pl && !strncmp(buf, prefix, pl) &&
	 (i = strtol(buf+pl, &endp, 10))>=0 && i<num &&
	 (endp - buf) == l)
	marks[i] = true;
    }
}

vmsaveResult writeSaveGame(const char *gamename, const char *data, int size,
			   const char *filename, class Icon &icon)
{
  vmsaveResult r, res = VMSAVE_NOVM;

  if(lastvm >= 0 &&
     (res = trySave(gamename, data, size, filename, icon, lastvm)) == VMSAVE_OK)
    return res;

  for(int i=0; i<24; i++)
    if((r = trySave(gamename, data, size, filename, icon, i)) == VMSAVE_OK) {
      lastvm = i;
      return r;
    } else if(r > res)
      res = r;

  return res;
}

bool readSaveGame(char *&buffer, int &size, const char *filename)
{
  if(lastvm >= 0 &&
     tryLoad(buffer, size, filename, lastvm))
    return true;

  for(int i=0; i<24; i++)
    if(tryLoad(buffer, size, filename, i)) {
      lastvm = i;
      return true;
    }

  return false;
}


class VMSave : public SaveFile {
private:
  bool issave;
  char *buffer;
  int pos, size;
  char filename[16];

protected:
  virtual int fread(void *buf, int size, int cnt);
  virtual int fwrite(const void *buf, int size, int cnt);

public:
  VMSave(const char *_filename, bool _saveOrLoad) 
    : issave(_saveOrLoad), pos(0), buffer(NULL)
  {
    strncpy(filename, _filename, 16);
    if(issave)
      buffer = new char[size = MAX_SAVE_SIZE];
  }

  ~VMSave();

  bool isOpen() const { return true; }

  bool readSaveGame()
  { return ::readSaveGame(buffer, size, filename); }

  void tryUncompress()
  {
    if(size > 0 && buffer[0] != 'S') {
      // Data does not start with "SCVM".  Maybe compressed?
      char *expbuf = new char[MAX_SAVE_SIZE];
      unsigned long destlen = MAX_SAVE_SIZE;
      if(!uncompress((Bytef*)expbuf, &destlen, (Bytef*)buffer, size)) {
	delete(buffer);
	buffer = expbuf;
	size = destlen;
      } else delete expbuf;
    }
  }
};

class VMSaveManager : public SaveFileManager {
  virtual SaveFile *open_savefile(const char *filename, const char *directory, bool saveOrLoad);
  virtual void list_savefiles(const char *prefix, const char *directory, bool *marks, int num);
};

SaveFile *VMSaveManager::open_savefile(const char *filename, const char *directory,
				       bool saveOrLoad)
{
  VMSave *s = new VMSave(filename, saveOrLoad);
  if(saveOrLoad)
    return s;
  else if(s->readSaveGame()) {
    s->tryUncompress();
    return s;
  } else {
    delete s;
    return NULL;
  }
}

VMSave::~VMSave()
{
  extern const char *gGameName;
  extern Icon icon;

  if(issave) {
    if(pos) {
      // Try compression
      char *compbuf = new char[pos];
      unsigned long destlen = pos;
      if(!compress((Bytef*)compbuf, &destlen, (Bytef*)buffer, pos)) {
	delete buffer;
	buffer = compbuf;
	pos = destlen;
      } else delete compbuf;
    }
    displaySaveResult(writeSaveGame(gGameName, buffer,
				    pos, filename, icon));
  }
  delete buffer;
}

int VMSave::fread(void *buf, int sz, int cnt)
{
  if (issave)
    return -1; 

  int nbyt = sz*cnt;
  if (pos + nbyt > size) {
    cnt = (size - pos)/sz;
    nbyt = sz*cnt;
  }
  if (nbyt)
    memcpy(buf, buffer + pos, nbyt);
  pos += nbyt;
  return cnt;
}

int VMSave::fwrite(const void *buf, int sz, int cnt)
{
  if (!issave)
    return -1;

  int nbyt = sz*cnt;
  if (pos + nbyt > size) {
    cnt = (size - pos)/sz;
    nbyt = sz*cnt;
  }
  if (nbyt)
    memcpy(buffer + pos, buf, nbyt);
  pos += nbyt;
  return cnt;
}


void VMSaveManager::list_savefiles(const char *prefix, const char *directory,
				   bool *marks, int num)
{
  memset(marks, false, num*sizeof(bool));
  
  for(int i=0; i<24; i++)
    tryList(prefix, marks, num, i);
}

SaveFileManager *OSystem_Dreamcast::get_savefile_manager()
{
  return new VMSaveManager();
}
