/* Copyright (C) 2021 @filterpaper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filterpaper.h"

#ifdef COMBO_ENABLE
#include "combos.h"
#endif

#ifdef OLED_ENABLE
uint32_t tap_timer = 0; // Timer for OLED animation
#endif


#ifdef TAPPING_TERM_PER_KEY // Reduce for thumb keys
uint16_t get_tapping_term(uint16_t keycode, keyrecord_t *record) {
	return (keycode & 0xFF00) == RMT_BITS ? TAPPING_TERM - 150 : TAPPING_TERM;
}
#endif


#ifdef PERMISSIVE_HOLD_PER_KEY // Disable for tap hold macros and home row mods
bool get_permissive_hold(uint16_t keycode, keyrecord_t *record) {
	return (keycode & 0xFF00) == LT0_BITS || (keycode & 0xF000) == LMT_BITS ? false : true;
}
#endif


#ifdef TAPPING_FORCE_HOLD_PER_KEY
bool get_tapping_force_hold(uint16_t keycode, keyrecord_t *record) {
	return keycode == RSFT_T(KC_SPC) ? true : false;
}
#endif


// Deactivate caps lock with following key codes
static void process_caps_word(uint16_t keycode, keyrecord_t *record) {
	// Get base key code from mod tap
	if (record->tap.count) {
		switch (keycode) {
			case QK_MOD_TAP...QK_MOD_TAP_MAX:
			case QK_LAYER_TAP...QK_LAYER_TAP_MAX:
				keycode &= 0x00FF;
		}
	}
	// Deactivate caps lock with listed key codes
	switch (keycode) {
		case KC_TAB:
		case KC_ESC:
		case KC_SPC:
		case KC_ENT:
		case KC_DOT:
		case KC_EQL:
		case KC_GESC:
			tap_code(KC_CAPS);
	}
}


// Handles mod tap by sending custom key code on hold after TAPPING_TERM
static bool process_tap_hold(uint16_t hold_keycode, keyrecord_t *record) {
	if (record->tap.count) { return true; }
	else if (record->event.pressed) { tap_code16(hold_keycode); }
	return false;
}


bool process_record_user(uint16_t const keycode, keyrecord_t *record) {
#ifdef OLED_ENABLE // Reset OLED animation tap timer
	if (record->event.pressed) { tap_timer = timer_read32(); }
#endif

	if (record->event.pressed && host_keyboard_led_state().caps_lock) { process_caps_word(keycode, record); }

	switch (keycode) {
		// Undo cut copy paste using LT(0,kc)
		case SLSH_TH: return process_tap_hold(Z_UND, record);
		case DOT_TH:  return process_tap_hold(Z_CUT, record);
		case COMM_TH: return process_tap_hold(Z_CPY, record);
		case M_TH:    return process_tap_hold(Z_PST, record);
		case V_TH:    return process_tap_hold(Z_PASTE, record);
	}

	return true; // Continue with unmatched keycodes
}
