#ifndef ACTION_H
#define ACTION_H

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot
{

    class Action : public Object
    {
        GDCLASS(Action, Object)

    private:
        String name;

    protected:
        static void _bind_methods();

    public:
        Action();
        ~Action();

        void set_name(const String p_name);
        String get_name() const;
    };

}

#endif