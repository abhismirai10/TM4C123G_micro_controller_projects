
#include "tm4c123gh6pm.h"
#include "stdint.h"

//Use of SysTick to delay for a specified amount of time (C10_SysTick_Wait).

//Activate the TM4C with a 16 MHz crystal to run at 80 MHz
void PLL_Init(void){
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

//Initialization of Systick
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;               // disable SysTick during setup
  NVIC_ST_CTRL_R = 0x00000005;      // enable SysTick with core clock
}

// The delay parameter is in units of the 80 MHz core clock. (12.5 ns)
void SysTick_Wait(unsigned long delay){
  NVIC_ST_RELOAD_R = delay-1;  // number of counts to wait
  NVIC_ST_CURRENT_R = 0;       // any value written to CURRENT clears
  while((NVIC_ST_CTRL_R&0x00010000)==0){ // wait for count flag
  }
}

// 800000*12.5ns equals 10ms
void SysTick_Wait100ms(unsigned long delay){
  unsigned long i;
  for(i=0; i<delay; i++){
    SysTick_Wait(8000000);  // wait 100ms
  }
}

unsigned long In;  // input from PF4
unsigned long Out; // outputs to PF3,PF2,PF1 (multicolor LED)

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 input, PF3,PF2,PF1 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x11;          // enable pullup resistors on PF4,PF0
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0

}

int main(void)
{
    // Initialize PLL and SysTick
    PLL_Init();
    SysTick_Init();
    PortF_Init();
    int t=0;
    // Your application code here
    while(1){
        In = GPIO_PORTF_DATA_R&0x10; // read PF4 into In
        if(In == 0x00){              // zero means SW1 is pressed
          GPIO_PORTF_DATA_R = 0x08;  // LED is green
         }
        else{                      // 0x10 means SW1 is not pressed
          GPIO_PORTF_DATA_R = 0x02;  // LED is red
         }
        SysTick_Wait100ms(1); // Wait for 1 second (10 * 10ms)
        GPIO_PORTF_DATA_R = 0x04;    // LED is blue
        SysTick_Wait100ms(1);
        t=t+1;
      }


}
