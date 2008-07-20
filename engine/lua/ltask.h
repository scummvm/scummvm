#ifndef ltask_h
#define ltask_h

#include "lua.h"
#include "lstate.h"

extern int32 task_tag;

void start_script (void);
void stop_script (void);
void next_script (void);
void identify_script (void);
void pause_scripts (void);
void unpause_scripts (void);
void find_script (void);
void break_here (void);

void gc_task (void);

#endif
