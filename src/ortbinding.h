#ifndef ORTBinding_H
#define ORTBinding_H

#include <godot_cpp/classes/object.hpp>

float _predict(std::array<float, 12> input);

namespace godot
{

    class ORTBinding : public Object
    {
        GDCLASS(ORTBinding, Object);

    protected:
        static void _bind_methods();

    public:
        float predict(const TypedArray<float> &p_array);

        ORTBinding();
        ~ORTBinding();
    };
}
#endif