#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "xsdk/gpu.h"
#include "xsdk/kernel.h"
#include "xsdk/log.h"
#include "xsdk/present.h"
#include "xsdk/splash.h"
#include "xsdk/text.h"

enum {
    SPLASH_MS = 1200,
    BUFFERS = 2,
    SWAP_INTERVAL = 1,
    BEGIN_TIMEOUT_MS = 5000,
    CATEGORY_COUNT = 6,
    ITEMS_PER_PAGE = 4
};

typedef struct {
    const char *name;
    const char *subtitle;
    const char *xex_path;
} Category;

static const Category categories[CATEGORY_COUNT] = {
    { "ARCADE",     "Arcade / MAME-compatible frontend", "game:\\Emulators\\Arcade\\default.xex" },
    { "NINTENDO",   "Nintendo 8-bit emulator",            "game:\\Emulators\\NES\\default.xex" },
    { "SUPER NES",  "Super Nintendo emulator",            "game:\\Emulators\\SNES\\default.xex" },
    { "MEGA DRIVE", "Sega Mega Drive emulator",           "game:\\Emulators\\MegaDrive\\default.xex" },
    { "GAME BOY",   "Game Boy / Game Boy Color emulator", "game:\\Emulators\\GameBoy\\default.xex" },
    { "ATARI",      "Atari 2600 emulator",                "game:\\Emulators\\Atari2600\\default.xex" }
};

static unsigned selected_category = 0;
static bool show_help = false;
static char status_line[96] = "A: abrir emulador   Y: ajuda   BACK: sair";

static bool pressed(uint16_t now, uint16_t before, uint16_t mask)
{
    return (now & mask) && !(before & mask);
}

static void draw_text(int x, int y, uint32_t scale, uint32_t color, const char *s)
{
    xsdk_text_draw(x, y, scale, color, s);
}

static void draw_menu(void)
{
    const Category *cat = &categories[selected_category];

    /* Text is drawn into the current GPU frame before it is resolved/presented. */
    draw_text(48, 34, 3, 0xFFFF4040, "RETRO GAMES");
    draw_text(49, 103, 1, 0xFFB8C4D8, "XBOX 360 EDITION  |  HOMEbrew GAME LIBRARY");

    draw_text(48, 166, 2, 0xFFFFFFFF, "PLATAFORMAS");
    for (unsigned i = 0; i < CATEGORY_COUNT; ++i) {
        const int y = 218 + (int)i * 48;
        const uint32_t color = (i == selected_category) ? 0xFFFF5555 : 0xFFB8C4D8;
        char line[48];
        snprintf(line, sizeof line, "%c  %s", (i == selected_category) ? '>' : ' ', categories[i].name);
        draw_text(64, y, 2, color, line);
    }

    draw_text(650, 190, 2, 0xFF70D8FF, cat->name);
    draw_text(650, 238, 1, 0xFFFFFFFF, cat->subtitle);
    draw_text(650, 300, 2, 0xFFFFFFFF, "EMULADOR");
    draw_text(650, 346, 1, 0xFFB8C4D8, cat->xex_path);
    draw_text(650, 402, 1, 0xFFFFC060, "Coloque o emulador XEX nesse caminho.");
    draw_text(650, 432, 1, 0xFFFFC060, "O launcher nao inclui ROMs nem emuladores.");

    if (show_help) {
        draw_text(48, 560, 1, 0xFFFFFFFF, "D-PAD / LB / RB: escolher plataforma");
        draw_text(48, 586, 1, 0xFFFFFFFF, "A: iniciar o emulador configurado");
        draw_text(48, 612, 1, 0xFFFFFFFF, "B ou BACK: voltar / sair");
    } else {
        draw_text(48, 630, 1, 0xFFB8C4D8, status_line);
    }

    draw_text(48, 680, 1, 0xFF778399, "RETRO GAMES LAUNCHER  |  BUILD 0.1");
}

static void run(uint32_t width, uint32_t height)
{
    (void)width;
    (void)height;
    uint16_t previous_buttons = 0;

    for (;;) {
        INPUT_STATE input = { 0 };
        if (XamInputGetState(0, 0, &input) != ERROR_SUCCESS)
            input = (INPUT_STATE){ 0 };

        const uint16_t buttons = input.gamepad.buttons;

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_BACK))
            break;

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_DPAD_UP) ||
            pressed(buttons, previous_buttons, INPUT_GAMEPAD_DPAD_LEFT) ||
            pressed(buttons, previous_buttons, INPUT_GAMEPAD_LEFT_SHOULDER)) {
            selected_category = (selected_category + CATEGORY_COUNT - 1) % CATEGORY_COUNT;
            show_help = false;
        }

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_DPAD_DOWN) ||
            pressed(buttons, previous_buttons, INPUT_GAMEPAD_DPAD_RIGHT) ||
            pressed(buttons, previous_buttons, INPUT_GAMEPAD_RIGHT_SHOULDER)) {
            selected_category = (selected_category + 1) % CATEGORY_COUNT;
            show_help = false;
        }

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_Y)) {
            show_help = !show_help;
        }

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_B)) {
            show_help = false;
            strcpy(status_line, "Use BACK para sair do launcher.");
        }

        if (pressed(buttons, previous_buttons, INPUT_GAMEPAD_A)) {
            const char *path = categories[selected_category].xex_path;
            xsdk_log_write("launcher: requested emulator %s", path);
            strcpy(status_line, "Abrindo emulador configurado...");
            /* The XAM loader starts another XEX and ends this launcher.
               The target file must exist at the configured game: path. */
            XamLoaderLaunchTitle(path, 0);
            strcpy(status_line, "Nao foi possivel iniciar. Confira o caminho do XEX.");
        }

        previous_buttons = buttons;

        uint32_t address = 0;
        const NTSTATUS begin = xsdk_present_begin(&address, BEGIN_TIMEOUT_MS);
        if (!NT_SUCCESS(begin)) {
            xsdk_log_write("launcher: xsdk_present_begin failed: 0x%08X", (unsigned)begin);
            break;
        }

        draw_menu();
        xsdk_gpu_resolve(address, 1280, 720, 0xFF101722);
        xsdk_present_end();
    }
}

int main(void)
{
    xsdk_log_write("Retro Games Launcher 0.1 starting");
    const NTSTATUS splash = xsdk_splash_show(SPLASH_MS);
    if (!NT_SUCCESS(splash))
        xsdk_log_write("launcher: splash failed: 0x%08X", (unsigned)splash);

    NTSTATUS status = xsdk_gpu_open();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("launcher: xsdk_gpu_open failed: 0x%08X", (unsigned)status);
        return 1;
    }

    status = xsdk_text_open();
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("launcher: xsdk_text_open failed: 0x%08X", (unsigned)status);
        xsdk_gpu_close();
        return 1;
    }

    uint32_t width = 0, height = 0;
    status = xsdk_present_open(XSDK_DISPLAY_720P, BUFFERS, SWAP_INTERVAL, &width, &height);
    if (!NT_SUCCESS(status)) {
        xsdk_log_write("launcher: xsdk_present_open failed: 0x%08X", (unsigned)status);
        xsdk_text_close();
        xsdk_gpu_close();
        return 1;
    }

    xsdk_log_write("launcher: display %ux%u", (unsigned)width, (unsigned)height);
    run(width, height);

    xsdk_present_close();
    xsdk_text_close();
    status = xsdk_gpu_close();
    if (!NT_SUCCESS(status))
        xsdk_log_write("launcher: xsdk_gpu_close failed: 0x%08X", (unsigned)status);

    xsdk_log_write("Retro Games Launcher ended");
    return 0;
}
