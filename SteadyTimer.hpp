#ifndef _STEADY_TIMER_HPP_
#define _STEADY_TIMER_HPP_

#if defined(__unix__) || defined(__linux__)
#elif defined(_WIN32) || defined(_WIN64)
#include <profileapi.h>
class SteadyTimer
{
public:
    LARGE_INTEGER m_frequency;
    bool __getTime(double &time) const
    {
        LARGE_INTEGER counter;
        if(QueryPerformanceCounter(&counter))
        {
            time = static_cast<double>(counter.QuadPart) / m_frequency.QuadPart;
            return true;
        }
        return false;
    }
public:
    double operator()(void) const
    {
        double currentTime;
        return __getTime(currentTime) ? currentTime : 0.0;
    }

    SteadyTimer()
    {
        QueryPerformanceFrequency(&m_frequency);
    }
};
#endif

#endif // _STEADY_TIMER_HPP_