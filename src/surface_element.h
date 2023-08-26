#ifndef SURFACE_ELEMENT_H
#define SURFACE_ELEMENT_H

#include <godot_cpp/classes/object.hpp>

namespace godot
{

    class SurfaceElement : public Object
    {
        GDCLASS(SurfaceElement, Object)

    protected:
        static void _bind_methods();

    public:
        SurfaceElement();
        ~SurfaceElement();
    };

}

#endif