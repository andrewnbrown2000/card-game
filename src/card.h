#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/classes/node2d.hpp"
#include "godot_cpp/classes/sprite2d.hpp"
#include "godot_cpp/classes/area2d.hpp"
#include "godot_cpp/classes/collision_shape2d.hpp"
#include "godot_cpp/classes/rectangle_shape2d.hpp"

#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/input_event_mouse_motion.hpp"

using namespace godot;

class Card : public Node2D {
	GDCLASS(Card, Node2D);

	Sprite2D *sprite;
	Ref<Texture2D> texture;

	// for hover signals
	Area2D *hover_area;
	CollisionShape2D *collision_shape;
	Ref<RectangleShape2D> shape; // not a node, a resource on the CollisionShape2D

	Vector2 drag_offset;

protected:
	static void _bind_methods();
	bool dragging = false;
	bool hovering = false;

public:
	Card();

	void set_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_texture() const;

	void set_size(const Vector2 &p_size);
	Vector2 get_size() const;

	//for drag functionality
	virtual void _unhandled_input(const Ref<InputEvent> &event) override;
	void _on_mouse_button_pressed(const Ref<InputEventMouseButton> &event);
	void _on_mouse_motion(const Ref<InputEventMouseMotion> &event);

	//hover animation using signals
	void _on_mouse_entered();
	void _on_mouse_exited();

};
