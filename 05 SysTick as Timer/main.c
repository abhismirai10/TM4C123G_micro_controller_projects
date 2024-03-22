
//Initialization of SysTick.
#define NVIC_ST_CTRL_R      (*((volatile unsigned long *)0xE000E010))
#define NVIC_ST_RELOAD_R    (*((volatile unsigned long *)0xE000E014))
#define NVIC_ST_CURRENT_R   (*((volatile unsigned long *)0xE000E018))

//Setting Systick as clock
void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;              // 1) disable SysTick during setup
  NVIC_ST_RELOAD_R = 0x00FFFFFF;   // 2) maximum reload value
  NVIC_ST_CURRENT_R = 0;           // 3) any write to current clears it
  NVIC_ST_CTRL_R = 0x00000005;     // 4) enable SysTick with core clock
}

//Use of SysTick to measure elapsed time.
unsigned long Now;      // 24-bit time at this call (12.5ns)
unsigned long Last;     // 24-bit time at previous call (12.5ns)
unsigned long Elapsed;  // 24-bit time between calls (12.5ns)
void Action(void){      // function under test
  Now = NVIC_ST_CURRENT_R;         // what time is it now?
  Elapsed = (Last-Now)&0x00FFFFFF; // 24-bit difference
  Last = Now;                      // set up for next...
}
