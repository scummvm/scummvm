#include <dmg.h>
#include <ddb.h>
#include <ddb_vid.h>
#include <os_lib.h>
#include <os_mem.h>

#if defined(_DOS) && !defined(__386__)
#include <dos.h>
#endif

#ifndef DEBUG_AMIGA_PICTURE_IO
#define DEBUG_AMIGA_PICTURE_IO 1
#endif

#if defined(_UNIX) || defined(_OSX) || defined(_WEB) || defined(_WIN32)
#define DMG_USE_SALVADOR_COMPRESSOR 1
#endif

#if DMG_USE_SALVADOR_COMPRESSOR
extern "C" {
#include <libsalvador.h>
}

// ZX0 format maximum match offset (MAX_OFFSET in Salvador format.h).
// Keep this explicit so generated DAT streams stay compatible with 16-bit offset decoders.
static const size_t DMG_ZX0_MAX_MATCH_OFFSET = 0x7f80;
#endif

#ifndef DMG_USE_ZX0_DOS32_ASM
    #if defined(_DOS) && defined(__386__)
        #define DMG_USE_ZX0_DOS32_ASM 1
    #else
        #define DMG_USE_ZX0_DOS32_ASM 0
    #endif
#endif

namespace
{
#if defined(_AMIGA) || defined(_ATARIST)
    extern "C" void zx0_decompress(const uint8_t* input, uint8_t* output);
    extern "C" void zx0_decompress_fast(const uint8_t* input, uint8_t* output);
    static const uint32_t ZX0_FAST_68K_MAX_OUTPUT = 65535;
#elif defined(_DOS) && defined(__386__) && DMG_USE_ZX0_DOS32_ASM
    static void ZX0_DecompressDOS32(const uint8_t* input, uint8_t* output);
    #pragma aux ZX0_DecompressDOS32 = \
        "push es" \
        "push ds" \
        "pop es" \
        "cld" \
        "mov al,80h" \
        "xor edx,edx" \
        "dec edx" \
    "literals:" \
        "call get_elias" \
        "rep movsb" \
        "add al,al" \
        "jc get_offset" \
        "call get_elias" \
    "copy_match:" \
        "push esi" \
        "mov esi,edi" \
        "add esi,edx" \
        "rep movsb" \
        "pop esi" \
        "add al,al" \
        "jnc literals" \
    "get_offset:" \
        "mov ecx,0feh" \
        "call elias_loop" \
        "inc cl" \
        "je zx0_exit32" \
        "mov dh,cl" \
        "mov ecx,1" \
        "mov dl,[esi]" \
        "inc esi" \
        "stc" \
        "rcr dx,1" \
        "pushfd" \
        "movsx edx,dx" \
        "popfd" \
        "jc got_offs" \
        "call elias_bt" \
    "got_offs:" \
        "inc ecx" \
        "jmp copy_match" \
    "get_elias:" \
        "mov ecx,1" \
    "elias_loop:" \
        "add al,al" \
        "jnz got_bit" \
        "lodsb" \
        "adc al,al" \
    "got_bit:" \
        "jc got_elias" \
    "elias_bt:" \
        "add al,al" \
        "adc ecx,ecx" \
        "jmp elias_loop" \
    "got_elias:" \
        "ret" \
    "zx0_exit32:" \
        "pop es" \
        parm [esi] [edi] \
        modify exact [eax ecx edx esi edi];
#elif defined(_DOS) && !defined(__386__)
    static void ZX0_DecompressDOS16(uint16_t inputSeg, uint16_t inputOff, uint16_t outputSeg, uint16_t outputOff);
    #pragma aux ZX0_DecompressDOS16 = \
        "push ds" \
        "push es" \
        "mov ds,ax" \
        "mov si,bx" \
        "mov es,cx" \
        "mov di,dx" \
        "cld" \
        "mov al,80h" \
        "xor dx,dx" \
        "dec dx" \
    "literals:" \
        "call get_elias" \
        "rep movsb" \
        "add al,al" \
        "jc get_offset" \
        "call get_elias" \
    "copy_match:" \
        "push ds" \
        "push si" \
        "push es" \
        "pop ds" \
        "mov si,di" \
        "add si,dx" \
        "rep movsb" \
        "pop si" \
        "pop ds" \
        "add al,al" \
        "jnc literals" \
    "get_offset:" \
        "mov cl,0feh" \
        "call elias_loop" \
        "inc cl" \
        "je zx0_exit16" \
        "mov dh,cl" \
        "mov cx,1" \
        "mov dl,[si]" \
        "inc si" \
        "stc" \
        "rcr dx,1" \
        "jc got_offs" \
        "call elias_bt" \
    "got_offs:" \
        "inc cx" \
        "jmp copy_match" \
    "get_elias:" \
        "mov cx,1" \
    "elias_loop:" \
        "add al,al" \
        "jnz got_bit" \
        "lodsb" \
        "adc al,al" \
    "got_bit:" \
        "jc got_elias" \
    "elias_bt:" \
        "add al,al" \
        "adc cx,cx" \
        "jmp elias_loop" \
    "got_elias:" \
        "ret" \
    "zx0_exit16:" \
        "pop es" \
        "pop ds" \
        parm [ax] [bx] [cx] [dx] \
        modify exact [ax bx cx dx si di];
#else
    struct ZX0_State
    {
        const uint8_t* input;
        const uint8_t* inputEnd;
        uint8_t* output;
        uint8_t* outputStart;
        uint8_t* outputEnd;
        int bitMask;
        int bitValue;
        bool backtrack;
        int lastByte;
    };

