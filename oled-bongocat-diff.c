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

/* Graphical bongocat animation, driven by key press timer or WPM.
   It has left and right aligned cats optimized for both OLEDs.
   This code saves firmware space by rendering base pixels followed
   by only changed pixels on subsequent animation frames.
   This should be rendered with OLED_ROTATION_270.

   Inspired by @j-inc's bongocat animation code
   (keyboards/kyria/keymaps/j-inc)

   Cat images courtesy of @plandevida

   Differential code is modified from @Dake's Modular Bongocat
   (https://github.com/Dakes/kyria/blob/main/keymaps/dakes/)


   Usage guide
   1 Place this file next to keymap.c or in userspace.
   2 Add the following lines into rules.mk:
        OLED_ENABLE = yes
        SRC += oled-bongocat-diff.c
   3 Left and right aligned Bongocat is default. To save space:
      * Add 'OPT_DEFS += -DLEFTCAT' into rules.mk
      * Or 'OPT_DEFS += -DRIGHTCAT' into rules.mk
   4 To animate with WPM, add 'WPM_ENABLE = yes' into rules.mk.
     Otherwise add the following integer variable and 'if'
     statement inside 'process_record_user()' in keymap.c:
        uint32_t tap_timer = 0;
        bool process_record_user(uint16_t keycode, keyrecord_t *record) {
            if (record->event.pressed) { tap_timer = timer_read32(); }
        }
   5 The 'oled_task_user()' calls 'render_mod_status()' for secondary OLED.
     It can be replaced with your own function, or delete the 'else' line.
 */

#include QMK_KEYBOARD_H

#define IDLE_FRAMES 5
#define TAP_FRAMES  2
#define FRAME_DURATION 200 // Milliseconds per frame
#define WIDTH OLED_DISPLAY_HEIGHT // 32px with OLED_ROTATION_270


// Base pixel frame
#ifndef LEFTCAT
static uint16_t const base[] PROGMEM = {192,
	0x8220,0x8240,0x8260,0x8280,0x82a0,0x82c1,0x82e1,0x8301,0x8321,0x8341,0x8362,0x8382,0x83a2,0x83c2,0x83e2,0x8403,
	0x8423,0x8443,0x8463,0x8483,0x84a4,0x84c4,0x84e4,0x8504,0x8524,0x8545,0x8565,0x8585,0x85a5,0x85c5,0x85e6,0x8606,
	0x8607,0x8608,0x8626,0x8629,0x862a,0x862b,0x8633,0x8634,0x8635,0x8636,0x8646,0x864c,0x864d,0x8650,0x8651,0x8652,
	0x8657,0x8667,0x866e,0x866f,0x8678,0x8687,0x8698,0x86a7,0x86b7,0x86c7,0x86d7,0x86e7,0x86f6,0x8707,0x8716,0x8726,
	0x8737,0x8746,0x8758,0x8765,0x8779,0x8785,0x8799,0x87a4,0x87ba,0x87c4,0x87cf,0x87d0,0x87da,0x87e4,0x87ef,0x87f0,
	0x87fa,0x8804,0x881a,0x8824,0x8829,0x883b,0x8845,0x8846,0x8847,0x8848,0x884a,0x885b,0x886a,0x886f,0x887b,0x888a,
	0x888e,0x889b,0x88ab,0x88ae,0x88bc,0x88cb,0x88cf,0x88dd,0x88eb,0x88ef,0x88fe,0x890b,0x890f,0x891f,0x892c,0x892f,
	0x8930,0x893f,0x894c,0x895e,0x896c,0x8972,0x8973,0x897c,0x897d,0x898c,0x8992,0x8993,0x899a,0x899b,0x89ac,0x89b9,
	0x89cd,0x89d8,0x89ed,0x89f7,0x8a0c,0x8a17,0x8a2c,0x8a36,0x8a4b,0x8a55,0x8a6b,0x8a74,0x8a8b,0x8a93,0x8aab,0x8ab2,
	0x8acb,0x8ad1,0x8aeb,0x8aef,0x8af0,0x8b0c,0x8b0d,0x8b0e,0x8b0f,0x8b2f,0x8b4f,0x8b70,0x8b90,0x8bb0,0x8bd0,0x8bf1,
	0x8c11,0x8c31,0x8c51,0x8c72,0x8c92,0x8cb2,0x8cd2,0x8cf3,0x8d13,0x8d33,0x8d53,0x8d74,0x8d94,0x8db4,0x8dd4,0x8df4,
	0x8e15,0x8e35,0x8e55,0x8e75,0x8e95,0x8eb6,0x8ed6,0x8ef6,0x8f16,0x8f37,0x8f57,0x8f77,0x8f97,0x8fb8,0x8fd8,0x8ff8};
