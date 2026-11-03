#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_ITEMS 256
#define MAX_LEN 256
#define WIN_WIDTH 600
#define ITEM_HEIGHT 24
#define MAX_VISIBLE 10

typedef struct {
    char name[MAX_LEN];
    char cmd[MAX_LEN];
} App;

App apps[MAX_ITEMS];
int num_apps = 0;
int filtered_indices[MAX_ITEMS];
int num_filtered = 0;
int selected = 0;
char query[MAX_LEN] = "";

char *strcasestr(const char *haystack, const char *needle) {
    if (!*needle) return (char *)haystack;
    for (; *haystack; ++haystack) {
        if (tolower(*haystack) == tolower(*needle)) {
            const char *h = haystack, *n = needle;
            while (*h && *n && tolower(*h) == tolower(*n)) { h++; n++; }
            if (!*n) return (char *)haystack;
        }
    }
    return NULL;
}

void filter_apps() {
    num_filtered = 0;
    for (int i = 0; i < num_apps; i++) {
        if (strcasestr(apps[i].name, query)) {
            filtered_indices[num_filtered++] = i;
        }
    }
    if (selected >= num_filtered) selected = num_filtered > 0 ? num_filtered - 1 : 0;
}

int main() {
    char line[MAX_LEN * 2];
    while (fgets(line, sizeof(line), stdin)) {
        char *sep = strchr(line, '|');
        if (sep) {
            *sep = '\0';
            strncpy(apps[num_apps].name, line, MAX_LEN - 1);
            strncpy(apps[num_apps].cmd, sep + 1, MAX_LEN - 1);
            apps[num_apps].cmd[strcspn(apps[num_apps].cmd, "\n")] = 0;
            num_apps++;
        }
    }
    
    if (num_apps == 0) return 1;

    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) return 1;
    
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int sw = DisplayWidth(dpy, screen);
    
    int win_h = ITEM_HEIGHT * (MAX_VISIBLE + 1);
    int x = (sw - WIN_WIDTH) / 2;
    int y = 50;

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = 0x1a1a1a;
    attrs.border_pixel = 0xFF69B4;
    attrs.event_mask = ExposureMask | KeyPressMask;

    Window win = XCreateWindow(dpy, root, x, y, WIN_WIDTH, win_h, 2,
                               CopyFromParent, InputOutput, CopyFromParent,
                               CWOverrideRedirect | CWBackPixel | CWBorderPixel | CWEventMask, &attrs);

    XMapWindow(dpy, win);
    XSetInputFocus(dpy, win, RevertToParent, CurrentTime);

    GC gc = XCreateGC(dpy, win, 0, NULL);
    XFontStruct *font = XLoadQueryFont(dpy, "fixed");
    if (font) XSetFont(dpy, gc, font->fid);
    int fh = font ? font->ascent + font->descent : 16;
    
    filter_apps();
    XEvent ev;
    int running = 1;

    while (running) {
        XNextEvent(dpy, &ev);
        if (ev.type == Expose) {
            XSetForeground(dpy, gc, 0x1a1a1a);
            XFillRectangle(dpy, win, gc, 0, 0, WIN_WIDTH, win_h);
            
            XSetForeground(dpy, gc, 0xFF69B4);
            XDrawString(dpy, win, gc, 10, fh + 4, "> ", 2);
            XDrawString(dpy, win, gc, 30, fh + 4, query, strlen(query));
            
            XDrawLine(dpy, win, gc, 0, ITEM_HEIGHT, WIN_WIDTH, ITEM_HEIGHT);

            int start = selected > MAX_VISIBLE/2 ? selected - MAX_VISIBLE/2 : 0;
            if (start + MAX_VISIBLE > num_filtered) start = num_filtered - MAX_VISIBLE;
            if (start < 0) start = 0;

            for (int i = 0; i < MAX_VISIBLE && (start + i) < num_filtered; i++) {
                int idx = filtered_indices[start + i];
                int item_y = ITEM_HEIGHT * (i + 1);
                
                if (start + i == selected) {
                    XSetForeground(dpy, gc, 0xFF69B4);
                    XFillRectangle(dpy, win, gc, 0, item_y, WIN_WIDTH, ITEM_HEIGHT);
                    XSetForeground(dpy, gc, 0x1a1a1a);
                } else {
                    XSetForeground(dpy, gc, 0xd0d0d0);
                }
                XDrawString(dpy, win, gc, 10, item_y + fh + 4, apps[idx].name, strlen(apps[idx].name));
            }
        } else if (ev.type == KeyPress) {
            char buf[32];
            KeySym ks;
            int len = XLookupString(&ev.xkey, buf, sizeof(buf), &ks, NULL);
            
            if (ks == XK_Escape) {
                running = 0;
            } else if (ks == XK_Return) {
                if (num_filtered > 0) {
                    int idx = filtered_indices[selected];
                    if (fork() == 0) {
                        execl("/bin/sh", "sh", "-c", apps[idx].cmd, NULL);
                        exit(0);
                    }
                }
                running = 0;
            } else if (ks == XK_BackSpace) {
                int qlen = strlen(query);
                if (qlen > 0) {
                    query[qlen - 1] = '\0';
                    filter_apps();
                }
            } else if (ks == XK_Up) {
                if (selected > 0) selected--;
            } else if (ks == XK_Down) {
                if (selected < num_filtered - 1) selected++;
            } else if (len > 0 && isprint(buf[0])) {
                strncat(query, buf, MAX_LEN - strlen(query) - 1);
                filter_apps();
            }
            XClearArea(dpy, win, 0, 0, 0, 0, True);
        }
    }
    
    XCloseDisplay(dpy);
    return 0;
}