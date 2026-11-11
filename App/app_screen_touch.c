/**
 * @file app_screen_touch.c
 * @brief Polaris OS - File Browser & Auto Slideshow with Multiple Cinematic Transitions
 */
#include "main.h"
#include "app_screen_touch.h"
#include "app_sw_jpeg.h"
#include "drv_gt9147.h"
#include "drv_snor_flash.h"
#include "drv_sdhc.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_font.h"

#define ENABLE_APP_DEBUG 1
#if ENABLE_APP_DEBUG
    #define APP_PRINTF(...) printf(__VA_ARGS__)
#else
    #define APP_PRINTF(...)
#endif

#define UI_FONT_SCALE   2
#define LCD_DIR_MODE    0

#define FB_BASE_ADDR        0xC0000000UL /* Front Buffer (Displayed LCD) */
#define BACK_BUFFER_ADDR    0xC0100000UL /* Back Buffer (1MB Offset) */

#define PHYSICAL_W      800
#define PHYSICAL_H      480
#define LOGICAL_W       480
#define LOGICAL_H       800

#define COLOR_RED       0xF800
#define COLOR_GREEN     0x07E0
#define COLOR_BLUE      0x001F
#define COLOR_BLACK     0x0000
#define COLOR_WHITE     0xFFFF
#define COLOR_YELLOW    0xFFE0
#define COLOR_ORANGE    0xFD20
#define COLOR_GRAY      0x7BEF
#define COLOR_CYAN      0x07FF
#define COLOR_MAGENTA   0xF81F

extern osSemaphoreId_t SdramReadySemHandle;
extern SPI_HandleTypeDef hspi2;

static FATFS fs __attribute__((aligned(32)));
//static FIL my_file __attribute__((aligned(32)));
//static uint8_t f_work_area[8192] __attribute__((aligned(32)));
//static uint16_t file_rw_buf[LOGICAL_W] __attribute__((aligned(32)));

static uint8_t hw_nor_flash_ok = 0;
static uint8_t hw_sdhc_ok = 0;
static char active_drive[4] = "0:/";
static char current_path[128] = "0:/";

#define MAX_FILES_ON_SCREEN 20
typedef struct {
    char name[64];
    uint8_t is_dir;
    uint16_t y_min;
    uint16_t y_max;
} file_node_t;

static file_node_t current_file_list[MAX_FILES_ON_SCREEN];
static uint8_t current_file_count = 0;

static uint8_t is_jpeg_file(const char *filename) {
    char *ext = strrchr(filename, '.');
    if (ext != NULL) {
        if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".JPG") == 0 ||
            strcmp(ext, ".jpeg") == 0 || strcmp(ext, ".JPEG") == 0) {
            return 1;
        }
    }
    return 0;
}

/* ==================================================================== */
/* Hardware & Screen Core APIs                                          */
/* ==================================================================== */
static void sys_flash_unlock(void) {
    uint8_t wren = 0x06, unlock = 0x98;
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, &wren, 1, 100);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    osDelay(5);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&hspi2, &unlock, 1, 100);
    HAL_GPIO_WritePin(GPIOF, GPIO_PIN_10, GPIO_PIN_SET);
    osDelay(20);
}

static void sys_probe_hardware(void) {
    uint32_t flash_id = 0;
    sys_flash_unlock();
    if (drv_snor_read_id(&flash_id) == SNOR_OK && (flash_id == 0xEF4019 || flash_id == 0xEF7019)) hw_nor_flash_ok = 1;
    else hw_nor_flash_ok = 0;
    if (drv_sdhc_init() == 0) hw_sdhc_ok = 1;
    else hw_sdhc_ok = 0;
}

static void ui_put_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= LOGICAL_W || y >= LOGICAL_H) return;
    uint16_t px = y;
    uint16_t py = (PHYSICAL_H - 1) - x;
    ((uint16_t *)FB_BASE_ADDR)[py * PHYSICAL_W + px] = color;
}

static void app_ui_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    for (uint16_t i = y; i < y + h; i++)
        for (uint16_t j = x; j < x + w; j++)
            ui_put_pixel(j, i, color);
    SCB_CleanDCache_by_Addr((uint32_t *)FB_BASE_ADDR, PHYSICAL_W * PHYSICAL_H * 2);
}

