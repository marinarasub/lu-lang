#ifndef LU_CSV_H
#define LU_CSV_H

#include <iostream>
#include <type_traits>
#include <utility>
#include <sstream>

#include "adt/vector.h"
#include "string.h"

namespace lu
{
class csv
{
public:
    struct write_settings
    {
        static const string_view COMMA_DELIM;
        static const string_view TAB_DELIM;
        static const string_view SPACE_DELIM;
        static const string_view DEFAULT_DELIM;

        static const size_t NO_COL_WIDTH;

        write_settings() : delim(DEFAULT_DELIM), col_widths(), pad_char(' ') {}

        string_view delim;
        vector<size_t> col_widths; // col_widths[i] = width for column i
        string::CharT pad_char;
        // TODO settings struct
    };



    csv() : _n_cols(0) {}
            
    ~csv() {}

    enum data_type
    {
        EMPTY,
        INT,
        REAL,
        TEXT, // TODO dates/times/currency/fractions etc if needed
    };

    struct cell
    {
        cell() : _type(EMPTY), _int_data(0) {}

        cell(cell&& other);
        cell(const cell& other);

        void clear();
        
        ~cell();

        data_type type() const { return _type; }

        template <typename T>
        void operator<<(const T& val)
        {
            if (_type != TEXT)
            {
                clear();
                _type = TEXT;
                new (&_str_data) std::stringstream();
            }
            // TODO assert good stream
            _str_data << val;
        }

        // helper for partial spec TODO destroy old type if not new type.

        template <typename T, typename = void>
        struct cell_set
        {
            // template <typename T>
            static void write(cell& c, const T& val)
            {
                if (c._type != TEXT)
                {
                    c.clear();
                    c._type = TEXT;
                    new (&c._str_data) std::stringstream();
                }
                c._str_data.str(""); // clear contents
                c._str_data.clear(); // clear flags
                c._str_data << val;
            }
        };

        template <typename T>
        struct cell_set<T, typename std::enable_if<std::is_integral<T>::value, void>::type>
        {
            static void write(cell& c, T val)
            {
                if (c._type != INT)
                {
                    c.clear();
                    c._type = INT;
                }
                c._int_data = static_cast<int64_t>(val); // TODO uint data
            }
        };

        template <typename T>
        struct cell_set<T, typename std::enable_if<std::is_floating_point<T>::value, void>::type>
        {
            static void write(cell& c, T val)
            {
                if (c._type != REAL)
                {
                    c.clear();
                    c._type = REAL;
                }
                c._float_data = val;
            }
        };

        /////////     ///////////      //////////

        template <data_type>
        struct cell_value {};

        template <>
        struct cell_value<INT>
        {
            static int64_t value(const cell& c)
            {
                return c._int_data;
            }
        };

        template <>
        struct cell_value<REAL>
        {
            static long double value(const cell& c)
            {
                return c._float_data;
            }
        };

        template <>
        struct cell_value<TEXT>
        {
            static std::stringstream& value(cell& c)
            {
                return c._str_data;
            }
        };

        template <data_type> friend struct cell_value;
        template <typename, typename> friend struct cell_set;

        ///////////////////////

        template <typename T>
        cell& operator=(const T& val)
        {
            // assert type < 64 signed?
            cell_set<T>::write(*this, val);
            return *this;
        }

        template <data_type DT>
        auto value() -> decltype(cell_value<DT>::value(*this))
        {
            // assert type < 64 signed?
            return cell_value<DT>::value(*this);
        }


        string get_string() const; // hm maybe just have spreadsheet do printing and not delegate to cell

        //string read(std::istream& is); // TODO

    private:
        data_type _type;
        union
        {
            int64_t _int_data;
            long double _float_data;
            std::stringstream _str_data;
        };
    };

    struct row
    {
        row() {}

        row(row&& other) noexcept : _cols(std::move(other._cols)) {}

        row(const std::initializer_list<cell>& cells);

        cell& operator[](size_t idx) { return _cols[idx]; }
        const cell& operator[](size_t idx) const { return _cols[idx]; }

        void clear() { _cols.clear(); }

        row& append(cell&& c);
        row& append(const cell& c);
        
        void write(std::ostream& os, const write_settings&) const;

        void read(std::istream& is); // TODO

        size_t size() const { return _cols.size(); }

        void resize(size_t n) { return _cols.resize(n); }
    private:
        void write_cell(std::ostream&os, const write_settings&, size_t col_idx) const;

        // TODO optimize for sparsity?
        std::vector<cell> _cols;
    };


    template <typename T>
    static cell make_cell(const T& val)
    {
        cell c;
        c = val;
        return c;
    }

    void write(std::ostream& os, const write_settings&) const;

    void read(std::istream& is); // TODO

    void clear();

    void resize(size_t n) { _rows.resize(n); }

    row& operator[](size_t idx);

    cell& operator()(size_t row, size_t col);

    csv& append(row&& row);

    size_t rows() const { return _rows.size(); }

    size_t cols() const { return _n_cols; }

    void auto_col_widths(write_settings&) const;

private:
    void probe(size_t row_idx, size_t col_idx);

    size_t max_col_width(size_t col_idx) const;

    size_t _n_cols;
    std::vector<row> _rows;
};
}

#endif // LU_CSV_H
