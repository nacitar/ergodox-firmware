/* ----------------------------------------------------------------------------
* ergoDOX layout : Workman-P (modified from the Kinesis layout)
* -----------------------------------------------------------------------------
* Copyright (c) 2012 Ben Blazak <benblazak.dev@gmail.com>
* Released under The MIT License (MIT) (see "license.md")
* Project located at <https://github.com/benblazak/ergodox-firmware>
* -------------------------------------------------------------------------- */

// TODO: there is a bug where if you hit an inverted key and a normal key, at
// the same time, when the 6 key buffer and modifier states are sent the
// inverted key's shift state can be used for the non-inverted key.
// Example: hit 1 and q at the same time in workman-p, you may end up with "!Q"
// instead of "!q".  Not sure how to fix this at present, but it may require
// sending two frames of updates to properly denote an order (shifted in one
// update, unshifted added in another, as though shift was released).

#include <stdint.h>
#include <stddef.h>
#include <avr/pgmspace.h>
#include "../../../lib/data-types/misc.h"
#include "../../../lib/usb/usage-page/keyboard--short-names.h"
#include "../matrix.h"
#include "../layout.h"

// PRIVATE FUNCTIONS ----------------------------------------------------------
#include "../../../lib/key-functions/private.h"
static uint8_t inverted_keys_pressed;
static bool physical_lshift_pressed;
static bool physical_rshift_pressed;

void _kbfun_invert_shift_state(void) {
  // make lshift's state the inverted shift state
  _kbfun_press_release(!(physical_lshift_pressed|physical_rshift_pressed), KEY_LeftShift);
  // release rshift
  _kbfun_press_release(false, KEY_RightShift);
}
void _kbfun_restore_shift_state(void) {
  // restore the state of left and right shift
  _kbfun_press_release(physical_lshift_pressed, KEY_LeftShift);
  _kbfun_press_release(physical_rshift_pressed, KEY_RightShift);
}


// PUBLIC FUNCTIONS -----------------------------------------------------------
#include "../../../lib/key-functions/public.h"
#include "../../../main.h"
#define  IS_PRESSED    main_arg_is_pressed
#define  LAYER         main_arg_layer
#define  ROW           main_arg_row
#define  COL           main_arg_col

// TODO: This replaces kbfun_shift_press_release as far as I'm concerned,
//     though it's technically different.
/*
 * [name]
 *   Invert shift + press|release
 *
 * [description]
 *   Generate a 'shift' press or release before the normal keypress or
 *   key release if shift is not pressed.  Generate a normal keypress or
 *   key release if shift is pressed.
 */
void kbfun_shift_inverted_press_release(void) {
  if (IS_PRESSED) {
    ++inverted_keys_pressed;
    _kbfun_invert_shift_state();
  }

  kbfun_press_release();

  if (!IS_PRESSED) {
    // if this is the last key we're releasing
    if (inverted_keys_pressed == 1) {
      _kbfun_restore_shift_state();
    }
    // avoid underflow
    if (inverted_keys_pressed) {
      --inverted_keys_pressed;
    }
  }
}
// TODO: This replaces the default kbfun_press_release,
//     adding support for shift inversion
/*
 * [name]
 *   Shift state fix + press|release
 *
 * [description]
 *   If no inverted keys are pressed, simply perform a press and release.
 *   If inverted keys are pressed, fix the shift state back to that of the
 *   physical keys before pressing the key.
 */
void kbfun_press_release_supporting_shift_inversion(void) {
  uint8_t keycode = kb_layout_get(LAYER, ROW, COL);
  switch (keycode) {
    // shift state toggles
    case KEY_LeftShift:
      physical_lshift_pressed = IS_PRESSED;
      break;
    case KEY_RightShift:
      physical_rshift_pressed = IS_PRESSED;
      break;
    // Keys which don't break it
    case KEY_CapsLock:
    case KEYPAD_NumLock_Clear:
      kbfun_press_release();
      return;
    default:
      // If we're not just changing the modifier, we need our true shift state.
      if (inverted_keys_pressed) {
        inverted_keys_pressed = 0;
        _kbfun_restore_shift_state();
      }
      kbfun_press_release();
      return;
  }
  // We only get here if we pressed left or right shift
  if (inverted_keys_pressed) {
    _kbfun_invert_shift_state();
  } else {
    kbfun_press_release();
  }
}