#endif
#ifndef RIGHTCAT
static uint16_t const left_base[] PROGMEM = {192,
	0x823f,0x825f,0x827f,0x829f,0x82bf,0x82de,0x82fe,0x831e,0x833e,0x835e,0x837d,0x839d,0x83bd,0x83dd,0x83fd,0x841c,
	0x843c,0x845c,0x847c,0x849c,0x84bb,0x84db,0x84fb,0x851b,0x853b,0x855a,0x857a,0x859a,0x85ba,0x85da,0x85f9,0x8617,
	0x8618,0x8619,0x8629,0x862a,0x862b,0x862c,0x8634,0x8635,0x8636,0x8639,0x8648,0x864d,0x864e,0x864f,0x8652,0x8653,
	0x8659,0x8667,0x8670,0x8671,0x8678,0x8687,0x8698,0x86a8,0x86b8,0x86c8,0x86d8,0x86e9,0x86f8,0x8709,0x8718,0x8728,
	0x8739,0x8747,0x8759,0x8766,0x877a,0x8786,0x879a,0x87a5,0x87bb,0x87c5,0x87cf,0x87d0,0x87db,0x87e5,0x87ef,0x87f0,
	0x87fb,0x8805,0x881b,0x8824,0x8836,0x883b,0x8844,0x8855,0x8857,0x8858,0x8859,0x885a,0x8864,0x8870,0x8875,0x8884,
	0x8891,0x8895,0x88a3,0x88b1,0x88b4,0x88c2,0x88d0,0x88d4,0x88e1,0x88f0,0x88f4,0x8900,0x8910,0x8914,0x8920,0x892f,
	0x8930,0x8933,0x8941,0x8953,0x8962,0x8963,0x896c,0x896d,0x8973,0x8984,0x8985,0x898c,0x898d,0x8993,0x89a6,0x89b3,
	0x89c7,0x89d2,0x89e8,0x89f2,0x8a08,0x8a13,0x8a29,0x8a33,0x8a4a,0x8a54,0x8a6b,0x8a74,0x8a8c,0x8a94,0x8aad,0x8ab4,
	0x8ace,0x8ad4,0x8aef,0x8af0,0x8af4,0x8b10,0x8b11,0x8b12,0x8b13,0x8b30,0x8b50,0x8b6f,0x8b8f,0x8baf,0x8bcf,0x8bee,
	0x8c0e,0x8c2e,0x8c4e,0x8c6d,0x8c8d,0x8cad,0x8ccd,0x8cec,0x8d0c,0x8d2c,0x8d4c,0x8d6b,0x8d8b,0x8dab,0x8dcb,0x8deb,
	0x8e0a,0x8e2a,0x8e4a,0x8e6a,0x8e8a,0x8ea9,0x8ec9,0x8ee9,0x8f09,0x8f28,0x8f48,0x8f68,0x8f88,0x8fa7,0x8fc7,0x8fe7};
#endif


// Function that loops through pixel frame array to read
// and render on or off state bit at pixel coordinates
static void render_array(uint16_t const *frame) {
	// Get size from first array element
	uint16_t const size = pgm_read_word(frame);

	for (uint16_t i = size; i > 0; --i) {
		uint16_t cur_px = pgm_read_word(frame + i);
		// Get pixel on/off state bit
		bool const on = (cur_px & (1 << 15)) >> 15;
		// Remove state bit from pixel coordinates
		cur_px &= ~(1 << 15);
		oled_write_pixel(cur_px % WIDTH, cur_px / WIDTH, on);
	}
}


static void render_frames(uint16_t const *base, uint16_t const *diff) {
	render_array(base);
	render_array(diff);
}


