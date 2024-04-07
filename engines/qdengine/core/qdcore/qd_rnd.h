#ifndef __QD_RND_H__
#define __QD_RND_H__

extern RandomGenerator qd_random_generator;

/// Возвращает случайное значение в интервале [0, m-1].
inline unsigned qd_rnd(unsigned m) {
	return qd_random_generator(m);
}
/// Возвращает случайное значение в интервале [-x, x].
inline float qd_frnd(float x) {
	return qd_random_generator.frnd(x);
}
/// Возвращает случайное значение в интервале [0, x].
inline float qd_fabs_rnd(float x) {
	return qd_random_generator.fabsRnd(x);
}

bool qd_rnd_init(int seed = 83);

#endif /* __QD_RND_H__ */

