#ifndef SURFACE_H
#define SURFACE_H

#include <unordered_map>
#include <vector>

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "surface_element.h"
#include "unit.h"

namespace godot
{

    class LosCheckResult : public Object
    {
        GDCLASS(LosCheckResult, Object)

    private:
        Vector2i collided_at;
        SurfaceElement *element;

    protected:
        static void _bind_methods();

    public:
        LosCheckResult();
        ~LosCheckResult();

        void set_collision_position(const Vector2i &p_pos);
        void set_collision_object(SurfaceElement *p_element);

        Vector2i get_collision_position() const;
        SurfaceElement *get_collision_object() const;
    };

    class AbstractCollisionProvider : public Object
    {
        GDCLASS(AbstractCollisionProvider, Object);

    protected:
        static void _bind_methods();

    public:
        LosCheckResult *getLosCheck(const Vector2i &from, const Vector2i &to);
    };

    class AbstractPathfindingProvider : public Object
    {
        GDCLASS(AbstractPathfindingProvider, Object);

    protected:
        static void _bind_methods();

    public:
        void getPath(const Vector2i &from, const Vector2i &to); // todo ret array
    };

    class Surface : public Object
    {
        GDCLASS(Surface, Object)

    private:
        // std::unordered_map<Vector2i, SurfaceElement> occupations;
        AbstractPathfindingProvider pathfinding_provider;
        AbstractCollisionProvider collision_provider;

    protected:
        static void _bind_methods();

    public:
        Surface();
        ~Surface();

        void set_pathfinding_provider(AbstractPathfindingProvider *p_provider);
        AbstractPathfindingProvider get_pathfinding_provider() const;

        void set_collision_provider(AbstractCollisionProvider *p_provider);
        AbstractCollisionProvider get_collision_provider() const;

        void occupy(const Vector2i p_pos, const SurfaceElement p_element);
        void reoccupy(const Vector2i p_pos, const SurfaceElement p_element);

        SurfaceElement get_occupation(const Vector2i p_pos) const;
        SurfaceElement clear_occupation(const Vector2i p_pos) const;

        // std::vector<Unit> get_all_units() const; // bad implementation pattern
    };

}

#endif