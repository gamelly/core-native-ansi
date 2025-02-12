#include "zeebo.h"

static int current_mode;

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

}

void tui_draw_text(app_t *const self, int16_t x, int16_t y, int16_t text_id, int16_t text_size)
{
    const char* text = tui_queue_get_text(text_id);
    int16_t pos = 0;

    if (geoclip_text(self, &x, &y, &pos)) {
        concat(self, out,  "\x1b[%d;%dH\x1B[%dm%s\x1B[0m", y, x, text_size == 2, &text[pos]);
    }
}
