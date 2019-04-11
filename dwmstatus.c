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
static const char *errstr(const char*);

static char errbuf[STRING_MAX];

static const char*
errstr(const char *str)
{
	snprintf(errbuf, sizeof(errbuf), "err: %s", str);

	return errbuf;
}

static const char*
draw_b(char *str, size_t n)
{
	const char *err = NULL;

	FILE *fd_0 = fopen("/sys/class/power_supply/BAT0/status", "r"),
	     *fd_1 = fopen("/sys/class/power_supply/BAT0/charge_now", "r"),
	     *fd_2 = fopen("/sys/class/power_supply/BAT0/charge_full", "r");

	if (fd_0 && fd_1 && fd_2) {

		const char *s;

		int c0 = 0,
		    c1 = 0;

		switch (fgetc(fd_0)) {
			case 'C': s = "+"; break;
			case 'D': s = "-"; break;
			case 'F': s = "";  break;
			default:
				err = errstr("s");
		}

		if (!err && EOF == fscanf(fd_1, "%d\n", &c0))
			err = errstr("c0");

		if (!err && EOF == fscanf(fd_2, "%d\n", &c1))
			err = errstr("c1");

		if (!err && (c0 <= 0 || c1 <= 0))
			err = errstr("invalid");

		if (!err && 0 > snprintf(str, n, "%s%d%%", s, (int)(c0 * 100.0 / c1)))
			err = errstr("snprintf");
	}

	if (fd_0) fclose(fd_0); else err = errstr("fd_0");
	if (fd_1) fclose(fd_1); else err = errstr("fd_1");
	if (fd_2) fclose(fd_2); else err = errstr("fd_2");

	return err ? err : str;
}

static const char*
draw_t(char *str, size_t n)
{
	const char *err = NULL;

	time_t now = time(0);

	if (0 == strftime(str, n, "%a %e %R", localtime(&now)))
		err = errstr("snprintf");

	return err ? err : str;
}

static const char*
draw_v(char *str, size_t n)
{
	const char *err = NULL;

	/* See `amixer -c N scontrols` */
	const char *card_name = "hw:0";

	int enabled_master = 0,
	    enabled_headphone = 0,
	    enabled_speaker = 0;

	long volcur   = 0, /* current volume */
	     volmin   = 0, /* minimum volume */
	     volmax   = 0, /* maximum volume */
	     volrange = 0; /* volume range */

	snd_mixer_t *handle = NULL;

	snd_mixer_elem_t *elem_master = NULL;
	snd_mixer_elem_t *elem_headphone = NULL;
	snd_mixer_elem_t *elem_speaker = NULL;

	snd_mixer_selem_id_t *sid_master = NULL;
	snd_mixer_selem_id_t *sid_headphone = NULL;
	snd_mixer_selem_id_t *sid_speaker = NULL;

	snd_mixer_selem_id_alloca(&sid_master);
	snd_mixer_selem_id_alloca(&sid_headphone);
	snd_mixer_selem_id_alloca(&sid_speaker);

	snd_mixer_selem_id_set_index(sid_master, 0);
	snd_mixer_selem_id_set_index(sid_headphone, 0);
	snd_mixer_selem_id_set_index(sid_speaker, 0);

	snd_mixer_selem_id_set_name(sid_master, "Master");
	snd_mixer_selem_id_set_name(sid_headphone, "Headphone");
	snd_mixer_selem_id_set_name(sid_speaker, "Speaker");

	if (0 > snd_mixer_open(&handle, 0))
		err = errstr("open");

	else if (0 > snd_mixer_attach(handle, card_name))
		err = errstr("attach");

	else if (0 > snd_mixer_selem_register(handle, NULL, NULL))
		err = errstr("selem register");

	else if (0 > snd_mixer_load(handle))
		err = errstr("load");

	else if ((NULL == (elem_master = snd_mixer_find_selem(handle, sid_master)))
	      || (NULL == (elem_headphone = snd_mixer_find_selem(handle, sid_headphone)))
	      || (NULL == (elem_speaker = snd_mixer_find_selem(handle, sid_speaker))))
		err = errstr("find selem");

	else if ((0 > snd_mixer_selem_get_playback_switch(elem_master, 0, &enabled_master))
	      || (0 > snd_mixer_selem_get_playback_switch(elem_headphone, 0, &enabled_headphone))
	      || (0 > snd_mixer_selem_get_playback_switch(elem_speaker, 0, &enabled_speaker)))
		err = errstr("get playback switch");

	else if (0 > snd_mixer_selem_get_playback_volume(elem_master, 0, &volcur))
		err = errstr("get playback vol");

	else if (0 > snd_mixer_selem_get_playback_volume_range(elem_master, &volmin, &volmax))
		err = errstr("get playback vol range");

	else if (0 > snd_mixer_detach(handle, card_name))
		err = errstr("detach");

	else if (0 >= (volrange = (volmax - volmin)))
		err = errstr("invalid range");

	else if (0 > snprintf(str, n, "%s %d%%%s%s%s",
				(enabled_master ? "" : "MUTE"),
				(int)(volcur * 100.0 / volrange),
				(enabled_headphone || enabled_speaker ? " +" : ""),
				(enabled_headphone ? "H" : ""),
				(enabled_speaker ? "S" : "")))
		err = errstr("snprintf");

	if (handle && 0 > snd_mixer_close(handle))
		err = errstr("close");

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
		char status[STRING_MAX + sizeof(" []  []  [] ")];

		char b_str[STRING_MAX];
		char t_str[STRING_MAX];
		char v_str[STRING_MAX];

		snprintf(status, sizeof(status), " [%s]  [%s]  [%s] ",
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
