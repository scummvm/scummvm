#ifndef ANIMATION_H
#define ANIMATION_H

#include <inttypes.h>
#ifdef USE_MPEG2
extern "C" {
	#include <mpeg2dec/mpeg2.h>
}
#endif

namespace Sword2 {

#define SQR(x) ((x)*(x))

#define SHIFT 3
#define BITDEPTH (1<<(8-SHIFT))
#define ROUNDADD (1<<(SHIFT-1))

#define BUFFER_SIZE 4096


typedef struct {


  int palnum;

  unsigned char lookup[2][BITDEPTH*BITDEPTH*BITDEPTH];
  unsigned char * lut;
  unsigned char * lut2;
  int lutcalcnum;

  int framenum;

  #ifdef USE_MPEG2
  mpeg2dec_t * decoder;
  const mpeg2_info_t * info;
  #endif
  File * mpgfile;

  int curpal;
  int cr;
  int pos;

  struct {
    int cnt;
    int end;
    unsigned char pal[4*256];
  } palettes[50];

  unsigned char buffer[BUFFER_SIZE];

  PlayingSoundHandle bgSound;

} AnimationState;

} // End of namespace Sword2

#endif
