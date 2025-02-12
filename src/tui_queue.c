#include "zeebo.h"

static uint8_t queue_command[2048];
static int16_t queue_param[8192];
static const char *queue_text[255];

static uint8_t index_text = 0;
static uint16_t index_cmd = 0;
static uint16_t index_param = 0;

const char *const tui_queue_get_text(uint8_t index)
{
    return queue_text[index];
}

uint8_t tui_queue_push_text(const char* text)
{
    queue_text[index_text] = text;
    return index_text++;
}

void tui_queue_push(uint8_t cmd, int16_t a, int16_t b, int16_t c, int16_t d)
{
    if (index_cmd < sizeof(queue_command)) {
        queue_command[index_cmd++] = cmd;
        queue_param[index_param++] = a;
        queue_param[index_param++] = b;
        queue_param[index_param++] = c;
        queue_param[index_param++] = d;
    }
}

void tui_queue_burn(app_t *const self)
{
    uint16_t index = 0;
    uint16_t param = 0;

    static const cmd_t commands[] = {
        tui_draw_mode,
        tui_draw_color,
        tui_draw_rect,
        tui_draw_line,
        tui_draw_text
    };

    while (index < index_cmd) {
        uint8_t cmd = queue_command[index++] - 48;
        int16_t param_1 = queue_param[param++];
        int16_t param_2 = queue_param[param++];
        int16_t param_3 = queue_param[param++];
        int16_t param_4 = queue_param[param++];
        if (cmd < sizeof(commands) / sizeof(commands[0]) && commands[cmd] != NULL) {
            commands[cmd](self, param_1, param_2, param_3, param_4);
        }
    }

    index_cmd = 0;
    index_text = 0;
    index_param = 0;
}
