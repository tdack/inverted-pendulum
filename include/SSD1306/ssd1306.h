/**
 * @brief Driver for SSD1306 OLED display.
 *
 * @changelog
 * + J. Bergeron <janick@bergeron.com>
 *	- Refactored driver into a Strategy Pattern
 *	- Port to BoneLib
 * + Jul 2015 - T. Dack <troy@dack.com.au>
 *	- removed test code
 *	- changed namespace to SSD1306
 *	- integrated with BlackLib
 *
 * @author Limor Fried/Ladyada, Janick Bergeron, Troy Dack <troy@dack.com.au>
 * @license
 * \verbinclude "SSD1306 - Adafruit BSD 3-Clause.txt"
 *
 * This is a library for our Monochrome OLEDs based on SSD1306 drivers
 *
 * Pick one up today in the adafruit shop!
 * ------> http://www.adafruit.com/category/63_98
 *
 * These displays use SPI to communicate, 4 or 5 pins are required to
 * interface
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Limor Fried/Ladyada  for Adafruit Industries.
 *
 * BSD license, check license.txt for more information
 * All text above, and the splash screen must be included in any redistribution
 *********************************************************************
**/

#ifndef INCLUDE_SSD1306_SSD1306_H_
#define INCLUDE_SSD1306_SSD1306_H_

#include <SSD1306/rgb_driver.h>
#include <BlackLib/BlackGPIO/BlackGPIO.h>
#include <BlackLib/BlackI2C/BlackI2C.h>
#include <BlackLib/BlackSPI/BlackSPI.h>
#include <linux/stddef.h>
#include <stdint.h>

namespace SSD1306 {

class SSD1306: public rgb_driver {

public:
	typedef enum {WIDTH = 128, HEIGHT = 64} size_t;

	SSD1306(BlackLib::BlackSPI* _spi, BlackLib::BlackGPIO* cs, BlackLib::BlackGPIO* rst, uint8_t height = 32);
	SSD1306(BlackLib::spiName spi, BlackLib::BlackGPIO* cs, BlackLib::BlackGPIO* rst, uint8_t height = 32);

	SSD1306(BlackLib::BlackI2C *i2c, BlackLib::BlackGPIO *rst = NULL, uint8_t height = 32);
	SSD1306(BlackLib::i2cName i2c, uint8_t slaveAddress, BlackLib::BlackGPIO* rst = NULL, uint8_t height = 32);

	virtual ~SSD1306();

	/**
	 * @brief Initialise the display
	 * Initialises the display and  sets defaults
	 */
	void begin();

	/**
	 * Sends a single byte in command mode to the display
	 * @param c data to send
	 */
	void command(uint8_t c);

	void data(uint8_t* buffer, size_t bufferSize);

	/**
	 * @brief Reset the display
	**/
	virtual void reset(void);

	/**
	 * @brief Clear the display
	**/
	virtual void clear(void);

	/**
	 * @brief Refresh the display
	**/
	virtual void refresh(void);

	/**
	 * @brief Return the width of the display
	 * @return width of the display in pixels
	 */
	virtual uint16_t get_width(void);

	/**
	 * @brief Return the height of the display
	 * @return height of the display in pixels
	 */
	virtual uint16_t get_height(void);

	/** Set a color pixel */
	/**
	 * @brief Get the color of a pixel
	 * @param x x-coordinate of pixel
	 * @param y y-coordinate of pixel
	 * @param color color to set pixel
	 */
	virtual void drawPixel(int16_t x, int16_t y, rgb_t color);

	/**
	 * @brief Get the color of a pixel
	 * @param x x-coordinate of pixel
	 * @param y y-coordinate of pixel
	 */
	virtual rgb_t getPixel(int16_t x, int16_t y);

	/**
	 * @brief Sets the contrast of the display
	 * @param contrast value between 0 and 255
	 */
	void setContrast(uint8_t contrast);

private:
	BlackLib::BlackSPI* m_spi;
	BlackLib::BlackI2C* m_i2c;
	BlackLib::BlackGPIO* m_din;
	BlackLib::BlackGPIO* m_sclk;
	BlackLib::BlackGPIO* m_dc;
	BlackLib::BlackGPIO* m_cs;
	BlackLib::BlackGPIO* m_rst;
	uint8_t m_height;

}; /* end class SSD1306 */

} /* SSD1306 */

#endif /* INCLUDE_SSD1306_SSD1306_H_ */
