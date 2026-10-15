#include "FreeRTOS.h"
#include "task.h"
#include <stdint.h>
#include <stddef.h>

void *memcpy(void *dest, const void *src, size_t n) {
    char *d = dest; const char *s = src;
    while (n--) *d++ = *s++; return dest;
}
void *memset(void *s, int c, size_t n) {
    unsigned char *p = s;
    while (n--) *p++ = (unsigned char)c; return s;
}

// Thanh ghi điều khiển GPIOA và VTOR
#define RCC_APB2ENR  (*((volatile uint32_t *)0x40021018))
#define GPIOA_CRL    (*((volatile uint32_t *)0x40010800))
#define GPIOA_ODR    (*((volatile uint32_t *)0x4001080C))
#define SCB_VTOR     (*((volatile uint32_t *)0xE000ED08))

typedef struct {
    uint8_t pin;
    uint32_t delay_ms;
} LED_Config_t;

LED_Config_t led1, led2, led3;

void vBlinkLED_Task(void *pvParameters) {
    LED_Config_t *config = (LED_Config_t *)pvParameters;
    while (1) {
        GPIOA_ODR ^= (1 << config->pin);
        vTaskDelay(config->delay_ms);
    }
}

int main(void) {
    // 1. Ép thanh ghi Vector trỏ về Flash để không bị treo HĐH
    SCB_VTOR = 0x08000000;

    // 2. Khởi tạo chân PA0, PA1, PA2 làm Output
    RCC_APB2ENR |= (1 << 2);
    GPIOA_CRL &= ~(0xFFF << 0);
    GPIOA_CRL |=  (0x333 << 0);
    
    // 3. Cấu hình tốc độ nháy
    led1.pin = 0; led1.delay_ms = 5000; // PA0: Nửa chu kỳ 5s (0.1Hz)
    led2.pin = 1; led2.delay_ms = 500;  // PA1: Nửa chu kỳ 0.5s (1Hz)
    led3.pin = 2; led3.delay_ms = 50;   // PA2: Nửa chu kỳ 0.05s (10Hz)

    // 4. Tạo 3 task độc lập
    xTaskCreate(vBlinkLED_Task, "Task1", 128, &led1, 1, NULL);
    xTaskCreate(vBlinkLED_Task, "Task2", 128, &led2, 1, NULL);
    xTaskCreate(vBlinkLED_Task, "Task3", 128, &led3, 1, NULL);

    vTaskStartScheduler();
    
    while (1) {}
    return 0;
}
