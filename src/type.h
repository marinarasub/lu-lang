#ifndef LU_TYPE_H
#define LU_TYPE_H

#include <cstddef>
#include <cstdint>
#include "string.h"
#include "expr.h"
#include "utility.h"
#include "adt/array.h"
#include "adt/vector.h"
#include "adt/set.h"
#include "adt/map.h"

// TODO
struct intermediate_expr;

namespace lu
{
using type_idx = size_t;

enum type_class : uint64_t
{
    VOID, //termianl//
    UNDEFINED, // terminal
    LITERAL, // terminal, stored as a string, can be converted.
    BUILTIN, // terminal
    //INTRINSIC, // terminal TODO
    FUNCTION, // 2 member
    INTRINSIC, // 2 member
    POINTER, // 1 member, TODO
    TUPLE, // n members
    STRUCT, // n members
    UNION, // n members
    INTERSECT, // n members

    _label_LAST = INTERSECT,
};

enum class builtin_type : uint64_t
{
    _label_FIRST,

    //POINTER,
    //INTRINSIC,//_FUNCTION, // * to function
    //VOID,
    BOOL,
    INT8,
    INT16,
    INT32,
    INT64,
    UINT8,
    UINT16,
    UINT32,
    UINT64,
    FLOAT16,
    FLOAT32,
    FLOAT64,
    //ASCII_STRING, // TODO temp since convinient
    BYTE,
    ASCII,
    UNICODE,
    TYPEID,

    _label_LAST = TYPEID,
};

// literal type does not "exist" - ie a variable cannot ahve a literal type as a value, but it can be converted to a real type: it is stored as string until converted.
enum class literal_type : uint64_t
{
    _label_FIRST,

    STRING,
    INTEGER,
    DECIMAL,
    TRUE,
    FALSE,
    EMPTY_TUPLE,

    _label_LAST = EMPTY_TUPLE,
};

literal_type expr_kind_to_literal_type(expr::expr_kind kind);

// TODO array type, pinter type

//(a type id)
struct type_id
{
    const static type_id UNDEFINED;
    const static type_id VOID;

    type_id(type_class tclass, type_idx id) : tclass(tclass), idx(id) {}
    type_id() : tclass(type_class::UNDEFINED), idx(0) {}

    bool is(type_class) const;
    template <typename ...RestT>
    bool is(type_class type, RestT... rest) const { return is(type) || is(rest...); }

    type_class tclass;
    type_idx idx;
};

constexpr bool operator==(const type_id& lhs, const type_id& rhs)
{
    return lhs.tclass == rhs.tclass && lhs.idx == rhs.idx;
}

constexpr bool operator!=(const type_id& lhs, const type_id& rhs)
{
    return !(lhs == rhs);
}

constexpr bool operator<(const type_id& lhs, const type_id& rhs)
{
    return (lhs.tclass != rhs.tclass) ? (lhs.tclass < rhs.tclass) : (lhs.idx < rhs.idx);
}

struct intrinsic_type
{
    enum param_config
    {
        NONE,
        DEST_ONLY,
        OP_ONLY,
        BOTH,
    };

    LU_CONSTEXPR static size_t DEST_PARAM = 0;
    LU_CONSTEXPR static size_t OP_PARAM = 1;

    intrinsic_type(param_config config, type_id tid1, type_id tid2) : config(config), params{tid1, tid2}
    {}

    param_config config;
    type_id params[2];

    type_id operator[](size_t idx) const;

    size_t param_count() const
    {
        if (config == NONE)
        {
            return 0;
        }
        if (config == DEST_ONLY || config == OP_ONLY)
        {
            return 1;
        }
        else
        {
            return 2;
        }
    }

    bool operator<(const intrinsic_type& other) const;
};

struct keyword_type
{
    keyword_type() {}
    keyword_type(type_id tid) : tid(tid), name("") {}
    keyword_type(type_id tid, string_view sname) : tid(tid), name(sname) {}

    keyword_type(keyword_type&& other) : tid(move(other.tid)), name(move(other.name)) {}
    keyword_type(const keyword_type& other) : tid((other.tid)), name((other.name)) {}

    keyword_type& operator=(keyword_type&& other)
    {
        this->tid = move(other.tid);
        this->name = move(other.name);
        return *this;
    }

    keyword_type& operator=(const keyword_type& other)
    {
        this->tid = (other.tid);
        this->name = (other.name);
        return *this;
    }

    type_id tid;
    string name;
    //intermediate_expr* dflt; //TODO deafult shouldn't be parse_expr so what should it be?

    bool operator<(const keyword_type& other) const;
    bool operator==(const keyword_type& other) const;
    bool operator!=(const keyword_type& other) const;
};

struct tuple_type
{   
    using member = keyword_type;

    tuple_type() {}
    tuple_type(array<member>&& mems) : members(move(mems)) {}
    
    tuple_type(tuple_type&& other) : members(move(other.members)) {}
    tuple_type(const tuple_type& other) : members(other.members) {}

    tuple_type& operator=(tuple_type&& other) { this->members = move(other.members); return *this; }
    tuple_type& operator=(const tuple_type& other) { this->members = other.members; return *this; }
    //tuple_type& push(member&&);

    size_t arity() const { return members.size(); }

    member& operator[](size_t idx) { return members[idx]; }
    const member& operator[](size_t idx) const { return members[idx]; }

    bool operator<(const tuple_type&) const;

    array<member> members; // ordered, check if name alr exists on creation
};

struct function_type
{
    using param = keyword_type;

    function_type(type_id ret, array<param>&& params) : ret(ret), params(move(params)) {}