static void render_cat_idle(void) {
	// Idle pixel frame differences
#ifndef LEFTCAT
	static uint16_t const idle0[] PROGMEM = {0};
	static uint16_t const idle1[] PROGMEM = {105,
		0x8632,0x0636,0x864b,0x064d,0x864f,0x0652,0x8656,0x0657,0x866d,0x066f,0x8677,0x0678,0x8697,0x0698,0x86b6,0x06b7,
		0x86d6,0x06d7,0x86f5,0x06f6,0x8715,0x0716,0x8736,0x0737,0x8757,0x0758,0x8778,0x0779,0x8798,0x0799,0x87b9,0x07ba,
		0x87ce,0x07d0,0x87d9,0x07da,0x87ee,0x07f0,0x87f9,0x07fa,0x8819,0x081a,0x883a,0x083b,0x885a,0x085b,0x886e,0x086f,
		0x887a,0x087b,0x888d,0x088e,0x889a,0x089b,0x88ad,0x08ae,0x88bb,0x08bc,0x88ce,0x08cf,0x88dc,0x08dd,0x88ee,0x08ef,
		0x88fd,0x08fe,0x890e,0x090f,0x891e,0x091f,0x892e,0x0930,0x893e,0x093f,0x895d,0x095e,0x8971,0x0973,0x897b,0x097d,
		0x8991,0x0993,0x8999,0x099b,0x89b8,0x09b9,0x89d7,0x09d8,0x89f6,0x09f7,0x8a16,0x0a17,0x8a35,0x0a36,0x8a54,0x0a55,
		0x8a73,0x0a74,0x8a92,0x0a93,0x8ab1,0x0ab2,0x8ad0,0x0ad1,0x0af0};
	static uint16_t const idle2[] PROGMEM = {31,
		0x8609,0x0629,0x862c,0x0635,0x0636,0x064c,0x0650,0x8655,0x0657,0x8670,0x8676,0x0678,0x8696,0x0698,0x86b6,0x06b7,
		0x86d6,0x06d7,0x88bb,0x08bc,0x88dc,0x08dd,0x88fc,0x08fe,0x891d,0x091f,0x893d,0x093f,0x895d,0x095e,0x097d};
	static uint16_t const idle3[] PROGMEM = {21,
		0x0636,0x8656,0x0657,0x8677,0x0678,0x8697,0x0698,0x86b6,0x06b7,0x86d6,0x06d7,0x88dc,0x08dd,0x88fd,0x08fe,0x891e,
		0x091f,0x893e,0x093f,0x097c,0x899c};
	static uint16_t const *idle_diff[IDLE_FRAMES] = {
		idle0, idle0, idle1, idle2, idle3 };
#endif
#ifndef RIGHTCAT
	static uint16_t const left_idle0[] PROGMEM = {0};
	static uint16_t const left_idle1[] PROGMEM = {105,
		0x0629,0x862d,0x0648,0x8649,0x064d,0x8650,0x0652,0x8654,0x0667,0x8668,0x0670,0x8672,0x0687,0x8688,0x06a8,0x86a9,
		0x06c8,0x86c9,0x06e9,0x86ea,0x0709,0x870a,0x0728,0x8729,0x0747,0x8748,0x0766,0x8767,0x0786,0x8787,0x07a5,0x87a6,
		0x07c5,0x87c6,0x07cf,0x87d1,0x07e5,0x87e6,0x07ef,0x87f1,0x0805,0x8806,0x0824,0x8825,0x0844,0x8845,0x0864,0x8865,
		0x0870,0x8871,0x0884,0x8885,0x0891,0x8892,0x08a3,0x88a4,0x08b1,0x88b2,0x08c2,0x88c3,0x08d0,0x88d1,0x08e1,0x88e2,
		0x08f0,0x88f1,0x0900,0x8901,0x0910,0x8911,0x0920,0x8921,0x092f,0x8931,0x0941,0x8942,0x0962,0x8964,0x096c,0x896e,
		0x0984,0x8986,0x098c,0x898e,0x09a6,0x89a7,0x09c7,0x89c8,0x09e8,0x89e9,0x0a08,0x8a09,0x0a29,0x8a2a,0x0a4a,0x8a4b,
		0x0a6b,0x8a6c,0x0a8c,0x8a8d,0x0aad,0x8aae,0x0ace,0x8acf,0x0aef};
	static uint16_t const left_idle2[] PROGMEM = {31,
		0x8616,0x0629,0x062a,0x8633,0x0636,0x0648,0x864a,0x064f,0x0653,0x0667,0x8669,0x866f,0x0687,0x8689,0x06a8,0x86a9,
		0x06c8,0x86c9,0x08a3,0x88a4,0x08c2,0x88c3,0x08e1,0x88e3,0x0900,0x8902,0x0920,0x8922,0x0941,0x8942,0x0962};
	static uint16_t const left_idle3[] PROGMEM = {21,
		0x0629,0x0648,0x8649,0x0667,0x8668,0x0687,0x8688,0x06a8,0x86a9,0x06c8,0x86c9,0x08c2,0x88c3,0x08e1,0x88e2,0x0900,
		0x8901,0x0920,0x8921,0x0963,0x8983};
	static uint16_t const *left_idle_diff[IDLE_FRAMES] = {
		left_idle0, left_idle0, left_idle1, left_idle2, left_idle3 };
#endif

	static uint8_t current_frame = 0;
	current_frame = current_frame < IDLE_FRAMES - 1 ? current_frame + 1 : 0;

#if defined(LEFTCAT)
	render_frames(left_base, left_idle_diff[current_frame]);
#elif defined(RIGHTCAT)
	render_frames(base, idle_diff[current_frame]);
#else
	is_keyboard_left() ? render_frames(left_base, left_idle_diff[current_frame]) : render_frames(base, idle_diff[current_frame]);
#endif
}


