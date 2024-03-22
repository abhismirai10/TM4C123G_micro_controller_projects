#include <stdint.h>
#include "tm4c123gh6pm.h"

void PLL_Init2(void);
void delay_ms(uint32_t mseconds);
void delay_1ms(uint32_t milliseconds);
void PWM_Init(void);
void setServoAngle(uint8_t angle);

int main(void)
{
    PLL_Init2();
    PWM_Init();
    uint8_t angle = 0;

    while(1)
    {
        //setServoAngle(angle);
        angle = (angle + 10);
        GPIO_PORTF_DATA_R ^= 0x04;  // Toggle LED

        PWM0_0_CMPA_R = 11250;
        delay_ms(2000);
        PWM0_0_CMPA_R = 11874;
        delay_ms(2000);
    }
}
//PWM on pin PF2
void PWMInit(void)
{
    // Enable the clock to the GPIO Port F
    SYSCTL_RCGCGPIO_R |= 0x20;
    // Wait for the GPIOF module to be ready
    while ((SYSCTL_PRGPIO_R & 0x20) == 0) {};

    // Configure the PF2 pin for PWM
    GPIO_PORTF_AFSEL_R |= 0x04; // enable alternate function on PF2
    GPIO_PORTF_PCTL_R &= ~0x00000F00; // clear PF2
    GPIO_PORTF_PCTL_R |= 0x00000500; // configure PF2 as M1PWM6
    GPIO_PORTF_AMSEL_R &= ~0x04; // disable analog functionality on PF2
    GPIO_PORTF_DEN_R |= 0x04; // enable digital I/O on PF2

    // Enable the clock to the PWM1 module
    SYSCTL_RCGCPWM_R |= 0x02;

    // Disable the PWM while initializing
    PWM1_3_CTL_R = 0;

    // Configure the PWM generator for count down mode with immediate updates to the parameters
    PWM1_3_GENA_R = (0x02 << 6) | (0x03 << 2);

    // Set the period. Since the system clock is 80MHz, the Frequency will be 80MHz / period
    PWM1_3_LOAD_R = 1600000 - 1;

    // Set the pulse width for PWM1 Generator for 0% duty cycle
    PWM1_3_CMPA_R = 160000;

    // Start the timers in PWM1 generator 3
    PWM1_3_CTL_R |= 0x01;

    // Enable the outputs of PWM1 generator 3
    PWM1_ENABLE_R |= 0x40;
}

void PWMSetDuty(uint32_t duty)
{
    // Set the pulse width for PWM1 Generator 3
    PWM1_3_CMPA_R = duty - 1;
}

void setServoAngle(uint8_t angle)
{
    if(angle > 180) angle = 180; // Limit angle to the range 0-180 degrees
    uint32_t pulseWidth = 1000 + (angle * 1000) / 180; // Calculate pulse width (1 ms to 2 ms) based on the angle
    PWM1_3_CMPB_R = pulseWidth - 1;
}
// For 80Mhz clock, 2 gives more control then 1, still use manual
void PLL_Init2(void) {

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
//both for delay in miliseconds
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
