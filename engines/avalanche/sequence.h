#ifndef __sequence_h__
#define __sequence_h__


const integer now_flip = 177;

const integer seq_length = 10;


#ifdef __sequence_implementation__
#undef EXTERN
#define EXTERN
#endif

EXTERN array<1,seq_length,byte> seq;
#undef EXTERN
#define EXTERN extern


void first_show(byte what);

void then_show(byte what);

void then_flip(byte where,byte ped);

void start_to_close();

void start_to_open();

void call_sequencer();

#endif