static void app_ui_draw_text(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg) {
    uint16_t cur_x = x;
    uint8_t scale = UI_FONT_SCALE;
    while (*str) {
        if (*str == '\n') { y += (16 * scale); cur_x = x; str++; continue; }
        uint8_t font_idx = (*str < ' ' || *str > '~') ? 0 : (*str - ' ');
        for (uint8_t r = 0; r < 16; r++) {
            uint8_t p_row = font_8x16[font_idx][r];
            for (uint8_t c = 0; c < 8; c++) {
                uint16_t color = (p_row & (0x80 >> c)) ? fg : bg;
                for(uint8_t dy=0; dy<scale; dy++)
                    for(uint8_t dx=0; dx<scale; dx++)
                        ui_put_pixel(cur_x + (c*scale) + dx, y + (r*scale) + dy, color);
            }
        }
        cur_x += (8 * scale);
        if (cur_x + (8 * scale) > LOGICAL_W) { cur_x = x; y += (16 * scale); }
        str++;
    }
    SCB_CleanDCache_by_Addr((uint32_t *)FB_BASE_ADDR, PHYSICAL_W * PHYSICAL_H * 2);
}

static void app_ui_draw_back_button(void) {
    app_ui_fill_rect(370, 10, 100, 40, COLOR_GRAY);
    app_ui_draw_text(395, 20, "BACK", COLOR_WHITE, COLOR_GRAY);
}

/* ==================================================================== */
/* 🌟 Advanced Cinematic Transition Engine                              */
/* ==================================================================== */

typedef enum {
    TRANSITION_BLINDS = 0,
    TRANSITION_WIPE_DOWN,
    TRANSITION_WIPE_RIGHT,
    TRANSITION_INTERLACE,
    TRANSITION_MAX_EFFECTS
} transition_effect_t;

/**
 * @brief Executes one of multiple visual transition effects.
 */
static void app_ui_transition_execute(uint32_t front_addr, uint32_t back_addr, transition_effect_t effect) {
    uint16_t *src = (uint16_t *)back_addr;
    uint16_t *dst = (uint16_t *)front_addr;

    switch (effect) {
        /* Effect 1: 百葉窗 (Classic Blinds) */
        case TRANSITION_BLINDS: {
            uint16_t strip_h = PHYSICAL_H / 10;
            for (uint16_t i = 0; i < strip_h; i++) {
                for (uint8_t s = 0; s < 10; s++) {
                    uint16_t y = s * strip_h + i;
                    memcpy(&dst[y * PHYSICAL_W], &src[y * PHYSICAL_W], PHYSICAL_W * 2);
                    SCB_CleanDCache_by_Addr((uint32_t *)(front_addr + (y * PHYSICAL_W * 2)), PHYSICAL_W * 2);
                }
                osDelay(15);
            }
            break;
        }

        /* Effect 2: 向下擦除 (Wipe Down) */
        case TRANSITION_WIPE_DOWN: {
            uint16_t step = 20;
            for (uint16_t y = 0; y < PHYSICAL_H; y += step) {
                memcpy(&dst[y * PHYSICAL_W], &src[y * PHYSICAL_W], step * PHYSICAL_W * 2);
                SCB_CleanDCache_by_Addr((uint32_t *)(front_addr + (y * PHYSICAL_W * 2)), step * PHYSICAL_W * 2);
                osDelay(25);
            }
            break;
        }

        /* Effect 3: 向右擦除 (Wipe Right) */
        case TRANSITION_WIPE_RIGHT: {
            uint16_t step = 20;
            for (uint16_t x = 0; x < PHYSICAL_W; x += step) {
                for (uint16_t y = 0; y < PHYSICAL_H; y++) {
                    memcpy(&dst[y * PHYSICAL_W + x], &src[y * PHYSICAL_W + x], step * 2);
                }
                /* Clean the whole screen cache since we wrote vertically */
                SCB_CleanDCache_by_Addr((uint32_t *)front_addr, PHYSICAL_W * PHYSICAL_H * 2);
                osDelay(15);
            }
            break;
        }

        /* Effect 4: 交錯浮現 (Interlace Fade) */
        case TRANSITION_INTERLACE: {
            uint16_t passes = 8;
            for (uint16_t pass = 0; pass < passes; pass++) {
                for (uint16_t y = pass; y < PHYSICAL_H; y += passes) {
                    memcpy(&dst[y * PHYSICAL_W], &src[y * PHYSICAL_W], PHYSICAL_W * 2);
                    SCB_CleanDCache_by_Addr((uint32_t *)(front_addr + (y * PHYSICAL_W * 2)), PHYSICAL_W * 2);
                }
                osDelay(70);
            }
            break;
        }

        default: break;
    }
}

