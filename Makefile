PREFIX  ?= /usr/local
DESTDIR ?=
VERSION := 0.1.1

BINDIR := $(DESTDIR)$(PREFIX)/bin
ETCDIR := $(DESTDIR)/etc/xde
XSDIR  := $(DESTDIR)/usr/share/xsessions

CC = gcc
CFLAGS = -O2 -Wall
X11LIBS = -lX11

.PHONY: all install uninstall test clean

all: xde-panel xde-menu
	@echo "XDE v$(VERSION)"
	@echo ""
	@echo "Targets:"
	@echo "  make install    install to $(PREFIX) and /etc/xde"
	@echo "  make uninstall  remove installed files"
	@echo "  make test       run XDE nested in Xephyr (Install first)"

xde-panel: src/xde-bar.c
	$(CC) $(CFLAGS) -o xde-panel src/xde-bar.c $(X11LIBS)

xde-menu: src/xde-menu.c
	$(CC) $(CFLAGS) -o xde-menu src/xde-menu.c $(X11LIBS)

install: xde-panel xde-menu
	install -d $(BINDIR) $(ETCDIR) $(XSDIR)
	install -m 755 bin/xde-session             $(BINDIR)/xde-session
	install -m 755 bin/xde-apps                $(BINDIR)/xde-apps
	install -m 755 bin/xde-bar                 $(BINDIR)/xde-bar
	install -m 755 xde-panel                   $(BINDIR)/xde-panel
	install -m 755 xde-menu                    $(BINDIR)/xde-menu
	install -m 755 bin/xde-launcher            $(BINDIR)/xde-launcher
	install -m 644 etc/xde/twmrc               $(ETCDIR)/twmrc
	install -m 644 etc/xde/Xresources          $(ETCDIR)/Xresources
	install -m 644 etc/xde/polka.xbm           $(ETCDIR)/polka.xbm
	install -m 644 etc/xde/xde-close.xbm       $(ETCDIR)/xde-close.xbm
	install -m 644 etc/xde/wifi_0.xbm          $(ETCDIR)/wifi_0.xbm
	install -m 644 etc/xde/wifi_1.xbm          $(ETCDIR)/wifi_1.xbm
	install -m 644 etc/xde/wifi_2.xbm          $(ETCDIR)/wifi_2.xbm
	install -m 644 etc/xde/wifi_3.xbm          $(ETCDIR)/wifi_3.xbm
	install -m 644 etc/xde/wifi_4.xbm          $(ETCDIR)/wifi_4.xbm
	install -m 644 etc/xde/xbindkeysrc         $(ETCDIR)/xbindkeysrc
	install -m 644 share/xsessions/xde.desktop $(XSDIR)/xde.desktop
	@echo "XDE v$(VERSION) installed."
	@echo "Select 'XDE' in your display manager session menu."

uninstall:
	rm -f  $(BINDIR)/xde-session
	rm -f  $(BINDIR)/xde-apps
	rm -f  $(BINDIR)/xde-bar
	rm -f  $(BINDIR)/xde-panel
	rm -f  $(BINDIR)/xde-menu
	rm -f  $(BINDIR)/xde-launcher
	rm -f  $(XSDIR)/xde.desktop
	rm -rf $(ETCDIR)/xde/
	@echo "XDE removed."

test: xde-panel xde-menu
	@command -v Xephyr >/dev/null 2>&1 || { \
		echo "Xephyr not found."; exit 1; }
	@echo "Launching XDE inside Xephyr (1280x800)..."
	@echo "Close the Xephyr window to end the test."
	@Xephyr :1 -screen 1280x800 -title "XDE (test)" & \
	XPID=$$!; \
	sleep 1; \
	DISPLAY=:1 bin/xde-session; \
	kill $$XPID 2>/dev/null || true

clean:
	rm -f xde-panel xde-menu
	@echo "Cleaned up build artifacts."