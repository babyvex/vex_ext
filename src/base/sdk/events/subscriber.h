//
// Created by natty on 15.11.2026.
//

#ifndef VEX_EVENT_SYSTEM_H
#define VEX_EVENT_SYSTEM_H
#include <cstdint>

#include "mJNI/memory/Memory.h"

struct call_event {
    uint64_t total_calls   { 0 };
    uint64_t last_tsc      { 0 };
    double   since_prev_us { 0.0 };

};
class hi_res_timer_scope {
public:
    explicit hi_res_timer_scope(UINT period_ms = 1) : period_(period_ms) {
        timeBeginPeriod(period_);
    }
    ~hi_res_timer_scope() { timeEndPeriod(period_); }

    hi_res_timer_scope(const hi_res_timer_scope&)            = delete;
    hi_res_timer_scope& operator=(const hi_res_timer_scope&) = delete;
private:
    UINT period_;
};


class subscriber {

};

#endif //VEX_EVENT_SYSTEM_H
