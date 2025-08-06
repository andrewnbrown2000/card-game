#include "card.h"

#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/input_event_mouse_motion.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/area2d.hpp"
#include "godot_cpp/classes/collision_shape2d.hpp"
#include "godot_cpp/classes/rectangle_shape2d.hpp"


void Card::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture"), &Card::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &Card::get_texture);
	
	ClassDB::bind_method(D_METHOD("set_size"), &Card::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &Card::get_size);
	
	ClassDB::bind_method(D_METHOD("_on_mouse_entered"), &Card::_on_mouse_entered);
	ClassDB::bind_method(D_METHOD("_on_mouse_exited"), &Card::_on_mouse_exited);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
}

Card::Card() {
    sprite = memnew(Sprite2D);
    add_child(sprite);
    sprite->set_centered(true);

    hover_area = memnew(Area2D);
    add_child(hover_area);
    
    collision_shape = memnew(CollisionShape2D);
    hover_area->add_child(collision_shape);
    
    shape.instantiate();
    collision_shape->set_shape(shape);

    hover_area->connect("mouse_entered", Callable(this, "_on_mouse_entered"));
    hover_area->connect("mouse_exited", Callable(this, "_on_mouse_exited"));

    set_process_unhandled_input(true); // required for _unhandled_input() callback
}

void Card::set_texture(const Ref<Texture2D> &p_texture) {
    texture = p_texture;
    if (sprite) {
        sprite->set_texture(texture);
    }

    if (texture.is_valid()) {
        set_size(texture->get_size());
    }
}

Ref<Texture2D> Card::get_texture() const {
	return texture;
}

void Card::set_size(const Vector2 &p_size) {
    if (texture.is_valid()) {
        Vector2 texture_size = texture->get_size();
        if (texture_size.x > 0 && texture_size.y > 0) {
            set_scale(p_size / texture_size);
            
            if (shape.is_valid()) { 
                shape->set_size(p_size);
            }
        }
    }
}

Vector2 Card::get_size() const {
    if (texture.is_valid()) {
        return texture->get_size() * get_scale();
    }
    return Vector2();
}

void Card::_unhandled_input(const Ref<InputEvent> &event) {    
    Ref<InputEventMouseButton> mouse_button = event;
    if (mouse_button.is_valid()) {
        _on_mouse_button_pressed(mouse_button);
        return;
    }

    Ref<InputEventMouseMotion> mouse_motion = event;
    if (mouse_motion.is_valid()) {
        _on_mouse_motion(mouse_motion);
        return;
    }
}

void Card::_on_mouse_button_pressed(const Ref<InputEventMouseButton> &event) {
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
                    dragging = true;
                    drag_offset = mouse_pos - get_global_position();
                    if (get_parent()) { // bring card to front
                        get_parent()->move_child(this, -1);
                    }
                }
            }
        } else {
            dragging = false;
            print_line("Mouse button released");
        }
    }
}

void Card::_on_mouse_motion(const Ref<InputEventMouseMotion> &event) {
    if (dragging) {
        Vector2 new_position = event->get_global_position() - drag_offset;
        Vector2 viewport_size = get_viewport()->get_visible_rect().size;
        new_position.x = CLAMP(new_position.x, 0.0f + (get_size().x) / 2, viewport_size.x - (get_size().x) / 2);
        new_position.y = CLAMP(new_position.y, 0.0f + (get_size().y) / 2, viewport_size.y - (get_size().y) / 2);
        set_global_position(new_position);
    }
}

void Card::_on_mouse_entered() {
    hovering = true;
    set_scale(Vector2(1.1, 1.1));
}

void Card::_on_mouse_exited() { //small bug when releasing mouse off screen
    if (dragging == false) {
        set_scale(Vector2(1.0, 1.0));
    }
    hovering = false;
}