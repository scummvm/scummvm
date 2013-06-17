#ifndef __enid_h__
#define __enid_h__


#include "gyro.h"


void edna_save(string name);

void edna_load(string name);

void edna_reload();    /* From Bootstrap's storage. */

void dir(string where);

void avvy_background();

void back_to_bootstrap(byte what);

boolean there_was_a_problem();

#endif
