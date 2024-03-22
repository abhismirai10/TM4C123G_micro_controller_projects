#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

#define SYSCTL_RCGC1_UART0       0x00000001  // UART0 Clock Gating Control
#define SYSCTL_RCGC2_GPIOA       0x00000001  // Port A Clock Gating Control
#define UART_CTL_UARTEN          0x00000001  // UART Enable
#define UART_LCRH_FEN            0x00000010  // Enable FIFOs
#define UART_LCRH_WLEN_8         0x00000060  // 8-bit word length
#define UART_FR_TXFF             0x00000020  // Transmit FIFO full

void delay_ms(uint32_t mseconds);
void delay_1ms(uint32_t milliseconds);

void UART0_Init(void);
void UART0_SendChar(char data);
void UART0_SendString(char *str);

int main(void) {
    int x = 12345;

    // Set the system clock to 50 MHz
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0xFC100000) + 0x04000000; // Use the main oscillator
    SYSCTL_RCC2_R = (SYSCTL_RCC2_R & ~0x00002000) + 0x00000800; // Set the system clock to 50 MHz

    UART0_Init();

    while (1) {
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "x = %d\n\r", x);
        UART0_SendString(buffer);
        delay_ms(1000); // 1-second delay
    }
}

void UART0_Init(void) {
    // Enable the peripherals for UART0 and GPIOA
    SYSCTL_RCGC1_R |= SYSCTL_RCGC1_UART0;
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA;

    // Configure the UART0
    UART0_CTL_R &= ~UART_CTL_UARTEN;
    UART0_IBRD_R = 260;        // Set the baud rate to 9600
    UART0_FBRD_R = 27;
    UART0_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;
    UART0_CTL_R |= UART_CTL_UARTEN;

    // Configure the GPIOA
    GPIO_PORTA_AFSEL_R |= 0x03;            // Enable UART0 on PA0-PA1
    GPIO_PORTA_DEN_R |= 0x03;              // Enable digital I/O on PA0-PA1
    GPIO_PORTA_PCTL_R = (GPIO_PORTA_PCTL_R & 0xFFFFFF00) + 0x00000011; // Configure PA0-PA1 for UART0
}

void UART0_SendChar(char data) {
    while ((UART0_FR_R & UART_FR_TXFF) != 0); // Wait for the transmit buffer to be empty
    UART0_DR_R = data;
}

void UART0_SendString(char *str) {
    while (*str) {
        UART0_SendChar(*str++);
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
