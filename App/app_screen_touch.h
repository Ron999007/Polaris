/**
 * @file app_screen_touch.h
 * @brief Polaris OS - Dual Drive UI Manager Header
 */

#ifndef APP_SCREEN_TOUCH_H
#define APP_SCREEN_TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/**
 * @brief Global UI States for the Polaris OS Media System.
 */
typedef enum {
    UI_STATE_INIT = 0,         /**< Initializing system and mounting FatFS */
    UI_STATE_SELECT_DRIVE,     /**< Selecting target drive (SPI NOR or SDHC) */
    UI_STATE_FORMAT_PROMPT,    /**< Waiting for user to confirm formatting */
    UI_STATE_FORMATTING,       /**< Actively formatting the selected drive */
    UI_STATE_MAIN_MENU,        /**< Main media player and file browser menu */
    UI_STATE_VIEWING_DIR,      /**< Displaying directory and file contents */
	UI_STATE_SLIDESHOW,        /**< Auto-playing slideshow from Picture folder */
    UI_STATE_IMG_TEST_MENU,    /**< Menu for Image Generator Test */
    UI_STATE_VIEWING_IMAGE,    /**< Displaying generated image or test pattern */
    UI_STATE_SD_INFO,          /**< Displaying storage hardware diagnostic info */
    UI_STATE_ERROR             /**< Unrecoverable system error state */
} ui_state_t;

/**
 * @brief Main UI task for handling touch inputs and rendering graphical elements.
 * @param pvParameters FreeRTOS task parameters (usually NULL).
 */
void vTask_SN_Touch(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_SCREEN_TOUCH_H */
