#ifndef SURFACE_H
#define SURFACE_H

#include <map>
#include <vector>
#include <queue>
#include <algorithm>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/vector2i.hpp>

#include "surface_element.h"
#include "unit.h"

namespace godot
{

    struct VectorHasher
    {
        std::size_t operator()(const Vector2i vec) const;
    };

    class Surface : public RefCounted
    {
        GDCLASS(Surface, RefCounted)

    private:
        std::unordered_map<Vector2i, Ref<SurfaceElement>, VectorHasher> element_positions;
        std::vector<Ref<Unit>> unit_order;
        int unit_order_counter = 0;

    protected:
        static void _bind_methods();

    public:
        Surface();
        ~Surface();
        Ref<Surface> clone() const;

        std::vector<Vector2i> get_free_neighbors(const Vector2i p_pos) const;
        PackedVector2Array get_shortest_path(const Vector2i path_start, const Vector2i path_end, const bool to_neighbor = false) const;
        Vector2i get_ray_collision(const Vector2i ray_start, const Vector2i ray_end) const;

        bool is_position_available(const Vector2i &p_pos) const; // TODO is there any point to pass Vector2i as ref?
        void place_element(const Vector2i &p_pos, const Ref<SurfaceElement> p_element);
        bool move_element(const Vector2i &p_pos_from, const Vector2i &p_pos_to);

        Ref<SurfaceElement> get_element(const Vector2i &p_pos) const;
        Ref<SurfaceElement> lift_element(const Vector2i &p_pos);

        TypedArray<Unit> get_only_units() const; // bad implementation pattern
        std::vector<Ref<Unit>> get_only_units_vec() const;

        void generate_new_unit_order();
        bool is_unit_order_finished() const;
        TypedArray<Unit> turn_get_order() const;
        Ref<Unit> turn_get_current_unit() const;
        void end_current_units_turn();
        void start_current_units_turn();
        void emit_action_cast(const int action, const Ref<SurfaceElement> caster, const Vector2i target);
        std::unordered_map<Vector2i, Ref<SurfaceElement>, VectorHasher> get_element_positions() const;
        Faction get_winner() const;
        int get_remaining_factions_count() const;
    };

}

#endif