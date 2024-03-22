
#include "tm4c123gh6pm.h"
#include "stdint.h"
#include <stdbool.h>

//Variables
volatile uint32_t interrupt_count = 0;
volatile uint32_t stopwatch_count = 0;
volatile int digit_display;
void Display(int);
void PortF_Handler(void);
bool push = false;

// Activate the TM4C with a 16 MHz crystal to run at 80 MHz
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

// For Push buttons
void PortF_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000020; // Enable clock for Port F
    while((SYSCTL_PRGPIO_R&0x20) == 0){} // Wait for Port F to be ready
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF
    GPIO_PORTF_CR_R = 0x11;           // 2) allow changes to PF4 and PF0
    GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
    GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
    GPIO_PORTF_DIR_R &= ~0x11;        // 5) Set PF0 and PF4 as Input
    GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
    GPIO_PORTF_DEN_R |= 0x11;         // 7) Enable digital functionality for PF2
    GPIO_PORTF_PUR_R |= 0x11;         // 8) Enable pull-up resistor for PF0 and PF4

    GPIO_PORTF_IS_R &= ~0x01;         // PF0 is edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x01;        // PF0 is not both edges
    GPIO_PORTF_IEV_R &= ~0x01;        // PF0 falling edge event
    GPIO_PORTF_ICR_R = 0x01;          // Clear PF0 flag
    GPIO_PORTF_IM_R |= 0x01;          // Enable interrupt for PF0
    NVIC_EN0_R |= 0x40000000;         // Enable interrupt 30 in NVIC (Port F)

    GPIO_PORTF_DATA_R &= ~0x02;       // Set PF1 (LED) to 0 initially
    GPIO_PORTF_DIR_R |= 0x02;         // Set PF1 as output
    GPIO_PORTF_DEN_R |= 0x02;         // Enable digital functionality for PF1

}

//For Display
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

void PortB_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000002; // Enable clock for Port B
    while((SYSCTL_PRGPIO_R&0x02) == 0){} // Wait for Port B to be ready
    GPIO_PORTB_LOCK_R = 0x4C4F434B;   // 2) unlock Port B
    GPIO_PORTB_CR_R = 0xFF;           // 2) allow changes to PB7-0
    GPIO_PORTB_AMSEL_R = 0x00;        // 3) disable analog function
    GPIO_PORTB_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
    GPIO_PORTB_DIR_R |= 0xFF;         // 5) Set PB7-0 as output
    GPIO_PORTB_AFSEL_R = 0x00;        // 6) no alternate function
    GPIO_PORTB_DEN_R |= 0xFF;         // 7) Enable digital functionality for PB7-0
}

//SysTick Init
void SysTick_Init(unsigned long period){
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and interrupts
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // Set priority 2
}

int main(void) {
    PLL_Init();             // set system clock to 80 MHz
    PortF_Init();           // initialize Port F
    PortA_Init();           // initialize Port A
    PortB_Init();           // initialize Port B
    SysTick_Init(80000);    // initialize SysTick Interrupt every 1 ms
    __asm("    cpsie   i"); // Enable global interrupts
    GPIO_PORTA_DATA_R &= ~0x3C;    // Set Port A2 - A7 zero
    GPIO_PORTB_DATA_R |= 0xFF;    // Set Port B0 - 7 to 1
    while(1) {
        __asm("wfi"); // Wait for the interrupt to occur
    }
}
// while interrupt
void SysTick_Handler(void) {
    if (!push){

        if (interrupt_count%10==0){
            stopwatch_count=stopwatch_count+1;
        }
    }
    interrupt_count=interrupt_count+1;
    if (interrupt_count%4==0){
        GPIO_PORTA_DATA_R |= 0x38;   // Set A3-5 to 1
        digit_display = stopwatch_count/1000;
        Display (digit_display);          //To set display count

        GPIO_PORTA_DATA_R &= ~0x04;    // Set Port A2 to 0

    }
    if (interrupt_count%4==1){
        GPIO_PORTA_DATA_R |= 0x34;   //
        digit_display = (stopwatch_count/100)%10;
        Display (digit_display);          //To set display count

        GPIO_PORTA_DATA_R &= ~0x08;    // Set Port A3 to 0
    }
    if (interrupt_count%4==2){
        GPIO_PORTA_DATA_R |= 0x2C;   //
        digit_display = (stopwatch_count/10)%10;
        Display (digit_display);          //To set display count

        GPIO_PORTA_DATA_R &= ~0x10;    // Set Port A4 to 0
    }
    if (interrupt_count%4==3){
        GPIO_PORTA_DATA_R |= 0x1C;   //
        digit_display = (stopwatch_count)%10;
        Display (digit_display);          //To set display count

        GPIO_PORTA_DATA_R &= ~0x20;    // Set Port A5 to 0
    }

}

//Digit to display
void Display(int i) {
    // Define 7-segment display patterns for digits 0 to 9
    const uint8_t patterns[] = {
        0b00111111, // 0
        0b00000110, // 1
        0b01011011, // 2
        0b01001111, // 3
        0b01100110, // 4
        0b01101101, // 5
        0b01111101, // 6
        0b00000111, // 7
        0b01111111, // 8
        0b01100111  // 9
    };


    // Set the appropriate pattern for the given digit on PortB
        GPIO_PORTB_DATA_R = patterns[i];
}

void PortF_Handler(void) {
    GPIO_PORTF_ICR_R = 0x01; // Clear PF0 flag
    push = !push;
}
