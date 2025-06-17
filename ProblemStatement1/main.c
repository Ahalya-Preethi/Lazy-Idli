#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Global Variables (Assumed to be updated elsewhere)
extern uint8_t G_DataID;
extern int32_t G_DataValue;

// Task Handles
TaskHandle_t TaskHandle_1;
TaskHandle_t TaskHandle_2;

// Queue Definition
typedef struct {
    uint8_t dataID;
    int32_t DataValue;
} Data_t;

QueueHandle_t Queue1;

// Task 1 - Sends data every 500ms
void ExampleTask1(void *pV) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    Data_t dataToSend;

    while (1) {
        dataToSend.dataID = G_DataID;
        dataToSend.DataValue = G_DataValue;

        xQueueSend(Queue1, &dataToSend, portMAX_DELAY);

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}

// Task 2 - Processes data from queue
void ExampleTask2(void *pV) {
    Data_t receivedData;
    UBaseType_t originalPriority = uxTaskPriorityGet(NULL);
    UBaseType_t priorityIncreased = 0;

    while (1) {
        if (xQueueReceive(Queue1, &receivedData, portMAX_DELAY) == pdTRUE) {
            printf("Received dataID: %d, DataValue: %ld\n", receivedData.dataID, receivedData.DataValue);

            if (receivedData.dataID == 0 || receivedData.DataValue == 2) {
                vTaskDelete(NULL);
            }

            if (receivedData.dataID == 1) {
                switch (receivedData.DataValue) {
                    case 0:
                        if (!priorityIncreased) {
                            vTaskPrioritySet(NULL, originalPriority + 2);
                            priorityIncreased = 1;
                        }
                        break;

                    case 1:
                        if (priorityIncreased) {
                            vTaskPrioritySet(NULL, originalPriority);
                            priorityIncreased = 0;
                        }
                        break;
                }
            }
        }
    }
}

// Main function to initialize
int main(void) {
    // Create Queue
    Queue1 = xQueueCreate(5, sizeof(Data_t));

    if (Queue1 != NULL) {
        // Create Tasks
        xTaskCreate(ExampleTask1, "Sender", 1000, NULL, 2, &TaskHandle_1);
        xTaskCreate(ExampleTask2, "Receiver", 1000, NULL, 2, &TaskHandle_2);

        // Start Scheduler
        vTaskStartScheduler();
    } else {
        printf("Queue creation failed\n");
    }

    return 0;
}