/**
 * @file mantissify.h
 * @brief Headerfile of the mantissify C API.
 *
 * @mainpage Mantissify C API
 * 
 * The mantissify API provides functions which can format measurement values
 * into human readable text, e.g. 0.000000123 to '123.0 nano' or 651234.8 to '651.2358 kilo'.
 * Also values in scientific format are supported, e.g. 4.223e-05 to '-42.23 micro' 
 * The API tries to find a suitable SI-prefix (multiple of three) for your number.
 * 
 * The mantissify C API functions work stateless and are thread-safe.
 *
 * Visit the mantissify page on GitHub for more informations: <br>
 * https://messner75.github.io/mantissify
 *
 * @version 1.0.0
 * @date 2025-12-30
 * @author Martin Messner
 * 
 * @copyright (c) 2026 Martin Messner / messner75 <br>
 *  This project is licensed unter CC BY-ND 4.0 <br>
 *  View the complete LICENSE file at https://github.com/messner75/mantissify
 * 
 * Additional disclaimer: <br>
 * This software is provided "as is", without warranty of any kind, express or implied,
 * including but not limited to the warranties of merchantability, fitness for a
 * particular purpose and noninfringement. In no event shall the author(s) be liable
 * for any claim, damages or other liability, whether in an action of contract, tort
 * or otherwise, arising from, out of or in connection with the software or the use
 * or other dealings in the software.
 */

#ifndef MESSNER75_MANTISSIFY_H
#define MESSNER75_MANTISSIFY_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup mantissify_definitions Definitions
 * @{
 */

/**
* @brief Magnitude formatting options
*/
typedef enum
{
  MANTISSIFY_MAG_FIX_ZERO_SIGN = 0, ///< Fixed magnitude with leading zeros and sign always visible
  MANTISSIFY_MAG_FIX_ZERO = 1, ///< Fixed magnitude with leading zeros
  MANTISSIFY_MAG_FIX_SPACE_SIGN = 2, ///< Fixed magnitude with leading zeros and sign always visible
  MANTISSIFY_MAG_FIX_SPACE = 3, ///< Fixed magnitude with leading spaces
  MANTISSIFY_MAG_VAR_SIGN = 4, ///< Variable magnitude with sign always visible
  MANTISSIFY_MAG_VAR = 5 ///< Variable magnitude
} MANTISSIFY_MAG_t;

/**
* @brief SI-Prefix formatting options
*/
typedef enum
{
  MANTISSIFY_SIP_SCIENTIFIC = 0, ///< Scientific exponent, e.g. 'e-03'
  MANTISSIFY_SIP_LETTER = 1, ///< Letter, e.g. 'm'
  MANTISSIFY_SIP_NAME = 2, ///< Name, e.g. 'milli'
  MANTISSIFY_SIP_UNDERLINE_LETTER = 3, ///< Unterline with letter, e.g. '_m'
  MANTISSIFY_SIP_UNDERLINE_NAME = 4, ///< Underline with name, e.g. '_milli'
  MANTISSIFY_SIP_SPACE_LETTER = 5, ///< Space with letter, e.g. ' m'
  MANTISSIFY_SIP_SPACE_NAME = 6 ///< Space with name, e.g. ' milli'
} MANTISSIFY_SIP_t;

#define MANTISSIFY_ERRCODE_GENERAL -1  ///< General (unspecific) error
#define MANTISSIFY_ERRCODE_PARAMETER -2 ///< A parameter passed to a function was invalid
#define MANTISSIFY_ERRCODE_BUFFERSIZE -3 ///< A given buffer size was not sufficient
#define MANTISSIFY_ERRCODE_VALUERANGE -4 ///< Given measurement value range is not supported

/** @} */ // end of mantissify_definitions

/**
 * @defgroup mantissify_functions Functions
 * @{
 */

/**
* @brief Formats a given measurement value into human friendly format
* 
* Note that the function uses snprintf from the C standard library,
* therefore the printed decimal point char my depend on your locale setting.
* 
* @param f The measurement value to format
* @param buffer Pointer to a buffer for the formatted result text. Pass NULL to make a 'virtual' write (like snprintf).
* @param bufferLength Length (number of bytes) of the given buffer
* @param fracs Number of desired fractional digits [0 .. 9]
* @param mag The magnitude formatting option, one of MANTISSIFY_MAG_t
* @param sip The SI-Prefix formatting option, one of MANTISSIFY_SIP_t
* 
* @return The number of written bytes (excluding the c-string zero termination), also when writing virtual.
*/
int MANTISSIFY_format_value(double f, char* buffer, size_t bufferLength, int fracs, MANTISSIFY_MAG_t mag, MANTISSIFY_SIP_t sip);

/**
* @brief Formats all measurement values within a given text into human friendly format
*
* Note that the function uses strtod from the C standard library,
* therefore the parser decimal point char depend on your locale setting.
* 
* @param input The text to convert as pointer to a const c-string
* @param buffer Pointer to a buffer for the formatted result text. Pass NULL to make a 'virtual' write (like snprintf).
* @param bufferLength Length (number of bytes) of the given buffer
* @param fracs Number of desired fractional digits [0 .. 9]
* @param mag The magnitude formatting option, one of MANTISSIFY_MAG_t
* @param sip The SI-Prefix formatting option, one of MANTISSIFY_SIP_t
*
* @return The number of written bytes (excluding the c-string zero termination), also when writing virtual.
*/
int MANTISSIFY_convert_text(const char* input, char* buffer, size_t bufferLength, int fracs, MANTISSIFY_MAG_t mag, MANTISSIFY_SIP_t sip);

/** @} */ // end of mantissify_functions

#ifdef __cplusplus
}
#endif

#endif // MESSNER75_MANTISSIFY_H