static void render_cat_prep(void) {
	// Prep pixel frame differences
#ifndef LEFTCAT
	static uint16_t const prep0[] PROGMEM = {82,
		0x86cb,0x86cc,0x86ed,0x86ee,0x870d,0x870f,0x0726,0x8728,0x872b,0x872c,0x8730,0x0746,0x8748,0x874b,0x874c,0x874f,
		0x8750,0x0765,0x8768,0x876d,0x8770,0x0785,0x8788,0x8789,0x878e,0x878f,0x07a4,0x87a9,0x87aa,0x87ab,0x87ac,0x87ad,
		0x07c4,0x87c9,0x07e4,0x87e9,0x0804,0x8809,0x0824,0x0845,0x0846,0x0847,0x0848,0x0a0c,0x8a0d,0x8a10,0x8a11,0x8a12,
		0x0a2c,0x8a2d,0x8a33,0x8a34,0x0a4b,0x8a4d,0x8a56,0x0a6b,0x8a6d,0x8a77,0x0a8b,0x8a8e,0x8a92,0x8a97,0x0aab,0x8aae,
		0x8ab3,0x8ab5,0x8ab7,0x0acb,0x8ace,0x0ad1,0x8ad7,0x0aeb,0x8af4,0x8af6,0x0b0c,0x0b0d,0x0b0e,0x8b11,0x8b12,0x8b13,
		0x8b14,0x8b15};
#endif
#ifndef RIGHTCAT
	static uint16_t const left_prep0[] PROGMEM = {82,
		0x86d3,0x86d4,0x86f1,0x86f2,0x8710,0x8712,0x872f,0x8733,0x8734,0x8737,0x0739,0x874f,0x8750,0x8753,0x8754,0x8757,
		0x0759,0x876f,0x8772,0x8777,0x077a,0x8790,0x8791,0x8796,0x8797,0x079a,0x87b2,0x87b3,0x87b4,0x87b5,0x87b6,0x07bb,
		0x87d6,0x07db,0x87f6,0x07fb,0x8816,0x081b,0x083b,0x0857,0x0858,0x0859,0x085a,0x8a0d,0x8a0e,0x8a0f,0x8a12,0x0a13,
		0x8a2b,0x8a2c,0x8a32,0x0a33,0x8a49,0x8a52,0x0a54,0x8a68,0x8a72,0x0a74,0x8a88,0x8a8d,0x8a91,0x0a94,0x8aa8,0x8aaa,
		0x8aac,0x8ab1,0x0ab4,0x8ac8,0x0ace,0x8ad1,0x0ad4,0x8ae9,0x8aeb,0x0af4,0x8b0a,0x8b0b,0x8b0c,0x8b0d,0x8b0e,0x0b11,
		0x0b12,0x0b13};
#endif

#if defined(LEFTCAT)
	render_frames(left_base, left_prep0);
#elif defined(RIGHTCAT)
	render_frames(base, prep0);
#else
	is_keyboard_left() ? render_frames(left_base, left_prep0) : render_frames(base, prep0);
#endif
}


