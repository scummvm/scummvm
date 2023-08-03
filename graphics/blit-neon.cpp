/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "graphics/blit.h"
#include "graphics/pixelformat.h"

namespace Graphics {

#if defined(__ARM_NEON__) || defined(__ARM_NEON)
template<bool doscale>
void BlendBlit::doBlitBinaryBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitOpaqueBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitMultiplyBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitSubtractiveBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitAdditiveBlendLogicNEON(Args &args) {
    (void)args;
}
template<bool doscale>
void BlendBlit::doBlitAlphaBlendLogicNEON(Args &args) {
    (void)args;
}

#define INSTANTIATE_BLIT_TEMPLATES(ext, b) \
    template void BlendBlit::doBlitBinaryBlendLogic##ext<b>(Args &); \
    template void BlendBlit::doBlitOpaqueBlendLogic##ext<b>(Args &); \
    template void BlendBlit::doBlitMultiplyBlendLogic##ext<b>(Args &); \
    template void BlendBlit::doBlitSubtractiveBlendLogic##ext<b>(Args &); \
    template void BlendBlit::doBlitAdditiveBlendLogic##ext<b>(Args &); \
    template void BlendBlit::doBlitAlphaBlendLogic##ext<b>(Args &);
INSTANTIATE_BLIT_TEMPLATES(NEON, true)
INSTANTIATE_BLIT_TEMPLATES(NEON, false)
#undef INSTANTIATE_BLIT_TEMPLATES
#endif // __ARM_NEON__

}