// DEFINITIONS ----------------------------------------------------------------
// --- key functions
#define  kprrel   &kbfun_press_release_supporting_shift_inversion // overridden
#define  mprrel   &kbfun_mediakey_press_release
#define  ktog     &kbfun_toggle
#define  ktrans   &kbfun_transparent
#define  sinvert  &kbfun_shift_inverted_press_release
#define  s2kcap   &kbfun_2_keys_capslock_press_release
// --- layer push/pop functions
#define  lpush1   &kbfun_layer_push_1
#define  lpush2   &kbfun_layer_push_2
#define  lpush3   &kbfun_layer_push_3
#define  lpush4   &kbfun_layer_push_4
#define  lpush5   &kbfun_layer_push_5
#define  lpop     &kbfun_layer_pop_all
#define  lpop1    &kbfun_layer_pop_1
#define  lpop2    &kbfun_layer_pop_2
#define  lpop3    &kbfun_layer_pop_3
#define  lpop4    &kbfun_layer_pop_4
#define  lpop5    &kbfun_layer_pop_5
#define  ltog1    &kbfun_layer_toggle_1
#define  ltog2    &kbfun_layer_toggle_2
#define  ltog3    &kbfun_layer_toggle_3
#define  ltog4    &kbfun_layer_toggle_4
#define  ltog5    &kbfun_layer_toggle_5
// --- device
#define  dbtldr   &kbfun_jump_to_bootloader

