/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2004-May-22 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
#ifndef _WIN32ZIP_H
#define _WIN32ZIP_H

/*
 * NT specific functions for ZIP.
 */

int GetFileMode(const char *name);
#if 0 /* never used */
long GetTheFileTime(const char *name, iztimes *z_times);
#endif

int IsFileNameValid(const char *name);
int IsFileSystemOldFAT(const char *dir);
void ChangeNameForFAT(char *name);

char *getVolumeLabel(int drive, ulg *vtime, ulg *vmode, time_t *vutim);

#if 0 /* never used ?? */
char *StringLower(char *);
#endif

char *GetLongPathEA(const char *name);

#endif /* _WIN32ZIP_H */
