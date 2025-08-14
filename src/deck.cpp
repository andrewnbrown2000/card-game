#include "deck.h"

#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/collision_shape2d.hpp"
#include "godot_cpp/classes/rectangle_shape2d.hpp"

void Deck::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_texture", "texture"), &Deck::set_texture);
    ClassDB::bind_method(D_METHOD("get_texture"), &Deck::get_texture);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
    
    ADD_SIGNAL(MethodInfo("deck_pressed"));
}

Deck::Deck() {
    sprite = memnew(Sprite2D);
    add_child(sprite);
    sprite->set_centered(true);

    collision_shape = memnew(CollisionShape2D);
    shape.instantiate();
    collision_shape->set_shape(shape);

    set_process_unhandled_input(true);
    
    set_texture(Ref<Texture2D>());
}

void Deck::set_texture(const Ref<Texture2D> &p_texture) {
    if (p_texture.is_valid()) {
        texture = p_texture;
    } else {
        texture = ResourceLoader::get_singleton()->load("res://assets/deck_static.png");
    }
    
    if (sprite && texture.is_valid()) {
        sprite->set_texture(texture);
    }
}

Ref<Texture2D> Deck::get_texture() const {
    return texture;
}

void Deck::_unhandled_input(const Ref<InputEvent> &event) {
    Ref<InputEventMouseButton> mouse_button = event;
    if (mouse_button.is_valid()) {
        _on_mouse_button_pressed(mouse_button);
    }
}

void Deck::_on_mouse_button_pressed(const Ref<InputEventMouseButton> &event) {
    if (event->get_button_index() == MOUSE_BUTTON_LEFT) {
        Vector2 mouse_pos = event->get_global_position();
        
        if (event->is_pressed()) {
            Vector2 local_mouse_pos = sprite->to_local(mouse_pos);
            
            if (sprite && texture.is_valid()) {
                Vector2 texture_size = texture->get_size();
                Rect2 sprite_rect;
                
                if (sprite->is_centered()) {
                    sprite_rect = Rect2(-texture_size * 0.5f, texture_size);
                } else {
                    sprite_rect = Rect2(Vector2(0, 0), texture_size);
                }
                
                if (sprite_rect.has_point(local_mouse_pos)) { 
                    emit_signal("deck_pressed");
                    print_line("Signal 'deck_pressed' emitted.");
                }
            }
        }
    }
}