#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "driver/uart.h"


typedef enum {
  OFF,
  LEFT_ON,
  RIGHT_ON,
  HAZARD
} SystemState;
SystemState currentState = OFF;

void setup_uart() {
  uart_config_t uart_cfg = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };
  uart_param_config(UART_NUM_1, &uart_cfg);
  uart_set_pin(UART_NUM_1, GPIO_NUM_17, GPIO_NUM_16, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  uart_driver_install(UART_NUM_1, 1024, 0, 0, NULL, 0);
}



void pwm_initialisation(void)
{
    printf("Inside pwminitialising");
        ledc_timer_config_t time = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK

    };

    ledc_timer_config(&time);

    ledc_channel_config_t left_led = {
        .gpio_num = 33,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
        
    };

    ledc_channel_config(&left_led);

    ledc_channel_config_t right_led = {
        .gpio_num = 32,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0
    };

    ledc_channel_config(&right_led);    
    ledc_fade_func_install(0);
}

void button_press()
{
    static uint32_t left_press_time = 0, right_press_time = 0;

    while (1)
    {
       if (!gpio_get_level(GPIO_NUM_22) && !gpio_get_level(GPIO_NUM_23))
        {
            left_press_time = xTaskGetTickCount();
            right_press_time = left_press_time;

            while (!gpio_get_level(GPIO_NUM_22) && !gpio_get_level(GPIO_NUM_23))
            {
                if ((xTaskGetTickCount() - left_press_time) >= pdMS_TO_TICKS(950))
                {
                    currentState = HAZARD;
                    printf("HAZARD \n");

                    // Wait for both buttons to be released
                    while (!gpio_get_level(GPIO_NUM_22) || !gpio_get_level(GPIO_NUM_23))
                    {
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }

                    
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        // Left button only
        else if (!gpio_get_level(GPIO_NUM_22) && gpio_get_level(GPIO_NUM_23))
        {
            left_press_time = xTaskGetTickCount();
            while (!gpio_get_level(GPIO_NUM_22) && gpio_get_level(GPIO_NUM_23))
            {
                if ((xTaskGetTickCount() - left_press_time) >= pdMS_TO_TICKS(950))
                {
                    if (currentState == HAZARD || currentState == LEFT_ON)
                    {
                        currentState = OFF;
                        printf("OFF");
                        uart_write_bytes(UART_NUM_1, "OFF\n", sizeof("OFF\n"));
                    }
                    else
                    {
                        currentState = LEFT_ON;
                        printf("LEFT_ON\n");
                        uart_write_bytes(UART_NUM_1, "LEFT_ON\n", sizeof("LEFT_ON\n"));
                    }

                    
                    while (!gpio_get_level(GPIO_NUM_22))
                    {
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }

                    
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        
        else if (gpio_get_level(GPIO_NUM_22) && !gpio_get_level(GPIO_NUM_23))
        {
            right_press_time = xTaskGetTickCount();

            while (gpio_get_level(GPIO_NUM_22) && !gpio_get_level(GPIO_NUM_23))
            {
                if ((xTaskGetTickCount() - right_press_time) >= pdMS_TO_TICKS(950))
                {
                    if (currentState == HAZARD || currentState == RIGHT_ON)
                    {
                        currentState = OFF;
                        printf("OFF\n");
                        uart_write_bytes(UART_NUM_1, "OFF\n", sizeof("OFF\n"));
                    }
                    else
                    {
                        currentState = RIGHT_ON;
                        printf("RIGHT_ON\n");
                        uart_write_bytes(UART_NUM_1, "RIGHT_ON\n", sizeof("RIGHT_ON\n"));
                    }

                    
                    while (!gpio_get_level(GPIO_NUM_23))
                    {
                        vTaskDelay(pdMS_TO_TICKS(50));
                    }

                    
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(50));
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50)); // General debounce
    }
}



void led_control()
{
    
    static uint32_t last_toggle = 0;    
    while(1)    
    {
    const char* state_names[] = {"OFF", "LEFT_ON", "RIGHT_ON", "HAZARD"};
    printf("%s\n", state_names[currentState]);
              
    switch (currentState)
    {
    case HAZARD:
    if(xTaskGetTickCount() - last_toggle >= pdMS_TO_TICKS(300))
    {
        last_toggle = xTaskGetTickCount();        
        uint32_t dutyleft = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        uint32_t dutyright = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, (dutyleft == 0) ? 255 : 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, (dutyright == 0) ? 255 : 0);  
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);      
    }
        break;


    case LEFT_ON:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);  
    if(xTaskGetTickCount() - last_toggle >= pdMS_TO_TICKS(300))
    {
        last_toggle = xTaskGetTickCount();
        uint32_t dutyleft = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, ( dutyleft == 0) ? 255 : 0);  
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);     
               
    }
        break;

    case RIGHT_ON:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0); 
    if(xTaskGetTickCount() - last_toggle >= pdMS_TO_TICKS(300))
    {
        last_toggle = xTaskGetTickCount();
        uint32_t dutyright = ledc_get_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, (dutyright == 0) ? 255 : 0);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);                        
    }
    break;


    case OFF:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
    break;    
    
    default:
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
        break;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
    }
}
void app_main(void)
{
    pwm_initialisation();
    
    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_22, GPIO_PULLUP_ONLY);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);
    gpio_set_pull_mode(GPIO_NUM_23, GPIO_PULLUP_ONLY);
    xTaskCreatePinnedToCore((TaskFunction_t)button_press, "Button Task", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore((TaskFunction_t)led_control, "LED Control Task", 2048, NULL, 1, NULL, 1);
}

