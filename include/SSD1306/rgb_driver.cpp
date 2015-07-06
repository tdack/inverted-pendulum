/**
 * @brief RGB Driver Class
 * @author Janick Bergeron
 * @license
 * \verbinclude "Janick Bergeron Apache-2.0.txt"
 */
//
// Copyright (c) 2012 Janick Bergeron
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


#include <SSD1306/rgb_driver.h>

namespace SSD1306 {

bool
operator==(rgb_t l, rgb_t r)
{
  return l.red == r.red && l.green == r.green && l.blue == r.blue;
}


bool
operator!=(rgb_t l, rgb_t r)
{
  return !(l == r);
}

}
