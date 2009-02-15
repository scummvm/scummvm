/***************************************************************************
 main.c Copyright (C) 2000 Rickard Lind


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.

***************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "mt32_timbres.c"
#include "gm_patches.c"

void analyze(unsigned char *patch001, unsigned int length001,
	     unsigned char *patch004, unsigned int length004);

int main(int argc, char **argv)
{
  int fd1, fd2;
  unsigned char *patch001;
  unsigned char *patch004;
  unsigned int length001, length004;

  if (argc < 2)
    return -1;

  if ((fd1 = open(argv[1], O_RDONLY)) < 0)
    return -1;
  if ((fd2 = open(argv[2], O_RDONLY)) < 0) {
    close(fd1);
    return -1;
  }

  patch001 = (unsigned char *)sci_malloc(65536);
  length001 = read(fd1, patch001, 65536);
  close(fd1);

  patch004 = (unsigned char *)sci_malloc(65536);
  length004 = read(fd2, patch004, 65536);
  close(fd2);

  if (patch001[0] == 0x89 && patch001[1] == 0x00)
    if (patch004[0] == 0x89 && patch004[1] == 0x00)
      analyze(patch001 + 2, length001 - 2, patch004 + 2, length004 - 2);
    else
      analyze(patch001 + 2, length001 - 2, patch004, length004);
  else
    if (patch004[0] == 0x89 && patch004[1] == 0x00)
      analyze(patch001, length001, patch004 + 2, length004 - 2);
    else
      analyze(patch001, length001, patch004, length004);

  free(patch001);
  free(patch004);

  return 0;
}

void analyze(unsigned char *patch001, unsigned int length001,
	     unsigned char *patch004, unsigned int length004)
{
  int i;
  unsigned char *mt32patch;
  unsigned char *mt32rhythm;

  printf("  ----------------------------------------------------------------------\n");
  printf("  | %.20s | %.20s | %.20s |\n", patch001, patch001 + 20, patch001 + 40);
  printf("--------------------------------------------------------------------------\n");
  printf("| ## | MT-32 Timbre | KSh FTn BR | General MIDI Patch      |  KSh VolA V |\n");
  printf("--------------------------------------------------------------------------\n");
  for (i = 0; i < 96; i++) {
    if (i < 48)
      mt32patch = patch001 + 107 + i * 8;
    else
      mt32patch = patch001 + 110 + i * 8 + patch001[491] * 246;

    if (!((mt32patch[0] == 0) &&
	  (mt32patch[1] == 0) &&
	  (mt32patch[2] == 0) &&
	  (mt32patch[3] == 0) &&
	  (mt32patch[4] == 0) &&
	  (mt32patch[5] == 0) &&
	  (mt32patch[6] == 0) &&
	  (mt32patch[7] == 0))) {
      printf("| %02i |", i);
      if (mt32patch[0] < 2)
	if (mt32patch[0] == 0)
	  printf("   %.10s", MT32_Timbre[mt32patch[1]]);
	else
	  printf("   %.10s", MT32_Timbre[mt32patch[1] + 64]);
      else if (mt32patch[0] == 2)
	printf(" m %.10s", patch001 + 492 + mt32patch[1] * 246);
      else if (mt32patch[0] == 3)
	printf(" r %.10s", MT32_RhythmTimbre[mt32patch[1]]);
      printf(" | % 03i % 03i %02i | ",
	     mt32patch[2] - 24,
	     mt32patch[3] - 50,
	     mt32patch[4]);
      if (patch004[i] != 0xFF) {
	printf("%-23s ", GM_Patch[patch004[i]]);
	printf("| % 04i % 04i %i |",
	       *((signed char *)(patch004) + i + 128),
	       *((signed char *)(patch004) + i + 256),
	       patch004[i + 513]);
      } else
	printf("                        |             |");
      printf("\n");
    }
  }
  printf("--------------------------------------------------------------------------\n");
  printf("         | ## | MT-32 Timbre |  OL PP | General MIDI Rhythm Key |\n");
  printf("         --------------------------------------------------------\n");
  for (i = 0; i < 64; i++)
    {
      mt32rhythm = patch001 + 880 + i * 4 + patch001[491] * 246;
      if ((mt32rhythm[0] < 94) &&
	  !((mt32rhythm[0] == 0) &&
	    (mt32rhythm[1] == 0) &&
	    (mt32rhythm[2] == 0) &&
	    (mt32rhythm[3] == 0)) &&
	  !((mt32rhythm[0] == 1) &&
	    (mt32rhythm[1] == 1) &&
	    (mt32rhythm[2] == 1) &&
	    (mt32rhythm[3] == 1))) {
	printf("         | %02i |", i + 24);
	if (mt32rhythm[0] < 64)
	  printf(" m %.10s", patch001 + 492 + mt32rhythm[0] * 246);
	else
	  printf(" r %.10s", MT32_RhythmTimbre[mt32rhythm[0] - 64]);
	printf(" | %03i %02i | ", mt32rhythm[1], mt32rhythm[2]);
	if (patch004[384 + i + 24] != 0xFF)
	  printf("%-23s |", GM_RhythmKey[patch004[384 + i + 24] - 35]);
	else
	  printf("                        |");
	printf("\n");
      }
    }
  printf("         --------------------------------------------------------\n");
  return;
}
