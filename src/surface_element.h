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
        Vector2i position;

    protected:
        static void _bind_methods();

    public:
        SurfaceElement();
        ~SurfaceElement();

        virtual int hit(int damage);

        virtual void kill();

        bool get_is_on_surface() const;
        void set_is_on_surface(const int p_is_on_surface);
        Vector2i get_position() const;
        void set_position(const Vector2i p_position);

        virtual bool _is_unit() const;
    };

}

#endif