// LAYOUT ---------------------------------------------------------------------
const uint8_t PROGMEM _kb_layout[KB_LAYERS][KB_ROWS][KB_COLUMNS] = {
// LAYER 0 - Base layout
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  KEY_Equal_Plus, KEY_1_Exclamation,     KEY_2_At,           KEY_3_Pound,   KEY_4_Dollar,   KEY_5_Percent, KEY_Application,
  KEY_Tab,        KEY_q_Q,               KEY_d_D,            KEY_r_R,       KEY_w_W,        KEY_b_B,       1,
  KEY_Escape,     KEY_a_A,               KEY_s_S,            KEY_h_H,       KEY_t_T,        KEY_g_G,       /*no key*/
  KEY_LeftShift,  KEY_z_Z,               KEY_x_X,            KEY_m_M,       KEY_c_C,        KEY_v_V,       KEY_LeftAlt,
  KEY_LeftGUI,    KEY_GraveAccent_Tilde, KEY_Backslash_Pipe, KEY_LeftArrow, KEY_RightArrow, /*no key*/    /*no key*/
  // left thumb
  /*no key*/           KEY_LeftControl,   KEY_PrintScreen,
  0 /*no key*/,        0 /*no key*/,      KEY_Home,
  KEY_DeleteBackspace, KEY_DeleteForward, KEY_End,

  // right hand
  2,            KEY_6_Caret, KEY_7_Ampersand, KEY_8_Asterisk,     KEY_9_LeftParenthesis,     KEY_0_RightParenthesis,      KEY_Dash_Underscore,
  1,            KEY_j_J,     KEY_f_F,         KEY_u_U,            KEY_p_P,                   KEY_Semicolon_Colon,         KEY_Backslash_Pipe,
  /*no key*/    KEY_y_Y,     KEY_n_N,         KEY_e_E,            KEY_o_O,                   KEY_i_I,                     KEY_SingleQuote_DoubleQuote,
  KEY_RightAlt, KEY_k_K,     KEY_l_L,         KEY_Comma_LessThan, KEY_Period_GreaterThan,    KEY_Slash_Question,          KEY_RightShift,
  /*no key*/    /*no key*/   KEY_UpArrow,     KEY_DownArrow,      KEY_LeftBracket_LeftBrace, KEY_RightBracket_RightBrace, KEY_RightGUI,
  // right thumb
  KEY_Pause,    KEY_RightControl, /*no key*/
  KEY_PageUp,   0 /*no key*/,     0 /*no key*/,
  KEY_PageDown, KEY_ReturnEnter,  KEY_Spacebar
),
// LAYER 1 - Function layer
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  KEY_CapsLock,  KEY_F1, KEY_F2, KEY_F3,              KEY_F4,              KEY_F5,    KEY_F11,
  0,             0,      0,      0,                   0,                   0,         0,
  0,             0,      0,      0,                   0,                   0,         /*no key*/
  0,             0,      0,      0,                   0,                   0,         0,
  0,             5,      0,      MEDIAKEY_PREV_TRACK, MEDIAKEY_NEXT_TRACK, /*no key*/ /*no key*/
  // left thumb
  /* no key*/    0,            0,
  0 /*no key*/,  0 /*no key*/, 0,
  MEDIAKEY_STOP, KEY_Insert,   0,

  // right hand
  KEY_F12,   KEY_F6,    KEY_F7,                KEY_F8,                  KEY_F9,              KEY_F10, KEY_ScrollLock,
  0,         0,         0,                     0,                       0,                   0,       0,
  /*no key*/ 0,         0,                     0,                       0,                   0,       0,
  0,         0,         0,                     0,                       0,                   0,       0,
  /*no key*/ /*no key*/ MEDIAKEY_AUDIO_VOL_UP, MEDIAKEY_AUDIO_VOL_DOWN, MEDIAKEY_AUDIO_MUTE, 4,       3,
  // right thumb
  0, 0,            /*no key*/
  0, 0 /*no key*/, 0 /*no key*/,
  0, 0,            MEDIAKEY_PLAY_PAUSE
),
// LAYER 2 - Numpad layer
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  0, 0, 0,          0, 0, 0,         0,
  0, 0, 0,          0, 0, 0,         0,
  0, 0, 0,          0, 0, 0,         /*no key*/
  0, 0, 0,          0, 0, 0,         0,
  0, 0, KEY_Insert, 0, 0, /*no key*/ /*no key*/
  // left thumb
  /*no key*/    0,            0,
  0 /*no key*/, 0 /*no key*/, 0,
  0,            0,            0,

  // right hand
  0,         0,         KEYPAD_NumLock_Clear, KEYPAD_Equal,       KEYPAD_Slash,         KEYPAD_Asterisk, 0,
  0,         0,         KEYPAD_7_Home,        KEYPAD_8_UpArrow,   KEYPAD_9_PageUp,      KEYPAD_Minus,    0,
  /*no key*/ 0,         KEYPAD_4_LeftArrow,   KEYPAD_5,           KEYPAD_6_RightArrow,  KEYPAD_Plus,     0,
  0,         0,         KEYPAD_1_End,         KEYPAD_2_DownArrow, KEYPAD_3_PageDown,    KEY_ReturnEnter, 0,
  /*no key*/ /*no key*/ 0,                    0,                  KEYPAD_Period_Delete, KEY_ReturnEnter, 0,
  // right thumb
  0, 0,            /*no key*/
  0, 0 /*no key*/, 0 /*no key*/,
  0, 0,            KEYPAD_0_Insert
),
// LAYER 3 - QWERTY conversion layer
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  0, 0,       0,       0,       0,       0,         0,
  0, KEY_q_Q, KEY_w_W, KEY_e_E, KEY_r_R, KEY_t_T,   0,
  0, KEY_a_A, KEY_s_S, KEY_d_D, KEY_f_F, KEY_g_G,   /*no key*/
  0, KEY_z_Z, KEY_x_X, KEY_c_C, KEY_v_V, KEY_b_B,   0,
  0, 0,       0,       0,       0,       /*no key*/ /*no key*/
  // left thumb
  /*no key*/    0,            0,
  0 /*no key*/, 0 /*no key*/, 0,
  0,            0,            0,

  // right hand
  0,         0,         0,       0,       0,       0,                   0,
  0,         KEY_y_Y,   KEY_u_U, KEY_i_I, KEY_o_O, KEY_p_P,             0,
  /*no key*/ KEY_h_H,   KEY_j_J, KEY_k_K, KEY_l_L, KEY_Semicolon_Colon, 0,
  0,         KEY_n_N,   KEY_m_M, 0,       0,       0,                   0,
  /*no key*/ /*no key*/ 0,       0,       0,       0,                   0,
  // right thumb
  0, 0,            /*no key*/
  0, 0 /*no key*/, 0 /*no key*/,
  0, 0,            0
),
// LAYER 4 - Workman-P to Workman conversion layer
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  0,  KEY_1_Exclamation,  KEY_2_At, KEY_3_Pound,  KEY_4_Dollar, KEY_5_Percent,  0,
  0,  0,                  0,        0,            0,            0,              0,
  0,  0,                  0,        0,            0,            0,              /*no key*/
  0,  0,                  0,        0,            0,            0,              0,
  0,  0,                  0,        0,            0,            /*no key*/      /*no key*/
  // left thumb
  /*no key*/    0,            0,
  0 /*no key*/, 0 /*no key*/, 0,
  0,            0,            0,

  // right hand
  0,          KEY_6_Caret,  KEY_7_Ampersand,  KEY_8_Asterisk, KEY_9_LeftParenthesis,  KEY_0_RightParenthesis, 0,
  0,          0,            0,                0,              0,                      0,                      0,
  /*no key*/  0,            0,                0,              0,                      0,                      0,
  0,          0,            0,                0,              0,                      0,                      0,
  /*no key*/ /*no key*/     0,                0,              0,                      0,                      0,
  // right thumb
  0, 0,            /*no key*/
  0, 0 /*no key*/, 0 /*no key*/,
  0, 0,            0
),
// LAYER 5 - Backspace/Space swap layer
KB_MATRIX_LAYER(
  0 /*no key*/,
  // left hand
  0,  0,  0,  0,  0,  0,         0,
  0,  0,  0,  0,  0,  0,         0,
  0,  0,  0,  0,  0,  0,         /*no key*/
  0,  0,  0,  0,  0,  0,         0,
  0,  0,  0,  0,  0,  /*no key*/ /*no key*/
  // left thumb
  /*no key*/    0,            0,
  0 /*no key*/, 0 /*no key*/, 0,
  KEY_Spacebar, 0,            0,

  // right hand
  0,         0,         0, 0, 0, 0, 0,
  0,         0,         0, 0, 0, 0, 0,
  /*no key*/ 0,         0, 0, 0, 0, 0,
  0,         0,         0, 0, 0, 0, 0,
  /*no key*/ /*no key*/ 0, 0, 0, 0, 0,
  // right thumb
  0, 0,            /*no key*/
  0, 0 /*no key*/, 0 /*no key*/,
  0, 0,            KEY_DeleteBackspace
)
};
// ----------------------------------------------------------------------------

