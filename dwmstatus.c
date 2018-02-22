#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define STRING_MAX 256
#define SLEEP_TIME  60

static const char *draw_bat(void);
static const char *draw_time(void);
static const char *draw_vol(void);

static const char*
draw_bat(void)
{
	static char str[STRING_MAX];

	char charging;

	FILE *fd_0,
	     *fd_1,
	     *fd_2;

	int charge_0 = 0,
	    charge_1 = 0;

	if (NULL == (fd_0 = fopen("/sys/class/power_supply/BAT0/charge_now", "r")))
		return "bat err 0";

	if (NULL == (fd_1 = fopen("/sys/class/power_supply/BAT0/charge_full_design", "r")))
		return "bat err 1";

	if (NULL == (fd_2 = fopen("/sys/class/power_supply/BAT0/status", "r")))
		return "bat err 2";

	switch (fgetc(fd_2)) {

	case 'C': charging = '+'; break;
	case 'F': charging = '='; break;
	case 'D': charging = '-'; break;

	default:
		return "bat err 3";
	}

	if (EOF == fscanf(fd_0, "%d\n", &charge_0))
		return "bat err 4";

	if (EOF == fscanf(fd_1, "%d\n", &charge_1))
		return "bat err 5";

	if (charge_0 <= 0 || charge_1 <= 0)
		return "bat err 6";

	snprintf(str, sizeof(str), "%c %d%%", charging, (int)(charge_0 * 100.0 / charge_1));

	return str;
}

static const char*
draw_time(void)
{
	static char str[STRING_MAX];

	time_t now = time(0);

	strftime(str, STRING_MAX, "%a %e  %R", localtime(&now));

	return str;
}

static const char*
draw_vol(void)
{
	static char str[STRING_MAX];

	return str;
}

int
main(void)
{
	Display *display;

	if (!(display = XOpenDisplay(NULL)))
		exit(EXIT_FAILURE);

	for (;;) {
		char status[STRING_MAX];

		snprintf(status, sizeof(status), " %s  ~  %s  ~  %s ",
			draw_vol(), draw_bat(), draw_time());

		XStoreName(display, DefaultRootWindow(display), status);
		XSync(display, 0);

		sleep(SLEEP_TIME);
	}

	XCloseDisplay(display);

	return 0;
}
