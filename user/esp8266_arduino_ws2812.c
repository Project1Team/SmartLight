/*-------------------------------------------------------------------------
  Arduino library to control a wide variety of WS2811- and WS2812-based RGB
  LED devices such as Adafruit FLORA RGB Smart Pixels and NeoPixel strips.
  Currently handles 400 and 800 KHz bitstreams on 8, 12 and 16 MHz ATmega
  MCUs, with LEDs wired for various color orders.  Handles most output pins
  (possible exception with upper PORT registers on the Arduino Mega).

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries,
  contributions by PJRC, Michael Miller and other members of the open
  source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  -------------------------------------------------------------------------
  This file is part of the Adafruit NeoPixel library.

  NeoPixel is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation, either version 3 of
  the License, or (at your option) any later version.

  NeoPixel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with NeoPixel.  If not, see
  <http://www.gnu.org/licenses/>.
  -------------------------------------------------------------------------*/

#include "esp8266_arduino_ws2812.h"
#include "esp8266_arduino_gpio.h"

//TODO	structure these data to initalize when WS2812_Init is called
#ifdef NEO_KHZ400  // If 400 KHz NeoPixel support enabled...
bool is800KHz;      // ...true if 800 KHz pixels
#endif
bool begun;         // true if begin() previously called
uint16_t numLEDs;       // Number of RGB LEDs in strip
uint16_t numBytes;      // Size of 'pixels' buffer below (3 or 4 bytes/pixel)
int8_t pin;           // Output pin number (-1 if not yet set)
uint8_t brightness;
uint8_t *pixels;        // Holds LED color values (3 or 4 bytes each)
uint8_t rOffset;       // Index of red byte within each 3- or 4-byte pixel
uint8_t gOffset;       // Index of green byte
uint8_t bOffset;       // Index of blue byte
uint8_t wOffset;       // Index of white byte (same as rOffset if no white)
uint32_t endTime;       // Latch timing reference

// Constructor when length, pin and type are known at compile-time:
void WS2812_Init(uint16_t n, uint8_t p, neoPixelType t)
// : begun(false), brightness(0), pixels(NULL), endTime(0)
{
	begun = false;
	brightness = 0;
	pixels = NULL;
	endTime = 0;

	WS2812_UpdateType(t);
	WS2812_UpdateLength(n);
	WS2812_SetPin(p);
}


void WS2812_Begin(void)
{
  if(pin >= 0)
  {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);

  }
  begun = true;
}

void WS2812_UpdateLength(uint16_t n)
{
  if(pixels) free(pixels); // Free existing data (if any)

  // Allocate new data -- note: ALL PIXELS ARE CLEARED
  numBytes = n * ((wOffset == rOffset) ? 3 : 4);
  if((pixels = (uint8_t *)malloc(numBytes)))
  {
    memset(pixels, 0, numBytes);
    numLEDs = n;
  }
  else
  {
    numLEDs = numBytes = 0;
  }
}

void WS2812_UpdateType(neoPixelType t)
{
  bool oldThreeBytesPerPixel = (wOffset == rOffset); // false if RGBW

  wOffset = (t >> 6) & 0b11; // See notes in header file
  rOffset = (t >> 4) & 0b11; // regarding R/G/B/W offsets
  gOffset = (t >> 2) & 0b11;
  bOffset =  t       & 0b11;
#ifdef NEO_KHZ400
  is800KHz = (t < 256);      // 400 KHz flag is 1<<8
#endif

  // If bytes-per-pixel has changed (and pixel data was previously
  // allocated), re-allocate to new size.  Will clear any data.
  if(pixels) {
    bool newThreeBytesPerPixel = (wOffset == rOffset);
    if(newThreeBytesPerPixel != oldThreeBytesPerPixel) WS2812_UpdateLength(numLEDs);
  }
}

#if defined(ESP8266) 
// ESP8266 show() is external to enforce ICACHE_RAM_ATTR execution
//extern "C" void ICACHE_RAM_ATTR espShow(
//  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
//#elif defined(ESP32)
//extern "C" void espShow(
//  uint8_t pin, uint8_t *pixels, uint32_t numBytes, uint8_t type);
#define F_CPU			80000000UL

#define _BV(b) (1UL << (b))

