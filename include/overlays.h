/**
 * @file
 * Header file for overlays.cpp
 *
 * @author Troy Dack <troy@dack.com.au>
 * @date Copyright (C) 2015
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 **/


#ifndef INCLUDE_OVERLAYS_H_
#define INCLUDE_OVERLAYS_H_

#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <rlutil.h>

/*!
 * @brief Ensure required device tree overlays are loaded
 *
 * @return True if loading overlays was successful
 * 		   False if loading overlays failed
 */
bool checkOverlays();

#endif /* INCLUDE_OVERLAYS_H_ */
