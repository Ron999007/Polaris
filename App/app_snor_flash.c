/**
 * @file    app_snor_flash.c
 * @brief   FreeRTOS Task for W25Q64JV - 8MB Full Flash Read/Write Test
 * @project Polaris
 */

#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "app_snor_flash.h"
#include "drv_snor_flash.h"
#include <stdio.h>
#include <string.h>
#include "stm32h7xx_hal.h"

/* 確保與 CubeMX 或其他檔案中定義的 Semaphore 名稱一致 */
extern osSemaphoreId_t SnorTestHandle;

#define TEST_SECTOR_SIZE     W25Q64JV_SECTOR_SIZE     /* 4096 bytes */
#define TEST_PAGE_SIZE       W25Q64JV_PAGE_SIZE       /* 256 bytes */
#define TOTAL_FLASH_SIZE     W25Q64JV_FLASH_SIZE      /* 8MB: 0x800000 */

/* Ensure 32-byte alignment to meet Cortex-M7 D-Cache requirements */
static uint8_t write_buffer[TEST_SECTOR_SIZE] __attribute__((aligned(32)));
static uint8_t read_buffer[TEST_SECTOR_SIZE]  __attribute__((aligned(32)));

void vTask_SNOR(void *pvParameters)
{
    (void)pvParameters;
    uint32_t flash_id = 0;

    /* 1. Initialize the driver */
    if (SNOR_Init() != SNOR_OK)
    {
        printf("[SNOR] ERROR: SNOR_Init() failed! Task aborting.\r\n");
        vTaskDelete(NULL);
        return;
    }

    /* 2. Read and verify JEDEC ID (Optional but recommended for validation) */
    if (SNOR_ReadID(&flash_id) != SNOR_OK)
    {
        printf("[SNOR] ERROR: Failed to read JEDEC ID!\r\n");
        vTaskDelete(NULL);
        return;
    }
    printf("[SNOR] W25Q64JV detected. JEDEC ID: 0x%06lX\r\n", flash_id);


    printf("[SNOR] W25Q64JV Initialized Successfully.\r\n");

    /* Initialize write buffer pattern (0x00 ~ 0xFF looping pattern) */
    for (uint32_t i = 0; i < TEST_SECTOR_SIZE; i++)
    {
        write_buffer[i] = (uint8_t)(i & 0xFF);
    }

    /* Task Infinite Loop */
    for(;;)
    {
        /* 3. Wait for trigger */
        printf("\r\n[SNOR] Task sleeping... Waiting for Touch UI trigger.\r\n");

        /* Drain semaphore to prevent accidental multiple triggers queuing up */
        while (osSemaphoreAcquire(SnorTestHandle, 0) == osOK) {}

        /* Block and wait for the touch event to release the semaphore */
        osSemaphoreAcquire(SnorTestHandle, osWaitForever);

        printf("\r\n===============================================\r\n");
        printf("[SNOR] --- 8MB FLASH FULL TEST STARTED ---\r\n");
        printf("===============================================\r\n");

        uint32_t current_addr = 0;
        uint8_t test_failed = 0;

        /* 4. Start 8MB Sector-by-Sector Test */
        while (current_addr < TOTAL_FLASH_SIZE)
        {
            /* Step A: Erase 4KB Sector */
            if (SNOR_EraseSector(current_addr) != SNOR_OK)
            {
                printf("[SNOR] ERROR: Erase failed at Addr: 0x%08lX\r\n", current_addr);
                test_failed = 1;
                break;
            }

            /* Step B: Write 4KB (via 16 isolated Page Programs of 256 bytes) */
            for (uint32_t p = 0; p < (TEST_SECTOR_SIZE / TEST_PAGE_SIZE); p++)
            {
                uint32_t page_addr = current_addr + (p * TEST_PAGE_SIZE);
                if (SNOR_WritePage(page_addr, &write_buffer[p * TEST_PAGE_SIZE], TEST_PAGE_SIZE) != SNOR_OK)
                {
                    printf("[SNOR] ERROR: Write failed at Page Addr: 0x%08lX\r\n", page_addr);
                    test_failed = 1;
                    break;
                }
            }
            if (test_failed) break;

            /* Step C: Read the entire 4KB Sector */
            memset(read_buffer, 0x00, TEST_SECTOR_SIZE);
            if (SNOR_Read(current_addr, read_buffer, TEST_SECTOR_SIZE) != SNOR_OK)
            {
                printf("[SNOR] ERROR: Read failed at Addr: 0x%08lX\r\n", current_addr);
                test_failed = 1;
                break;
            }

            /* Step D: Verify Data Integrity */
            if (memcmp(write_buffer, read_buffer, TEST_SECTOR_SIZE) != 0)
            {
                printf("[SNOR] ERROR: Data verification mismatch at Addr: 0x%08lX!\r\n", current_addr);
                test_failed = 1;
                break;
            }

            current_addr += TEST_SECTOR_SIZE;

            /* Progress Report & OS Yielding */
            /* Every 1MB, print progress and yield to prevent watchdog resets and UI freezing */
            if (current_addr % (1024 * 1024) == 0)
            {
                printf("[SNOR] Progress: %lu MB verified successfully...\r\n", current_addr / (1024 * 1024));
                osDelay(10);
            }
            else
            {
                /* Small yield between sectors to keep Touch Task responsive */
                osDelay(2);
            }
        }

        /* Test Result Output */
        if (!test_failed)
        {
            printf("\r\n===============================================\r\n");
            printf("[SNOR] SUCCESS: 8MB FULL FLASH TEST PASSED!\r\n");
            printf("===============================================\r\n");
        }
        else
        {
            printf("\r\n[SNOR] TEST ABORTED DUE TO HARDWARE ERROR.\r\n");
        }
    }
}
