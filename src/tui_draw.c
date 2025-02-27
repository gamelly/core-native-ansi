#include "zeebo.h"

#define GLY_TYPE_INT int16_t
#define GLY_TYPE_SAFE
#include "font/gly_type_render.h"

static int current_mode;
static char current_char = 'x';
static app_t *other;

static void draw_line(int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        concat(other, out, "\x1b[%d;%dH%c", y1, x1, current_char);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void tui_draw_mode(app_t *const self, int16_t drawmode, int16_t flushmode, int16_t change_mode, int16_t change_flush)
{
    if (change_mode) {
        current_mode = drawmode;
    }
}

void tui_draw_color(app_t *const self, int16_t r, int16_t g, int16_t b, int16_t a)
{

}

void tui_draw_rect(app_t *const self, int16_t x, int16_t y, int16_t w, int16_t h)
{
    x = MAX(1, x);
    y = MAX(1, y);
    w = MAX(1, w);
    h = MAX(1, h);

    if (!geoclip_rect(self, &x, &y, &w, &h)) {
        return;
    }

    if (current_mode) {
        int16_t x2 = x + w;
        int16_t y2 = y + h - 1;

        concat(self, out,  "\x1b[%d;%dH+\x1b[%d;%dH+", y, x, y2, x2);
        concat(self, out,  "\x1b[%d;%dH+\x1b[%d;%dH+", y2, x, y, x2);

        for (uint16_t row = 1; (row + 1) < h; row++) {
            concat(self, out,  "\x1b[%d;%dH|\x1b[%d;%dH|", y + row, x, y + row, x + w);
        }

        if (x + 1 < x2) {
            concat(self, out,  "\x1b[%d;%dH", y, x + 1);
            memset(&self->out.msg[self->out.len], '-', w - 1);
            self->out.len += w - 1;
            concat(self, out,  "\x1b[%d;%dH", y + h - 1, x + 1);
            memset(&self->out.msg[self->out.len], '-', w - 1);
            self->out.len += w - 1;
        }
    }
    else {
        for (uint16_t row = 0; row < h; row++) {
            concat(self, out,  "\x1b[%d;%dH", row + y, x);
            memset(&self->out.msg[self->out.len], '#', w);
            self->out.len += w;
        }
    }
}

void tui_draw_line(app_t *const self, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    if (geoclip_line(self, &x1, &y1, &x2, &y2)) {
        int dx = x2 - x1, dy = y2 - y1;
        /*if (dx == 0) {
            current_char = '|';
        }
        if (dy == 0) {
            current_char = '-';
        }
        else {
            current_char = (dx * dy > 0) ? '\\' : '/'; 
        }*/
        current_char = '+';
        other = self;
        draw_line(x1, y1, x2, y2);
    }
}

/**
 * @todo this core is bullshit!
 */
void tui_draw_text(app_t *const self, int16_t x, int16_t y, int16_t text_id, int16_t text_size)
{
    const char* text = tui_queue_get_text(text_id);
    int16_t text_len = strlen(text);
    int16_t pos = 0;

    if (text_size >= 3 && text_size <= 4) {
        int16_t index = 0;
        char* text_upper = malloc(text_len + 1);
        while (index < text_len) {
            text_upper[index] = toupper((unsigned char)text[index]);
            index++;
        }
        text = text_upper;
    }

    if (text_size <= 2 && geoclip_text(self, &x, &y, &pos, &text_len)) {
        concat(self, out, "\x1b[%d;%dH\x1B[%dm%.*s\x1B[0m", y, x, text_size == 2, text_len, &text[pos]);
    }
    if (text_size > 2 && geoclip_text2(self, &x, &y, &pos, &text_len)) {
        other = self;
        current_char = 'x';
        gly_type_render(x, y, text_size, &text[pos], text_len, draw_line);
    }

    if (text_size >= 3 && text_size <= 4) {
        free((void*)text);
    }
}
