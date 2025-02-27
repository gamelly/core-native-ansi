#include "zeebo.h"

enum {
    KEY_MENU,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_COUNT,
};

static const char *const key_names[] = { "menu", "a", "b", "c", "d", "up", "down", "left", "right" };

static const struct {
    const int key;
    const uint8_t id;
} key_bindings[] = {
    { 0x7f, KEY_MENU }, // clang-format off
    { 0xa, KEY_A }, 
    { 'z', KEY_A },  
    { 'x', KEY_B },
    { 'c', KEY_C },
    { 'v', KEY_D },
    { 'w', KEY_UP },
    { 'a', KEY_LEFT },
    { 's', KEY_DOWN },
    { 'd', KEY_RIGHT }// clang-format on 
};

static const struct {
    const int key;
    const uint8_t id;
} esc_key_bindings[] = {
    { 'A', KEY_UP }, // clang-format off 
    { 'D', KEY_LEFT }, 
    { 'B', KEY_DOWN },
    { 'C', KEY_RIGHT } // clang-format on
};

int
native_keys_update(lua_State *L, int callback, uint8_t debounce) {
    static bool old_pressed[KEY_COUNT];
    static bool pressed[KEY_COUNT];
    static uint8_t old_count;
    static uint8_t count;
    static uint8_t skip;
    int c;

    count = 0;
    while ((c = getchar()) != EOF) {
        uint8_t index = 0;
        if (c == 0x1b && (c = getchar()) == '[') {
            c = getchar();
            while (index < sizeof(esc_key_bindings) / sizeof(*esc_key_bindings)) {
                if (esc_key_bindings[index].key == c) {
                    pressed[esc_key_bindings[index].id] = true;
                    count++;
                }
                index++;
            }
        } else {
            while (index < sizeof(key_bindings) / sizeof(*key_bindings)) {
                if (key_bindings[index].key == c) {
                    pressed[key_bindings[index].id] = true;
                    count++;
                }
                index++;
            }
        }
    }

    if (count > 0 || skip >= debounce) {
        uint8_t index = 0;
        while (index < KEY_COUNT) {
            if (pressed[index] ^ old_pressed[index]) {
                lua_getglobal(L, "native_callback_keyboard");
                lua_pushstring(L, key_names[index]);
                lua_pushinteger(L, pressed[index]);
                lua_pcall(L, 2, 0, 0);
            }
            index++;
        }
        memcpy(old_pressed, pressed, KEY_COUNT);
        memset(pressed, 0, KEY_COUNT);
        old_count = count;
        count = 0;
        skip = 0;
    }

    if (count == 0 && old_count > 0) {
        skip++;
    }

    return LUA_OK;
}
