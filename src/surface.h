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

    class LosCheckResult : public RefCounted
    {
        GDCLASS(LosCheckResult, RefCounted)

    private:
        Vector2i collided_at;
        Ref<SurfaceElement> element;

    protected:
        static void _bind_methods();

    public:
        LosCheckResult();
        ~LosCheckResult();

        void set_collision_position(const Vector2i &p_pos);
        void set_collision_element(Ref<SurfaceElement> p_element);

        Vector2i get_collision_position() const;
        Ref<SurfaceElement> get_collision_element() const;
    };

    class CollisionProvider : public RefCounted
    {
        GDCLASS(CollisionProvider, RefCounted);

    protected:
        static void _bind_methods();

    public:
        Ref<LosCheckResult> getLosCheck(const Vector2i &from, const Vector2i &to);
    };

    class PathfindingProvider : public RefCounted
    {
        GDCLASS(PathfindingProvider, RefCounted);

    protected:
        static void _bind_methods();

    public:
        TypedArray<Vector2i> getPath(const Vector2i &from, const Vector2i &to);
    };

    class Surface : public RefCounted
    {
        GDCLASS(Surface, RefCounted)

    private:
        std::map<Vector2i, Ref<SurfaceElement>> element_positions;
        Ref<PathfindingProvider> pathfinding_provider;
        Ref<CollisionProvider> collision_provider;

    protected:
        static void _bind_methods();

    public:
        Surface();
        ~Surface();

        void set_pathfinding_provider(const Ref<PathfindingProvider> p_provider);
        Ref<PathfindingProvider> get_pathfinding_provider() const;

        void set_collision_provider(const Ref<CollisionProvider> p_provider);
        Ref<CollisionProvider> get_collision_provider() const;

        bool is_position_available(const Vector2i &p_pos) const;
        void place_element(const Vector2i &p_pos, const Ref<SurfaceElement> p_element);
        bool move_element(const Vector2i &p_pos_from, const Vector2i &p_pos_to);

        Ref<SurfaceElement> get_element(const Vector2i &p_pos) const;
        Ref<SurfaceElement> lift_element(const Vector2i &p_pos);

        TypedArray<Unit> get_only_units() const; // bad implementation pattern
    };

}

#endif