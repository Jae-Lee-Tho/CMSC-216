// scale_update.c: functions that read scale hardware, convert its
// encode its state in a struct, and adjusts the display to show the
// user information.

#include "scale.h"

int scale_from_ports(scale_t *scale)
// Uses the values in SCALE_SENSOR_PORT, SCALE_TARE_PORT, and
// SCALE_STATUS_PORT to set the fields of the `scale` struct to
// appropriate values.
//
// Does bound checking so that if the SCALE_SENSOR_PORT or
// SCALE_TARE_PORT is out of range (see their docs) sets the fields of
// scale to be 0's, sets the scale->mode to be MODE_ERROR, and
// returns 1.
//
// If SCALE_STATUS_PORT indicates the Tare button is being pressed
// (see that variable's docs), sets fields of `scale` to be 0, sets
// the mode to be MODE_TARE, and returns 2.
//
// Otherwise sets the fields of scale as follows.
//
// - mode :: set to MODE_SHOW to show the display weight as normal
//
// - weight :: set to (SCALE_SENSOR_PORT - SCALE_TARE_PORT).  Converts
//   to pounds if SCALE_STATUS_PORT indicates that is the desired
//   unit. Conversion is done using bit shifting / masking and rounds
//   pounds up if the ounces are halfway to the next pound.
//
// - indicators :: sets a bit to turn on either the Ounce or Pound
//   Indicator light. If the SCALE_TARE_PORT is not zero, sets a bit
//   to turn on the Tare indicator light. See documentation for the
//   scale_t struct for which bits control which indicator lights.
//
// CONSTRAINTS: Uses only integer operations. No floating point
// operations are used as the target machine does not have a FPU. Does
// not use any math functions such as abs().
//
// CONSTRAINTS: Does not use integer division; uses bit shifting and
// masking to compute division by powers of two and perform rounding.
//
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use conditionals nested deeper than 2 levels. Seek to make the
// code as short, and simple as possible. Code longer than 70 lines
// may be penalized for complexity.
{
    scale->weight = 0;
    scale->mode = MODE_SHOW;
    scale->indicators = 0;

    short sensor = SCALE_SENSOR_PORT;
    short tare = SCALE_TARE_PORT;
    unsigned char status = SCALE_STATUS_PORT;

    if (sensor < 0 || sensor > 999 || tare < 0 || tare > 999)
    {
        scale->mode = MODE_ERROR;
        return 1;
    }

    if (status & (1 << 5))
    {
        scale->mode = MODE_TARE;
        return 2;
    }

    int weight = sensor - tare;

    if (status & (1 << 2))
    {

        weight = (weight + 8) >> 4;
        scale->indicators |= (1 << 1);
    }
    else
    {
        scale->indicators |= (1 << 0);
    }
    scale->weight = weight;

    if (tare != 0)
    {
        scale->indicators |= (1 << 2);
    }

    return 0;
}

int scale_display_special(scale_t scale, int *display)
// Sets the bits pointed at by `display to indicate one of two
// "special" cases.
//
// If scale.mode is MODE_ERROR, sets the display to "ERR" to indicate
// that the weight is out of range or some other hardware error has
// occurred.
//
// If scale.mode is MODE_TARE, sets the display to "STOR" to indicate
// that the Tare button is being pressed and the current weight will be
// stored in the internal saved weight space.
//
// In both the above cases, only changes the bits pointed at by
// `display` and returns 0. Does not make any other changes to the
// machine state.
//
// If scale.mode is a different value that one of the above two cases,
// this function has been called in error and returns 1.
//
// CONSTRAINT: This function should not access or change any global
// variables.
//
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use conditionals nested deeper than 2 levels. Seek to make the
// code as short, and simple as possible. Code longer than 35 lines
// may be penalized for complexity.
{
    // Define bit patterns for "ERR" and "STOR" based on 7-segment display encoding
    const int ERR_PATTERN = (0b0110111 << 21) | (0b1011111 << 14) | (0b1011111 << 7);              // "E", "R", "R"
    const int STOR_PATTERN = (0b1100111 << 21) | (0b1001001 << 14) | (0b1111011 << 7) | 0b1011111; // "S", "T", "O", "R"

    // Check for error mode
    if (scale.mode == MODE_ERROR)
    {
        *display = ERR_PATTERN;
        return 0;
    }

    // Check for tare mode
    if (scale.mode == MODE_TARE)
    {
        *display = STOR_PATTERN;
        return 0;
    }

    // Invalid mode, return error
    return 1;
}

