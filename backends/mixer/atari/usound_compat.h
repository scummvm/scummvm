/*
 * Copyright 2023-2026 Miro Kropacek <miro.kropacek@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Presents the uSound 2 API (USoundSpec / USoundContext / USoundInitXbios())
 * on top of any installed usound.h, including the pre-2 one which had
 * AudioSpec, AtariSoundSetup*() and no context.
 *
 * Include this instead of <usound.h> and copy it into your own source tree --
 * a sysroot with a pre-2 usound.h will not have it.
 *
 * Nothing here is needed once the oldest usound.h you care about is >= 2 --
 * including it against such a header is a hard error telling you to remove it.
 */

#ifndef USOUND_COMPAT_H
#define USOUND_COMPAT_H

/*
 * Pre-2 entry points have external linkage and some libSDL builds export the
 * very same symbols, so keep ours out of their way. Rename unconditionally:
 * usound >= 2 has no such identifiers, which is what lets us decide everything
 * else *after* the #include, where USOUND_VERSION finally becomes visible.
 */
#ifndef USOUND_COMPAT_INIT
#define USOUND_COMPAT_INIT		USoundCompatInitXbios
#endif
#ifndef USOUND_COMPAT_DEINIT
#define USOUND_COMPAT_DEINIT	USoundCompatDeinitXbios
#endif
#define AtariSoundSetupInitXbios	USOUND_COMPAT_INIT
#define AtariSoundSetupDeinitXbios	USOUND_COMPAT_DEINIT

#include <usound.h>

#ifndef USOUND_VERSION
/*
 * Pre-2 usound.h. Layout of AudioSpec is identical to USoundSpec, only the
 * names differ, so plain typedefs do the job.
 */
typedef AudioFormat	USoundFormat;
typedef AudioSpec	USoundSpec;

#define USoundFormatSigned8			AudioFormatSigned8
#define USoundFormatSigned16LSB		AudioFormatSigned16LSB
#define USoundFormatSigned16MSB		AudioFormatSigned16MSB
#define USoundFormatUnsigned8		AudioFormatUnsigned8
#define USoundFormatUnsigned16LSB	AudioFormatUnsigned16LSB
#define USoundFormatUnsigned16MSB	AudioFormatUnsigned16MSB
#define USoundFormatCount			AudioFormatCount

/*
 * Pre-2 keeps its state in the header itself, so this is a placeholder --
 * pass it around exactly as you would with usound >= 2. Note that it cannot
 * give you the one property the real context adds: with a pre-2 header the
 * state is still per-translation-unit, so init and deinit have to stay in the
 * same file.
 */
typedef struct {
	int unused;
} USoundContext;

#ifndef USOUND_DEF
#ifdef __GNUC__
/* a translation unit doesn't have to use both of them */
#define USOUND_DEF	static __attribute__((__unused__))
#else
#define USOUND_DEF	static
#endif
#endif	/* USOUND_DEF */

USOUND_DEF int USoundInitXbios(const USoundSpec* desired, USoundSpec* obtained, USoundContext* context)
{
	(void)context;
	return AtariSoundSetupInitXbios(desired, obtained);
}

USOUND_DEF int USoundDeinitXbios(USoundContext* context)
{
	(void)context;
	return AtariSoundSetupDeinitXbios();
}

#else
#error "usound.h >= 2 is installed: drop usound_compat.h and #include <usound.h> directly"
#endif	/* !USOUND_VERSION */

#endif	/* USOUND_COMPAT_H */
