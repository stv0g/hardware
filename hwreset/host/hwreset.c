#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#include <libusb-1.0/libusb.h>

#include "../config.h"

void usage() {
	printf("usage: hwreset COMMAND [OPTIONS]\n");
	printf(" available COMMANDs and their specific OPTIONS:\n\n");
	printf("   push [power/reset] DURATION  press button for DURATION miliseconds\n");
	printf("   set [power/reset] [on/off]   set button on or off\n");
	printf("   status                       return status of power led\n\n");
	printf("hwreset 0.1 - by Steffen Vogel <stv0g@0l.de>\n");
	printf("please send bugreports to http://bugs.0l.de\n");
}

int main(int argc, char *argv[]) {
	uint8_t ret = 0;

	/* initalize libusb */
	int r = libusb_init(NULL);
	if (r < 0) {
		return EXIT_FAILURE;
	}

	/* search & open device */
	struct libusb_device_handle *handle = libusb_open_device_with_vid_pid(NULL, DEVICE_VID, DEVICE_PID);
	if (handle == NULL) {
		fprintf(stderr, "failed to open the device\n");
		return EXIT_FAILURE;
	}

	struct libusb_device_descriptor desc;
	r = libusb_get_device_descriptor(libusb_get_device(handle), &desc);
	if (r < 0) {
		fprintf(stderr, "failed to get device descriptor\n");
		return EXIT_FAILURE;
	}

	if (argc == 1) {
		usage();
		exit(EXIT_FAILURE);
	}
	else if (argc == 2 && !strcmp(argv[1], "status")) {
		libusb_control_transfer(
			handle,
			LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, /* bmRequestType */
			USBRQ_VENDOR_GET,	/* bRequest */
			0,			/* wValue */
			0,			/* wIndex */
			&ret,			/* data */
			1,			/* wLength */
			500			/* timeout */
		);
	}
	else if (argc == 4 && !strcmp(argv[1], "push")) {
		uint16_t port;
		uint16_t duration = atoi(argv[3]);

		if (!strcmp(argv[2], "reset"))
			port = 1;
		else if (!strcmp(argv[2], "power"))
			port = 2;
		else if (!strcmp(argv[2], "both"))
			port = 3;

		printf("hitting the %s button for %u ms...\n", argv[2], duration);

		libusb_control_transfer(
			handle,
			LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, /* bmRequestType */
			USBRQ_VENDOR_PULSE,	/* bRequest */
			port,			/* wValue */
			duration,		/* wIndex */
			&ret,			/* data */
			1,			/* wLength */
			500			/* timeout */
		);
	}
	else if (argc == 4 && !strcmp(argv[1], "set")) {
		uint16_t port;
		uint16_t state;

		if (!strcmp(argv[2], "reset"))
			port = 1;
		else if (!strcmp(argv[2], "power"))
			port = 2;
		else if (!strcmp(argv[2], "both"))
			port = 3;
		else {
			usage();
			exit(EXIT_FAILURE);
		}

		if (!strcmp(argv[3], "on"))
			state = port;
		else if (!strcmp(argv[3], "off"))
			state = 0;
		else {
			usage();
			exit(EXIT_FAILURE);
		}

		printf("set %s button %s...\n", argv[2], argv[3]);

		libusb_control_transfer(
			handle,
			LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR | LIBUSB_RECIPIENT_DEVICE, /* bmRequestType */
			USBRQ_VENDOR_SET,	/* bRequest */
			state,			/* wValue */
			0,			/* wIndex */
			&ret,			/* data */
			1,			/* wLength */
			500			/* timeout */
		);
	}
	else {
		usage();
		exit(EXIT_FAILURE);
	}

	printf("r = %u\n", ret);

	return ret;
}
