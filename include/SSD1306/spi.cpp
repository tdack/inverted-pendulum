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

#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "spi.hpp"

#define MAX_N_SPIS 2
#define MAX_N_DEVS 16

static BeagleBone::spi *m_singletons[MAX_N_SPIS][MAX_N_DEVS];


BeagleBone::spi::spi(unsigned char spiNum, unsigned char csNum)
  : m_spiNum(spiNum), m_csNum(csNum)
{
  // Make the /dev/spidevX.Y filename from the spi/cs numbers
  sprintf(m_dev, "/dev/spidev%d.%d", m_spiNum+1, m_csNum);
  m_fd = open(m_dev, O_RDWR);
  if (m_fd < 0) {
    fprintf(stderr, "ERROR: Cannot open \"%s\" for SPI interface #%d, device #%d: ",
	    m_dev, m_spiNum, m_csNum);
    perror(0);
    return;
  }

  m_singletons[m_spiNum][m_csNum] = this;
}


BeagleBone::spi*
BeagleBone::spi::get(unsigned char spiNum, unsigned char csNum)
{
  if (spiNum >= MAX_N_SPIS) {
    fprintf(stderr, "ERROR: SPI interface #%d exceeds max of %0d.\n", spiNum, MAX_N_SPIS);
    return NULL;
  }
  if (csNum >= MAX_N_DEVS) {
    fprintf(stderr, "ERROR: Device number #%d exceeds max of %0d.\n", csNum, MAX_N_DEVS);
    return NULL;
  }

  if (m_singletons[spiNum][csNum] != NULL) {
    return m_singletons[spiNum][csNum];
  }

  (void) new spi(spiNum, csNum);

  return m_singletons[spiNum][csNum];
}


int
BeagleBone::spi::send(unsigned char n, const char* wrbuf)
{
  if (write(m_fd, wrbuf, n) != n) {
    fprintf(stderr, "ERROR: Cannot write to SPI interface #%0d, device #%0d (%s): ",
	    m_spiNum, m_csNum, m_dev);
    perror(0);
    return 0;
  }

  return 1;
}


int
BeagleBone::spi::receive(unsigned char n, char* rdbuf)
{
  if (read(m_fd, rdbuf, n) != n) {
    fprintf(stderr, "ERROR: Cannot read from SPI interface #%0d, device #%0d (%s): ",
	    m_spiNum, m_csNum, m_dev);
    perror(0);
    return 0;
  }

  return 1;
}


#ifdef TEST
int
main(int argc, char *argv[])
{
  printf("Done!\n");
}

#endif
