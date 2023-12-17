#ifndef ORTBinding_H
#define ORTBinding_H

#include <godot_cpp/classes/object.hpp>

void _procedure();

namespace godot
{

    class ORTBinding : public Object
    {
        GDCLASS(ORTBinding, Object);

    protected:
        static void _bind_methods();

    public:
        void dummy_method();

        ORTBinding();
        ~ORTBinding();
    };
}
#endif