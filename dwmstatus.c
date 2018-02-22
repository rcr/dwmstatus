#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define STRING_MAX 128
#define SLEEP_TIME  60

static const char *battery_str(void);
static const char *datetime_str(void);
static const char *volume_str(void);
static void xsetroot(const char*);

static const char*
battery_str(void)
{
	static char str[STRING_MAX];

	return str;
}

static const char*
datetime_str(void)
{
	static char str[STRING_MAX];

	time_t now = time(0);

	strftime(str, STRING_MAX, "%a %e  %R", localtime(&now));

	return str;
}

static const char*
volume_str(void)
{
	static char str[STRING_MAX];

	return str;
}

static void
xsetroot(const char *str)
{
	Display *display;

	if (!(display = XOpenDisplay(NULL)))
		exit(EXIT_FAILURE);

	XStoreName(display, DefaultRootWindow(display), str);
	XSync(display, 0);

	XCloseDisplay(display);
}

int
main(void)
{
	char status[STRING_MAX];

	for (;;) {

		snprintf(status, sizeof(status),
			"%s ~ %s ~ %s", volume_str(), battery_str(), datetime_str());

		xsetroot(status);

		sleep(SLEEP_TIME);
	}

	return 0;
}