// PRESS ----------------------------------------------------------------------
const void_funptr_t PROGMEM _kb_layout_press[KB_LAYERS][KB_ROWS][KB_COLUMNS] = {
// LAYER 0 - Base layout
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  kprrel, sinvert, sinvert, sinvert, sinvert, sinvert,   kprrel,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    lpush1,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    /*no key*/
  s2kcap, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    kprrel,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  /*no key*/ /*no key*/
  // left thumb
  /*no key*/       kprrel,          kprrel,
  NULL /*no key*/, NULL /*no key*/, kprrel,
  kprrel,          kprrel,          kprrel,

  // right hand
  ltog2,     sinvert,   sinvert, sinvert, sinvert, sinvert, kprrel,
  lpush1,    kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  /*no key*/ kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  kprrel,    kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  s2kcap,
  /*no key*/ /*no key*/ kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  // right thumb
  kprrel, kprrel,          /*no key*/
  kprrel, NULL /*no key*/, NULL /*no key*/,
  kprrel, kprrel,          kprrel
),
// LAYER 1 - Function layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  kprrel, kprrel, kprrel, kprrel, kprrel, kprrel,    kprrel,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  lpop,   ltog5,  ktrans, mprrel, mprrel, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  mprrel,          kprrel,          ktrans,

  // right hand
  kprrel,    kprrel,    kprrel, kprrel, kprrel, kprrel, kprrel,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ mprrel, mprrel, mprrel, ltog4,  ltog3,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          mprrel
),
// LAYER 2 - Numpad layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, kprrel, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, kprrel, kprrel, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          kprrel
),
// LAYER 3 - QWERTY conversion layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    /*no key*/
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    kprrel,    kprrel, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          ktrans
),
// LAYER 4 - Workman-P to Workman conversion layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          ktrans
),
// LAYER 5 - Backspace/Space swap layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  kprrel,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          kprrel
)
};
// ----------------------------------------------------------------------------

