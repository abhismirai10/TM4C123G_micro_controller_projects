#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "math.h"

//LED colors
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define RED2    0x4000
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GREEN2  0x2FA4
#define CYAN2   0x07FF

//LED display pins with port A and port B
#define RD (1U << 2)
#define WR (1U << 3)
#define RS (1U << 4)
#define CS (1U << 5) //(0=Command/1=Data)
#define RST (1U << 6)

#define D0 (1U << 0)
#define D1 (1U << 1)
#define D2 (1U << 2)
#define D3 (1U << 3)
#define D4 (1U << 4)
#define D5 (1U << 5)
#define D6 (1U << 6)
#define D7 (1U << 7)

void PLL_Init(void);
void PortA_Init(void);
void PortB_Init(void);
void delay_ms(uint32_t milliseconds);
void delay_1ms(uint32_t milliseconds);
void ILI9341_init(void);
void Address_set(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void writeCommand(uint8_t cmd);
void writeData(uint8_t data);
void LCD_write(uint8_t d);
void drawPixel(int16_t x, int16_t y, uint16_t color);

volatile int i;

int main(void) {
    PLL_Init();
    PortA_Init();
    PortB_Init();
    ILI9341_init();

    while (1) {
        for (i = 5; i < 90; i++) {
          drawPixel(i, 60, RED);
          drawPixel(i, 70, RED);
          drawPixel(i, 80, RED);
          drawPixel(i, 90, RED);
          drawPixel(i, 100, RED);
          drawPixel(i, 110, RED);
          drawPixel(i, 120, RED);
          drawPixel(i, 130, RED);
          drawPixel(i, 140, RED);
          drawPixel(i, 150, RED);
        }
    }
}

void PLL_Init(void) {
    // Code for PLL initialization
    // 1) bypass PLL while initializing
    SYSCTL_RCC_R |= SYSCTL_RCC_BYPASS;

    // 2) select the crystal value and oscillator source
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~0x000007C0)   // clear XTAL field, bits 10-6
                   + 0x00000540;   // 10101, configure for 16 MHz crystal
    SYSCTL_RCC_R &= ~SYSCTL_RCC_OSCSRC_M;  // configure for main oscillator source

    // 3) activate PLL by clearing PWRDN
    SYSCTL_RCC_R &= ~SYSCTL_RCC_PWRDN;

    // 4) set the desired system divider and the use of 400 MHz PLL
    SYSCTL_RCC_R |= SYSCTL_RCC_USESYSDIV;
    SYSCTL_RCC_R = (SYSCTL_RCC_R & ~SYSCTL_RCC_SYSDIV_M) + (4 << SYSCTL_RCC_SYSDIV_S); // clear system clock divider field and configure for 80 MHz clock (400 MHz / 5)

    // 5) wait for the PLL to lock by polling PLLLRIS
    while ((SYSCTL_RIS_R & SYSCTL_RIS_PLLLRIS) == 0) {};  // wait for PLLRIS bit

    // 6) enable use of PLL by clearing BYPASS
    SYSCTL_RCC_R &= ~SYSCTL_RCC_BYPASS;
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
void ILI9341_init(void) {
    // Reset the display
    GPIO_PORTA_DATA_R |= RST;
    delay_ms(5);
    GPIO_PORTA_DATA_R &= ~RST;
    delay_ms(15);
    GPIO_PORTA_DATA_R |= RST;
    delay_ms(15);

    //The below is just preparation for Write Cycle Seq.
    GPIO_PORTA_DATA_R |= CS;    //Chip-Select, Active Low Signal
    GPIO_PORTA_DATA_R |= WR;
    GPIO_PORTA_DATA_R |= RD;
    GPIO_PORTA_DATA_R &= ~CS;   //Chip-Select Active Low Signal

    // Send initialization commands
    writeCommand(0x01); // Software reset
    delay_ms(50);       // Wait for 50ms

    writeCommand(0xCF);
    writeData(0x00);
    writeData(0xC1);
    writeData(0x30);

    writeCommand(0xED);
    writeData(0x64);
    writeData(0x03);
    writeData(0x12);
    writeData(0x81);

    writeCommand(0xE8);
    writeData(0x85);
    writeData(0x00);
    writeData(0x78);

    writeCommand(0xCB);
    writeData(0x39);
    writeData(0x2C);
    writeData(0x00);
    writeData(0x34);
    writeData(0x02);

    writeCommand(0xF7);
    writeData(0x20);

    writeCommand(0xEA);
    writeData(0x00);
    writeData(0x00);

    writeCommand(0xC0); // Power control
    writeData(0x23);    // VRH[5:0]

    writeCommand(0xC1); // Power control
    writeData(0x10);    // SAP[2:0];BT[3:0]

    writeCommand(0xC5); // VCM control
    writeData(0x3E);
    writeData(0x28);

    writeCommand(0xC7); // VCM control2
    writeData(0x86);

    //Memory Access Control
    writeCommand(0x36); // Memory Access Control | DataSheet Page 127
    writeData(0x48);    //Adjust this value to get right color and starting point of x and y

    //Pixel Format Set
    writeCommand(0x3A);    //COLMOD: Pixel Format Set | DataSheet Page 134
    writeData(0x55);       //16 Bit RGB and MCU

    writeCommand(0xB1);
    writeData(0x00);
    writeData(0x18);

    writeCommand(0xB6); // Display Function Control
    writeData(0x08);
    writeData(0x82);
    writeData(0x27);

    writeCommand(0xF2); // 3Gamma Function Disable
    writeData(0x00);

    writeCommand(0x26); // Gamma curve selected
    writeData(0x01);

    writeCommand(0xE0); // Set Gamma
    writeData(0x0F);
    writeData(0x31);
    writeData(0x2B);
    writeData(0x0C);
    writeData(0x0E);
    writeData(0x08);
    writeData(0x4E);
    writeData(0xF1);
    writeData(0x37);
    writeData(0x07);
    writeData(0x10);
    writeData(0x03);
    writeData(0x0E);
    writeData(0x09);
    writeData(0x00);

    writeCommand(0xE1); // Set Gamma
    writeData(0x00);
    writeData(0x0E);
    writeData(0x14);
    writeData(0x03);
    writeData(0x11);
    writeData(0x07);
    writeData(0x31);
    writeData(0xC1);
    writeData(0x48);
    writeData(0x08);
    writeData(0x0F);
    writeData(0x0C);
    writeData(0x31);
    writeData(0x36);
    writeData(0x0F);

    //Sleep Out
    writeCommand(0x11);    //Sleep Out | DataSheet Page 245
    delay_ms(50);          //Necessary to wait 5msec before sending next command
    writeCommand(0x29);    //Display on.
    delay_ms(50);

    writeCommand(0x2c);    //Memory Write | DataSheet Page 245
}
void Address_set(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Set column address | DataSheet Page 110
    writeCommand(0x2A);
    writeData(x0 >> 8);
    writeData(x0);
    writeData(x1 >> 8);
    writeData(x1);

    // Set page address
    writeCommand(0x2B);
    writeData(y0 >> 8);
    writeData(y0);
    writeData(y1 >> 8);
    writeData(y1);

    writeCommand(0x2c); // REG 2Ch = Memory Write
}
void writeCommand(uint8_t cmd) {
    GPIO_PORTA_DATA_R &= ~RS;
    LCD_write(cmd);
}
void writeData(uint8_t data) {
    GPIO_PORTA_DATA_R |= RS;
    LCD_write(data);
}
void LCD_write(uint8_t d){
    GPIO_PORTA_DATA_R &= ~WR;
    GPIO_PORTB_DATA_R = d;
    GPIO_PORTA_DATA_R |= WR;
}
void drawPixel(int16_t x, int16_t y, uint16_t color) {
    GPIO_PORTA_DATA_R &= ~CS;   // Chip Select active
    Address_set(x,y, x + 1, y + 1 );
    writeCommand(0x2C);
    writeData(color >> 8);
    writeData(color);
}
