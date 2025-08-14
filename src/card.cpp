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
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "AtlasTexture"), "set_texture", "get_texture");

	ClassDB::bind_method(D_METHOD("set_card", "card_name"), &Card::set_card);
	ClassDB::bind_method(D_METHOD("get_card"), &Card::get_card);
    // create string for a dropdown list
	String card_options = "spades_ace,spades_two,spades_three,spades_four,spades_five,spades_six,spades_seven,spades_eight,spades_nine,spades_ten,spades_jack,spades_queen,spades_king,"
						  "hearts_ace,hearts_two,hearts_three,hearts_four,hearts_five,hearts_six,hearts_seven,hearts_eight,hearts_nine,hearts_ten,hearts_jack,hearts_queen,hearts_king,"
						  "diamonds_ace,diamonds_two,diamonds_three,diamonds_four,diamonds_five,diamonds_six,diamonds_seven,diamonds_eight,diamonds_nine,diamonds_ten,diamonds_jack,diamonds_queen,diamonds_king,"
						  "clubs_ace,clubs_two,clubs_three,clubs_four,clubs_five,clubs_six,clubs_seven,clubs_eight,clubs_nine,clubs_ten,clubs_jack,clubs_queen,clubs_king";
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "card_name", PROPERTY_HINT_ENUM, card_options), "set_card", "get_card");
	
	ClassDB::bind_method(D_METHOD("set_size"), &Card::set_size);
	ClassDB::bind_method(D_METHOD("get_size"), &Card::get_size);

	ClassDB::bind_method(D_METHOD("_on_area_entered", "area"), &Card::_on_area_entered);
	ClassDB::bind_method(D_METHOD("_on_area_exited", "area"), &Card::_on_area_exited);
}

Card::Card() {
    current_card = "spades_ace"; // default card
     
    sprite = memnew(Sprite2D);
    add_child(sprite);
    sprite->set_centered(true);

    interaction_area = memnew(Area2D);
    add_child(interaction_area);
    
    collision_shape = memnew(CollisionShape2D);
    interaction_area->add_child(collision_shape);
    
    shape.instantiate();
    collision_shape->set_shape(shape);

    interaction_area->connect("area_entered", Callable(this, "_on_area_entered"));
    interaction_area->connect("area_exited", Callable(this, "_on_area_exited"));

    set_process_unhandled_input(true); // required for _unhandled_input() callback
    set_physics_process(true); // required for _physics_process() callback
    
    set_texture(Ref<AtlasTexture>()); // necessary to initialize texture and size
}

void Card::_ready() {
    //load card positions from file if not already loaded
    if (card_positions.is_empty()) {
        Ref<FileAccess> file = FileAccess::open("res://data/card_texture_positions.txt", FileAccess::READ);
        if (!file.is_valid()) {
            print_line("Could not load card positions file, using defaults");
            Array ace_pos;
            ace_pos = Array::make(12, 2); // add fallback pos so it compiles
            card_positions["spades_ace"] = ace_pos;
        } else {
            print_line("Loading card positions from file...");
            int loaded_count = 0;
            
            while (!file->eof_reached()) {
                String line = file->get_line().strip_edges();
                
                if (line.is_empty() || line.begins_with("#")) { // skip empty lines and comments
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

    set_texture(Ref<AtlasTexture>()); //necessary to initialize texture and size
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

void Card::set_card(const String &card_name) {
    if (current_card != card_name) {
        current_card = card_name;
        set_texture(Ref<AtlasTexture>()); // reload texture with new card
        
        notify_property_list_changed(); // so it updates in editor when changed
    }
}

String Card::get_card() const {
    return current_card;
}

//manual event handling
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

//manual handled methods (managed by _unhandled_input)
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

//engine handled methods
void Card::_on_area_entered(Area2D *area) {
    Card *other_card = Object::cast_to<Card>(area->get_parent());
    if (!other_card) {
        return;
    }
    
    // Add the card to our collision list if it's not already there
    if (!colliding_cards.has(other_card)) {
        colliding_cards.push_back(other_card);
    }
    
    colliding = !colliding_cards.is_empty();
    
    if (other_card) {
        print_line(other_card->get_card() + " entered collision with " + get_card());
    }
}

void Card::_on_area_exited(Area2D *area) {
    Card *other_card = Object::cast_to<Card>(area->get_parent());
    if (other_card) {
        // Remove the card from our collision list
        colliding_cards.erase(other_card);
        colliding = !colliding_cards.is_empty();
        
        print_line(other_card->get_card() + " exited collision with " + get_card());
    }
}

void Card::_physics_process(double delta) {
    if (dragging) {
        return;
    }
    
    // Process all currently colliding cards
    for (int i = 0; i < colliding_cards.size(); i++) {
        Variant card_variant = colliding_cards[i];
        Card *other_card = Object::cast_to<Card>(card_variant);
        
        // Skip if the other card is being dragged or invalid
        if (!other_card || other_card->dragging) {
            continue;
        }
        
        // Calculate direction vector from other card to this card
        Vector2 direction = get_global_position() - other_card->get_global_position();
        
        // Only apply force if cards are close enough
        if (direction.length() > 0 && direction.length() < 100.0f) { // Adjust threshold as needed
            direction = direction.normalized();
            float push_force = 100.0f * delta; // Adjust force as needed
            
            // Move this card away from the other card
            Vector2 new_position = get_global_position() + (direction * push_force);
            set_global_position(new_position);
        }
    }
}