static uint32_t _getCycleCount(void) __attribute__((always_inline));
static inline uint32_t _getCycleCount(void) {
  uint32_t ccount;
  __asm__ __volatile__("rsr %0,ccount":"=a" (ccount));
  return ccount;
}

void WS2812_EspShow( uint8_t pin, uint8_t *pixels, uint32_t numBytes, bool is800KHz)
{
#define CYCLES_800_T0H  (F_CPU / 2500000) // 0.4us
#define CYCLES_800_T1H  (F_CPU / 1250000) // 0.8us
#define CYCLES_800      (F_CPU /  800000) // 1.25us per bit
#define CYCLES_400_T0H  (F_CPU / 2000000) // 0.5uS
#define CYCLES_400_T1H  (F_CPU /  833333) // 1.2us
#define CYCLES_400      (F_CPU /  400000) // 2.5us per bit

  uint8_t *p, *end, pix, mask;
  uint32_t t, time0, time1, period, c, startTime, pinMask;

  pinMask   = _BV(pin);
  p         =  pixels;
  end       =  p + numBytes;
  pix       = *p++;
  mask      = 0x80;
  startTime = 0;

#ifdef NEO_KHZ400
  if(is800KHz) {
#endif
    time0  = CYCLES_800_T0H;
    time1  = CYCLES_800_T1H;
    period = CYCLES_800;
#ifdef NEO_KHZ400
  } else { // 400 KHz bitstream
    time0  = CYCLES_400_T0H;
    time1  = CYCLES_400_T1H;
    period = CYCLES_400;
  }
#endif

  for(t = time0;; t = time0) {
    if(pix & mask) t = time1;                             // Bit high duration
    while(((c = _getCycleCount()) - startTime) < period); // Wait for bit start
#ifdef ESP8266
    GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, pinMask);       // Set high
#else
    gpio_set_level(pin, HIGH);
#endif
    startTime = c;                                        // Save start time
    while(((c = _getCycleCount()) - startTime) < t);      // Wait high duration
#ifdef ESP8266
    GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, pinMask);       // Set low
#else
    gpio_set_level(pin, LOW);
#endif
    if(!(mask >>= 1)) {                                   // Next bit/byte
      if(p >= end) break;
      pix  = *p++;
      mask = 0x80;
    }
  }
  while((_getCycleCount() - startTime) < period); // Wait for last bit
}
#endif // ESP8266

bool WS2812_CanShow(void)
{
	return (system_get_time() - endTime) >= 300L;
}

#define __STRINGIFY(a) #a

#define xt_rsil(level) (__extension__({uint32_t state; __asm__ __volatile__("rsil %0," __STRINGIFY(level) : "=a" (state)); state;}))

#define interrupts() xt_rsil(0)
#define noInterrupts() xt_rsil(15)

void WS2812_Show(void) {

  if(!pixels) return;

  // Data latch = 300+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while(!WS2812_CanShow());
  // endTime is a private member (rather than global var) so that mutliple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.
  noInterrupts();


// ESP8266 ----------------------------------------------------------------

  // ESP8266 show() is external to enforce ICACHE_RAM_ATTR execution
  WS2812_EspShow(pin, pixels, numBytes, is800KHz);

// END ARCHITECTURE SELECT ------------------------------------------------
  interrupts();

  endTime = system_get_time(); // Save EOD time for latch on next call
}

// Set the output pin number
void WS2812_SetPin(uint8_t p) {
//  if(begun && (pin >= 0)) pinMode(pin, INPUT);
    pin = p;
    if(begun) {
      pinMode(p, OUTPUT);
      digitalWrite(p, LOW);
    }
}



void WS2812_SetPixelColor( uint16_t n, uint8_t r, uint8_t g, uint8_t b, uint8_t w)
{
  if(n < numLEDs) {
    if(brightness) { // See notes in setBrightness()
      r = (r * brightness) >> 8;
      g = (g * brightness) >> 8;
      b = (b * brightness) >> 8;
      w = (w * brightness) >> 8;
    }
    uint8_t *p;
    if(wOffset == rOffset) { // Is an RGB-type strip
      p = &pixels[n * 3];    // 3 bytes per pixel (ignore W)
    } else {                 // Is a WRGB-type strip
      p = &pixels[n * 4];    // 4 bytes per pixel
      p[wOffset] = w;        // Store W
    }
    p[rOffset] = r;          // Store R,G,B
    p[gOffset] = g;
    p[bOffset] = b;
  }
}



