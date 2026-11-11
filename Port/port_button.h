#ifndef __PORT_BUTTON_H__
#define __PORT_BUTTON_H__

#include "drv_button.h"

/* Define system-wide Button IDs based on your specification */
typedef enum {
    BTN_ID_UP = 1,      /* PA0  (Low Active)  */
    BTN_ID_DOWN,        /* PH2  (High Active) */
    BTN_ID_LEFT,        /* PC13 (High Active) */
    BTN_ID_RIGHT,       /* PH3  (High Active) */
    BTN_ID_MAX
} system_btn_id_e;

/**
 * @brief Initialize hardware binding for the button driver
 */
void Port_Button_Hardware_Init(void);

#endif /* __PORT_BUTTON_H__ */
