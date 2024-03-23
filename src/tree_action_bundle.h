#ifndef TREE_ACTION_BUNDLE_H
#define TREE_ACTION_BUNDLE_H

#include "action_bundle.h"
#include "monte_carlo.h"

class TreeActionBundle : public ActionBundle
{

private:
public:
    TreeActionBundle(){};
    ~TreeActionBundle();
    // TreeActionBundle(Node *p_node, Ref<Unit> p_surface, Ref<Unit> p_caster)
    //     : node(p_node), surface(p_surface), caster(p_caster){};
    Node *root = nullptr;
    Node *node = nullptr;
    Ref<Surface> surface;
    Ref<Unit> caster;

    bool cast_until_finished() override;
};

#endif