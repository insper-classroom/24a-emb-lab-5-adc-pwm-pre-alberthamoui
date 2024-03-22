#include <FreeRTOS.h>          
#include <task.h>              
#include <semphr.h>            
#include <queue.h>             


#include "pico/stdlib.h"      
#include <stdio.h>            

#include "data.h"     
QueueHandle_t xQueueData;   

#define TAMANHO_JANELA 5

int filtroMediaMovel(int newData, int dataBuffer[], int *indexPtr) {
    static int sum = 0;
    sum -= dataBuffer[*indexPtr]; // Tira o valor antigo
    sum += newData; // Adiciona o valor novo
    dataBuffer[*indexPtr] = newData; // Guarda a soma
    *indexPtr = (*indexPtr + 1) % TAMANHO_JANELA; // Atualiza o índice
    return sum / TAMANHO_JANELA;
}

void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));
    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Aguarda um segundo
    }
}

void process_task(void *p) {
    int data_buffer[TAMANHO_JANELA] = {0}; // Tudo zero
    int buffer_index = 0;

    while (true) {
        int data = 0;
        if (xQueueReceive(xQueueData, &data, 100)) { // Tem dados na fila?
            int filtered_data = moving_average_filter(data, data_buffer, &buffer_index);
            printf("Filtrado: %d\n", filtered_data);
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();
    xQueueData = xQueueCreate(64, sizeof(int));
    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);
    vTaskStartScheduler();
    while (true)
        ;
}