/* ==================================================================== */
/* UI Screens                                                           */
/* ==================================================================== */
static void app_ui_draw_drive_selection(void) {
    app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
    app_ui_draw_text(20, 40, "Select Target Drive", COLOR_YELLOW, COLOR_BLACK);
    uint16_t btn_color_nor = hw_nor_flash_ok ? COLOR_BLUE : COLOR_GRAY;
    app_ui_fill_rect(80, 200, 320, 100, btn_color_nor);
    app_ui_draw_text(110, 230, "SPI NOR [1:/]", COLOR_WHITE, btn_color_nor);
    uint16_t btn_color_sd = hw_sdhc_ok ? COLOR_GREEN : COLOR_GRAY;
    app_ui_fill_rect(80, 400, 320, 100, btn_color_sd);
    app_ui_draw_text(140, 430, "SDHC [0:/]", COLOR_BLACK, btn_color_sd);
}

static void app_ui_draw_main_menu(void) {
    char title[32];
    snprintf(title, 32, "Polaris OS (%s)", active_drive);
    app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
    app_ui_draw_text(20, 40, title, COLOR_GREEN, COLOR_BLACK);
    app_ui_fill_rect(80, 150, 320, 70, COLOR_RED);
    app_ui_draw_text(150, 175, "BROWSE", COLOR_WHITE, COLOR_RED);
    app_ui_fill_rect(80, 250, 320, 70, COLOR_GREEN);
    app_ui_draw_text(130, 275, "IMG TEST", COLOR_WHITE, COLOR_GREEN);
    app_ui_fill_rect(80, 350, 320, 70, COLOR_BLUE);
    app_ui_draw_text(150, 375, "FORMAT", COLOR_WHITE, COLOR_BLUE);
    app_ui_fill_rect(80, 450, 320, 70, COLOR_CYAN);
    app_ui_draw_text(130, 475, "DRIVE INFO", COLOR_BLACK, COLOR_CYAN);
    app_ui_fill_rect(80, 550, 320, 70, COLOR_ORANGE);
    app_ui_draw_text(110, 575, "CHANGE DRIVE", COLOR_BLACK, COLOR_ORANGE);

    app_ui_fill_rect(80, 650, 320, 70, COLOR_MAGENTA);
    app_ui_draw_text(170, 675, "PLAY", COLOR_WHITE, COLOR_MAGENTA);
}

static void app_ui_show_directory(void) {
    DIR d; FILINFO f;
    app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
    app_ui_draw_text(10, 10, current_path, COLOR_YELLOW, COLOR_BLACK);
    app_ui_draw_text(10, 45, "(Tap folder to enter, .jpg to view)", COLOR_GRAY, COLOR_BLACK);
    app_ui_draw_back_button();
    current_file_count = 0;
    char dir_path_to_open[128];
    snprintf(dir_path_to_open, sizeof(dir_path_to_open), "%s", current_path);
    int len = strlen(dir_path_to_open);
    if (len > 3 && dir_path_to_open[len - 1] == '/') dir_path_to_open[len - 1] = '\0';

    if (f_opendir(&d, dir_path_to_open) == FR_OK) {
        uint16_t y_pos = 80;
        uint16_t step_h = (16 * UI_FONT_SCALE) + 10;
        for (;;) {
            if (f_readdir(&d, &f) != FR_OK || f.fname[0] == 0) break;
            if (f.fname[0] == '.') continue;
            char disp_name[300];
            if (f.fattrib & AM_DIR) {
                snprintf(disp_name, sizeof(disp_name), "[DIR] %s", f.fname);
                app_ui_draw_text(10, y_pos, disp_name, COLOR_CYAN, COLOR_BLACK);
            } else {
                snprintf(disp_name, sizeof(disp_name), "      %s", f.fname);
                app_ui_draw_text(10, y_pos, disp_name, COLOR_WHITE, COLOR_BLACK);
            }
            if (current_file_count < MAX_FILES_ON_SCREEN) {
                strncpy(current_file_list[current_file_count].name, f.fname, 63);
                current_file_list[current_file_count].is_dir = (f.fattrib & AM_DIR) ? 1 : 0;
                current_file_list[current_file_count].y_min = y_pos;
                current_file_list[current_file_count].y_max = y_pos + step_h - 2;
                current_file_count++;
            }
            y_pos += step_h;
            if(y_pos > LOGICAL_H - 40) {
                app_ui_draw_text(10, y_pos, "...(More)", COLOR_GRAY, COLOR_BLACK);
                break;
            }
        }
        f_closedir(&d);
    }
}