int scale_display_weight(scale_t scale, int *display)
// Called when scale.mode is MODE_SHOW. If it is not, this function
// returns 1 immediately and makes no other changes.
//
// For scale.mode of MODE_SHOW, sets `display` bits to indicate the
// weight in scale.weight. Uses integer division to determine digits
// to be shown in `display` and whether a negative sign should be
// present. Sets bits of `display` according to the bits in
// scale.indicators to show ounces / pounds as the unit and indicate a
// non-zero tare. Returns 0 on completion.
//
// CONSTRAINT: This function should not access or change any global
// PORT variables. It may use a global array of bit patterns if deemed
// useful.
//
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use conditionals nested deeper than 2 levels. Seek to make the
// code as short, and simple as possible. Code longer than 65 lines
// may be penalized for complexity.
{
    if (scale.mode != MODE_SHOW)
        return 1;

    const int digit_patterns[10] = {
        0b1111011, // 0
        0b1001000, // 1
        0b0111101, // 2
        0b1101101, // 3
        0b1001110, // 4
        0b1100111, // 5
        0b1110111, // 6
        0b1001001, // 7
        0b1111111, // 8
        0b1101111  // 9
    };

    // 7-segment encoding for negative sign (horizontal line)
    const int negative_sign = 0b0000100;

    int weight = scale.weight;
    int negative = (weight < 0);
    if (negative)
        weight = -weight;

    // Extract digits (assumes weight < 1000)
    int ones = weight % 10;
    int tens = (weight / 10) % 10;
    int hundreds = (weight / 100) % 10;
    int num_digits = (weight >= 100) ? 3 : (weight >= 10) ? 2 : 1;

    // Reset display
    *display = 0;

    if (num_digits == 3) {
        // For 3-digit numbers, if negative, display the negative sign in an extra left slot.
        if (negative)
            *display |= negative_sign << 21;
        *display |= digit_patterns[hundreds] << 14;
        *display |= digit_patterns[tens] << 7;
        *display |= digit_patterns[ones];
    }
    else {
        // For both 2-digit and 1-digit numbers, negative sign goes to the left (shift 14).
        if (negative)
            *display |= negative_sign << 14;
        // For a 2-digit number, show the tens digit; for 1-digit, display a zero in the tens slot.
        int tens_digit = (num_digits == 2) ? tens : 0;
        *display |= digit_patterns[tens_digit] << 7;
        *display |= digit_patterns[ones];
    }

    // Set unit indicators: LB (bit 29) if indicator bit 1 is set, otherwise OZ (bit 28).
    *display |= (scale.indicators & (1 << 1)) ? (1 << 29) : (1 << 28);

    // Set Tare Indicator if needed (bit 30).
    if (scale.indicators & (1 << 2))
        *display |= (1 << 30);

    return 0;
}

int scale_update()
// Updates the state of scale using previously defined functions. Uses
// stack space for a scale_t struct which is set from the ports and
// then used to alter the display. If the MODE_TARE is indicated, then
// copies SCALE_SENSOR_PORT to SCALE_TARE_PORT. Returns 0 if the
// display is set properly and 1 if an error occurs while setting the
// display.
//
// CONSTRAINT: Does not allocate any heap memory as malloc() is NOT
// available on the target microcontroller.  Uses stack and global
// memory only.
//
// CONSTRAINT: Limit the complexity of code as much as possible. Do
// not use conditionals nested deeper than 2 levels. Seek to make the
// code as short, and simple as possible. Code longer than 20 lines
// may be penalized for complexity.
{
    scale_t scale;
    int display = 0;

    // Update the scale structure using current port values
    scale_from_ports(&scale);

    // If mode is MODE_TARE, copy SCALE_SENSOR_PORT to SCALE_TARE_PORT
    if (scale.mode == MODE_TARE) {
        SCALE_TARE_PORT = SCALE_SENSOR_PORT;
    }

    // Update the display based on mode
    int ret = 0;
    if (scale.mode == MODE_ERROR) {
        ret = scale_display_special(scale, &display);
    } else if (scale.mode == MODE_TARE) {
        ret = scale_display_special(scale, &display); 
    } else if (scale.mode == MODE_SHOW) {
        ret = scale_display_weight(scale, &display);
    }

    // Ensure the display value is correctly updated
    SCALE_DISPLAY_PORT = display;

    return ret;
}
