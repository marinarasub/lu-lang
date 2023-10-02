#include "profile.h"

#include "timer.h"
#include "csv.h"
#include "string.h"

namespace lu
{
namespace profile
{

void time(const std::function<void()>& cb, const time_settings& settings, std::ostream& result_os)
{
    stopwatch sw;
    csv result_csv;
    csv::write_settings csv_settings;
    // TODO csv set col width/col align
    result_csv.append({ csv::make_cell("#"), csv::make_cell("N"), csv::make_cell("time (ms)") });
    for (size_t i = 0; i < settings.sizes.size(); ++i)
    {
        size_t n = settings.sizes[i];
        sw.start();
        for (size_t k = 0; k < n; ++k)
        {
            cb();
        }
        stopwatch::DefaultDurationType::rep t_in_s = sw.stop().count();
        result_csv.append({ csv::make_cell(i), csv::make_cell(n), csv::make_cell(t_in_s * 1000.0) });
    }
    result_os << "time " << settings.name << ":\n";
    csv_settings.delim = " | ";
    result_csv.auto_col_widths(csv_settings);
    result_csv.write(result_os, csv_settings);
    result_os.flush();
}

}
}