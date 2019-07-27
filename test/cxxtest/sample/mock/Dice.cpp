#include "Dice.h"
#include <T/stdlib.h>

Dice::Dice() {
	T::srand(T::time(0));
}

unsigned Dice::roll() {
	return (T::rand() % 6) + 1;
}
