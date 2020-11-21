//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================

#include "ac/timer.h"

#include "core/platform.h"
#if AGS_PLATFORM_DEBUG && defined (__GNUC__)
#include <stdio.h>
#include <execinfo.h>
#include <unistd.h>
#endif
#include <thread>
#include "platform/base/agsplatformdriver.h"

namespace {

const auto MAXIMUM_FALL_BEHIND = 3;

auto tick_duration = std::chrono::microseconds(1000000LL/40);
auto framerate_maxed = false;

auto last_tick_time = AGS_Clock::now();
auto next_frame_timestamp = AGS_Clock::now();

}

std::chrono::microseconds GetFrameDuration()
{
    if (framerate_maxed) {
        return std::chrono::microseconds(0);
    }
    return tick_duration;
}

void setTimerFps(int new_fps) 
{
    tick_duration = std::chrono::microseconds(1000000LL/new_fps);
    framerate_maxed = new_fps >= 1000;

    last_tick_time = AGS_Clock::now();
    next_frame_timestamp = AGS_Clock::now();
}

bool isTimerFpsMaxed()
{
    return framerate_maxed;
}

void WaitForNextFrame()
{
    auto now = AGS_Clock::now();
    auto frameDuration = GetFrameDuration();

    // early exit if we're trying to maximise framerate
    if (frameDuration <= std::chrono::milliseconds::zero()) {
        next_frame_timestamp = now;
        return;
    }

    // jump ahead if we're lagging
    if (next_frame_timestamp < (now - MAXIMUM_FALL_BEHIND*frameDuration)) {
        next_frame_timestamp = now;
    }

    auto frame_time_remaining = next_frame_timestamp - now;
    if (frame_time_remaining > std::chrono::milliseconds::zero()) {
        std::this_thread::sleep_for(frame_time_remaining);
    }
    
    next_frame_timestamp += frameDuration;
}

bool waitingForNextTick() 
{
    auto now = AGS_Clock::now();

    if (framerate_maxed) {
        last_tick_time = now;
        return false;
    }

    auto is_lagging = (now - last_tick_time) > (MAXIMUM_FALL_BEHIND*tick_duration);
    if (is_lagging) {
#if AGS_PLATFORM_DEBUG && defined (__GNUC__)
        auto missed_ticks = ((now - last_tick_time)/tick_duration);
        printf("Lagging! Missed %lld ticks!\n", (long long)missed_ticks);
        void *array[10];
        auto size = backtrace(array, 10);
        backtrace_symbols_fd(array, size, STDOUT_FILENO);
        printf("\n");
#endif
        last_tick_time = now;
        return false;
    }

    auto next_tick_time = last_tick_time + tick_duration;
    if (next_tick_time <= now) {
        last_tick_time = next_tick_time;
        return false;
    }

    return true;
}

void skipMissedTicks() 
{
    last_tick_time = AGS_Clock::now();
    next_frame_timestamp = AGS_Clock::now();
}
