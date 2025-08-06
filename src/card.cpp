#include "card.h"

#include "godot_cpp/classes/input_event_mouse_button.hpp"

void Card::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture"), &Card::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &Card::get_texture);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
}

void Card::set_texture(const Ref<Texture2D> &p_texture) {
	texture = p_texture;
	if (texture_rect) {
        texture_rect->set_texture(texture);
    }
}

Ref<Texture2D> Card::get_texture() const {
	return texture;
}

Card::Card() {
    texture_rect = memnew(TextureRect);
    add_child(texture_rect);
	texture_rect->set_anchors_preset(Control::LayoutPreset::PRESET_FULL_RECT);
    //texture_rect->set_expand_mode(TextureRect::ExpandMode::EXPAND_IGNORE_SIZE);
}

void Card::_gui_input(const Ref<InputEvent> &event) {
    if (event->is_class("InputEventMouseButton")) {
        const Ref<InputEventMouseButton> &mouse_event = event;
        if (mouse_event->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
            if (mouse_event->is_pressed()) {
                dragging = true;
                drag_offset = get_global_mouse_position() - get_global_position(); // Store offset
                print_line("Card drag started!");
            } else {
                dragging = false;
                print_line("Card drag ended!");
            }
        }
    }
    if (event->is_class("InputEventMouseMotion") && dragging) {
        set_global_position(get_global_mouse_position() - drag_offset); // Use offset
    }
}