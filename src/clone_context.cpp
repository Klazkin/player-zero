#include <clone_context.h>

std::size_t RefHasher::operator()(const Ref<RefCounted> ref) const
{
    static const size_t shift = (size_t)log2(1 + sizeof(Ref<RefCounted>));
    return (size_t)(*ref) >> shift;
}

bool RefComparator::operator()(const Ref<RefCounted> a, const Ref<RefCounted> b) const // todo check if needed
{
    return a == b;
}