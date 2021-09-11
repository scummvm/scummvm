#ifndef _ANIM_H_
#define _ANIM_H_

void PlayAnim(unsigned char index, unsigned char x, unsigned char y);
void CopyScreenBlockWithDotEffect(unsigned char *source, unsigned char x, unsigned char y, unsigned char width, unsigned char height, unsigned char *target);

extern unsigned char dot_effect_step;
extern unsigned int dot_effect_delay;

#endif
