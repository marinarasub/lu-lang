#include "timer.h"

#include <chrono>


namespace lu
{

stopwatch::stopwatch()
    : running(false)
{
    reset();
}

void stopwatch::reset()
{
    using namespace std::chrono;

    last_lap_point = steady_clock::now();
    start_point = last_lap_point;
    stop_point = start_point;
}

void stopwatch::start()
{
    reset();
    running = true;
}

}
