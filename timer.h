#ifndef LU_TIMER_H_
#define LU_TIMER_H_


#include <chrono>
#include <ratio>


namespace lu
{
/**
    * \brief The Stopwatch class provides stopwatch funcitonality for measuring time intervals
    */
class stopwatch
{
public:

    /**
        * \brief Create a stopwatch object
        *
        */
    stopwatch();

    /**
        * \brief Reset the stopwatch state to default state
        *
        */
    void reset();

    /**
        * \brief Start the timer, marks time when called as start time
        *
        */
    void start();

    using DefaultDurationType = std::chrono::duration<double, std::ratio<1>>;

    /**
        * \brief Lap the stopwatch
        *
        * \tparam Duration A std::duration<Rep, Period> type
        * \return Duration lap time in as Duration type
        */
    template <class Duration = DefaultDurationType>
    Duration lap()
    {
        using namespace std::chrono;

        Duration elapsed;
        if (running)
        {
            steady_clock::time_point now_point = steady_clock::now();
            elapsed = std::chrono::duration_cast<Duration>(now_point - last_lap_point);
            last_lap_point = now_point;
        }
        else
        {
            elapsed = std::chrono::duration_cast<Duration>(stop_point - last_lap_point);
        }
        return elapsed;
    }

    // TODO this is not reallya split,
    /**
        * \brief Split time since last lap
        *
        * \tparam Duration
        * \return Duration
        */
    template <class Duration = DefaultDurationType>
    Duration split() const
    {
        using namespace std::chrono;

        Duration elapsed;
        if (running)
        {
            steady_clock::time_point now_point = steady_clock::now();
            elapsed = std::chrono::duration_cast<Duration>(now_point - last_lap_point);
        }
        else
        {
            elapsed = std::chrono::duration_cast<Duration>(stop_point - last_lap_point);
        }
        return elapsed;
    }

    /**
        * \brief Query the current running time (split) without modifying the stopwatch
        *
        * \tparam Duration
        * \return Duration
        */
    template <class Duration = DefaultDurationType>
    Duration time() const
    {
        using namespace std::chrono;

        Duration elapsed;
        if (running)
        {
            steady_clock::time_point now_point = steady_clock::now();
            elapsed = std::chrono::duration_cast<Duration>(now_point - start_point);
        }
        else
        {
            elapsed = std::chrono::duration_cast<Duration>(stop_point - start_point);
        }
        return elapsed;
    }

    /**
        * \brief Stop the stopwatch. Any subsequent calls to query time will not include time after a call to stop()
        * \note May not be resumed with start(), which resets the stopwatch
        *
        * \tparam Duration
        * \return Duration
        */
    template <class Duration = DefaultDurationType>
    Duration stop()
    {
        using namespace std::chrono;

        if (running)
        {
            stop_point = steady_clock::now();
        }
        running = false;
        return time<Duration>();
    }

private:

    std::chrono::steady_clock::time_point start_point;
    std::chrono::steady_clock::time_point last_lap_point;
    std::chrono::steady_clock::time_point stop_point;
    bool running;

};

// TODO
// pretty print times
// template <class T, class Period = std::ratio<1>>
// std::string timeFormat(const T& value) // isoTime
// {
//     //std::milli;
//     constexpr 
// }

}


#endif // LU_TIMER_H_