// Convert separate R,G,B into packed 32-bit RGB color.
// Packed format is always RGB, regardless of LED strand color order.
uint32_t WS2812_Color(uint8_t r, uint8_t g, uint8_t b) {
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

// Convert separate R,G,B,W into packed 32-bit WRGB color.
// Packed format is always WRGB, regardless of LED strand color order.
//uint32_t Color(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
//  return ((uint32_t)w << 24) | ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
//}

// Query color from previously-set pixel (returns packed 32-bit RGB value)
uint32_t WS2812_GetPixelColor(uint16_t n) {
  if(n >= numLEDs) return 0; // Out of bounds, return no color.

  uint8_t *p;

  if(wOffset == rOffset) { // Is RGB-type device
    p = &pixels[n * 3];
    if(brightness) {
      // Stored color was decimated by setBrightness().  Returned value
      // attempts to scale back to an approximation of the original 24-bit
      // value used when setting the pixel color, but there will always be
      // some error -- those bits are simply gone.  Issue is most
      // pronounced at low brightness levels.
      return (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else {
      // No brightness adjustment has been made -- return 'raw' color
      return ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  } else {                 // Is RGBW-type device
    p = &pixels[n * 4];
    if(brightness) { // Return scaled color
      return (((uint32_t)(p[wOffset] << 8) / brightness) << 24) |
             (((uint32_t)(p[rOffset] << 8) / brightness) << 16) |
             (((uint32_t)(p[gOffset] << 8) / brightness) <<  8) |
             ( (uint32_t)(p[bOffset] << 8) / brightness       );
    } else { // Return raw color
      return ((uint32_t)p[wOffset] << 24) |
             ((uint32_t)p[rOffset] << 16) |
             ((uint32_t)p[gOffset] <<  8) |
              (uint32_t)p[bOffset];
    }
  }
}

// Returns pointer to pixels[] array.  Pixel data is stored in device-
// native format and is not translated here.  Application will need to be
// aware of specific pixel data format and handle colors appropriately.
uint8_t *WS2812_GetPixels(void) {
  return pixels;
}

uint16_t WS2812_NumPixels(void) {
  return numLEDs;
}

// Adjust output brightness; 0=darkest (off), 255=brightest.  This does
// NOT immediately affect what's currently displayed on the LEDs.  The
// next call to show() will refresh the LEDs at this level.  However,
// this process is potentially "lossy," especially when increasing
// brightness.  The tight timing in the WS2811/WS2812 code means there
// aren't enough free cycles to perform this scaling on the fly as data
// is issued.  So we make a pass through the existing color data in RAM
// and scale it (subsequent graphics commands also work at this
// brightness level).  If there's a significant step up in brightness,
// the limited number of steps (quantization) in the old data will be
// quite visible in the re-scaled version.  For a non-destructive
// change, you'll need to re-render the full strip data.  C'est la vie.
void WS2812_SetBrightness(uint8_t b) {
  // Stored brightness value is different than what's passed.
  // This simplifies the actual scaling math later, allowing a fast
  // 8x8-bit multiply and taking the MSB.  'brightness' is a uint8_t,
  // adding 1 here may (intentionally) roll over...so 0 = max brightness
  // (color values are interpreted literally; no scaling), 1 = min
  // brightness (off), 255 = just below max brightness.
  uint8_t newBrightness = b + 1;
  uint16_t i = 0;
  if(newBrightness != brightness) { // Compare against prior value
    // Brightness has changed -- re-scale existing data in RAM
    uint8_t  c,
            *ptr           = pixels,
             oldBrightness = brightness - 1; // De-wrap old brightness value
    uint16_t scale;
    if(oldBrightness == 0) scale = 0; // Avoid /0
    else if(b == 255) scale = 65535 / oldBrightness;
    else scale = (((uint16_t)newBrightness << 8) - 1) / oldBrightness;
    for(i=0; i<numBytes; i++) {
      c      = *ptr;
      *ptr++ = (c * scale) >> 8;
    }
    brightness = newBrightness;
  }
}

//Return the brightness value
uint8_t WS2812_GetBrightness(void) {
  return brightness - 1;
}

void WS2812_Clear() {
  memset(pixels, 0, numBytes);
}
