/* ---------------------------- INCLUDE SECTION ----------------------------- */

#include "qdengine/core/qd_precomp.h"
#include "qdengine/core/qdcore/qd_rnd.h"


namespace QDEngine {

/* ----------------------------- STRUCT SECTION ----------------------------- */
/* ----------------------------- EXTERN SECTION ----------------------------- */
/* --------------------------- PROTOTYPE SECTION ---------------------------- */
/* --------------------------- DEFINITION SECTION --------------------------- */

RandomGenerator qd_random_generator;

bool qd_rnd_init(int seed) {
	qd_random_generator.set(seed);
	return true;
}
} // namespace QDEngine
