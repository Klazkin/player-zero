#ifndef SURFACE_ELEMENT_H
#define SURFACE_ELEMENT_H

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot
{

    class SurfaceElement : public RefCounted
    {
        GDCLASS(SurfaceElement, RefCounted)

    private:
        bool is_on_surface;

    protected:
        static void _bind_methods();

    public:
        SurfaceElement();
        ~SurfaceElement();

        int hit(int damage) const;

        void kill();

        bool get_is_on_surface() const;
        void set_is_on_surface(const int p_is_on_surface);

        virtual bool _is_unit() const;
    };

}

#endif