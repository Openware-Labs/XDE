# XDE — X Desktop Environment

A minimalist desktop environment for Linux, built on pure X11 and the classic X tools. It features a custom panel and application launcher written entirely in C using Xlib, requiring no external dependencies like D-Bus, compositors, or daemons.

**Goal:** be the "X11 reference desktop" — fast, lightweight, and built from the ground up to demonstrate the power of raw X11.

## Status

Active — Working.

## Releases

v0.1.0 — First Release.

## Features

- **Pure X11 Session:** Runs `twm` as the window manager with classic X.Org tools.
- **Custom Panel (`xde-panel`):** A lightweight top bar written in C/Xlib that displays the window list, clock, and clickable regions.
- **Custom Launcher (`xde-menu`):** A native application launcher written in C/Xlib that reads `.desktop` files, supports live filtering, and handles keyboard navigation.
- **Minimalist Aesthetic:** Polka dot wallpaper generated via `xsetroot` and cohesive Xresources theming.
- **Global Keybindings:** Powered by `xbindkeys`.
- **Clipboard Management:** Integrated with `clipmenu` and `xclip`.

## Requirements

### Core Dependencies

- `base-devel` (build tools: `gcc`, `make`)
- `libx11` (Xlib headers and libraries)
- `twm` (window manager)
- `xterm` (terminal emulator)
- `xinit` (session startup)
- `xrdb`, `xsetroot` (X resource and root window utilities)
- `xbindkeys` (global keybindings)
- `xdotool` (window manipulation for the panel)
- `clipmenu`, `xclip` (clipboard management)

## X11 Utilities

The following is a comprehensive list of official X.Org utilities and applications. Most are included in the `xorg-apps` group (Arch) or `x11-apps` / `x11-utils` packages (Debian/Ubuntu). You can install individual packages if you prefer a minimal setup.

### Core Applications

- `twm` — Tab Window Manager
- `xterm` — Terminal emulator
- `xinit` / `startx` — X session initialization
- `xdm` — X Display Manager

### Demo & Classic Apps

- `xeyes` — Googly eyes that follow the mouse
- `xclock` — Analog or digital clock
- `oclock` — Round analog clock
- `xcalc` — Scientific calculator
- `xlogo` — Displays the X.Org logo
- `xedit` — Simple text editor
- `xman` — Manual page browser
- `xmessage` — Displays a message or prompt
- `xload` — System load average monitor
- `xgc` — Graphics demo
- `ico` — Animated polyhedron demo
- `xmore` — Text pager

### Configuration & Information

- `xdpyinfo` — Display information
- `xev` — Event tester
- `xprop` — Window property display/editor
- `xwininfo` — Window information
- `xlsclients` — List client applications
- `xlsfonts` — List available fonts
- `xlsatoms` — List interned atoms
- `xvinfo` — X-Video extension information
- `xrandr` — Screen resize and rotate
- `xrdb` — X resource database manager
- `xset` — User preferences for X
- `xsetroot` — Root window parameters
- `xgamma` — Gamma correction
- `xbacklight` — Backlight adjustment
- `xcmsdb` — Color management database
- `xhost` — X server access control
- `xinput` — Input device configuration
- `xmodmap` — Keyboard and mouse mapping
- `setxkbmap` — Keyboard layout
- `xkbcomp` — XKB description compiler

### Bitmap & Cursor Tools

- `bitmap` — Bitmap editor
- `atobm` / `bmtoa` — ASCII/bitmap converters
- `xmag` — Screen magnifier
- `xwd` — Window dump (screenshot) utility
- `xwud` — Window dump viewer
- `xcursorgen` — Cursor file generator

### Session & Auth

- `xauth` — X authority file utility
- `iceauth` — ICE authority utility
- `sessreg` — Session registration
- `smproxy` — Session Manager proxy
- `xsm` — X Session Manager

### Miscellaneous

- `xkill` — Kill a client by its X resource
- `xrefresh` — Refresh the screen
- `appres` — List application resources
- `editres` — Dynamic resource editor
- `listres` — List widget resources
- `luit` — Locale/encoding conversion
- `xbiff` — Mailbox notifier
- `xconsole` — Console message monitor
- `xclipboard` — Clipboard manager
- `x11perf` / `x11perfcomp` — Graphics benchmarking
- `rendercheck` — Render extension tester
- `transset` — Window opacity setter

## Install

    make
    sudo make install

Then log out and choose **XDE** in your display manager session menu (Ly, GDM, LightDM, SDDM).

## Uninstall

    sudo make uninstall

## User Configuration

Override system defaults by creating files in your home directory:

- `~/.Xresources` — X client colors and fonts.
- `~/.twmrc` — Full twm configuration (overrides the XDE default).
- `~/.xbindkeysrc` — Custom keyboard shortcuts.

## Default Keybindings

- `Shift + Alt + T` — Open `xterm`
- `Super L` — Open `xde-menu` (Application Launcher)
- `Alt + F4` — Close the active window

## Project Layout

    bin/
      xde-session      Session startup script
      xde-bar          Shell script generating panel data
      xde-apps         Shell script parsing .desktop files
      xde-launcher     Wrapper script for the menu

    src/
      xde-bar.c        Source code for the custom panel (xde-panel)
      xde-menu.c       Source code for the custom launcher (xde-menu)

    etc/xde/
      twmrc            Default window manager configuration
      Xresources       Default X11 resources
      polka.xbm        Wallpaper bitmap pattern
      xde-close.xbm    Custom close button bitmap
      xbindkeysrc      Default global keybindings

    share/xsessions/
      xde.desktop      Display manager session entry

## License

MIT. See [LICENSE](/LICENSE).
