.POSIX:

CC      = cc
CFLAGS  = -std=c99 -Wall -Wextra -pedantic -O3
LDFLAGS = -s -lX11 -lasound

DESTDIR = /usr/local/bin

all: dwmstatus

dwmstatus: dwmstatus.c
	@echo "CC      = $(CC)"
	@echo "CFLAGS  = $(CFLAGS)"
	@echo "LDFLAGS = $(LDFLAGS)"
	@echo CC $@
	@$(CC) $(CFLAGS) $(LDFLAGS) dwmstatus.c -o $@

install: dwmstatus
	mkdir -p $(DESTDIR)
	cp -f dwmstatus $(DESTDIR)
	chmod 755 $(DESTDIR)/dwmstatus

uninstall:
	rm -f $(DESTDIR)/dwmstatus

clean:
	@echo cleaning
	@rm -f dwmstatus

.PHONY: all clean install uninstall
