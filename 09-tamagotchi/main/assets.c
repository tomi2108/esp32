#include "assets.h"

typedef Icon (*IconGetter)(void);

typedef struct ItemAsset {
  char *code;
  IconGetter icon;
} ItemAsset;

static const ItemAsset item_assets[] = {
    // Consumables
    {"strawberry", icons_food_fruit_apple},
    {"apple", icons_food_fruit_apple},
    {"banana", icons_food_fruit_apple},
    {"cookie", icons_food_cookie_biscuit},
    {"carrot", icons_tools_crafting_farming_plant_crop},
    {"fish", icons_food_fish_bones_trash},
    {"cake", icons_food_donut_doughnut_pastry},
    {"candy", icons_food_candy},
    {"sandwich", icons_food_bread_loaf},
    {"energy_potion",
     icons_alchemy_potion_vial_bottle_lightning_bolt_zap_speed_full},
    {"healing_potion", icons_alchemy_potion_vial_bottle_heart_health_life_full},
    {"medicine", icons_misc_medkit_medicine_health_pickup},
    {"vitamin", icons_misc_pills_medicine_health_1},
    {"syrup", icons_alchemy_potion_vial_bottle_full},

    // Seeds
    {"strawberry_seed", icons_tools_crafting_farming_plant_crop},
    {"carrot_seed", icons_tools_crafting_farming_plant_crop},
    {"tomato_seed", icons_tools_crafting_farming_plant_crop},
    {"lettuce_seed", icons_tools_crafting_farming_plant_crop},
    {"pumpkin_seed", icons_tools_crafting_farming_plant_crop},

    // Head
    {"straw_hat", icons_hats_summer_lady_straw},
    {"crown", icons_hats_crown_king_monarch_ruler_emperor},
    {"wool_hat", icons_hats_winter},
    {"cap", icons_hats_baseball_cap_1},
    {"wizard_hood", icons_hats_wizard_mage_magician_sorcerer},
    {"cat_ears", icons_rpg_stat_pet_paw_animal_handling_tame_beast_mastery},

    // Body
    {"blue_shirt", icons_rpg_item_armor_equipment_slot_chest_breastplate_body},
    {"yellow_raincoat", icons_weather_umbrella_rain_parasol},
    {"cozy_sweater", icons_hats_winter},
    {"frog_costume", icons_rpg_character_humanoid},
    {"armor", icons_rpg_item_armor_equipment_slot_chestplate_body_armour},
    {"apron", icons_hats_chef_cooking_kitchen},

    // Accessories
    {"bell_collar",
     icons_rpg_item_accessory_trinket_equipment_slot_necklace_amulet_talisman_jewelry},
    {"lucky_charm", icons_rpg_stat_luck_four_leaf_clover},
    {"red_bow", icons_hats_bow_ribbon},
    {"glasses", icons_hats_glasses_spectacles},
    {"scarf", icons_hats_winter},
    {"backpack", icons_travel_backpack_bag_school},
    {"fairy_wings", icons_emoji_face_angel_halo},
    {"bandana", icons_hats_ribbon},

    // Harvestable crops
    {"tomato", icons_food_fruit_apple},
    {"lettuce", icons_food_grain_crop_plant},
    {"pumpkin", icons_food_fruit_apple},
};

Icon get_item_bitmap(const char *code) {
  for (int i = 0; i < sizeof(item_assets) / sizeof(item_assets[0]); i++) {
    if (strcmp(item_assets[i].code, code) == 0) {
      return item_assets[i].icon();
    }
  }

  return icons_controller_face_buttons_xbox();
}