    type_id ret; // TODO also keyword?
    array<param> params;

    param& operator[](size_t idx) { return params[idx]; }
    const param& operator[](size_t idx) const { return params[idx]; }

    size_t param_count() const { return params.size(); }

    bool operator<(const function_type&) const; // TODO!!! some of the comparisons might be broken since i did && across for <
};

// struct intrinsic_type
// {
//     string name;
//     type_id tid;

//     bool operator<(const intrinsic_type&) const;
// };

struct union_type
{
    union_type& operator |(type_id);

    flat_set<type_id> types; // order doesn't really matter

    size_t size() const { return types.size(); }

    bool operator<(const union_type&) const;
};


struct type
{
    //template <typename... ArgsT> static type emplace_literal_type(ArgsT&&... args) { return create_literal_type(forward<ArgsT...>(args)...); }
    //template <typename... ArgsT> static type emplace_builtin_type(ArgsT&&... args) { return create_builtin_type(forward<ArgsT...>(args)...); }
    template <typename... ArgsT> static type emplace_function_type(ArgsT&&... args) { return create_function_type(function_type(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static type emplace_tuple_type(ArgsT&&... args) { return create_tuple_type(tuple_type(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static type emplace_union_type(ArgsT&&... args) { return create_union_type(union_type(forward<ArgsT>(args)...)); }
    template <typename... ArgsT> static type emplace_intrinsic_type(ArgsT&&... args) { return create_intrinsic_type(intrinsic_type(forward<ArgsT>(args)...)); }

    static type create_void_type();
    static type create_literal_type(literal_type&&);
    static type create_builtin_type(builtin_type&&);
    static type create_function_type(function_type&&);
    static type create_tuple_type(tuple_type&&);
    static type create_union_type(union_type&&);
    static type create_intrinsic_type(intrinsic_type&&);

    type();
    type(type&&);
    type(const type&);
    ~type();

    type& operator=(type&&);
    type& operator=(const type&);
    bool operator<(const type&) const;

    //type_id 
    bool callable() const;
    // if callable, supports:
    type_id return_type() const;
    size_t param_count() const;
    type_id param_type(size_t idx) const;

    type_class tclass;
    union
    {
        // UNDEFINED, // terminal
        literal_type lit;// LITERAL, // terminal, stored as a string, can be converted.
        builtin_type bin;// BUILTIN, // terminal
        intrinsic_type intr;
        function_type fun;// FUNCTION, // terminal
        tuple_type tup;// TUPLE, // n members
        // STRUCT, // n members
        union_type un;// UNION, // n members
        // INTERSECT, // n members
    };

private:
    bool less_than_cmp(const type& other) const;
    type& create(type&& other);
    type& create(const type& other);
    type& assign(type&& other);
    type& assign(const type& other);
    void destroy();
    
};

string to_string(const type_id&);
string to_string(literal_type);
string to_string(builtin_type);

string eng_us_name(builtin_type);
// string to_string(const tuple_type&);
// string to_string(const function_type&);
// string to_string(const union_type&);

// bimap
struct type_registry
{
public:
    static constexpr type_idx INVALID_ID = std::numeric_limits<type_idx>::max();
    
    type_registry();

    type& find_type(type_id);
    const type& find_type(type_id) const;
    
    type_id find_type_id(const type&) const;
    type_id find_type_id_auto_register(const type&); // reguster if not found and not basic type (literal, builtin, intrinsic)
    type_id register_type(const type&);

    type_id find_void_type() const;
    type_id find_builtin_type_id(builtin_type) const;
    type_id find_literal_type_id(literal_type) const;

    bool exists(type_id) const;
    bool exists(const type&) const;
    // literal_type& find_literal_info(type_idx);
    // builtin_type& find_builtin_info(type_idx);
    // function_type& find_function_info(type_idx);
    // //struct_type& find_struct_info(type_idx);
    // tuple_type& find_tuple_info(type_idx);
    // union_type& find_union_info(type_idx);

    // type_id find_literal(const literal_type&);
    // type_id find_builtin(const builtin_type&);
    // type_id find_function(const function_type&);
    // //type_idx find_struct(const struct_type&);
    // type_id find_tuple(const tuple_type&);
    // type_id find_union(const union_type&);

    // // FIND will registrer tuype if not found.
    // type_id register_literal_info(literal_type);
    // type_id register_builtin_info(builtin_type);
    // type_id register_function_info(function_type&&);
    // //type_idx register_struct(struct_type&&);
    // type_id register_union_info(union_type&&);

    // bool exists(type_id) const;

    string tyname(type_id) const;
    string tyname(const type&) const;

    void merge(const type_registry&);

private:
    void check_exists(type_id);
    void check_subtypes(const type&);

    vector<type> _id2t_map[type_class::_label_LAST + 1]; // [type_class][type_idx]
    map<type, type_id> _t2id_map;
    // vector<literal_type> _l_map;
    // vector<builtin_type> _b_map;
    // vector<function_type> _f_map;
    // vector<tuple_type> _t_map;
    // //vector<struct_type> _s_map;
    // vector<union_type> _u_map;

    // map<literal_type, type_id> _l_id_map;
    // map<builtin_type, type_id> _b_id_map;
    // map<function_type, type_id> _f_id_map;
    // map<tuple_type, type_id> _t_id_map;
    // map<union_type, type_id> _u_id_map;
};

// // registered minimum supported types
// void register_void(type_registry&);
// void register_literal_types(type_registry&);
// void register_builtin_types(type_registry&);

}

#endif // LU_TYPE_H
