#include "zeebo.h"
#include "engine/bytecode.h"
#include "game/bytecode.h"

#ifndef FPS_DELTA
#define FPS_DELTA (16u)
#endif

typedef struct {
    char* file;
    char* buf;
    size_t len;
} script_t;

static int native_callback_loop;
static int native_callback_draw;
static int native_callback_resize;
static int native_callback_keyboard;

static bool engine_load_file(char *const file_name, char **file_buf, size_t *file_len)
{
    bool success = false;
    FILE *file_ptr = NULL;

    do {
        if (!file_name) {
            break;
        }

        file_ptr = fopen(file_name, "r");
        if (!file_ptr) {
            break;
        }

        do {
            *file_buf = realloc(*file_buf, *file_len + 4096);
            *file_len += fread(*file_buf + (*file_len), 1, 4096, file_ptr);
        } while (!feof(file_ptr));
        success = true;
    }
    while (0);

    if (!success) {
        *file_len = 0;   
    }

    if (file_ptr) {
        fclose(file_ptr);
    }

    return success;
}

void engine_init(app_t *const self, int argc, char *argv[])
{
    static script_t engine;
    static script_t game;
    int opt;

    self->cfg.debounce = 2;
    while ((opt = getopt(argc, argv, "d:e:g:")) != -1) {
        switch (opt) {
            case 'd': self->cfg.debounce = atoi(optarg); break;
            case 'e': engine.file = optarg; break;
            case 'g': game.file = optarg; break;
        }
    }

    do {
        if (engine.file && !engine_load_file(engine.file, &engine.buf, &engine.len)) {
            concat(self, err, "error: cannot open engine file!\n%s\n", engine.file);
            break;
        }

        if (game.file && !engine_load_file(game.file, &game.buf, &game.len)) {
            concat(self, err, "error: cannot open game file!\n%s\n", game.file);
            break;
        }

        if (game_bytecode_lua_len == 0 && game.len == 0 && !engine_load_file("game.lua", &game.buf, &game.len)) {
            concat(self, err, "error: copy your game.lua near the executable folder!\n");
            break;
        }

        if (!engine.file) {
            engine.buf = (char*) engine_bytecode_lua;
            engine.len = (size_t) engine_bytecode_lua_len;
        }

        if (!game.file) {
            game.buf = (char*) game_bytecode_lua;
            game.len = (size_t) game_bytecode_lua_len;
        }

        self->L = luaL_newstate();

        luaL_openlibs(self->L);
        native_draw_install(self->L);
        native_text_install(self->L);
        native_http_install(self->L);
        native_json_install(self->L);

        luaL_loadbuffer(self->L, engine.buf, engine.len, "engine");
        if (lua_pcall(self->L, 0, 0, 0) != LUA_OK) {
            concat(self, err, "error: engine cannot be loadded!\n%s\n%s\n",
                game.file? game.file: "(embed)",
                lua_tostring(self->L, -1));
            break;
        }

        lua_getglobal(self->L, "native_callback_init");
        lua_pushnumber(self->L, self->ctx.width);
        lua_pushnumber(self->L, self->ctx.height);

        luaL_loadbuffer(self->L, game.buf, game.len, "game");
        if (lua_pcall(self->L, 0, 1, 0) != LUA_OK) {
            concat(self, err, "error: game file has a syntax error!\n%s\n", game.file? game.file: "(embed)");
            break;
        }
        if(lua_pcall(self->L, 3, 0, 0) != LUA_OK) {
            concat(self, err, "error: native_callback_init\n%s\n", lua_tostring(self->L, -1));
            break;
        }

        lua_getglobal(self->L, "native_callback_loop");
        native_callback_loop = luaL_ref(self->L, LUA_REGISTRYINDEX);

        lua_getglobal(self->L, "native_callback_draw");
        native_callback_draw = luaL_ref(self->L, LUA_REGISTRYINDEX);

        lua_getglobal(self->L, "native_callback_resize");
        native_callback_resize = luaL_ref(self->L, LUA_REGISTRYINDEX);
        
        lua_getglobal(self->L, "native_callback_keyboard");
        native_callback_keyboard = luaL_ref(self->L, LUA_REGISTRYINDEX);
    }
    while(0);

    if (engine.buf && (char*) engine.buf != (char*) engine_bytecode_lua) {
        free(engine.buf);
    }

    if (game.buf && (char*) game.buf != (char*) game_bytecode_lua) {
        free(game.buf);
    }
}

void engine_update(app_t *const self)
{
    self->out.len = 0;
    concat(self, out, "\x1B[3J\x1B[H\x1B[2J");
    if(native_keys_update(self->L, native_callback_keyboard, self->cfg.debounce) != LUA_OK) {
        concat(self, err, "error: native_callback_keyboard\n%s\n", lua_tostring(self->L, -1));
    }
    if (self->ctx.width != self->old.width || self->ctx.height != self->old.height) {
        if(native_draw_resize(self->L, native_callback_resize, self->ctx.width, self->ctx.height) != LUA_OK) {
            concat(self, err, "error: native_callback_resize\n%s\n", lua_tostring(self->L, -1));
        }
    }
    if(native_loop_update(self->L, native_callback_loop, FPS_DELTA) != LUA_OK) {
        concat(self, err, "error: native_callback_loop\n%s\n", lua_tostring(self->L, -1));
    }
    if(native_draw_update(self->L, native_callback_draw) != LUA_OK) {
        concat(self, err, "error: native_callback_draw\n%s\n", lua_tostring(self->L, -1));
    }
    tui_queue_burn(self);
    concat(self, out, "\x1b[0;0H");
}
