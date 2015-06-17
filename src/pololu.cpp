/**
 * @file
 * Inverted Pendulum
 *
 * Main function to initialise and control inverted pendulum connected
 * hardware.
 *
 * @author Troy Dack
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

#include <fcntl.h>
#include <linux/input.h>
#include <pololu.h>
#include <unistd.h>
#include <cstdio>
#include <iostream>  // Input-Output streams

using namespace std;

#define KEY_PRESS 1
#define KEY_RELEASE 0

int main(int argc, char const *argv[]) {

	int fd, count = 0;
	struct input_event event[64];
	if (getuid() != 0) {
		cout << "You must run this program as root. Exiting." << endl;
		return -1;
	}
	cout << "Starting BB-BONE-GPIO Test (press 10 times to end):" << endl;
	if ((fd = open("/dev/input/event1", O_RDONLY)) < 0) {
		perror("Failed to open event1 input device. Exiting.");
		return -1;
	}
	while (count < 20) {  // Press and Release are one loop each
		int numbytes = (int) read(fd, event, sizeof(event));
		if (numbytes < (int) sizeof(struct input_event)) {
			perror("The input read was invalid. Exiting.");
			return -1;
		}
		for (int i = 0; i < numbytes / sizeof(struct input_event); i++) {
			int type = event[i].type;
			int val = event[i].value;
			int code = event[i].code;
			if (type == EV_KEY) {
				if (val == KEY_PRESS) {
					cout << "Press  : Code " << code << " Value " << val
							<< endl;
				}
				if (val == KEY_RELEASE) {
					cout << "Release: Code " << code << " Value " << val
							<< endl;
				}
			}
		}
		count++;
	}
	close(fd);

	cout << "Done!" << endl;
	return 0;
}
