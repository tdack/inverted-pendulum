/**
 * @brief Adafruit graphics core library
 *
 * @changelog
 * + J. Bergeron <janick@bergeron.com>
 *	- Refactored driver into a Strategy Pattern
 *	- Port to BoneLib
 * + T. Dack <troy@dack.com.au> (Jul 2015)
 *	- removed test code
 *	- changed namespace to SSD1306
 *	- integrated with BlackLib
 *
 * @author Limor Fried/Ladyada, Janick Bergeron, Troy Dack <troy@dack.com.au>
 * @license
 * \verbinclude "Adafruit BSD 3-Clause.txt"
 *
 * This is a our graphics core library, for all our displays.
 * We'll be adapting all the existing libaries to use this core to make
 * updating, support and upgrading easier!
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada  for Adafruit Industries.
 *
 * BSD license, check license.txt for more information
 * All text above, and the splash screen must be included in any redistribution
**/

#ifndef INCLUDE_SSD1306_GFX_H
#define INCLUDE_SSD1306_GFX_H

#include <SSD1306/rgb_driver.h>

namespace SSD1306 {

class gfx {
public:
  gfx(SSD1306::rgb_driver &driver);

  /** Return the width of the display */
  uint16_t getWidth(void);

  /** Return the height of the display */
  uint16_t getHeight(void);

  /** Explicitly update the screen */
  virtual void refreshScreen(void);

  /** Automatically (or not) update the screen after every action
   *  Return the previous setting.
   *  By default, auto-refresh is ON.
   */
  virtual bool setAutoRefresh(bool is_on);

  /** Fill the entire screen with one color */
  virtual void clearScreen(void);

  /** Clear the screen */
  virtual void fillScreen(rgb_t color);

  /** Draw a line from (x0,y0) to (x1,y1) */
  virtual void drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
			rgb_t color);

  /** Draw a line from (x,y) to (x,y_h) */
  virtual void drawFastVLine(uint16_t x, uint16_t y, uint16_t h, rgb_t color);

  /** Draw a line from (x,y) to (x+w,y) */
  virtual void drawFastHLine(uint16_t x, uint16_t y, uint16_t w, rgb_t color);

  /** Draw a hollow rectangle from (x,y) ro (x+w,y+h) */
  virtual void drawRect(uint16_t x, uint16_t y,
			uint16_t w, uint16_t h, 
			rgb_t color);

  /** Draw a filled rectangle from (x,y) ro (x+w,y+h) */
  virtual void fillRect(uint16_t x, uint16_t y,
			uint16_t w, uint16_t h, 
		rgb_t color);

  /** Draw a hollow rounded rectangle from (x,y) ro (x+w,y+h) with corner radius r*/
  void drawRoundRect(uint16_t x, uint16_t y,
		     uint16_t w, uint16_t h,
		     uint16_t r, rgb_t color);

  /** Draw a hollow rounded rectangle from (x,y) ro (x+w,y+h) with corner radius r*/
  void fillRoundRect(uint16_t x, uint16_t y,
		     uint16_t w, uint16_t h,
		     uint16_t r, rgb_t color);

  /** Draw a hollow circle centered at (x0,y0) and radius r */
  void drawCircle(uint16_t x, uint16_t y, uint16_t r, rgb_t color);

  /** Draw a filled circle centered at (x0,y0) and radius r */
  void fillCircle(uint16_t x, uint16_t y, uint16_t r, rgb_t color);

  /** Draw a hollow triangle between (x0,y0), (x1,y1) and (x2,y2) */
  void drawTriangle(uint16_t x0, uint16_t y0,
		    uint16_t x1, uint16_t y1,
		    uint16_t x2, uint16_t y2,
		    rgb_t color);

  /** Draw a filled triangle between (x0,y0), (x1,y1) and (x2,y2) */
  void fillTriangle(uint16_t x0, uint16_t y0,
		    uint16_t x1, uint16_t y1,
		    uint16_t x2, uint16_t y2,
		    rgb_t color);

  /** Draw a bitmap in the area between (x,y) and (x+w,y+h) */
  void drawBitmap(uint16_t x, uint16_t y, 
		  const uint8_t *bitmap,
		  uint16_t w, uint16_t h,
		  rgb_t color);

  /** Position the text curser at (x,y) in pixels */
  void setCursor(uint16_t x, uint16_t y);

  /** Move the text curser by (x,y) in pixels */
  void moveCursor(uint16_t x, uint16_t y);

  /** Return the X position the text curser at in pixels */
  uint16_t getCursorX(void);

  /** Return the Y position the text curser at in pixels */
  uint16_t getCursorY(void);

  /** Set the text color with transparent background */
  void setTextColor(rgb_t c);

  /** Set the text and background color
   *  If the background color is the same as the foreground color, the background is transparent.
   */
  void setTextColor(rgb_t c, rgb_t bg);

  /** Set the text size */
  void setTextSize(uint8_t s);

  /** Set if text wraps or not */
  void setTextWrap(bool w);

  /** Write the character at the current cursor position */
  void write(char c);

  /** Write the characters at the current cursor position */
  void write(const char* c);

 protected:
  rgb_driver &m_drv;
  bool        m_auto_refresh;
  uint16_t    m_cursor_x, m_cursor_y;
  rgb_t       m_textcolor, m_textbgcolor;
  uint8_t     m_textsize;
  bool        m_wrap; // If set, 'wrap' text at right edge of display

  inline void m_refresh(void);

  typedef enum {TopLeft = 1, TopRight = 2, BotLeft = 4, BotRight= 8} corner_t;

  void drawCircleHelper(uint16_t x, uint16_t y,
			uint16_t r, uint8_t corners,
			rgb_t color);
  void fillCircleHelper(uint16_t x, uint16_t y,
			uint16_t r, uint8_t corners,
			rgb_t color);

  void drawChar(unsigned char c,
		uint16_t x, uint16_t y,
		rgb_t color, rgb_t bg,
		uint8_t size);
};

}

#endif
