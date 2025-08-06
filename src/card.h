#pragma once

#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/classes/wrapped.hpp"
#include "godot_cpp/variant/variant.hpp"

#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/classes/control.hpp"

#include "godot_cpp/classes/texture_rect.hpp"
#include "godot_cpp/classes/input_event.hpp"

using namespace godot;

class Card : public Control {
	GDCLASS(Card, Control);

	TextureRect *texture_rect;
	Ref<Texture2D> texture;

	Vector2 drag_offset;

protected:
	static void _bind_methods();
	bool dragging = false;

public:
	Card();

	void set_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_texture() const;

	void _gui_input(const Ref<InputEvent> &event);

};
