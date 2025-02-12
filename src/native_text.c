#include "zeebo.h"

static int8_t previous_font_size = 2;
static int8_t font_size = 2;

static int native_text_mock(lua_State *L)
{
    lua_settop(L, 0);
    return 0;
}

static int native_text_print(lua_State *L)
{
    int16_t x = ceil(luaL_checknumber(L, 1));
    int16_t y = ceil(luaL_checknumber(L, 2));
    int16_t text_index = tui_queue_push_text(luaL_checkstring(L, 3));
    tui_queue_push(52, x, y, text_index, font_size);
    lua_settop(L, 0);
    return 0;
}

static int native_text_mensure(lua_State *L)
{
    uint8_t s = font_size <= 2? 1: font_size;
    const char *text = luaL_checkstring(L, 1);
    lua_settop(L, 0);
    lua_pushnumber(L, strlen(text) * s);
    lua_pushnumber(L, s);
    return 2;
}

static int native_text_font_size(lua_State *L)
{
    previous_font_size = font_size;
    font_size = ceil(luaL_checknumber(L, 1));
    font_size = MAX(1, font_size);
    lua_settop(L, 0);
    return 0;
}

static int native_text_font_previous(lua_State *L)
{
    font_size = previous_font_size;
    lua_settop(L, 0);
    return 0;
}

void native_text_install(lua_State* L)
{
    int i = 0;
    static const luaL_Reg lib[] = {
        {"native_text_print", native_text_print},
        {"native_text_mensure", native_text_mensure},
        {"native_text_font_size", native_text_font_size},
        {"native_text_font_name", native_text_mock},
        {"native_text_font_default", native_text_mock},
        {"native_text_font_previous", native_text_font_previous},
    };

    while(i < sizeof(lib)/sizeof(luaL_Reg)) {
        lua_pushcfunction(L, lib[i].func);
        lua_setglobal(L, lib[i].name);
        i = i + 1;
    }
}
