#include "zeebo.h"

static int outcode(int x, int y, int w, int h) {
    int code = 0;
    if (x < 0)    code |= 0x01;
    if (x > w)    code |= 0x02;
    if (y < 0)    code |= 0x04;
    if (y > h)    code |= 0x08;
    return code;
}

/**
 * @details Cohen-Sutherland
 * @li left   0x01
 * @li right  0x02
 * @li bottom 0x04
 * @li top    0x08
 */
bool geoclip_line(app_t *const self, int16_t *const x1, int16_t *const y1, int16_t *const x2, int16_t *const y2)
{
    int16_t w = self->ctx.width;
    int16_t h = self->ctx.height;
    int c1 = outcode(*x1, *y1, w, h), c2 = outcode(*x2, *y2, w, h);
    
    while (c1 | c2) {
        if (c1 & c2) return false;

        int c = c1 ? c1 : c2, x, y;

        if (c & 0x08) { 
            x = *x1 + (*x2 - *x1) * (h - *y1) / (*y2 - *y1);
            y = h;
        } else if (c & 0x04) { 
            x = *x1 + (*x2 - *x1) * (0 - *y1) / (*y2 - *y1);
            y = 0;
        } else if (c & 0x02) { 
            y = *y1 + (*y2 - *y1) * (w - *x1) / (*x2 - *x1);
            x = w;
        } else { 
            y = *y1 + (*y2 - *y1) * (0 - *x1) / (*x2 - *x1);
            x = 0;
        }

        if (c == c1) {
            *x1 = x;
            *y1 = y;
            c1 = outcode(*x1, *y1, w, h);
        } else {
            *x2 = x;
            *y2 = y;
            c2 = outcode(*x2, *y2, w, h);
        }
    }
    return true;
}

bool geoclip_rect(app_t *const self, int16_t *const x, int16_t *const y, int16_t *const w, int16_t *const h)
{
    if (*x < 0) {
        *w += *x;
        *x = 0;
    }

    if (*y < 0) {
        *h += *y;
        *y = 0;
    }

    if ((*x + *w) > self->ctx.width) {
        *w = self->ctx.width - *x;
    }

    if ((*y + *h) > self->ctx.height) {
        *h = self->ctx.height - *y;
    }

    return (*h > 0) && (*w > 0);
}

bool geoclip_text(app_t *const self, int16_t *const x, int16_t *const y, int16_t *const pos, int16_t *const len)
{
    if (0 > *x || *x > self->ctx.width) {
        return false;
    }

    if (0 > *y || *y > self->ctx.height) {
        return false;
    }

    return true;
}

bool geoclip_text2(app_t *const self, int16_t *const x, int16_t *const y, int16_t *const pos, int16_t *const len)
{
    if (0 > *x || *x > self->ctx.width) {
        return false;
    }

    if (0 > *y || *y > self->ctx.height) {
        return false;
    }

    return true;
}
