/**
 * @file app_can.c
 * @brief Application layer for CAN Bus management
 */

#include "app_can.h"
#include "app_led.h"
#include "queue.h"
#include <string.h>

/* ====================================================================
 * Debug Configuration
 * ==================================================================== */
#define APP_CAN_DEBUG_ENABLE  1  /* Set to 1 to enable debug logging, 0 to disable completely */

#if APP_CAN_DEBUG_ENABLE
    #include <stdio.h>
    #define APP_CAN_LOG(fmt, ...) printf("[APP_CAN] " fmt "\r\n", ##__VA_ARGS__)
#else
    #define APP_CAN_LOG(fmt, ...) /* Define to nothing when disabled */
#endif

/* ====================================================================
 * Private Data Structures
 * ==================================================================== */

/**
 * @brief Internal message envelope structure for the FreeRTOS Queue
 */
typedef struct {
    app_can_evt_e  evt_type;  /**< Type of the event */
    Port_CAN_Msg_t msg;       /**< The actual CAN message payload/ID */
} can_queue_msg_t;

/**
 * @brief Table-driven function pointer definition for parsing specific CAN IDs
 */
typedef void (*can_rx_handler_t)(const Port_CAN_Msg_t *p_msg);

typedef struct {
    uint32_t target_id;
    can_rx_handler_t handler;
} can_dispatch_entry_t;

/* ====================================================================
 * Private Global Variables (Encapsulated)
 * ==================================================================== */
/* ADDED STATIC: Strictly encapsulate the queue so port_can.c cannot bypass the wrapper */
static QueueHandle_t s_can_queue = NULL;

/* ====================================================================
 * Private Function Prototypes
 * ==================================================================== */
static void App_CAN_Process_Rx_Message(const Port_CAN_Msg_t *p_msg);

/* Table-driven architecture for clean and scalable CAN ID parsing. */
static const can_dispatch_entry_t s_rx_dispatch_table[] = {
    { 0x000, NULL } /* Placeholder entry */
};
#define RX_DISPATCH_TABLE_SIZE (sizeof(s_rx_dispatch_table) / sizeof(s_rx_dispatch_table[0]))

/* ====================================================================
 * Public Thread-Safe API
 * ==================================================================== */

BaseType_t App_CAN_Transmit(uint32_t id, const uint8_t *p_data, uint8_t len, bool is_ext_id)
{
    if (s_can_queue == NULL || p_data == NULL || len > 8) return pdFALSE;

    can_queue_msg_t q_msg;
    q_msg.evt_type = CAN_EVT_TX_REQ;
    q_msg.msg.id = id;
    q_msg.msg.length = len;
    q_msg.msg.is_ext_id = is_ext_id;
    memcpy(q_msg.msg.data, p_data, len);

    APP_CAN_LOG("API TX Request Queued | ID: 0x%03X, Len: %d", (unsigned int)id, len);

    return xQueueSend(s_can_queue, &q_msg, pdMS_TO_TICKS(10));
}

BaseType_t App_CAN_ISR_Receive(const Port_CAN_Msg_t *p_msg, BaseType_t *pxHigherPriorityTaskWoken)
{
    if (s_can_queue == NULL || p_msg == NULL) return pdFALSE;

    can_queue_msg_t q_msg;
    q_msg.evt_type = CAN_EVT_RX_MSG;
    q_msg.msg = *p_msg;

    /* Safely send properly wrapped event from ISR context */
    return xQueueSendFromISR(s_can_queue, &q_msg, pxHigherPriorityTaskWoken);
}

void App_CAN_Send_Test_Msg(void)
{
    static uint32_t s_test_can_id = 0x100;
    uint8_t test_payload[8] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};

    APP_CAN_LOG("Triggering Test Message | ID: 0x%03X", (unsigned int)s_test_can_id);

    if (App_CAN_Transmit(s_test_can_id, test_payload, 8, false) == pdTRUE)
    {
        s_test_can_id++;
        if (s_test_can_id > 0x7FF) s_test_can_id = 0x100;
    }
    else
    {
        APP_CAN_LOG("Warning: Queue Full, Test Message Dropped!");
    }
}

/* ====================================================================
 * Main CAN Task
 * ==================================================================== */

void vTask_CAN(void *pvParameters)
{
    can_queue_msg_t received_evt;
    uint32_t heartbeat_timer = 0;

    APP_CAN_LOG("Task Started");

    s_can_queue = xQueueCreate(20, sizeof(can_queue_msg_t));
    if (s_can_queue == NULL) {
        APP_CAN_LOG("Error: Queue Creation Failed!");
        vTaskDelete(NULL);
    }

    if (Port_CAN_Init() != 0) {
        APP_CAN_LOG("Error: Hardware Init Failed!");
        App_LED_Set_State(LED_ID_0, LED_CMD_START_BLINK);
    } else {
        APP_CAN_LOG("Hardware Init Success");
    }

    for(;;)
    {
        if (xQueueReceive(s_can_queue, &received_evt, pdMS_TO_TICKS(50)) == pdTRUE)
        {
            switch (received_evt.evt_type) {

                case CAN_EVT_TX_REQ:
                    APP_CAN_LOG("Processing TX | ID: 0x%03X", (unsigned int)received_evt.msg.id);
                    Port_CAN_Transmit(&received_evt.msg);
                    break;

                case CAN_EVT_RX_MSG:
                    /* THIS WILL NOW PRINT PERFECTLY ALIGNED DATA! */
                    APP_CAN_LOG("Processing RX | ID: 0x%03X, Len: %d", (unsigned int)received_evt.msg.id, received_evt.msg.length);
                    App_CAN_Process_Rx_Message(&received_evt.msg);
                    break;

                case CAN_EVT_CTRL_CMD:
                    APP_CAN_LOG("Processing CTRL CMD");
                    break;

                default:
                    APP_CAN_LOG("Warning: Unknown Event Type!");
                    break;
            }
        }
        else
        {
            heartbeat_timer++;
            if (heartbeat_timer >= 20) {
                heartbeat_timer = 0;
                //Port_CAN_Print_Diagnostics();
            }
        }
    }
}

/* ====================================================================
 * Private Implementation
 * ==================================================================== */

static void App_CAN_Process_Rx_Message(const Port_CAN_Msg_t *p_msg)
{
    if (p_msg == NULL) return;

    for (uint8_t i = 0; i < RX_DISPATCH_TABLE_SIZE; i++) {
        if (s_rx_dispatch_table[i].target_id == p_msg->id && s_rx_dispatch_table[i].handler != NULL) {
            APP_CAN_LOG("Dispatching RX ID: 0x%03X to Handler", (unsigned int)p_msg->id);
            s_rx_dispatch_table[i].handler(p_msg);
            return;
        }
    }
}
