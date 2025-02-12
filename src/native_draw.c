#include "zeebo.h"

static int native_draw_start(lua_State *L)
{
    return 0;
}

static int native_draw_flush(lua_State *L)
{
    return 0;
}

static int native_draw_clear(lua_State *L)
{
    //color_t t = { .pixel2 = luaL_checkinteger(L, 1) };
    //draw_queue_push(48, 3, 0, 1, 0);
    //draw_queue_push(49, t.c32.color.r, t.c32.color.g, t.c32.color.b, 0);
    lua_settop(L, 0);
    return 0;
}

static int native_draw_color(lua_State *L)
{
    //color_t t = { .pixel2 = luaL_checkinteger(L, 1) };
    // draw_queue_push(48, 0, 0, 1, 0);
    // draw_queue_push(49, t.c32.color.r, t.c32.color.g, t.c32.color.b, 0);
    lua_settop(L, 0);
    return 0;
}

static int native_draw_rect(lua_State *L)
{
    uint8_t mode = luaL_checkinteger(L, 1);
    int16_t x = (int16_t) luaL_checknumber(L, 2);
    int16_t y = (int16_t) luaL_checknumber(L, 3);
    int16_t w = (int16_t) luaL_checknumber(L, 4);
    int16_t h = (int16_t) luaL_checknumber(L, 5);

    tui_queue_push(48, mode, 0, 1, 0);
    tui_queue_push(50, x, y, w, h);

    lua_settop(L, 0);
    return 0;
}

static int native_draw_line(lua_State *L)
{
    int16_t x1 = luaL_checknumber(L, 1);
    int16_t y1 = luaL_checknumber(L, 2);
    int16_t x2 = luaL_checknumber(L, 3);
    int16_t y2 = luaL_checknumber(L, 4);

    tui_queue_push(51, x1, y1, x2, y2);

    lua_settop(L, 0);
    return 0;
}


static int native_draw_image(lua_State *L) {
    uint8_t x = luaL_checknumber(L, 2);
    uint8_t y = luaL_checknumber(L, 3);
    // draw_queue_push(48, 0, 0, 1, 0);
    // draw_queue_push(49, 0XFF, 0xFF, 0xFF, 0);
    // draw_queue_push(50, x, y, 1, 1);
    lua_settop(L, 0);
    return 0;
}

int native_draw_update(lua_State* L, int callback)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    return lua_pcall(L, 0, 0, 0);
}

int native_draw_resize(lua_State* L, int callback, int16_t width, int16_t height)
{
    lua_rawgeti(L, LUA_REGISTRYINDEX, callback);
    lua_pushnumber(L, width);
    lua_pushnumber(L, height);
    return lua_pcall(L, 2, 0, 0);
}

void native_draw_install(lua_State* L)
{
    int i = 0;
    static const luaL_Reg lib[] = {
        {"native_draw_start", native_draw_start},
        {"native_draw_flush", native_draw_flush},
        {"native_draw_clear", native_draw_clear},
        {"native_draw_color", native_draw_color},
        {"native_draw_rect", native_draw_rect},
        {"native_draw_line", native_draw_line},
        {"native_draw_image", native_draw_image}
    };

    while(i < sizeof(lib)/sizeof(luaL_Reg)) {
        lua_pushcfunction(L, lib[i].func);
        lua_setglobal(L, lib[i].name);
        i = i + 1;
    }
}
