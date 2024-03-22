#include <stdint.h>
#include "tm4c123gh6pm.h"

#define STEPPER_PORT_BASE GPIO_PORTA_DATA_R
#define STEP_PIN_1 0x04
#define STEP_PIN_2 0x08
#define STEP_PIN_3 0x10
#define STEP_PIN_4 0x20
#define DELAY_MS 5

void stepper_set_output(uint8_t step);
void stepper_step_sequence(int32_t steps);
void delay_ms(uint32_t milliseconds);
void delay_1ms(uint32_t milliseconds);
void PLL_Init(void);
void PortA_Init(void);

int main(void)
{
    PLL_Init();
    PortA_Init();
    uint32_t steps_per_revolution = 4096; // 4096 steps per revolution for the 28BYJ-48 stepper motor
    float angle = 90; // Desired angle of rotation
    uint32_t steps_to_rotate = (uint32_t)(angle / 360 * steps_per_revolution);
    while (1)
    {
        stepper_step_sequence(steps_to_rotate);
        delay_ms(2000);
        stepper_step_sequence(-steps_to_rotate);
        delay_ms(2000);
    }
}

void stepper_set_output(uint8_t step)
{
    STEPPER_PORT_BASE = (STEPPER_PORT_BASE & ~(STEP_PIN_1 | STEP_PIN_2 | STEP_PIN_3 | STEP_PIN_4)) | step;
}

void stepper_step_sequence(int32_t steps)
{
    int32_t i;
    uint8_t step_index = 0;
    uint8_t sequence[8] = {STEP_PIN_1, STEP_PIN_1 | STEP_PIN_2, STEP_PIN_2, STEP_PIN_2 | STEP_PIN_3, STEP_PIN_3, STEP_PIN_3 | STEP_PIN_4, STEP_PIN_4, STEP_PIN_4 | STEP_PIN_1};

    for (i = 0; i < abs(steps); i++)
    {
        stepper_set_output(sequence[step_index]);
        delay_ms(DELAY_MS);

        if (steps > 0)
        {
            step_index = (step_index + 1) % 8;
        }
        else
        {
            step_index = (step_index - 1 + 8) % 8;
        }
    }
}

void delay_1ms(uint32_t milliseconds)
{
    uint32_t ticks = 80000 * milliseconds;
    NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
    NVIC_ST_RELOAD_R = ticks - 1;
    NVIC_ST_CURRENT_R = 0;
    NVIC_ST_CTRL_R = 0x00000005; // enable SysTick with core clock.
    while((NVIC_ST_CTRL_R & 0x00010000) == 0)
    {
    }
}
void delay_ms(uint32_t mseconds)
{   uint32_t i;
    for(i = 0; i < mseconds; i++)
        {
            delay_1ms(1);
        }

}
void PortA_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000001; // Enable clock for Port A
    while((SYSCTL_PRGPIO_R&0x01) == 0){} // Wait for Port A to be ready
    GPIO_PORTA_LOCK_R = 0x4C4F434B;   // 2) unlock PortA
    GPIO_PORTA_CR_R = 0xFC;           // 2) allow changes to PA7-2
    GPIO_PORTA_AMSEL_R = 0x00;        // 3) Disable analog function
    GPIO_PORTA_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
    GPIO_PORTA_DIR_R |= 0xFC;         // 5) Set PA7-2 on Port A as output
    GPIO_PORTA_AFSEL_R = 0x00;        // 6) No alternate function
    GPIO_PORTA_DEN_R |= 0xFC;          // 7) Enable digital functionality for all pins on Port A
}
void PLL_Init(void) {
        // Code for PLL initialization
        // 0) Use RCC2
          SYSCTL_RCC2_R |=  0x80000000;  // USERCC2
          // 1) bypass PLL while initializing
          SYSCTL_RCC2_R |=  0x00000800;  // BYPASS2, PLL bypass
          // 2) select the crystal value and oscillator source
          SYSCTL_RCC_R = (SYSCTL_RCC_R &~0x000007C0)   // clear XTAL field, bits 10-6
                         + 0x00000540;   // 10101, configure for 16 MHz crystal
          SYSCTL_RCC2_R &= ~0x00000070;  // configure for main oscillator source
          // 3) activate PLL by clearing PWRDN
          SYSCTL_RCC2_R &= ~0x00002000;
          // 4) set the desired system divider
          SYSCTL_RCC2_R |= 0x40000000;   // use 400 MHz PLL
          SYSCTL_RCC2_R = (SYSCTL_RCC2_R&~ 0x1FC00000)  // clear system clock divider
                          + (4<<22);      // configure for 80 MHz clock
          // 5) wait for the PLL to lock by polling PLLLRIS
          while((SYSCTL_RIS_R&0x00000040)==0){};  // wait for PLLRIS bit
          // 6) enable use of PLL by clearing BYPASS
          SYSCTL_RCC2_R &= ~0x00000800;
}
