#ifndef _PORTRAIT_H_
#define _PORTRAIT_H_

extern unsigned char *cur_image_pixels;
extern unsigned char cur_image_size_w;
extern unsigned char cur_image_size_h;
extern unsigned char cur_image_coords_x;
extern unsigned char cur_image_coords_y;
extern unsigned int cur_image_offs;
extern unsigned int cur_image_end;
extern unsigned char cur_image_idx;
extern unsigned char cur_image_anim1;
extern unsigned char cur_image_anim2;
extern unsigned int cur_frame_width;

int DrawPortrait(unsigned char **desc, unsigned char *x, unsigned char *y, unsigned char *width, unsigned char *height);
void AnimPortrait(unsigned char layer, unsigned char index, unsigned char delay);

void DrawBoxAroundSpot(void);

void MergeImageAndSpriteData(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h);
void MergeImageAndSpriteDataFlip(unsigned char *target, signed int pitch, unsigned char *source, unsigned int w, unsigned int h);

void BlinkToRed(void);
void BlinkToWhite(void);


#endif