    static inline bool ZX0_ReadByte(ZX0_State& s, int* value)
    {
        if (s.input >= s.inputEnd)
            return false;
        s.lastByte = *s.input++;
        *value = s.lastByte;
        return true;
    }

    static inline bool ZX0_ReadBit(ZX0_State& s, int* value)
    {
        if (s.backtrack)
        {
            s.backtrack = false;
            *value = s.lastByte & 1;
            return true;
        }

        s.bitMask >>= 1;
        if (s.bitMask == 0)
        {
            s.bitMask = 128;
            if (!ZX0_ReadByte(s, &s.bitValue))
                return false;
        }

        *value = (s.bitValue & s.bitMask) ? 1 : 0;
        return true;
    }

    static bool ZX0_ReadInterlacedEliasGamma(ZX0_State& s, bool inverted, int* value)
    {
        int bit;
        int out = 1;

        while (true)
        {
            if (!ZX0_ReadBit(s, &bit))
                return false;
            if (bit)
                break;
            if (!ZX0_ReadBit(s, &bit))
                return false;
            out = (out << 1) | (bit ^ (inverted ? 1 : 0));
        }

        *value = out;
        return true;
    }

    static bool ZX0_WriteByte(ZX0_State& s, int value)
    {
        if (s.output >= s.outputEnd)
            return false;
        *s.output++ = (uint8_t)value;
        return true;
    }

    static bool ZX0_WriteBytes(ZX0_State& s, int offset, int length)
    {
        if (offset <= 0 || s.output - offset < s.outputStart)
            return false;

        while (length-- > 0)
        {
            if (s.output >= s.outputEnd)
                return false;
            *s.output = *(s.output - offset);
            s.output++;
        }
        return true;
    }
#endif
}

