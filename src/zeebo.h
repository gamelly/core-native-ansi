#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>

#include <math.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#define delay(ms) Sleep(ms)
#else
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#define delay(ms) usleep((ms) * 1000)
#endif

#ifdef __GNUC__
#define tui_lua_check_top(a,b) tui_lua_check_top_fname(a,b,__func__)
#else
#define tui_lua_check_top(a,b) tui_lua_check_top_fname(a,b,"function")
#endif

#include "lua/lua.h"
#include "lua/lualib.h"
#include "lua/lauxlib.h"

#ifndef MIN
#define MIN(a,b) (((a)<(b))?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif
#define print(b,l)          write(STDOUT_FILENO,b,l)
#define concat(s, t, ...)   (s)->t.len+=snprintf((s)->t.msg+(s)->t.len,sizeof((s)->t.msg)-(s)->t.len,##__VA_ARGS__)

typedef struct {
    lua_State *L;
    struct {
        bool tty;
    } started;
    struct {
        char msg[256];
        uint8_t len;
    } err;
    struct {
        char msg[81920];
        size_t len;
    } out;
    struct {
        uint8_t color;
        int16_t width;
        int16_t height;
        uint8_t background;
        uint16_t keyboard;
    } ctx;
    struct {
        int16_t width;
        int16_t height;
        uint16_t keyboard;
    } old;
} app_t;

typedef void (*cmd_t)(app_t *const, int16_t, int16_t, int16_t, int16_t);

void engine_init(app_t *const, int argc, char *argv[]);
void engine_update(app_t *const);

bool geoclip_line(app_t *const, int16_t *const, int16_t *const, int16_t *const, int16_t *const);
bool geoclip_rect(app_t *const, int16_t *const, int16_t *const, int16_t *const, int16_t *const);
bool geoclip_text(app_t *const, int16_t *const, int16_t *const, int16_t *const, int16_t *const);
bool geoclip_text2(app_t *const, int16_t *const, int16_t *const, int16_t *const, int16_t *const);

bool tui_update(app_t *const);
void tui_delete(app_t *const);

void tui_draw_mode(app_t *const, int16_t, int16_t, int16_t, int16_t);
void tui_draw_color(app_t *const, int16_t, int16_t, int16_t, int16_t);
void tui_draw_rect(app_t *const, int16_t, int16_t, int16_t, int16_t);
void tui_draw_line(app_t *const, int16_t, int16_t, int16_t, int16_t);
void tui_draw_text(app_t *const, int16_t, int16_t, int16_t, int16_t);

const char *const tui_queue_get_text(uint8_t);
uint8_t tui_queue_push_text(const char*);
void tui_queue_push(uint8_t, int16_t, int16_t, int16_t, int16_t);
void tui_queue_burn(app_t *const);
void tui_termios_start();
void tui_termios_exit();

void native_text_install(lua_State*);
void native_draw_install(lua_State*);
void native_http_install(lua_State*);
void native_json_install(lua_State*);
int native_keys_update(lua_State*, int);
int native_draw_update(lua_State*, int);
int native_draw_resize(lua_State*, int, int16_t, int16_t);
int native_loop_update(lua_State*, int, uint8_t);
