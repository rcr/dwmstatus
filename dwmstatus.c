#include <alloca.h>
#include <alsa/asoundlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <X11/Xlib.h>

#define STRING_MAX 256

static const char *draw_b(char*, size_t);
static const char *draw_t(char*, size_t);
static const char *draw_v(char*, size_t);

static const char*
draw_b(char *str, size_t n)
{
	char *err = NULL;

	FILE *fd_0 = fopen("/sys/class/power_supply/BAT0/status", "r"),
	     *fd_1 = fopen("/sys/class/power_supply/BAT0/charge_now", "r"),
	     *fd_2 = fopen("/sys/class/power_supply/BAT0/charge_full_design", "r");

	if (fd_0 && fd_1 && fd_2) {

		char s = 0;

		int c0 = 0,
		    c1 = 0;

		switch (fgetc(fd_0)) {
			case 'C': s = '+'; break;
			case 'D': s = '-'; break;
			case 'F': s = ' '; break;
			default: err = "err: s";
		}

		if (!err && EOF == fscanf(fd_1, "%d\n", &c0))
			err = "err: c0";

		if (!err && EOF == fscanf(fd_2, "%d\n", &c1))
			err = "err: c1";

		if (!err && (c0 <= 0 || c1 <= 0))
			err = "err: invalid";

		if (!err && 0 > snprintf(str, n, "%c%d%%", s, (int)(c0 * 100.0 / c1)))
			err = "err: snprintf";
	}

	if (fd_0) fclose(fd_0); else err = "err: fd_0";
	if (fd_1) fclose(fd_1); else err = "err: fd_1";
	if (fd_2) fclose(fd_2); else err = "err: fd_2";

	return err ? err : str;
}

static const char*
draw_t(char *str, size_t n)
{
	char *err = NULL;

	time_t now = time(0);

	if (0 == strftime(str, n, "%a %e  %R", localtime(&now)))
		err = "err: snprintf";

	return err ? err : str;
}

static const char*
draw_v(char *str, size_t n)
{
	char *err = NULL;

	int enabled = 0;

	long volcur   = 0, /* current volume */
	     volmin   = 0, /* minimum volume */
	     volmax   = 0, /* maximum volume */
	     volrange = 0; /* volume range */

	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem = NULL;
	snd_mixer_selem_id_t *sid = NULL;

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "Master");

	if (0 > snd_mixer_open(&handle, 0))
		err = "err: open";

	else if (0 > snd_mixer_attach(handle, "default"))
		err = "err: attach";

	else if (0 > snd_mixer_selem_register(handle, NULL, NULL))
		err = "err: selem register";

	else if (0 > snd_mixer_load(handle))
		err = "err: load";

	else if (NULL == (elem = snd_mixer_find_selem(handle, sid)))
		err = "err: find selem";

	else if (0 > snd_mixer_selem_get_playback_switch(elem, 0, &enabled))
		err = "err: get playback switch";

	else if (0 > snd_mixer_selem_get_playback_volume(elem, 0, &volcur))
		err = "err: get playback vol";

	else if (0 > snd_mixer_selem_get_playback_volume_range(elem, &volmin, &volmax))
		err = "err: get playback vol range";

	else if (0 > snd_mixer_detach(handle, "default"))
		err = "err: detach";

	else if (0 >= (volrange = (volmax - volmin)))
		err = "err: invalid range";

	else if (0 > snprintf(str, n, "%c %d%%", (enabled ? ' ' : 'M'), (int)(volcur * 100.0 / volrange)))
		err = "err: snprintf";

	if (handle && 0 > snd_mixer_close(handle))
		err = "err: close";

	return err ? err : str;
}

int
main(int argc, char **argv)
{
	unsigned int seconds = 0;

	if (argc > 1)
		seconds = (unsigned int)(strtoul(argv[1], NULL, 0));

	Display *display;

	if (!(display = XOpenDisplay(NULL)))
		exit(EXIT_FAILURE);

	do {
		char status[STRING_MAX];

		char b_str[STRING_MAX];
		char t_str[STRING_MAX];
		char v_str[STRING_MAX];

		snprintf(status, sizeof(status), " %s  ~  %s  ~  %s ",
			draw_v(v_str, sizeof(v_str)),
			draw_b(b_str, sizeof(b_str)),
			draw_t(t_str, sizeof(t_str))
		);

		XStoreName(display, DefaultRootWindow(display), status);
		XSync(display, 0);

		sleep(seconds);

	} while (seconds);

	XCloseDisplay(display);

	return 0;
}
