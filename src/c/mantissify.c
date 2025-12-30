/**
 * @file mantissify.c
 * @brief Implementation of the mantissify C API.
 *
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

#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h> // strtod
#include <stdio.h> // snprintf
#include <string.h> // memset, memcpy
#include <ctype.h> // isdigit
#include <math.h> // fabs, copysign
#include <locale.h> // localeconv

#include "mantissify.h"

// MANTISSIFY_MAG_FIX_ZERO_SIGN = 0,
// MANTISSIFY_MAG_FIX_ZERO = 1,
// MANTISSIFY_MAG_FIX_SPACE_SIGN = 2,
// MANTISSIFY_MAG_FIX_SPACE = 3,
// MANTISSIFY_MAG_VAR_SIGN = 4,
// MANTISSIFY_MAG_VAR = 5
static const char* _magFormatArrays[6][10] = // [mag][fracs] 
{
  {"%+05.0f%s","%+06.1f%s","%+07.2f%s","%+08.3f%s","%+09.4f%s","%+010.5f%s","%+011.6f%s","%+012.7f%s","%+013.8f%s","%+014.9f%s"},
  {"%05.0f%s","%06.1f%s","%07.2f%s","%08.3f%s","%09.4f%s","%010.5f%s","%011.6f%s","%012.7f%s","%013.8f%s","%014.9f%s"}, 
  {"%+5.0f%s","%+6.1f%s","%+7.2f%s","%+8.3f%s","%+9.4f%s","%+10.5f%s","%+11.6f%s","%+12.7f%s","%+13.8f%s","%+14.9f%s"}, 
  {"%5.0f%s","%6.1f%s","%7.2f%s","%8.3f%s","%9.4f%s","%10.5f%s","%11.6f%s","%12.7f%s","%13.8f%s","%14.9f%s"}, 
  {"%+.0f%s","%+.1f%s","%+.2f%s","%+.3f%s","%+.4f%s","%+.5f%s","%+.6f%s","%+.7f%s","%+.8f%s","%+.9f%s"}, 
  {"%.0f%s","%.1f%s","%.2f%s","%.3f%s","%.4f%s","%.5f%s","%.6f%s","%.7f%s","%.8f%s","%.9f%s"}, 
};

// MANTISSIFY_SIP_SCIENTIFIC = 0,
// MANTISSIFY_SIP_LETTER = 1,
// MANTISSIFY_SIP_NAME = 2,
// MANTISSIFY_SIP_UNDERLINE_LETTER = 3,
// MANTISSIFY_SIP_UNDERLINE_NAME = 4,
// MANTISSIFY_SIP_SPACE_LETTER = 5,
// MANTISSIFY_SIP_SPACE_NAME = 6
static const char* _sipNegArrays[7][11] = // [exp][-e/3]
{
  { "    ", "e-03", "e-06", "e-09", "e-12", "e-15", "e-18", "e-21" , "e-24" , "e-27" , "e-30" },
  { " ", "m", "u", "n", "p", "f", "a", "z" , "y" , "r" , "q" },
  { "", "milli", "micro", "nano", "pico", "femto", "atto", "zepto" , "yokto" , "ronto" , "quekto" },
  { "  ", "_m", "_u", "_n", "_p", "_f", "_a", "_z" , "_y" , "_r" , "_q" },
  { "", "_milli", "_micro", "_nano", "_pico", "_femto", "_atto", "_zepto" , "_yokto" , "_ronto" , "_quekto" },
  { "  ", " m", " u", " n", " p", " f", " a", " z" , " y" , " r" , " q" },
  { "", " milli", " micro", " nano", " pico", " femto", " atto", " zepto" , " yokto" , " ronto" , " quekto" }
};
static const char* _sipPosArrays[7][11] = // [exp][e/3]
{
  { "    ", "e+03", "e+06", "e+09", "e+12", "e+15", "e+18", "e+21" , "e+24" , "e+27" , "e+30" },
  { " ", "k", "M", "G", "T", "P", "E", "Z" , "Y" , "R" , "Q" },
  { "", "kilo", "mega", "giga", "tera", "peta", "exa", "zetta" , "yotta" , "ronna" , "quetta" },
  { "  ", "_k", "_M", "_G", "_T", "_P", "_E", "_Z" , "_Y" , "_R" , "_Q" },
  { "", "_kilo", "_mega", "_giga", "_tera", "_peta", "_exa", "_zetta" , "_yotta" , "_ronna" , "_quetta" },
  { "  ", " k", " M", " G", " T", " P", " E", " Z" , " Y" , " R" , " Q" },
  { "", " kilo", " mega", " giga", " tera", " peta", " exa", " zetta" , " yotta" , " ronna" , " quetta" },
};

//  starts with value f that has no exponent (raw value) exp 00
static inline int _shift_exponent_from_zero(double* f)
{
  int e = 0; // value is assummed to have no exponent yet
  double x = fabs(*f);
  
  if (x < 1.0) while (x < 1.0) { x *= 1000.0; e -= 3; }
  else while (x >= 1000.0) { x /= 1000.0; e += 3; }
  
  *f = copysign(x, *f);
  return e;
}

// note outEnd > outStart if a number was found. outEnd points to char behind the number
static double _find_next_number(const char *szString, const char* *outStart, const char* *outEnd)
{
  // determine the decimmal point character of the users actual locale setting
  struct lconv* lc = localeconv();
  const char decPoint = (lc && lc->decimal_point) ? lc->decimal_point[0] : '.';

  // pre-init values as 'nothing found'
  const char* p = *outStart = *outEnd = szString;
  const char* todEndPtr;

  while (*p != '\0')
  {
    if (isdigit(*p) || *p == '-' || *p == '+' || (*p == decPoint && isdigit(p[1])))
    {
      // possible number start found, use strtod to verify 
      double f = strtod(p, &todEndPtr);
      if (todEndPtr > p)
      {
        *outStart = p; // p points to the first char of found number
        *outEnd = todEndPtr; // todEndPtr points to char behind the parsed number
        return f; // finished, early return
      }
    }
    p++;
  }
  return 0.0;
}

static inline void _append_and_terminate(char* buffer, int* bufferIndex, const char* text, int lenText)
{
  // note that buffer can be NULL for 'virtual write'
  if (buffer) memcpy(&buffer[*bufferIndex], text, lenText); // copy text chars (no termination)
  *bufferIndex += lenText; // increase current bufferIndex
  if (buffer) buffer[*bufferIndex] = 0; // place zero termination
}

int MANTISSIFY_format_value(double f, char* buffer, size_t bufferLength, int fracs, MANTISSIFY_MAG_t mag, MANTISSIFY_SIP_t sip)
{
  // parameter check with early returns in case of error ('buffer' can be NULL for 'virtual write')
  if (fracs < 0 || fracs > 9) return MANTISSIFY_ERRCODE_PARAMETER;
  if (mag < 0 || mag > 5) return MANTISSIFY_ERRCODE_PARAMETER;
  if (sip < 0 || sip > 6) return MANTISSIFY_ERRCODE_PARAMETER;

  // try to split value in new value with extracted si-prefix
  int e = _shift_exponent_from_zero(&f);
  if ((e < -30) || (e > 30) || (e % 3 != 0)) return MANTISSIFY_ERRCODE_VALUERANGE;

  // choose a si prefix string corresponding to exp option
  const char* siPrefix = (e > 0) ? _sipPosArrays[sip][e / 3] : _sipNegArrays[sip][-e / 3];

  // choose a format string depending on selected mag option and fracs
  const char* format = _magFormatArrays[mag][fracs];

  return snprintf(buffer, bufferLength, format, f, siPrefix); // could also used to test how many bytes are required
}

int MANTISSIFY_convert_text(const char* input, char* buffer, size_t bufferLength, int fracs, MANTISSIFY_MAG_t mag, MANTISSIFY_SIP_t sip)
{
  // parameter check with early returns in case of error ('buffer' can be NULL for 'virtual write')
  if (NULL == input) return MANTISSIFY_ERRCODE_PARAMETER;
  
  if (fracs < 0 || fracs > 9) return MANTISSIFY_ERRCODE_PARAMETER;
  if (mag < 0 || mag > 5) return MANTISSIFY_ERRCODE_PARAMETER;
  if (sip < 0 || sip > 6) return MANTISSIFY_ERRCODE_PARAMETER;

  char temp[32] = { 0 }; // temp char for formatting numbers: +123.123456789_u, +123.123456789e-99
  const char *inPosition = input, *outStart = 0, *outEnd = 0;
  int bufferIndex = 0;
  if (buffer) buffer[bufferIndex] = 0; // start with zero termination

  while (1)
  {
    const double f = _find_next_number(inPosition, &outStart, &outEnd);
    if (outEnd > outStart)
    {
      const int lenPreText = (int)(outStart - inPosition);
      if (buffer && (lenPreText >= (bufferLength - bufferIndex))) return MANTISSIFY_ERRCODE_BUFFERSIZE;
      _append_and_terminate(buffer, &bufferIndex, inPosition, lenPreText);
      
      const int lenNumber = MANTISSIFY_format_value(f, temp, sizeof(temp), fracs, mag, sip);
      if (lenNumber > 0)
      {
        if (buffer && (lenNumber >= (bufferLength - bufferIndex))) return MANTISSIFY_ERRCODE_BUFFERSIZE;
        _append_and_terminate(buffer, &bufferIndex, temp, lenNumber);
      }
      else
      {
        // number formatting was not possible, so we append the original string instead
        const int lenOriginal = (int)(outEnd - outStart);
        if (buffer && (lenOriginal >= (bufferLength - bufferIndex))) return MANTISSIFY_ERRCODE_BUFFERSIZE;
        _append_and_terminate(buffer, &bufferIndex, outStart, lenOriginal);
      }

      inPosition = outEnd; // continue with next number (char after current number)
    }
    else
    {
      // no more number found, just copy remaining chars to output and break
      const int lenFinalText = (int)strlen(inPosition); 
      if (buffer && (lenFinalText >= (bufferLength - bufferIndex))) return MANTISSIFY_ERRCODE_BUFFERSIZE;
      
      _append_and_terminate(buffer, &bufferIndex, inPosition, lenFinalText);
      break; // no more text to parse
    }
  }
  return bufferIndex; // return the length of the written c-string excluding the zero termination character
}
