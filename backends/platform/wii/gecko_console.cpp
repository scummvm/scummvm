/*-------------------------------------------------------------

Copyright (C) 2008
Hector Martin (marcan)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.

-------------------------------------------------------------*/

#include <stdio.h>
#include <sys/iosupport.h>
#include <ogcsys.h>
#include <gccore.h>
#include <reent.h>

#include "gecko_console.h"

#ifdef __cplusplus
extern "C" {
#endif

static const devoptab_t *dotab_console;
int usb_sendbuffer_safe(s32 chn,const void *buffer,int size);
int usb_sendbuffer(s32 chn,const void *buffer,int size);

int __gecko_write(struct _reent *r,int fd,const char *ptr,int len) {
	char *tmp = (char*)ptr;
	u32 level;
	if(dotab_console)
		dotab_console->write_r(r,fd,ptr,len);
		
	if(!tmp || len<=0)
		return -1;
	level = IRQ_Disable();
	usb_sendbuffer(1, ptr, len);
	IRQ_Restore(level);
	return len;
}

const devoptab_t dotab_gecko = {
	"stdout",	// device name
	0,			// size of file structure
	NULL,		// device open
	NULL,		// device close
	__gecko_write,	// device write
	NULL,		// device read
	NULL,		// device seek
	NULL,		// device fstat
	NULL,		// device stat
	NULL,		// device link
	NULL,		// device unlink
	NULL,		// device chdir
	NULL,		// device rename
	NULL,		// device mkdir
	0,			// dirStateSize
	NULL,		// device diropen_r
	NULL,		// device dirreset_r
	NULL,		// device dirnext_r
	NULL,		// device dirclose_r
	NULL		// device statvfs_r
};

void gecko_console_init(int chain) {
	dotab_console = NULL;
	if(chain)
		dotab_console = devoptab_list[STD_OUT];
	devoptab_list[STD_OUT] = &dotab_gecko;
	devoptab_list[STD_ERR] = &dotab_gecko;
}

#ifdef __cplusplus
}
#endif

