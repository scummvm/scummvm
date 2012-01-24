/*
** Built-in functions
** See Copyright Notice in lua.h
*/

#ifndef GRIM_LBUILTIN_H
#define GRIM_LBUILTIN_H

namespace Grim {

void luaB_predefine();
void stop_script();
void start_script();
void find_script();
void identify_script();
void next_script();
void break_here();
void sleep_for();
void pause_scripts();
void unpause_scripts();

} // end of namespace Grim

#endif
