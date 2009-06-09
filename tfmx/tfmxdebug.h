#ifndef TFMXDEBUG_H
#define TFMXDEBUG_H

void displayTrackstep(const void *const vptr);
void displayPatternstep(const void *const vptr);
void displayMacroStep(const void *const vptr);
void displayMacroStep(const void *const vptr, int chan, int index);
void dumpTracksteps(Audio::Tfmx &player, uint16 first, uint16 last);
void dumpTrackstepsBySong(Audio::Tfmx &player, int song);
void dumpMacro(Audio::Tfmx &player, uint16 macroIndex, uint16 len = 0, uint16 start = 0);
void dumpPattern(Audio::Tfmx &player, uint16 pattIndex, uint16 len = 0, uint16 start = 0);
void countAllMacros(Audio::Tfmx &player);

#endif // TFMXDEBUG_H