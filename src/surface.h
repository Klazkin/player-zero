#ifndef SURFACE_H
#define SURFACE_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include "surface_element.h"

namespace godot
{

    class Surface : public Object
    {
        GDCLASS(Surface, Object)

    private:
        AbstractPathfindingProvider pathfinding_provider;
        AbstractCollisionProvider collision_provider;

    protected:
        static void _bind_methods();

    public:
        Surface();
        ~Surface();

        void set_pathfinding_provider(const AbstractPathfindingProvider p_provider);
        AbstractPathfindingProvider get_pathfinding_provider() const;

        void set_collision_provider(const AbstractCollisionProvider p_provider);
        AbstractCollisionProvider get_collision_provider() const;

        void occupy(const Vector2i p_pos, const SurfaceElement p_element);
        void reoccupy(const Vector2i p_pos, const SurfaceElement p_element);

        SurfaceElement get_occupation(const Vector2i p_pos) const;
        SurfaceElement clear_occupation(const Vector2i p_pos) const;
    };

    class LosCheckResult : public Object
    {
        GDCLASS(LosCheckResult, Object)

    private:
        Vector2i collided_at;
        SurfaceElement element;

    protected:
        static void
        _bind_methods();

    public:
        LosCheckResult();
        ~LosCheckResult();

        Vector2 get_collision_position() const;
        SurfaceElement get_collision_object() const;
    };

    class AbstractCollisionProvider : public Object
    {
        GDCLASS(AbstractCollisionProvider, Object);

    protected:
        static void _bind_methods() {}

    public:
        LosCheckResult getLosCheck(const Vector2i from, const Vector2i to);
    };

    class AbstractPathfindingProvider : public Object
    {
        GDCLASS(AbstractPathfindingProvider, Object);

    protected:
        static void _bind_methods() {}

    public:
        void getPath(const Vector2i from, const Vector2i to);
    };
}

#endif