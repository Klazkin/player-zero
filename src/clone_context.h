#ifndef CLONE_CONTEXT_H
#define CLONE_CONTEXT_H

#include <unordered_map>
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

struct RefHasher
{
    std::size_t operator()(const Ref<RefCounted> ref) const;
};

struct RefComparator
{
    bool operator()(const Ref<RefCounted> a, const Ref<RefCounted> b) const;
};

using CloneContext = std::unordered_map<Ref<RefCounted>, Ref<RefCounted>, RefHasher, RefComparator>;

#endif