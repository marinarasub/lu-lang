#include "csv.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <type_traits>
#include <algorithm>
#include <cassert>
#include <utility>
#include <sstream>

#include "utility.h"
#include "string.h"
#include "print.h"

namespace lu
{

const string_view csv::write_settings::COMMA_DELIM = ",";
const string_view csv::write_settings::TAB_DELIM = "\t";
const string_view csv::write_settings::SPACE_DELIM = " ";
const string_view csv::write_settings::DEFAULT_DELIM = COMMA_DELIM;

const size_t csv::write_settings::NO_COL_WIDTH = 0;

csv::cell::cell(cell&& other) : _type(other._type)
{
    switch (_type)
    {
    case lu::csv::EMPTY:
        break;
    case lu::csv::INT:
        _int_data = move(other._int_data);
        break;
    case lu::csv::REAL:
        _float_data = move(other._float_data);
        break;
    case lu::csv::TEXT:
        new (&_str_data) std::stringstream();
        _str_data = move(other._str_data);
        break;
    default:
        assert("WTF?");
    }
}

csv::cell::cell(const csv::cell& other) : _type(other._type)
{
    switch (_type)
    {
    case lu::csv::EMPTY:
        break;
    case lu::csv::INT:
        _int_data = (other._int_data);
        break;
    case lu::csv::REAL:
        _float_data = (other._float_data);
        break;
    case lu::csv::TEXT:
        new (&_str_data) std::stringstream();
        _str_data << (other._str_data).rdbuf();
        break;
    default:
        assert("WTF?");
    }
}

csv::row::row(const std::initializer_list<csv::cell>& cells)
{
    for (const cell& c : cells) // cannot move - maybe don't use then
    {
        append(c);
    }
}

void csv::cell::clear()
{
    switch (_type)
    {
    case lu::csv::EMPTY:
        break;
    case lu::csv::INT:
        break;
    case lu::csv::REAL:
        break;
    case lu::csv::TEXT:
        _str_data.~basic_stringstream();
        break;
    default:
        assert("WTF?");
    }
}

csv::cell::~cell()
{
    clear();
}

size_t calc_pad(string_view sv, size_t fit_width)
{
    size_t width = sv.size(); // TODO printabe size
    return width > fit_width ? 0 : fit_width - width;
}

string csv::cell::get_string() const
{
    string str;
    switch (_type)
    {
    case lu::csv::EMPTY:
        break;
    case lu::csv::INT:
        str = to_string(_int_data);
        break;
    case lu::csv::REAL:
        str = to_string(_float_data);
        break;
    case lu::csv::TEXT:
        str = string("\"").append(_str_data.str().c_str()).append("\"");
        break;
    default:
        assert("WTF?");
    }
    return str;
}

csv::row& csv::row::append(csv::cell&& c)
{
    _cols.push_back(move(c));
    return *this;
}

csv::row& csv::row::append(const csv::cell& c)
{
    _cols.push_back(c);
    return *this;
}

void csv::row::write_cell(std::ostream& os, const write_settings& settings, size_t col_idx) const
{
    string str = _cols[col_idx].get_string();
    if ((col_idx < settings.col_widths.size()) && (settings.col_widths[col_idx] != settings.NO_COL_WIDTH))
    {
        str = pad(str, calc_pad(str, settings.col_widths[col_idx]), 0, settings.pad_char);
    }
    os << str;
}

void csv::row::write(std::ostream& os, const write_settings& settings) const
{
    if (_cols.size() > 1)
    {   
        size_t col_idx;
        for (col_idx = 0; col_idx < _cols.size() - 1; ++col_idx)
        {
            write_cell(os, settings, col_idx);
            os << settings.delim;
        }
        // last one without delim
        write_cell(os, settings, col_idx);
    }
    os << "\n";
}

void csv::write(std::ostream& os, const write_settings& settings) const
{
    for (const row& row : _rows)
    {
        row.write(os, settings);
    }
}

void csv::clear()
{
    _rows.clear();
    _n_cols = 0;
}

csv::row& csv::operator[](size_t idx)
{
    return _rows[idx];
}

csv::cell& csv::operator()(size_t row, size_t col)
{
    probe(row, col);
    return _rows[row][col];
}

csv& csv::append(row&& row)
{
    _rows.push_back(std::move(row));
    _n_cols = std::max(_n_cols, _rows.back().size());
    return *this;
}

void csv::auto_col_widths(write_settings& settings) const
{
    if (settings.col_widths.size() < _n_cols)
    {
        settings.col_widths.resize(_n_cols, settings.NO_COL_WIDTH);
    }
    for (size_t i = 0; i < settings.col_widths.size(); ++i)
    {
        settings.col_widths[i] = max_col_width(i);
    }
}

size_t csv::max_col_width(size_t col_idx) const
{
    size_t res = 0;
    for (size_t j = 0; j < _rows.size(); ++j)
    {
        if (col_idx < _rows[j].size())
        {
            size_t cw = _rows[j][col_idx].get_string().size(); // TODO printable wdith
            res = std::max(res, cw);
        }
    }
    return res;
}

void csv::probe(size_t row_idx, size_t col_idx)
{
    if (_rows.size() <= row_idx)
    {
        _rows.resize(row_idx + 1);
    }
    row& row = _rows[row_idx];
    if (row.size() <= col_idx)
    {
        row.resize(col_idx + 1);
    }
    _n_cols = std::max(_n_cols, col_idx + 1);
}


}