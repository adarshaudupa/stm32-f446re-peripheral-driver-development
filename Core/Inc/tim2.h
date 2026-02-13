#ifndef TIM2_H
#define TIM2_H

// In tim2.h
typedef enum {
    LED_MANUAL_OFF,
    LED_MANUAL_ON,
    LED_AUTO_BLINK
} led_state_t;

extern volatile led_state_t led_state;

void TIM2_Init(void);
void timer_start(void);
void timer_stop(void);


#endif
