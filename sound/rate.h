// HACK: Instead of using the full st_i.h (and then st.h and stconfig.h etc.)
// from SoX, we use this minimal variant which is just sufficient to make
// resample.c and rate.c compile.

#ifndef RATE_H
#define RATE_H

#include <stdio.h>
#include <assert.h>
#include "common/scummsys.h"
#include "common/engine.h"
#include "common/util.h"

#include "audiostream.h"

typedef int16 st_sample_t;
typedef uint16 st_volume_t;
typedef uint32 st_size_t;
typedef uint32 st_rate_t;

typedef struct {
	byte priv[1024];
} eff_struct;
typedef eff_struct *eff_t;

/* Minimum and maximum values a sample can hold. */
#define ST_SAMPLE_MAX 0x7fffL
#define ST_SAMPLE_MIN (-ST_SAMPLE_MAX - 1L)

#define ST_EOF (-1)
#define ST_SUCCESS (0)

static inline void clampedAdd(int16& a, int b) {
	int val = a + b;

	if (val > ST_SAMPLE_MAX)
		a = ST_SAMPLE_MAX;
	else if (val < ST_SAMPLE_MIN)
		a = ST_SAMPLE_MIN;
	else
		a = val;
}

// Q&D hack to get this SOX stuff to work
#define st_report warning
#define st_warn warning
#define st_fail error


class RateConverter {
protected:
	eff_struct effp;
public:
	RateConverter() {}
	virtual ~RateConverter() {}
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) = 0;
	virtual int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) = 0;
};

class LinearRateConverter : public RateConverter {
	bool _reverseStereo;
public:
	LinearRateConverter(st_rate_t inrate, st_rate_t outrate, bool reverseStereo);
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol);
	virtual int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol);
};

class ResampleRateConverter : public RateConverter {
public:
	ResampleRateConverter(st_rate_t inrate, st_rate_t outrate, int quality);
	~ResampleRateConverter();
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol);
	virtual int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol);
};

template<bool stereo, bool reverseStereo>
class CopyRateConverter : public RateConverter {
public:
	virtual int flow(AudioInputStream &input, st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
		int16 tmp[2];
		st_size_t len = *osamp;
		assert(input.isStereo() == stereo);
		while (!input.eof() && len--) {
			tmp[0] = tmp[1] = input.read() * vol / 256;
			if (stereo)
				tmp[reverseStereo ? 0 : 1] = input.read() * vol / 256;
			clampedAdd(*obuf++, tmp[0]);
			clampedAdd(*obuf++, tmp[1]);
		}
		return (ST_SUCCESS);
	}
	virtual int drain(st_sample_t *obuf, st_size_t *osamp, st_volume_t vol) {
		return (ST_SUCCESS);
	}
};

static inline RateConverter *makeRateConverter(st_rate_t inrate, st_rate_t outrate, bool stereo, bool reverseStereo = false) {
	if (inrate != outrate) {
		return new LinearRateConverter(inrate, outrate, reverseStereo);
		//return new ResampleRateConverter(inrate, outrate, 1);
	} else {
		if (stereo) {
			if (reverseStereo)
				return new CopyRateConverter<true, true>();
			else
				return new CopyRateConverter<true, false>();
		} else
			return new CopyRateConverter<false, false>();
	}
}

#endif