/* ==================================================================== */
/* Main Application Task                                                */
/* ==================================================================== */
void vTask_SN_Touch(void *pvParameters) {
    drv_gt9147_point_t p;
    ui_state_t state = UI_STATE_SELECT_DRIVE;
    FRESULT res;
    uint8_t touch_was_pressed = 0;
    uint16_t last_touch_x = 0;
    uint16_t last_touch_y = 0;

    /* Slideshow Engine Variables */
    static DIR slide_dir;
    static FILINFO slide_fno;
    static uint32_t last_slide_time = 0;
    static uint8_t force_next_slide = 0;

    /* 🌟 Cycle variable for dynamic transitions */
    static uint8_t current_transition_effect = 0;

    osDelay(200);
    if (SdramReadySemHandle) osSemaphoreAcquire(SdramReadySemHandle, osWaitForever);
    app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
    app_ui_draw_text(100, 300, "System Booting...", COLOR_WHITE, COLOR_BLACK);
    sys_probe_hardware();
    state = UI_STATE_SELECT_DRIVE;
    app_ui_draw_drive_selection();

    for(;;) {
        osDelay(20);

        if (drv_gt9147_read_touch(&p) == 0) {
            if (p.is_pressed) {
                if (p.x <= 1000 && p.y <= 1000) {
                    int16_t cal_x = (LOGICAL_W - 1) - p.y;
                    int16_t cal_y = p.x;
                    cal_x += 35; cal_y -= 50;
                    if (cal_x < 0) { cal_x = 0; }
					if (cal_x >= LOGICAL_W) { cal_x = LOGICAL_W - 1; }

					if (cal_y < 0) { cal_y = 0; }
					if (cal_y >= LOGICAL_H) { cal_y = LOGICAL_H - 1; }
                    last_touch_x = (uint16_t)cal_x;
                    last_touch_y = (uint16_t)cal_y;
                    if (!touch_was_pressed) touch_was_pressed = 1;
                }
                continue;
            }
            else if (!p.is_pressed && touch_was_pressed) {
                touch_was_pressed = 0;
                uint16_t touch_x = last_touch_x;
                uint16_t touch_y = last_touch_y;

                switch (state) {
                    case UI_STATE_SELECT_DRIVE:
                        if (touch_x >= 80 && touch_x <= 400 && touch_y >= 190 && touch_y <= 310 && hw_nor_flash_ok) {
                            snprintf(active_drive, 4, "1:/"); state = UI_STATE_INIT;
                        } else if (touch_x >= 80 && touch_x <= 400 && touch_y >= 390 && touch_y <= 510 && hw_sdhc_ok) {
                            snprintf(active_drive, 4, "0:/"); state = UI_STATE_INIT;
                        }
                        break;

                    case UI_STATE_MAIN_MENU:
                        if (touch_x >= 80 && touch_x <= 400) {
                            if (touch_y >= 140 && touch_y <= 230) {
                                snprintf(current_path, sizeof(current_path), "%s", active_drive);
                                app_ui_show_directory(); state = UI_STATE_VIEWING_DIR;
                            }
                            else if (touch_y >= 540 && touch_y <= 630) {
                                f_mount(NULL, active_drive, 0);
                                app_ui_draw_drive_selection(); state = UI_STATE_SELECT_DRIVE;
                            }
                            /* Start Slideshow */
                            else if (touch_y >= 640 && touch_y <= 730) {
                                char path[64];
                                snprintf(path, sizeof(path), "%sPicture", active_drive);
                                if (f_opendir(&slide_dir, path) == FR_OK) {
                                    app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
                                    state = UI_STATE_SLIDESHOW;
                                    force_next_slide = 1;
                                    last_slide_time = HAL_GetTick();
                                    current_transition_effect = 0; /* 每次播放從頭開始循環效果 */
                                } else {
                                    app_ui_draw_text(20, 100, "Error: Create 'Picture' folder!", COLOR_RED, COLOR_BLACK);
                                    osDelay(2000);
                                    app_ui_draw_main_menu();
                                }
                            }
                        }
                        break;

                    case UI_STATE_VIEWING_DIR:
                        if (touch_x >= 350 && touch_x <= 480 && touch_y >= 0 && touch_y <= 60) {
                            if (strlen(current_path) <= 3) {
                                app_ui_draw_main_menu(); state = UI_STATE_MAIN_MENU;
                            } else {
                                int len = strlen(current_path);
                                if (len > 0 && current_path[len-1] == '/') current_path[len-1] = '\0';
                                char *last_slash = strrchr(current_path, '/');
                                if (last_slash != NULL) *(last_slash + 1) = '\0';
                                else snprintf(current_path, sizeof(current_path), "%s", active_drive);
                                app_ui_show_directory();
                            }
                        } else {
                            for (int i = 0; i < current_file_count; i++) {
                                if (touch_y >= current_file_list[i].y_min && touch_y <= current_file_list[i].y_max) {
                                    if (current_file_list[i].is_dir) {
                                        char temp[384];
                                        snprintf(temp, sizeof(temp), "%s%s/", current_path, current_file_list[i].name);
                                        snprintf(current_path, sizeof(current_path), "%s", temp);
                                        app_ui_show_directory();
                                    } else if (is_jpeg_file(current_file_list[i].name)) {
                                        char jpeg_path[256];
                                        snprintf(jpeg_path, sizeof(jpeg_path), "%s%s", current_path, current_file_list[i].name);
                                        app_ui_fill_rect(0, 0, LOGICAL_W, LOGICAL_H, COLOR_BLACK);
                                        APP_SW_JPEG_Draw(jpeg_path, 0, 0);
                                        app_ui_draw_back_button();
                                        state = UI_STATE_VIEWING_IMAGE;
                                    }
                                    break;
                                }
                            }
                        }
                        break;

                    case UI_STATE_VIEWING_IMAGE:
                        if (touch_x >= 350 && touch_x <= 480 && touch_y >= 0 && touch_y <= 60) {
                            app_ui_show_directory(); state = UI_STATE_VIEWING_DIR;
                        }
                        break;

                    /* 點擊螢幕任何地方退出輪播 */
                    case UI_STATE_SLIDESHOW:
                        f_closedir(&slide_dir);
                        app_ui_draw_main_menu();
                        state = UI_STATE_MAIN_MENU;
                        break;

                    default: break;
                }
            }
        }

        /* 🌟 Advanced Slideshow Background Processing 🌟 */
        if (state == UI_STATE_SLIDESHOW) {
            if (force_next_slide || (HAL_GetTick() - last_slide_time > 3000)) {
                force_next_slide = 0;
                FRESULT res = f_readdir(&slide_dir, &slide_fno);

                if (res != FR_OK || slide_fno.fname[0] == 0) {
                    char path[64];
                    snprintf(path, sizeof(path), "%sPicture", active_drive);
                    f_closedir(&slide_dir);
                    f_opendir(&slide_dir, path);
                    continue;
                }

                if (slide_fno.fname[0] == '.') {
                    force_next_slide = 1;
                    continue;
                }

                if (is_jpeg_file(slide_fno.fname)) {
                    char filepath[384];
                    snprintf(filepath, sizeof(filepath), "%sPicture/%s", active_drive, slide_fno.fname);

                    /* Phase 1: Redirect Decoder to Back Buffer & Clear it */
                    jpeg_decode_target = BACK_BUFFER_ADDR;
                    memset((void*)BACK_BUFFER_ADDR, 0, PHYSICAL_W * PHYSICAL_H * 2);
                    SCB_CleanDCache_by_Addr((uint32_t *)BACK_BUFFER_ADDR, PHYSICAL_W * PHYSICAL_H * 2);

                    /* Phase 2: Decode JPEG silently in the background */
                    if (APP_SW_JPEG_Draw(filepath, 0, 0) == 0) {
                        /* Phase 3: Execute Dynamic Cinematic Transition */
                        app_ui_transition_execute(FB_BASE_ADDR, BACK_BUFFER_ADDR, current_transition_effect);

                        /* Automatically cycle to the next transition effect for the next picture */
                        current_transition_effect = (current_transition_effect + 1) % TRANSITION_MAX_EFFECTS;
                    }

                    /* Restore Engine Default Target */
                    jpeg_decode_target = FB_BASE_ADDR;

                    /* Reset timer AFTER everything is drawn (so transition time doesn't count towards the 3s delay) */
                    last_slide_time = HAL_GetTick();
                } else {
                     force_next_slide = 1;
                }
            }
        }

        if (state == UI_STATE_INIT) {
            res = f_mount(&fs, active_drive, 1);
            if (res == FR_OK) {
                app_ui_draw_main_menu(); state = UI_STATE_MAIN_MENU;
            } else {
                app_ui_fill_rect(40, 200, 400, 400, COLOR_YELLOW);
                app_ui_draw_text(60, 250, "Need Format?", COLOR_BLACK, COLOR_YELLOW);
                app_ui_fill_rect(80, 450, 320, 80, COLOR_GREEN);
                app_ui_draw_text(180, 470, "YES", COLOR_BLACK, COLOR_GREEN);
                state = UI_STATE_FORMAT_PROMPT;
            }
        }
    }
}