static void render_cat_tap(void) {
	// Tap pixel frame differences
#ifndef LEFTCAT
	static uint16_t const tap0[] PROGMEM = {156,
		0x86cb,0x86cc,0x86ed,0x86ee,0x870d,0x870f,0x0726,0x8728,0x872b,0x872c,0x8730,0x0746,0x8748,0x874b,0x874c,0x874f,
		0x8750,0x0765,0x8768,0x876d,0x8770,0x0785,0x8789,0x878e,0x878f,0x07a4,0x87a9,0x87aa,0x87ab,0x87ac,0x87ad,0x07c4,
		0x87c9,0x07e4,0x87e9,0x0804,0x8809,0x0824,0x0845,0x0846,0x0847,0x0848,0x8ad6,0x8ad7,0x8ad8,0x8ad9,0x8ada,0x8adb,
		0x8adc,0x8af5,0x8af6,0x8af7,0x8af8,0x8af9,0x8afa,0x8afb,0x8afc,0x8b15,0x8b16,0x8b17,0x8b18,0x8b19,0x8b1a,0x8b1b,
		0x8b1c,0x8b37,0x8b38,0x8b39,0x8b3a,0x8b3b,0x8b3c,0x8b4a,0x8b4b,0x8b4c,0x8b4d,0x8b69,0x8b6a,0x8b6b,0x8b6c,0x8b6d,
		0x8b72,0x8b73,0x8b74,0x8b88,0x8b89,0x8b8a,0x8b8b,0x8b8c,0x8b8d,0x8b92,0x8b93,0x8b94,0x8b95,0x8ba7,0x8ba8,0x8ba9,
		0x8baa,0x8bab,0x8bac,0x8bad,0x8bb2,0x8bb3,0x8bb4,0x8bb5,0x8bb6,0x8bc6,0x8bc7,0x8bc8,0x8bc9,0x8bca,0x8bcb,0x8bcc,
		0x8bcd,0x8bd3,0x8bd4,0x8bd5,0x8bd6,0x8bd7,0x8be6,0x8be7,0x8be8,0x8be9,0x8bea,0x8beb,0x8bec,0x8bed,0x8bf3,0x8bf4,
		0x8bf5,0x8bf6,0x8bf7,0x8bf8,0x8c08,0x8c09,0x8c0a,0x8c0b,0x8c0c,0x8c0d,0x8c13,0x8c14,0x8c15,0x8c16,0x8c17,0x8c18,
		0x8c2a,0x8c2b,0x8c2c,0x8c2d,0x8c34,0x8c35,0x8c36,0x8c37,0x8c4c,0x8c4d,0x8c54,0x8c55};
	static uint16_t const tap1[] PROGMEM = {72,
		0x8621,0x8622,0x8623,0x8624,0x8641,0x8642,0x8643,0x8644,0x8661,0x8662,0x8663,0x8664,0x8681,0x8682,0x8683,0x8684,
		0x86a1,0x86a2,0x86a3,0x86a4,0x86c1,0x86c2,0x86c3,0x86c4,0x86e2,0x86e3,0x86e4,0x8702,0x8703,0x8800,0x8820,0x8840,
		0x8860,0x0a0c,0x8a0d,0x8a10,0x8a11,0x8a12,0x0a2c,0x8a2d,0x8a33,0x8a34,0x0a4b,0x8a4d,0x8a56,0x0a6b,0x8a6d,0x8a77,
		0x0a8b,0x8a8e,0x8a92,0x8a97,0x0aab,0x8aae,0x8ab3,0x8ab5,0x8ab7,0x0acb,0x8ace,0x0ad1,0x8ad7,0x0aeb,0x8af4,0x8af6,
		0x0b0c,0x0b0d,0x0b0e,0x8b11,0x8b12,0x8b13,0x8b14,0x8b15};
	static uint16_t const *tap_diff[TAP_FRAMES] = {
		tap0, tap1 };
#endif
#ifndef RIGHTCAT
	static uint16_t const left_tap0[] PROGMEM = {156,
		0x86d3,0x86d4,0x86f1,0x86f2,0x8710,0x8712,0x872f,0x8733,0x8734,0x8737,0x0739,0x874f,0x8750,0x8753,0x8754,0x8757,
		0x0759,0x876f,0x8772,0x8777,0x077a,0x8790,0x8791,0x8796,0x079a,0x87b2,0x87b3,0x87b4,0x87b5,0x87b6,0x07bb,0x87d6,
		0x07db,0x87f6,0x07fb,0x8816,0x081b,0x083b,0x0857,0x0858,0x0859,0x085a,0x8ac3,0x8ac4,0x8ac5,0x8ac6,0x8ac7,0x8ac8,
		0x8ac9,0x8ae3,0x8ae4,0x8ae5,0x8ae6,0x8ae7,0x8ae8,0x8ae9,0x8aea,0x8b03,0x8b04,0x8b05,0x8b06,0x8b07,0x8b08,0x8b09,
		0x8b0a,0x8b23,0x8b24,0x8b25,0x8b26,0x8b27,0x8b28,0x8b52,0x8b53,0x8b54,0x8b55,0x8b6b,0x8b6c,0x8b6d,0x8b72,0x8b73,
		0x8b74,0x8b75,0x8b76,0x8b8a,0x8b8b,0x8b8c,0x8b8d,0x8b92,0x8b93,0x8b94,0x8b95,0x8b96,0x8b97,0x8ba9,0x8baa,0x8bab,
		0x8bac,0x8bad,0x8bb2,0x8bb3,0x8bb4,0x8bb5,0x8bb6,0x8bb7,0x8bb8,0x8bc8,0x8bc9,0x8bca,0x8bcb,0x8bcc,0x8bd2,0x8bd3,
		0x8bd4,0x8bd5,0x8bd6,0x8bd7,0x8bd8,0x8bd9,0x8be7,0x8be8,0x8be9,0x8bea,0x8beb,0x8bec,0x8bf2,0x8bf3,0x8bf4,0x8bf5,
		0x8bf6,0x8bf7,0x8bf8,0x8bf9,0x8c07,0x8c08,0x8c09,0x8c0a,0x8c0b,0x8c0c,0x8c12,0x8c13,0x8c14,0x8c15,0x8c16,0x8c17,
		0x8c28,0x8c29,0x8c2a,0x8c2b,0x8c32,0x8c33,0x8c34,0x8c35,0x8c4a,0x8c4b,0x8c52,0x8c53};
	static uint16_t const left_tap1[] PROGMEM = {72,
		0x863b,0x863c,0x863d,0x863e,0x865b,0x865c,0x865d,0x865e,0x867b,0x867c,0x867d,0x867e,0x869b,0x869c,0x869d,0x869e,
		0x86bb,0x86bc,0x86bd,0x86be,0x86db,0x86dc,0x86dd,0x86de,0x86fb,0x86fc,0x86fd,0x871c,0x871d,0x881f,0x883f,0x885f,
		0x887f,0x8a0d,0x8a0e,0x8a0f,0x8a12,0x0a13,0x8a2b,0x8a2c,0x8a32,0x0a33,0x8a49,0x8a52,0x0a54,0x8a68,0x8a72,0x0a74,
		0x8a88,0x8a8d,0x8a91,0x0a94,0x8aa8,0x8aaa,0x8aac,0x8ab1,0x0ab4,0x8ac8,0x0ace,0x8ad1,0x0ad4,0x8ae9,0x8aeb,0x0af4,
		0x8b0a,0x8b0b,0x8b0c,0x8b0d,0x8b0e,0x0b11,0x0b12,0x0b13};
	static uint16_t const *left_tap_diff[TAP_FRAMES] = {
		left_tap0, left_tap1 };
#endif

	static uint8_t current_frame = 0;
	current_frame = (current_frame + 1) & 1;

#if defined(LEFTCAT)
	render_frames(left_base, left_tap_diff[current_frame]);
#elif defined(RIGHTCAT)
	render_frames(base, tap_diff[current_frame]);
#else
	is_keyboard_left() ? render_frames(left_base, left_tap_diff[current_frame]) : render_frames(base, tap_diff[current_frame]);
#endif
}


