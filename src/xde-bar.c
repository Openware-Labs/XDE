#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

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

    int right_total_width = 0;
    int r_count = 0;
    char r_texts[20][256];
    char r_cmds[20][256];
    int r_widths[20];
    int r_is_img[20];

    if (strlen(right_part) > 0) {
        char *p = right_part;
        while (*p && r_count < 20) {
            while (*p == ' ') p++;
            if (!*p) break;

            if (*p == '~') {
                char *end = strchr(p + 1, '~');
                if (end) {
                    int len = end - p - 1;
                    if (len >= 255) len = 255;
                    snprintf(r_texts[r_count], sizeof(r_texts[r_count]), "%.*s", len, p + 1);
                    r_is_img[r_count] = 1;
                    r_widths[r_count] = 16;
                    r_count++;
                    p = end + 1;
                } else break;
            } else if (*p == '@') {
                char *cmd_end = strchr(p + 1, '@');
                if (cmd_end) {
                    int cmd_len = cmd_end - p - 1;
                    if (cmd_len >= 255) cmd_len = 255;
                    snprintf(r_cmds[r_count], sizeof(r_cmds[r_count]), "%.*s", cmd_len, p + 1);

                    char *text_start = cmd_end + 1;
                    char *text_end = text_start;
                    while (*text_end && *text_end != ' ' && *text_end != '~' && *text_end != '@') text_end++;
                    int text_len = text_end - text_start;
                    if (text_len >= 255) text_len = 255;
                    snprintf(r_texts[r_count], sizeof(r_texts[r_count]), "%.*s", text_len, text_start);
                    
                    r_is_img[r_count] = 0;
                    r_widths[r_count] = XTextWidth(font, r_texts[r_count], strlen(r_texts[r_count]));
                    r_count++;
                    p = text_end;
                } else break;
            } else {
                char *text_end = p;
                while (*text_end && *text_end != ' ' && *text_end != '~' && *text_end != '@') text_end++;
                int text_len = text_end - p;
                if (text_len >= 255) text_len = 255;
                snprintf(r_texts[r_count], sizeof(r_texts[r_count]), "%.*s", text_len, p);
                
                r_cmds[r_count][0] = '\0';
                r_is_img[r_count] = 0;
                r_widths[r_count] = XTextWidth(font, r_texts[r_count], strlen(r_texts[r_count]));
                r_count++;
                p = text_end;
            }
            while (*p == ' ') p++;
        }

        for (int i = 0; i < r_count; i++) right_total_width += r_widths[i] + PADDING;
    }

    int current_x = PADDING;
    char *ptr = temp;

    while (ptr && *ptr) {
        while (*ptr == ' ') ptr++;
        if (*ptr == '\0') break;

        char *next_pipe = strchr(ptr, '|');
        if (next_pipe) *next_pipe = '\0';

        if (*ptr == '~') {
            char *img_end = strchr(ptr + 1, '~');
            if (img_end) {
                char img_name[64];
                int len = img_end - ptr - 1;
                if (len >= 63) len = 63;
                snprintf(img_name, sizeof(img_name), "%.*s", len, ptr + 1);
                
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "/etc/xde/%s.xbm", img_name);
                
                int x, y;
                unsigned int w, h;
                Pixmap bitmap;
                
                if (XReadBitmapFile(dpy, win, filepath, &w, &h, &bitmap, &x, &y) == BitmapSuccess) {
                    XSetBackground(dpy, gc, 0x1a1a1a);
                    XSetForeground(dpy, gc, 0xFF69B4);
                    XCopyPlane(dpy, bitmap, win, gc, 0, 0, w, h, current_x, (BAR_HEIGHT - h) / 2, 1);
                    current_x += w + PADDING;
                    XFreePixmap(dpy, bitmap);
                } else {
                    current_x += PADDING;
                }
            }
        } 
        else {
            char display_text[512] = "";
            char command[512] = "";

            if (*ptr == '@') {
                char *cmd_end = strchr(ptr + 1, '@');
                if (cmd_end) {
                    int cmd_len = cmd_end - ptr - 1;
                    if (cmd_len >= 511) cmd_len = 511;
                    snprintf(command, sizeof(command), "%.*s", cmd_len, ptr + 1);
                    snprintf(display_text, sizeof(display_text), "%.*s", (int)sizeof(display_text)-1, cmd_end + 1);
                } else {
                    snprintf(display_text, sizeof(display_text), "%.*s", (int)sizeof(display_text)-1, ptr);
                }
            } else {
                snprintf(display_text, sizeof(display_text), "%.*s", (int)sizeof(display_text)-1, ptr);
            }

            if (strlen(display_text) > 0) {
                int text_width = XTextWidth(font, display_text, strlen(display_text));
                
                if (current_x + text_width < width - right_total_width - 20) {
                    XSetForeground(dpy, gc, 0xFF69B4);
                    XDrawString(dpy, win, gc, current_x, font_ascent + 6, display_text, strlen(display_text));

                    if (strlen(command) > 0 && num_regions < 20) {
                        regions[num_regions].x = current_x;
                        regions[num_regions].width = text_width;
                        snprintf(regions[num_regions].command, sizeof(regions[num_regions].command), "%.*s", (int)sizeof(regions[num_regions].command)-1, command);
                        num_regions++;
                    }
                    current_x += text_width + PADDING;
                }
            }
        }

        if (next_pipe) {
            XSetForeground(dpy, gc, 0xFF69B4);
            XDrawString(dpy, win, gc, current_x, font_ascent + 6, "|", 1);
            current_x += XTextWidth(font, "|", 1) + PADDING;
            ptr = next_pipe + 1;
        } else {
            ptr = NULL;
        }
    }

    if (r_count > 0) {
        int current_right_x = width - right_total_width;
        for (int i = 0; i < r_count; i++) {
            if (r_is_img[i]) {
                char filepath[256];
                snprintf(filepath, sizeof(filepath), "/etc/xde/%s.xbm", r_texts[i]);
                int x, y; unsigned int w, h; Pixmap bitmap;
                if (XReadBitmapFile(dpy, win, filepath, &w, &h, &bitmap, &x, &y) == BitmapSuccess) {
                    XSetBackground(dpy, gc, 0x1a1a1a);
                    XSetForeground(dpy, gc, 0xFF69B4);
                    XCopyPlane(dpy, bitmap, win, gc, 0, 0, w, h, current_right_x, (BAR_HEIGHT - h) / 2, 1);
                    XFreePixmap(dpy, bitmap);
                }
            } else {
                if (strlen(r_texts[i]) > 0) {
                    XSetForeground(dpy, gc, 0xFF69B4);
                    XDrawString(dpy, win, gc, current_right_x, font_ascent + 6, r_texts[i], strlen(r_texts[i]));
                    if (strlen(r_cmds[i]) > 0 && num_regions < 20) {
                        regions[num_regions].x = current_right_x;
                        regions[num_regions].width = r_widths[i];
                        snprintf(regions[num_regions].command, sizeof(regions[num_regions].command), "%.*s", (int)sizeof(regions[num_regions].command)-1, r_cmds[i]);
                        num_regions++;
                    }
                }
            }
            current_right_x += r_widths[i] + PADDING;
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

        XRaiseWindow(dpy, win);

        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(ConnectionNumber(dpy), &fds);
        FD_SET(STDIN_FILENO, &fds);

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000;

        if (select(ConnectionNumber(dpy) + 1, &fds, NULL, NULL, &tv) > 0) {
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