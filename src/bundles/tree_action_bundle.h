#ifndef TREE_ACTION_BUNDLE_H
#define TREE_ACTION_BUNDLE_H

#include "action_bundle.h"
#include "monte_carlo.h"

class TreeActionBundle : public ActionBundle
{

private:
    Ref<Surface> surface;
    Ref<Unit> caster;
    Node *root;
    Node *current_node;
    bool forced_to_finish = false;
    const bool is_probabilistic = false; // todo enable

public:
    TreeActionBundle(Ref<Surface> p_surface, Ref<Unit> p_caster)
        : surface(p_surface), caster(p_caster)
    {
        Ref<Surface> surface_clone = surface->clone();
        surface_clone->set_random_events_enabled(false);
        Ref<Unit> caster_clone = surface_clone->get_element(caster->get_position());
        root = new Node(surface_clone, caster_clone);
        current_node = root;
    };

    ~TreeActionBundle();

    bool is_finished() const override;
    void cast_next() override;
    Node *get_root() const;
    void save_data(const String &path, const int visits_threshold) override;
};

#endif