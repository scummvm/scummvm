#ifndef ltask_h
#define ltask_h

#include "lua.h"
#include "lstate.h"

extern int task_tag;

void start_script (void);
void stop_script (void);
void next_script (void);
void identify_script (void);
void find_script (void);
void break_here (void);
void GetCurrentScript (void);

void gc_task (void);

#endif
