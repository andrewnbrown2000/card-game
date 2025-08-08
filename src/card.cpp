#include "card.h"

#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/input_event_mouse_motion.hpp"
#include "godot_cpp/classes/viewport.hpp"
#include "godot_cpp/classes/area2d.hpp"
#include "godot_cpp/classes/collision_shape2d.hpp"
#include "godot_cpp/classes/rectangle_shape2d.hpp"
#include "godot_cpp/classes/resource_loader.hpp"
#include "godot_cpp/classes/file_access.hpp"

void Card::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture"), &Card::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &Card::get_texture);
	
	ClassDB::bind_method(D_METHOD("set_card", "card_name"), &Card::set_card);
	
	ClassDB::bind_method(D_METHOD("set_size"), &Card::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &Card::get_size);
	
	ClassDB::bind_method(D_METHOD("_on_mouse_entered"), &Card::_on_mouse_entered);
	ClassDB::bind_method(D_METHOD("_on_mouse_exited"), &Card::_on_mouse_exited);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "AtlasTexture"), "set_texture", "get_texture");
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

void Card::_ready() {
    if (card_positions.is_empty()) {
        Ref<FileAccess> file = FileAccess::open("res://data/card_texture_positions.txt", FileAccess::READ);
        if (!file.is_valid()) {
            print_line("Could not load card positions file, using defaults");
            // Add fallback positions
            Array ace_pos;
            ace_pos.push_back(12);
            ace_pos.push_back(2);
            card_positions["spades_ace"] = ace_pos;
        } else {
            print_line("Loading card positions from file...");
            int loaded_count = 0;
            
            while (!file->eof_reached()) {
                String line = file->get_line().strip_edges();
                
                // Skip empty lines and comments
                if (line.is_empty() || line.begins_with("#")) {
                    continue;
                }
                
                Array parts = line.split("=");
                if (parts.size() == 2) {
                    String card_name = String(parts[0]).strip_edges();
                    Array coords = String(parts[1]).strip_edges().split(",");
                    
                    if (coords.size() >= 2) {
                        Array position_data;
                        position_data.push_back(String(coords[0]).to_int());
                        position_data.push_back(String(coords[1]).to_int());
                        card_positions[card_name] = position_data;
                        loaded_count++;
                    }
                }
            }
            file->close();
            print_line("Loaded " + String::num_int64(loaded_count) + " card positions from file");
        }
    }
}

void Card::set_texture(const Ref<AtlasTexture> &p_texture) {
    if (p_texture.is_valid()) {
        texture = p_texture;
    } else {
        // create default texture only if needed
        static Ref<Texture2D> cached_base_texture;
        if (!cached_base_texture.is_valid()) {
            cached_base_texture = ResourceLoader::get_singleton()->load("res://assets/deck_classic_sepia_2color_0.png");
        }
        
        Ref<AtlasTexture> atlas_texture = memnew(AtlasTexture);
        atlas_texture->set_atlas(cached_base_texture);
        
        if (card_positions.has(current_card)) {
            Array pos_data = card_positions[current_card];
            if (pos_data.size() >= 2) {
                Vector2 card_pos = Vector2(pos_data[0], pos_data[1]);
                atlas_texture->set_region(Rect2(card_pos.x, card_pos.y, 40, 60));
                print_line("Set card: " + current_card + " at position: " + String::num(card_pos.x) + "," + String::num(card_pos.y));
            }
        } else {
            atlas_texture->set_region(Rect2(12, 2, 40, 60)); // default to ace of spades
            print_line("Card not found: " + current_card + ", using default");
        }
        
        texture = atlas_texture;
    }

    if (sprite) {
        sprite->set_texture(texture);
    }

    if (texture.is_valid()) {
        set_size(texture->get_size());
    }
}

Ref<AtlasTexture> Card::get_texture() const {
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
            if (hovering) {
                set_scale(Vector2(1.1, 1.1));
            } else {
                set_scale(Vector2(1.0, 1.0));
            }
            dragging = false;
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
    set_scale(Vector2(1.1, 1.1));
    hovering = true;
}

void Card::_on_mouse_exited() {
    if (dragging == false) {
        set_scale(Vector2(1.0, 1.0));
    }
    hovering = false;
}

void Card::set_card(const String &card_name) {
    current_card = card_name;
    set_texture(Ref<AtlasTexture>()); // reload texture with new card
}