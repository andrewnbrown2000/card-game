#pragma once

#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/sprite2d.hpp"

#include "godot_cpp/classes/area2d.hpp"
#include "godot_cpp/classes/collision_shape2d.hpp"
#include "godot_cpp/classes/rectangle_shape2d.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"

using namespace godot;

class Deck : public Node2D {
    GDCLASS(Deck, Node2D);

    Sprite2D *sprite;
    Ref<Texture2D> texture;

	CollisionShape2D *collision_shape;
	Ref<RectangleShape2D> shape; // not a node, a resource on the CollisionShape2D


protected:
	static void _bind_methods();


public:
	Deck();

    void _unhandled_input(const Ref<InputEvent> &event) override;
    void set_texture(const Ref<Texture2D> &p_texture);
    Ref<Texture2D> get_texture() const;

private:
    void _on_mouse_button_pressed(const Ref<InputEventMouseButton> &event);
};