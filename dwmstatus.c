#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define STRING_MAX 256

static const char *draw_bat(void);
static const char *draw_time(void);
static const char *draw_vol(void);

static const char*
draw_bat(void)
{
	static char *err = NULL, str[STRING_MAX];

	int c0 = 0,
	    c1 = 0;

	FILE *fd_0 = fopen("/sys/class/power_supply/BAT0/status", "r"),
	     *fd_1 = fopen("/sys/class/power_supply/BAT0/charge_now", "r"),
	     *fd_2 = fopen("/sys/class/power_supply/BAT0/charge_full_design", "r");

	if (fd_0 && fd_1 && fd_2) {

		char s = 0;

		switch (fgetc(fd_0)) {
			case 'C': s = '+'; break;
			case 'D': s = '-'; break;
			case 'F': s = ' '; break;
			default:
				err = "err: s";
				break;
		}

		if (!err && EOF == fscanf(fd_1, "%d\n", &c0))
			err = "err: c0";

		if (!err && EOF == fscanf(fd_2, "%d\n", &c1))
			err = "err: c1";

		if (!err && (c0 <= 0 || c1 <= 0))
			err = "err: invalid";

		if (!err && 0 > snprintf(str, sizeof(str), "%c%d%%", s, (int)(c0 * 100.0 / c1)))
			err = "err: snprintf";
	}

	if (fd_0) fclose(fd_0); else err = "err: fd_0";
	if (fd_1) fclose(fd_1); else err = "err: fd_1";
	if (fd_2) fclose(fd_2); else err = "err: fd_2";

	return err ? err : str;
}

static const char*
draw_time(void)
{
	static char *err = NULL, str[STRING_MAX];

	time_t now = time(0);

	if (0 == strftime(str, sizeof(str), "%a %e  %R", localtime(&now)))
		err = "err: snprintf";

	return err ? err : str;
}

static const char*
draw_vol(void)
{
	static char str[STRING_MAX];

	return str;
}

int
main(int argc, char **argv)
{
	unsigned int seconds = 0;

	if (argc > 1)
		seconds = (unsigned int)(strtol(argv[1], NULL, 0));

	Display *display;

	if (!(display = XOpenDisplay(NULL)))
		exit(EXIT_FAILURE);

	do {
		char status[STRING_MAX];

		snprintf(status, sizeof(status), " %s  ~  %s  ~  %s ",
			draw_vol(), draw_bat(), draw_time());

		XStoreName(display, DefaultRootWindow(display), status);
		XSync(display, 0);

		sleep(seconds);

	} while (seconds);

	XCloseDisplay(display);

	return 0;
}
