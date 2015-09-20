/**
 * @file
 * Check that device tree overlays are loaded, if not load them!
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

#include <overlays.h>
#include <pendulum.h>

bool checkOverlays(){
	std::map<std::string, std::vector<std::string> > overlay_devices {
		{POLOLU_TTY, { "ADAFRUIT-UART2" } },		// /dev/ttyXX
		{"/sys/bus/platform/devices/48300180.eqep",	// eqep device path
					{ "PyBBIO-epwmss0",				// Enhanced PWM Sub System 0
					  "PyBBIO-eqep0" }				// EQEP 0
		},
		{"/sys/bus/platform/devices/48302180.eqep",	// eqep device path
					{ "PyBBIO-epwmss1",				// Enhanced PWM Sub System 1
					  "PyBBIO-eqep1" }				// EQEP 1
		}
	};
	struct stat buffer;
	bool overlays_loaded = true;
	std::string SLOTS = "/sys/devices/bone_capemgr.9/slots"; // Path to Cape Manager slots file
	std::ofstream fSlots;

	fSlots.open(SLOTS);
	if (!fSlots.is_open()) {
		std::cout << "Couldn't open " << SLOTS << ", can't load overlays." << std::endl;
		return false;
	}

	// Iterate over devices we need
	for (auto &dev : overlay_devices) {
		rlutil::setColor(rlutil::YELLOW);
		std::cout << dev.first << " ";
		// Check if file exists
		if (stat(dev.first.c_str(), &buffer) != 0) {
			rlutil::setColor(rlutil::YELLOW);
			std::cout << dev.first << " ";
			rlutil::setColor(rlutil::RED);
			std::cout << "not found .... ";
			// Load the overlays for this device
			for (auto &f : dev.second ) {
				fSlots << f.c_str() << std::flush;
			}
			rlutil::setColor(rlutil::GREEN);
			std::cout << "loaded!" << std::endl;
		} else {
			std::cout << std::endl;
		}
	}
	rlutil::setColor(rlutil::WHITE);
	fSlots.close();

	return overlays_loaded;
}
