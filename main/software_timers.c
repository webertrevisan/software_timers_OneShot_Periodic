#include <stdio.h> // Biblioteca padrão para entrada e saída (ex: printf)
#include "freertos/FreeRTOS.h" // Cabeçalho principal do FreeRTOS
#include "freertos/task.h" // Necessário para criar e gerenciar tarefas
#include "freertos/timers.h" // Necessário para criar e usar timers de software

#include "driver/gpio.h" // Biblioteca para controle de GPIOs do ESP
#include "esp_log.h" // Biblioteca para geração de logs no ESP-IDF

#define LED1_PIN 3 // Define o pino GPIO 3 como LED1
#define LED2_PIN 33 // Define o pino GPIO 33 como LED2
#define BUTTON1_PIN 0 // Define o pino GPIO 0 como botão de entrada

TaskHandle_t xTask1; // Handle (ponteiro) para a tarefa task1
TimerHandle_t xTimer1,xTimer2; // Handles para os timers de software Timer1 e Timer2

void task1 (void *pvParameters); // Protótipo da função da tarefa task1

void timer1_callback(TimerHandle_t xTimer); // Protótipo da função de callback do timer1
void timer2_callback(TimerHandle_t xTimer); // Protótipo da função de callback do timer2

void app_main(void) // Função principal que é executada ao iniciar o programa
{

    gpio_config_t io_conf = {}; // Estrutura usada para configurar os pinos GPIO

    // Configure LED
    io_conf.pin_bit_mask = (1ULL<<LED1_PIN) | (1ULL<<LED2_PIN); // Máscara de bits para os pinos dos LEDs
    io_conf.mode = GPIO_MODE_OUTPUT; // Define os pinos como saída
    io_conf.pull_up_en = 0; // Desativa o resistor pull-up
    io_conf.pull_down_en = 0; // Desativa o resistor pull-down
    io_conf.intr_type = GPIO_INTR_DISABLE; // Desativa interrupções nesses pinos
    gpio_config(&io_conf); // Aplica a configuração para os pinos dos LEDs

    // Configure button
    io_conf.pin_bit_mask = (1ULL<<BUTTON1_PIN); // Máscara de bits para o pino do botão
    io_conf.mode = GPIO_MODE_INPUT; // Define o pino como entrada
    io_conf.pull_up_en = 1; // Ativa o resistor pull-up (evita flutuação do sinal)
    io_conf.pull_down_en = 0; // Desativa o resistor pull-down
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // Configura interrupção na borda de descida
    gpio_config(&io_conf); // Aplica a configuração para o botão

    xTimer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(1000),pdTRUE,(void*)0,timer1_callback); // Cria o Timer1 com período de 1 segundo e repetição contínua
    xTimer2 = xTimerCreate("Timer2", pdMS_TO_TICKS(5000),pdFALSE,(void*)0,timer2_callback); // Cria o Timer2 com período de 5 segundos e sem repetição

    xTaskCreate(task1,"task1", configMINIMAL_STACK_SIZE+1024,NULL,1,&xTask1); // Cria a tarefa task1 com prioridade 1

    xTimerStart(xTimer1,0); // Inicia o Timer1 imediatamente

    vTaskDelete(NULL); // Deleta a tarefa atual (app_main), pois já terminou a configuração

}


void task1 (void *pvParameters) // Função da tarefa que monitora o botão
{
    uint8_t debouncingTime = 0; // Variável para controlar o tempo de debounce
    while (1) // Loop infinito da tarefa
    {
        if (gpio_get_level(BUTTON1_PIN) == 0) // Verifica se o botão foi pressionado (nível baixo)
        {
            debouncingTime++; // Incrementa o contador de debounce
            if(debouncingTime >=10) // Se pressionado por tempo suficiente (100ms)
            {
                gpio_set_level(LED2_PIN, 1); // Liga o LED2
                debouncingTime = 0; // Reseta o contador de debounce
                xTimerStart(xTimer2,0); // Inicia o Timer2 para desligar LED2 após 5 segundos
                ESP_LOGI("xTimer2","Timer 2 Start"); // Loga o início do Timer2

            }
        }
        else {
            debouncingTime = 0; // Se botão solto, reseta contador de debounce
        }
        
        vTaskDelay(pdMS_TO_TICKS(10)); // Aguarda 10ms (tick do debounce)

    }
    

}


void timer1_callback(TimerHandle_t xTimer) // Função chamada a cada 1 segundo pelo Timer1
{
    static uint8_t ledstate = 0; // Variável estática que guarda estado do LED
    gpio_set_level(LED1_PIN,ledstate^=1); // Alterna o estado do LED1
    ESP_LOGI("xTimer1","Timer 1 Callback"); // Log do callback do Timer1

}

void timer2_callback(TimerHandle_t xTimer) // Função chamada uma vez pelo Timer2 após 5 segundos
{
    gpio_set_level(LED2_PIN,0); // Desliga o LED2
    ESP_LOGI("xTimer2","Timer 2 Callback"); // Log do callback do Timer2

}
