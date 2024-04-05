#ifndef __QDSCR_ERROR_H__
#define __QDSCR_ERROR_H__

void qdscr_SetErrorMode(int mode);
void qdscr_SetErrorOutHandler(void (*hfnc)(char*));

void qdscr_Error(char* msg);
void qdscr_Error(char* msg,int code,char* text = NULL);


#endif /* __QDSCR_ERROR_H__ */