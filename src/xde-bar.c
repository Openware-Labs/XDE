#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAR_HEIGHT 24
#define PADDING 10

typedef struct {
    int x;
    int width;
    char command[512];
} ClickableRegion;

ClickableRegion regions[20];
int num_regions = 0;

void draw_bar(Display *dpy, Window win, GC gc, XFontStruct *font, int width, int font_ascent, char *buffer) {
    XClearWindow(dpy, win);
    num_regions = 0;

    char temp[2048];
    strcpy(temp, buffer);

    char *last_pipe = strrchr(temp, '|');
    char right_part[512] = "";
    
    if (last_pipe) {
        strcpy(right_part, last_pipe + 1);
        *last_pipe = '\0'; 
    }

    int current_x = PADDING;
    char *ptr = temp;

    while (ptr && *ptr) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        char *next_pipe = strchr(ptr, '|');
        if (next_pipe) *next_pipe = '\0';

        char display_text[512] = "";
        char command[512] = "";

        if (*ptr == '@') {
            char *cmd_end = strchr(ptr + 1, '@');
            if (cmd_end) {
                strncpy(command, ptr + 1, cmd_end - ptr - 1);
                command[cmd_end - ptr - 1] = '\0';
                strcpy(display_text, cmd_end + 1);
            } else {
                strcpy(display_text, ptr);
            }
        } else {
            strcpy(display_text, ptr);
        }

        if (strlen(display_text) > 0) {
            XDrawString(dpy, win, gc, current_x, font_ascent + 6, display_text, strlen(display_text));
            int text_width = XTextWidth(font, display_text, strlen(display_text));

            if (strlen(command) > 0 && num_regions < 20) {
                regions[num_regions].x = current_x;
                regions[num_regions].width = text_width;
                strcpy(regions[num_regions].command, command);
                num_regions++;
            }

            current_x += text_width + PADDING;
        }

        if (next_pipe) {
            XDrawString(dpy, win, gc, current_x, font_ascent + 6, "|", 1);
            current_x += XTextWidth(font, "|", 1) + PADDING;
            ptr = next_pipe + 1;
        } else {
            ptr = NULL;
        }
    }

    if (strlen(right_part) > 0) {
        char *right_ptr = right_part;
        while (*right_ptr == ' ') right_ptr++;

        char display_text[512] = "";
        char command[512] = "";

        if (*right_ptr == '@') {
            char *cmd_end = strchr(right_ptr + 1, '@');
            if (cmd_end) {
                strncpy(command, right_ptr + 1, cmd_end - right_ptr - 1);
                command[cmd_end - right_ptr - 1] = '\0';
                strcpy(display_text, cmd_end + 1);
            } else {
                strcpy(display_text, right_ptr);
            }
        } else {
            strcpy(display_text, right_ptr);
        }

        int right_width = XTextWidth(font, display_text, strlen(display_text));
        int right_x = width - right_width - PADDING;
        
        XDrawString(dpy, win, gc, right_x, font_ascent + 6, display_text, strlen(display_text));

        if (strlen(command) > 0 && num_regions < 20) {
            regions[num_regions].x = right_x;
            regions[num_regions].width = right_width;
            strcpy(regions[num_regions].command, command);
            num_regions++;
        }
    }
}

int main() {
    Display *dpy = XOpenDisplay(NULL);
    if (!dpy) { fprintf(stderr, "Cannot open display\n"); return 1; }

    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    int width = DisplayWidth(dpy, screen);

    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixel = 0x1a1a1a;
    attrs.event_mask = ExposureMask | ButtonPressMask;

    Window win = XCreateWindow(dpy, root, 0, 0, width, BAR_HEIGHT, 0,
                               CopyFromParent, InputOutput, CopyFromParent,
                               CWOverrideRedirect | CWBackPixel | CWEventMask, &attrs);

    XWMHints *wmhints = XAllocWMHints();
    wmhints->flags = InputHint;
    wmhints->input = False;
    XSetWMHints(dpy, win, wmhints);
    XFree(wmhints);

    XMapWindow(dpy, win);
    GC gc = XCreateGC(dpy, win, 0, NULL);
    XSetForeground(dpy, gc, 0xFF69B4);

    XFontStruct *font = XLoadQueryFont(dpy, "fixed");
    if (font) XSetFont(dpy, gc, font->fid);
    int font_ascent = font ? font->ascent : 12;

    char buffer[2048] = " XDE Menu | ... | 00:00 ";
    XEvent ev;

    draw_bar(dpy, win, gc, font, width, font_ascent, buffer);

    while (1) {
        while (XPending(dpy)) {
            XNextEvent(dpy, &ev);
            if (ev.type == Expose) {
                draw_bar(dpy, win, gc, font, width, font_ascent, buffer);
            }
            else if (ev.type == ButtonPress) {
                int mouse_x = ev.xbutton.x;
                for (int i = 0; i < num_regions; i++) {
                    if (mouse_x >= regions[i].x && mouse_x <= regions[i].x + regions[i].width) {
                        if (fork() == 0) {
                            execl("/bin/sh", "sh", "-c", regions[i].command, NULL);
                            exit(0);
                        }
                        break;
                    }
                }
            }
        }

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ConnectionNumber(dpy), &fds);
        FD_SET(STDIN_FILENO, &fds);

        if (select(ConnectionNumber(dpy) + 1, &fds, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                    buffer[strcspn(buffer, "\n")] = 0;
                    draw_bar(dpy, win, gc, font, width, font_ascent, buffer);
                }
            }
        }
    }

    XFreeGC(dpy, gc);
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
    return 0;
}