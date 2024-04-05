#include "xglobal.h"

bool XBuffer::search(char* what, int mode, int cs)
{
	register int i,j;
	unsigned int wlen = strlen(what) - 1;
	if(mode == XB_GLOBAL) { i = 0; mode = XB_FORWARD; } else i = offset_;
	if(mode == XB_FORWARD){
		if(cs == XB_CASEON){
			while(buffer_[i]){
				if(buffer_[i] == *what){
					j = wlen;
					while(buffer_[i + j] == what[j] && j) j--;
					if(j <= 0) { offset_ = i; return 1; };
					}
				i++;
				}
			}
		else {
			while(buffer_[i]){
				if(toupper(buffer_[i]) == toupper(*what)){
					j = wlen;
					while(toupper(buffer_[i + j]) == toupper(what[j]) && j) j--;
					if(j <= 0) { offset_ = i; return 1; };
					}
				i++;
				}
			}
		}
	else {
		i-=wlen + 1;
		if(cs == XB_CASEON){
			while(i >= 0){
				if(buffer_[i] == *what){
					j = wlen;
					while(buffer_[i + j] == what[j] && j) j--;
					if(j <= 0) { offset_ = i; return 1; };
					}
				i--;
				}
			}
		else {
			while(i >= 0){
				if(toupper(buffer_[i]) == toupper(*what)){
					j = wlen;
					while(toupper(buffer_[i + j]) == toupper(what[j]) && j) j--;
					if(j <= 0) { offset_ = i; return 1; };
					}
				i--;
				}
			}
		}
	return 0;
}