void render_bongocat(void) {
	// Animation timer
	static uint16_t anim_timer = 0;

#ifdef WPM_ENABLE
	static uint8_t prev_wpm = 0;
	static uint32_t tap_timer = 0;
	// tap_timer updated by sustained WPM
	if (get_current_wpm() > prev_wpm) { tap_timer = timer_read32(); }
	prev_wpm = get_current_wpm();
#else
	// tap_timer updated by key presses in process_record_user()
	extern uint32_t tap_timer;
#endif

	// Time gap between tap_timer updates
	uint32_t keystroke = timer_elapsed32(tap_timer);

	void animate_cat(void) {
		oled_clear();
		if (keystroke < FRAME_DURATION*2) { render_cat_tap(); }
		else if (keystroke < FRAME_DURATION*8) { render_cat_prep(); }
		else { render_cat_idle(); }
	}

	if (keystroke > OLED_TIMEOUT) { oled_off(); }
	else if (timer_elapsed(anim_timer) > FRAME_DURATION) {
		anim_timer = timer_read();
		animate_cat();
	}
}


// Init and rendering calls
oled_rotation_t oled_init_user(oled_rotation_t const rotation) {
	return OLED_ROTATION_270;
}

void oled_task_user(void) {
	extern void render_mod_status(void);
	if (is_keyboard_master()) { render_bongocat(); }
	else { render_mod_status(); }
}
