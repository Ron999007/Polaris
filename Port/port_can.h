/**
 * @file port_can.h
 * @brief Hardware Abstraction Port Layer for CAN Bus
 */

#ifndef PORT_CAN_H
#define PORT_CAN_H

#include <stdint.h>
#include <stdbool.h>

/* --- Platform Specific Includes --- */
/* Include STM32 HAL here so the upper layers don't need to know about it */
#include "stm32h7xx_hal.h"

/* --- Generic CAN Data Structures --- */

/**
 * @brief Generic CAN message structure independent of hardware platform.
 */
typedef struct {
    uint32_t id;         /**< CAN Message ID */
    uint8_t  data[8];    /**< Data payload */
    uint8_t  length;     /**< Data length (0-8 bytes) */
    bool     is_ext_id;  /**< true: 29-bit Extended ID, false: 11-bit Standard ID */
} Port_CAN_Msg_t;

/* --- API Prototypes --- */

/**
 * @brief Initializes the CAN hardware peripheral and starts the module.
 * @return 0 on success, 1 on hardware initialization failure.
 */
uint8_t Port_CAN_Init(void);

/**
 * @brief Transmits a CAN message directly via hardware registers.
 * @param p_msg Pointer to the generic CAN message structure.
 * @return 0 on success, 1 if transmission fails or TX mailbox is full.
 */
uint8_t Port_CAN_Transmit(const Port_CAN_Msg_t *p_msg);

/**
 * @brief Configures a hardware acceptance filter for incoming CAN messages.
 * @param filter_id The target ID to accept.
 * @param filter_mask The mask to apply (0 means don't care).
 * @param is_ext_id true if filtering extended IDs, false for standard IDs.
 * @return 0 on success, 1 on failure.
 */
uint8_t Port_CAN_SetFilter(uint32_t filter_id, uint32_t filter_mask, bool is_ext_id);

void Port_CAN_Print_Diagnostics(void);

#endif /* PORT_CAN_H */
