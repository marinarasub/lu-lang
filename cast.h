#ifndef LU_CAST_H
#define LU_CAST_H

#include "adt/vector.h"
#include "adt/map.h"
#include "flag.h"
#include "type.h"
#include "value.h"

namespace lu
{

enum class cast_flag : uint32_t
{
    STATIC = (1 << 0),
    // INTRINSIC = (1 << 2),
};

struct cast_node;
struct cast_edge;

using cast_node_id = size_t;

typedef intermediate_value(*cast_function)(const intermediate_value&);

// uindirectional
struct cast_graph
{
    cast_node_id push(cast_node&&);
    cast_function search(cast_node_id start, size_t max_steps);
private:
    map<type_id, cast_node_id> _tid2node;
    vector<cast_node> _nodes; // id -> node
};

// graph using adj list to check if castable from certain type
struct cast_node
{
    void add_edge(size_t cost, flags<cast_flag> flags, cast_function func, cast_node_id to);

    type_id tid;
    vector<cast_edge> edges;
};

struct cast_edge
{
    size_t cost;
    flags<cast_flag> flags;
    cast_function cast;
    cast_node_id from;
    cast_node_id to;
};

// builtin casts
intermediate_value literal_to_builtin_cast(const type_registry&, type_id to, const intermediate_value&);

}


#endif // LU_CAST_H
