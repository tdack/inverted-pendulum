//
// Copyright (c) 2013 Janick Bergeron
// All Rights Reserved
//
//   Licensed under the Apache License, Version 2.0 (the
//   "License"); you may not use this file except in
//   compliance with the License.  You may obtain a copy of
//   the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in
//   writing, software distributed under the License is
//   distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
//   CONDITIONS OF ANY KIND, either express or implied.  See
//   the License for the specific language governing
//   permissions and limitations under the License.
//

#ifndef __BONELIB_SPI__
#define __BONELIB_SPI__

namespace BeagleBone {

typedef enum {SPI0=0, SPI1=1} spi_enum_t;
typedef enum {CS0 =0, CS1 =1} spi_dev_t;

class spi
{
public:

private:
  unsigned char m_spiNum;    // ID of SPI interface
  unsigned char m_csNum;     // ID of SPI device on SPI interface
  char          m_dev[32];   // "/dev/spidevX.Y" filename
  int           m_fd;        // File handle to /dev/spidevX.Y

  /** Create an API class for SPI device */
  spi(unsigned char spiNum, unsigned char csNum);
  
public:
  /** Get the API class singleton for the device on the SPI interface
   *  Currently, only device 0 on SPI interface 1 is supported.
   */
  static spi* get(unsigned char spiNum, unsigned char csNum);

  /** Write character stream to device as one stream
   * (i.e. do not de-assert CS between characters)
   * Returns FALSE on failure.
   */
  int send(unsigned char n, const char* wrbuf);

  /** Read the specified number of characters from the device as one stream
   *  into the specified character buffer.
   *  Returns FALSE on failure.
   */
  int receive(unsigned char n, char* rdbuf);

};

}

#endif
