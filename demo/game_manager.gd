extends Control

@export var card_scene: PackedScene = preload("res://scenes/card.tscn") # load card scene

@onready var deck: Deck = $Deck # reference to the deck node

var cards_in_play: Array[Card] = [] # track played cards
var available_cards: Array[Dictionary] = []
var drawn_cards: Array[String] = []
var cards_dict: Dictionary = {} # cards from position file

func _ready():
	load_card_data()
	
	if deck:
		deck.deck_pressed.connect(_on_deck_pressed)
		print("Connected to deck signal")
		print("Loaded ", available_cards.size(), " cards from deck")
	else:
		print("Warning: Deck node not found!")

func load_card_data():
	"""Load all card data from the texture positions file"""
	var file = FileAccess.open("res://data/card_texture_positions.txt", FileAccess.READ)
	if not file:
		print("Error: Could not open card_texture_positions.txt")
		return
	
	while not file.eof_reached():
		var line = file.get_line().strip_edges()
		
		if line.is_empty() or line.begins_with("#"): # skip empty and comments
			continue
		
		var parts = line.split("=")
		if parts.size() != 2:
			continue
		
		var card_name = parts[0].strip_edges()
		var coords = parts[1].strip_edges().split(",")
		
		if coords.size() != 2:
			continue
		
		var x = coords[0].to_int()
		var y = coords[1].to_int()
		
		var card_data = {
			"name": card_name,
			"region": Rect2(x, y, 40, 60)  # w,h always 40,60 according to file
		}
		
		cards_dict[card_name] = card_data
		available_cards.append(card_data)
	
	file.close()
	print("Loaded ", available_cards.size(), " unique cards from file")

func reset_deck():
	available_cards.clear()
	drawn_cards.clear()
	
	# reload the deck
	for card_name in cards_dict.keys():
		available_cards.append(cards_dict[card_name])
	
	print("Deck reset! ", available_cards.size(), " cards available again")

func _on_deck_pressed():
	print("Game Manager: Deck was pressed! Creating a new card...")
	create_new_card()

func create_new_card():
	if not card_scene:
		print("Error: Card scene not loaded!")
		return
	
	if available_cards.is_empty():
		print("No more cards in deck! Consider calling reset_deck() to reshuffle.")
		return
	
	var new_card = card_scene.instantiate() as Card
	if not new_card:
		print("Error: Failed to instantiate card!")
		return

	var random_index = randi() % available_cards.size() # random from remaining only
	var selected_card = available_cards[random_index]
	
	available_cards.remove_at(random_index)
	drawn_cards.append(selected_card.name)
	
	# texture setup
	var atlas_texture = AtlasTexture.new()
	var main_texture = load("res://assets/deck_classic_sepia_2color_0.png") as Texture2D
	atlas_texture.atlas = main_texture
	atlas_texture.region = selected_card.region
	
	new_card.texture = atlas_texture
	new_card.card_name = selected_card.name
	
	new_card.position = deck.position + Vector2(randi_range(-50, 50), randi_range(-100, -50)) # where the card spawns
	
	add_child(new_card) # add new card to scene
	cards_in_play.append(new_card)
	
	print("Drew card: ", selected_card.name, " (", available_cards.size(), " cards remaining)")
	
	if available_cards.is_empty():
		reset_deck()
