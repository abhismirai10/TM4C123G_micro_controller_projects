

/**
 * main.c
 */

// to define registers for port E
#define GPIO_PORTE_DATA_R       (*((volatile unsigned long *)0x400243FC))
#define GPIO_PORTE_DIR_R        (*((volatile unsigned long *)0x40024400))
#define GPIO_PORTE_AFSEL_R      (*((volatile unsigned long *)0x40024420))
#define GPIO_PORTE_DEN_R        (*((volatile unsigned long *)0x4002451C))
#define GPIO_PORTE_AMSEL_R      (*((volatile unsigned long *)0x40024528))
#define GPIO_PORTE_PCTL_R       (*((volatile unsigned long *)0x4002452C))
#define SYSCTL_RCGC2_R          (*((volatile unsigned long *)0x400FE108))

//to define variables and subroutines
unsigned long arm,sensor;
void delayms(unsigned long ms);
void PortE_Init(void);

int main(void)
{
    PortE_Init();    // to set registers of port E
    while(1){
        arm = GPIO_PORTE_DATA_R & 0x04;     //reading from PE2
        sensor = GPIO_PORTE_DATA_R & 0x03;  //reading from PE0 and PE1

        if ((arm==0x04)&&(sensor!=03)){     //check for swich position
            GPIO_PORTE_DATA_R ^= 0x10;      //alert
            delayms(100);                   //call delay function
        }
        else{
            GPIO_PORTE_DATA_R &= ~0x10;      //off alert
        }
    }
}

void PortE_Init(void){

  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000010;     //  activate clock for Port E
  delay = SYSCTL_RCGC2_R;           //  allow time for clock to start
  GPIO_PORTE_DIR_R |= 0x10;         //  PE4 out
  GPIO_PORTE_DIR_R &= ~0x07;        //  PE0, PE1, PE2 in
  GPIO_PORTE_AFSEL_R &= ~0x17;      //  disable alt funct on PF pins that is used
  GPIO_PORTE_AMSEL_R &= ~0x17;      //  disable analog on PF pins that is used
  GPIO_PORTE_PCTL_R &= ~0x000F0FFF; //  bit for PE 4-0
  GPIO_PORTE_DEN_R |= 0x17;          // enable digital I/O on PE4-0

}

void delayms (unsigned long ms){
  unsigned long i;
  while(ms > 0){
      i = 16000;
      while(i > 0){
          i = i - 1;
      }
      ms = ms - 1;
  }
}
