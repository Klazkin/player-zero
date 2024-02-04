#ifndef SURFACE_H
#define SURFACE_H

#include <map>
#include <vector>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "surface_element.h"
#include "unit.h"

namespace godot
{

    class Surface : public RefCounted
    {
        GDCLASS(Surface, RefCounted)

    private:
        std::map<Vector2i, Ref<SurfaceElement>> element_positions;

    protected:
        static void _bind_methods();

    public:
        Surface();
        ~Surface();

        PackedVector3Array get_shortest_path(const Vector2i path_start, const Vector2i path_end);
        Vector2i get_ray_collision(const Vector2i ray_start, const Vector2i ray_end);

        bool is_position_available(const Vector2i &p_pos) const; // TODO is there any point to pass Vector2i as ref?
        void place_element(const Vector2i &p_pos, const Ref<SurfaceElement> p_element);
        bool move_element(const Vector2i &p_pos_from, const Vector2i &p_pos_to);

        Ref<SurfaceElement> get_element(const Vector2i &p_pos) const;
        Ref<SurfaceElement> lift_element(const Vector2i &p_pos);

        TypedArray<Unit> get_only_units() const; // bad implementation pattern
    };

}

#endif