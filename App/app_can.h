/**
 * @file app_can.h
 * @brief Application layer for CAN Bus management (Thread-Safe)
 */

#ifndef __APP_CAN_H__
#define __APP_CAN_H__

#include "FreeRTOS.h"
#include "task.h"
#include "port_can.h" /* For Port_CAN_Msg_t */

/* ====================================================================
 * Public Data Structures
 * ==================================================================== */

/**
 * @brief Defines explicit event types for the CAN Task Queue
 */
typedef enum {
    CAN_EVT_RX_MSG = 0,    /**< Incoming message from hardware ISR */
    CAN_EVT_TX_REQ,        /**< Outgoing transmission request from other tasks */
    CAN_EVT_CTRL_CMD       /**< Control commands (e.g., bus recovery, sleep) */
} app_can_evt_e;

/* ====================================================================
 * Public Thread-Safe API Prototypes
 * ==================================================================== */

/**
 * @brief Entry point for the Application CAN task.
 * @param pvParameters Task parameters (usually NULL).
 */
void vTask_CAN(void *pvParameters);

/**
 * @brief Thread-safe API for other tasks to transmit CAN messages.
 * @param id The target CAN identifier.
 * @param p_data Pointer to the payload array.
 * @param len Length of the payload (0-8).
 * @param is_ext_id Set to true for 29-bit ID, false for 11-bit ID.
 * @retval pdTRUE if successfully queued, pdFALSE if queue is full.
 */
BaseType_t App_CAN_Transmit(uint32_t id, const uint8_t *p_data, uint8_t len, bool is_ext_id);

/**
 * @brief Thread-safe API meant to be called ONLY from the Hardware ISR.
 * @param p_msg Pointer to the received message from the driver layer.
 * @param pxHigherPriorityTaskWoken Pointer to flag for forced context switch.
 * @retval pdTRUE if successfully queued.
 */
BaseType_t App_CAN_ISR_Receive(const Port_CAN_Msg_t *p_msg, BaseType_t *pxHigherPriorityTaskWoken);

/**
 * @brief Thread-safe API to send a test CAN message with an auto-incrementing ID.
 * Intended to be triggered by external events (e.g., Button press).
 */
void App_CAN_Send_Test_Msg(void);

#endif /* __APP_CAN_H__ */