// RELEASE --------------------------------------------------------------------
const void_funptr_t PROGMEM _kb_layout_release[KB_LAYERS][KB_ROWS][KB_COLUMNS] = {
// LAYER 0 - Base layout
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  kprrel, sinvert, sinvert, sinvert, sinvert, sinvert,   kprrel,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    lpop1,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    /*no key*/
  s2kcap, kprrel,  kprrel,  kprrel,  kprrel,  kprrel,    kprrel,
  kprrel, kprrel,  kprrel,  kprrel,  kprrel,  /*no key*/ /*no key*/
  // left thumb
  /*no key*/       kprrel,          kprrel,
  NULL /*no key*/, NULL /*no key*/, kprrel,
  kprrel,          kprrel,          kprrel,

  // right hand
  NULL,      sinvert,   sinvert, sinvert, sinvert, sinvert, kprrel,
  lpop1,     kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  /*no key*/ kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  kprrel,    kprrel,    kprrel,  kprrel,  kprrel,  kprrel,  s2kcap,
  /*no key*/ /*no key*/ kprrel,  kprrel,  kprrel,  kprrel,  kprrel,
  // right thumb
  kprrel, kprrel,          /*no key*/
  kprrel, NULL /*no key*/, NULL /*no key*/,
  kprrel, kprrel,          kprrel
),
// LAYER 1 - Function layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  kprrel, kprrel, kprrel, kprrel, kprrel, kprrel,    kprrel,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  NULL,   NULL,   ktrans, mprrel, mprrel, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  mprrel,          kprrel,          ktrans,

  // right hand
  kprrel,    kprrel,    kprrel, kprrel, kprrel, kprrel, kprrel,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ mprrel, mprrel, mprrel, NULL, NULL,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          mprrel
),
// LAYER 2 - Numpad layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, kprrel, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, kprrel, kprrel, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          kprrel
),
// LAYER 3 - QWERTY conversion layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    /*no key*/
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  /*no key*/ kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    kprrel,    kprrel, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          ktrans
),
// LAYER 4 - Workman-P to Workman conversion layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, kprrel, kprrel, kprrel, kprrel, kprrel,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  ktrans,          ktrans,          ktrans,

  // right hand
  ktrans,    kprrel,    kprrel, kprrel, kprrel, kprrel, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          ktrans
),
// LAYER 5 - Backspace/Space swap layer
KB_MATRIX_LAYER(
  NULL /*no key*/,
  // left hand
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    /*no key*/
  ktrans, ktrans, ktrans, ktrans, ktrans, ktrans,    ktrans,
  ktrans, ktrans, ktrans, ktrans, ktrans, /*no key*/ /*no key*/
  // left thumb
  /*no key*/       ktrans,          ktrans,
  NULL /*no key*/, NULL /*no key*/, ktrans,
  kprrel,          ktrans,          ktrans,

  // right hand
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  ktrans,    ktrans,    ktrans, ktrans, ktrans, ktrans, ktrans,
  /*no key*/ /*no key*/ ktrans, ktrans, ktrans, ktrans, ktrans,
  // right thumb
  ktrans, ktrans,          /*no key*/
  ktrans, NULL /*no key*/, NULL /*no key*/,
  ktrans, ktrans,          kprrel
)
};
// ----------------------------------------------------------------------------