bool DMG_DecompressZX0(const uint8_t* data, uint32_t dataLength, uint8_t* buffer, uint32_t outputSize)
{
#if DEBUG_ZX0
    #if defined(_AMIGA) || defined(_ATARIST)
    const char* zx0Path = outputSize <= ZX0_FAST_68K_MAX_OUTPUT ? "fast68k" : "68k";
    #elif defined(_DOS) && defined(__386__) && DMG_USE_ZX0_DOS32_ASM
    const char* zx0Path = "dos32-asm";
    #elif defined(_DOS) && !defined(__386__)
    const char* zx0Path = "dos16-asm";
    #else
    const char* zx0Path = "c";
    #endif
    uint32_t zx0StartMs;
    VID_GetMilliseconds(&zx0StartMs);
    DebugPrintf("DMG_DecompressZX0: begin path=%s in=%lu out=%lu\n",
        zx0Path,
        (unsigned long)dataLength,
        (unsigned long)outputSize);
#endif
#if defined(_AMIGA) || defined(_ATARIST)
    (void)dataLength;
    #if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
    DebugPrintf("DMG_DecompressZX0: begin src=%p dst=%p out=%lu path=%s\n",
        data,
        buffer,
        (unsigned long)outputSize,
        outputSize <= ZX0_FAST_68K_MAX_OUTPUT ? "fast68k" : "68k");
    #endif
    if (outputSize <= ZX0_FAST_68K_MAX_OUTPUT)
        zx0_decompress_fast(data, buffer);
    else
        zx0_decompress(data, buffer);
#if DEBUG_ZX0
    uint32_t zx0EndMs;
    VID_GetMilliseconds(&zx0EndMs);
    DebugPrintf("DMG_DecompressZX0: end path=%s elapsed=%lu ms\n",
        zx0Path,
        (unsigned long)(zx0EndMs - zx0StartMs));
#endif
    #if defined(_AMIGA) && DEBUG_AMIGA_PICTURE_IO
    DebugPrintf("DMG_DecompressZX0: end dst=%p out=%lu\n",
        buffer,
        (unsigned long)outputSize);
    #endif
    return true;
#elif defined(_DOS) && defined(__386__) && DMG_USE_ZX0_DOS32_ASM
    (void)dataLength;
    (void)outputSize;
    ZX0_DecompressDOS32(data, buffer);
#if DEBUG_ZX0
    uint32_t zx0EndMs;
    VID_GetMilliseconds(&zx0EndMs);
    DebugPrintf("DMG_DecompressZX0: end path=%s elapsed=%lu ms\n",
        zx0Path,
        (unsigned long)(zx0EndMs - zx0StartMs));
#endif
    return true;
#elif defined(_DOS) && !defined(__386__)
    (void)dataLength;
    (void)outputSize;
    ZX0_DecompressDOS16(FP_SEG(data), FP_OFF(data), FP_SEG(buffer), FP_OFF(buffer));
#if DEBUG_ZX0
    uint32_t zx0EndMs;
    VID_GetMilliseconds(&zx0EndMs);
    DebugPrintf("DMG_DecompressZX0: end path=%s elapsed=%lu ms\n",
        zx0Path,
        (unsigned long)(zx0EndMs - zx0StartMs));
#endif
    return true;
#else
    #if DEBUG_ZX0
    #define ZX0_RETURN_C(ok) do { \
        bool _zx0Ok = (ok); \
        uint32_t zx0EndMs; \
        VID_GetMilliseconds(&zx0EndMs); \
        DebugPrintf("DMG_DecompressZX0: end path=%s elapsed=%lu ms result=%s\n", \
            zx0Path, \
            (unsigned long)(zx0EndMs - zx0StartMs), \
            _zx0Ok ? "ok" : "fail"); \
        return _zx0Ok; \
    } while (0)
    #else
    #define ZX0_RETURN_C(ok) return (ok)
    #endif

    ZX0_State state;
    state.input = data;
    state.inputEnd = data + dataLength;
    state.output = buffer;
    state.outputStart = buffer;
    state.outputEnd = buffer + outputSize;
    state.bitMask = 0;
    state.bitValue = 0;
    state.backtrack = false;
    state.lastByte = 0;

    int lastOffset = 1;
    int length = 0;
    int bit = 0;
    int value = 0;

