// Copyright 2021 @filterpaper
// SPDX-License-Identifier: GPL-2.0+

#include "filterpaper.h"


void matrix_init_user(void) {
	// Remap under glow LEDs to nearby keys
#ifdef KEYBOARD_boardsource_the_mark
	g_led_config = (led_config_t){ {
		{ 10, 10, 9 , 9 , 8 , 7 , 7 , 6 , 5 , 5 , 4 , 3 , 3 , 2 , 1 , 1  },
		{ 11, 11, 9 , 9 , 9 , 8 , 7 , 7 , 6 , 5 , 4 , 4 , 3 , 2 , 1 , 0  },
		{ 12, 12, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 21, 21, 23, 23 },
		{ 13, 13, 14, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 21, 22, 22 },
		{ 13, 13, 14, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 21, 22, 22 },
	}, {
		{224, 42}, {224, 21}, {209, 21}, {179, 21}, {164, 21}, {134, 21}, {119, 21}, {89, 21}, {74, 21}, {45, 21}, {30, 21}, {30, 42},
		{30, 64}, {30, 85}, {45, 85}, {74, 85}, {89, 85}, {119, 85}, {134, 85}, {164, 85}, {179, 85}, {209, 85}, {224, 85}, {224, 64}
	}, {
		255, 255, 255, 4, 4, 4, 4, 4, 4, 255, 255, 255,
		255, 255, 255, 4, 4, 4, 4, 4, 4, 255, 255, 255
	} };
#endif
	// Disable underglow LEDs
#if defined(KEYBOARD_boardsource_technik_o) || defined(LP)
	for (uint8_t i = 0; i < DRIVER_LED_TOTAL; ++i) {
		if (HAS_FLAGS(g_led_config.flags[i], LED_FLAG_UNDERGLOW)) {
			g_led_config.flags[i] = LED_FLAG_NONE;
		}
	}
#endif
	// Change thumb key flags
#ifdef KEYBOARD_crkbd_rev1
	g_led_config.flags[6] = g_led_config.flags[33] = LED_FLAG_KEYLIGHT;
#endif
}


void keyboard_post_init_user(void) {
	rgb_matrix_mode_noeeprom(DEF_MODE);
}


layer_state_t layer_state_set_user(layer_state_t const state) {
	layer_state_is(CMK) ? rgb_matrix_mode_noeeprom(CMK_MODE) : rgb_matrix_mode_noeeprom(DEF_MODE);
	return state;
}


void rgb_matrix_indicators_user(void) {
	// Caps lock indicator
	if (host_keyboard_led_state().caps_lock) {
		for (uint8_t i = 0; i < DRIVER_LED_TOTAL; ++i) {
			if (g_led_config.flags[i] & CAP_FLAG) { rgb_matrix_set_color(i, RGB_CAPS); }
		}
	}
	// Modifier keys indicator
	if (get_mods() & MOD_MASK_CSAG) {
		for (uint8_t i = 0; i < DRIVER_LED_TOTAL; ++i) {
			if (g_led_config.flags[i] & MOD_FLAG) { rgb_matrix_set_color(i, RGB_MODS); }
		}
	}

	if (get_highest_layer(layer_state) > CMK) {
	#ifdef KEYBOARD_boardsource_the_mark
		rgb_matrix_set_color_all(RGB_LAYER);
	#else
		// Layer keys indicator by @rgoulter
		uint8_t const layer = get_highest_layer(layer_state);
		for (uint8_t row = 0; row < MATRIX_ROWS; ++row) {
			for (uint8_t col = 0; col < MATRIX_COLS; ++col) {
				if (g_led_config.matrix_co[row][col] != NO_LED &&
					keymap_key_to_keycode(layer, (keypos_t){col,row}) > KC_TRNS) {
					rgb_matrix_set_color(g_led_config.matrix_co[row][col], RGB_LAYER);
				}
			}
		}
	#endif // KEYBOARD_boardsource_the_mark
	}
}

/*
static keypos_t led_index_key_position[DRIVER_LED_TOTAL];
void rgb_matrix_init_user(void) {
	for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
		for (uint8_t col = 0; col < MATRIX_COLS; col++) {
			uint8_t led_index = g_led_config.matrix_co[row][col];
			if (led_index != NO_LED) {
				led_index_key_position[led_index] = (keypos_t){.row = row, .col = col};
			}
		}
	}
}

void rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
	// Caps lock indicator
	if (host_keyboard_led_state().caps_lock) {
		for (uint8_t i = led_min; i <= led_max; ++i) {
			if (g_led_config.flags[i] & CAP_FLAG) { rgb_matrix_set_color(i, RGB_CAPS); }
		}
	}
	// Modifier keys indicator
	if (get_mods() & MOD_MASK_CSAG) {
		for (uint8_t i = led_min; i <= led_max; ++i) {
			if (g_led_config.flags[i] & MOD_FLAG) { rgb_matrix_set_color(i, RGB_MODS); }
		}
	}

	if (get_highest_layer(layer_state) > CMK) {
	#ifdef KEYBOARD_boardsource_the_mark
		for (uint8_t i = led_min; i <= led_max; ++i) {
			if (HAS_ANY_FLAGS(g_led_config.flags[i], LED_FLAG_ALL)) {
				RGB rgb = hsv_to_rgb((HSV){rgb_matrix_config.hsv.h >> get_highest_layer(layer_state), rgb_matrix_config.hsv.v, rgb_matrix_config.hsv.v});
				rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
			}
		}
	#else
		// Layer keys indicator by @rgoulter
		uint8_t const layer = get_highest_layer(layer_state);
		for (uint8_t i = led_min; i <= led_max; ++i) {
			if (keymap_key_to_keycode(layer, led_index_key_position[i]) > KC_TRNS) {
				rgb_matrix_set_color(i, RGB_LAYER);
			}
		}
	#endif // KEYBOARD_boardsource_the_mark
	}
}*/
