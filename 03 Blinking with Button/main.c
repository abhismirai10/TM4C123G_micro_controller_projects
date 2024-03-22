

/**
 * main.c
 */

// to define registers for port F
#define GPIO_PORTF_DATA_R       (*((volatile unsigned long *)0x400253FC))
#define GPIO_PORTF_DIR_R        (*((volatile unsigned long *)0x40025400))
#define GPIO_PORTF_AFSEL_R      (*((volatile unsigned long *)0x40025420))
#define GPIO_PORTF_PUR_R        (*((volatile unsigned long *)0x40025510))
#define GPIO_PORTF_DEN_R        (*((volatile unsigned long *)0x4002551C))
#define GPIO_PORTF_LOCK_R       (*((volatile unsigned long *)0x40025520))
#define GPIO_PORTF_CR_R         (*((volatile unsigned long *)0x40025524))
#define GPIO_PORTF_AMSEL_R      (*((volatile unsigned long *)0x40025528))
#define GPIO_PORTF_PCTL_R       (*((volatile unsigned long *)0x4002552C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))


unsigned long In;  // input from PF4

unsigned long Out; // output to PF2 (blue LED)

void PortF_Init(void);
void Delay(unsigned long time);


int main(void){    // initialize PF0 and PF4 and make them inputs

  PortF_Init();    // make PF3-1 out (PF3-1 built-in LEDs)

  while(1){
    In = GPIO_PORTF_DATA_R&0x10;   // read PF4 into Sw1

    In = In>>2;                    // shift into position PF2
    while(In == 0x00){
        Out = GPIO_PORTF_DATA_R;
        Out = Out|0x04;
        GPIO_PORTF_DATA_R = Out;        // output
        Delay(1);
        Out = Out&~(0x04);
        GPIO_PORTF_DATA_R = Out;        // output
        Delay(1);
        In = GPIO_PORTF_DATA_R&0x10;
        In = In>>2;
    }
    if (In ==0x04){
        Out = GPIO_PORTF_DATA_R;
        Out = Out|0x04;
        GPIO_PORTF_DATA_R = Out;        // output
    }

  }

}

void Delay (unsigned long time){
  unsigned long i;
  while(time > 0){
    i = 133333;  // this number means 100ms
    while(i > 0){
      i = i - 1;
    }
    time = time - 1; // decrements every 100 ms
  }
}

void PortF_Init(void){ volatile unsigned long delay;

  SYSCTL_RCGC2_R |= 0x00000020;     // 1) activate clock for Port F

  delay = SYSCTL_RCGC2_R;           // allow time for clock to start

  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F

  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0

  // only PF0 needs to be unlocked, other bits can't be locked

  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog on PF

  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) PCTL GPIO on PF4-0

  GPIO_PORTF_DIR_R = 0x0E;          // 5) PF4,PF0 in, PF3-1 out

  GPIO_PORTF_AFSEL_R = 0x00;        // 6) disable alt funct on PF7-0

  GPIO_PORTF_PUR_R = 0x11;          // enable pull-up on PF0 and PF4

  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital I/O on PF4-0

}