COPY_LITERALS:
    if (!ZX0_ReadInterlacedEliasGamma(state, false, &length))
        ZX0_RETURN_C(false);
    if (state.inputEnd - state.input < length || state.outputEnd - state.output < length)
        ZX0_RETURN_C(false);
    if (length > 0)
    {
        MemCopy(state.output, state.input, length);
        state.lastByte = state.input[length - 1];
        state.input += length;
        state.output += length;
    }
    if (state.output >= state.outputEnd)
        ZX0_RETURN_C(true);
    if (!ZX0_ReadBit(state, &bit))
        ZX0_RETURN_C(false);
    if (bit)
        goto COPY_FROM_NEW_OFFSET;

//COPY_FROM_LAST_OFFSET:
    if (!ZX0_ReadInterlacedEliasGamma(state, false, &length))
        ZX0_RETURN_C(false);
    if (lastOffset <= 0 || state.output - lastOffset < state.outputStart ||
        state.outputEnd - state.output < length)
        ZX0_RETURN_C(false);
    if (lastOffset == 1)
    {
        MemSet(state.output, *(state.output - 1), length);
        state.output += length;
    }
    else
    {
        while (length-- > 0)
        {
            *state.output = *(state.output - lastOffset);
            state.output++;
        }
    }
    if (state.output >= state.outputEnd)
        ZX0_RETURN_C(true);
    if (!ZX0_ReadBit(state, &bit))
        ZX0_RETURN_C(false);
    if (!bit)
        goto COPY_LITERALS;

COPY_FROM_NEW_OFFSET:
    if (!ZX0_ReadInterlacedEliasGamma(state, true, &value))
        ZX0_RETURN_C(false);
    if (value == 256)
        ZX0_RETURN_C(state.output == state.outputEnd && state.input == state.inputEnd);
    if (!ZX0_ReadByte(state, &bit))
        ZX0_RETURN_C(false);
    lastOffset = value * 128 - (bit >> 1);
    state.backtrack = true;
    if (!ZX0_ReadInterlacedEliasGamma(state, false, &length))
        ZX0_RETURN_C(false);
    length++;
    if (lastOffset <= 0 || state.output - lastOffset < state.outputStart ||
        state.outputEnd - state.output < length)
        ZX0_RETURN_C(false);
    if (lastOffset == 1)
    {
        MemSet(state.output, *(state.output - 1), length);
        state.output += length;
    }
    else
    {
        while (length-- > 0)
        {
            *state.output = *(state.output - lastOffset);
            state.output++;
        }
    }
    if (state.output >= state.outputEnd)
        ZX0_RETURN_C(true);
    if (!ZX0_ReadBit(state, &bit))
        ZX0_RETURN_C(false);
    if (bit)
        goto COPY_FROM_NEW_OFFSET;
    goto COPY_LITERALS;

    #undef ZX0_RETURN_C
#endif
}

uint8_t* DMG_CompressZX0(const uint8_t* data, uint32_t dataLength, uint32_t* outputSize)
{
#if !DMG_USE_SALVADOR_COMPRESSOR
    (void)data;
    (void)dataLength;
    if (outputSize)
        *outputSize = 0;
    return 0;
#else
    const uint32_t maxCompressedSize = (uint32_t)salvador_get_max_compressed_size((size_t)dataLength);
    uint8_t* compressed = Allocate<uint8_t>("ZX0 compressed", maxCompressedSize, false);
    if (compressed == 0)
    {
        if (outputSize)
            *outputSize = 0;
        return 0;
    }

    salvador_stats stats;
    const size_t compressedSize = salvador_compress(
        data,
        compressed,
        (size_t)dataLength,
        (size_t)maxCompressedSize,
        FLG_IS_INVERTED,
        DMG_ZX0_MAX_MATCH_OFFSET,
        0,
        0,
        &stats);

    if (compressedSize == (size_t)-1)
    {
        Free(compressed);
        if (outputSize)
            *outputSize = 0;
        return 0;
    }

    if ((size_t)stats.max_offset > DMG_ZX0_MAX_MATCH_OFFSET)
    {
        Free(compressed);
        if (outputSize)
            *outputSize = 0;
        return 0;
    }

    if (outputSize)
        *outputSize = (uint32_t)compressedSize;
    return compressed;
#endif
}
