
#include "tm4c123gh6pm.h"
#include "stdint.h"

unsigned volatile int x=0,y=0,i=0;

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


void SysTick_Handler(void) {
    // Toggle LED at PF2
    i=i+1;
    if (i%10==0){
    x=x+1;
    GPIO_PORTF_DATA_R ^= 0x04;
    }
}

void SysTick_Init(unsigned long period){ // priority 2
  NVIC_ST_CTRL_R = 0;         // disable SysTick during setup
  NVIC_ST_RELOAD_R = period-1;// reload value


  NVIC_ST_CTRL_R = 0x00000007; // enable SysTick with core clock and interrupts
  // finish all initializations and then enable interrupts
  NVIC_ST_CURRENT_R = 0;      // any write to current clears it
  NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // Set priority 2
}

void PortF_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000020; // Enable clock for Port F
    while((SYSCTL_PRGPIO_R&0x20) == 0){} // Wait for Port F to be ready
    GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0
    GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
    GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
    GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
    GPIO_PORTF_DIR_R |= 0x04; // Set PF2 as output
    GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
    GPIO_PORTF_DEN_R |= 0x04; // Enable digital functionality for PF2

}

int main(void) {
    PLL_Init();       // set system clock to 80 MHz
    PortF_Init();     // initialize Port F
    SysTick_Init(8000000);   // initialize SysTick
    // Enable global interrupts
    __asm("    cpsie   i");
    while(1) {
        // main loop, everything is handled by the interrupt
        __asm("wfi"); // Wait for the interrupt to occur
        y=y+1;
    }
}
