//Adam Syed
//syeda139
//400578109
//Deliverable 2 Code


#include <stdint.h>
#include <stdio.h>
#include "PLL.h"
#include "SysTick.h"
#include "uart.h"
#include "onboardLEDs.h"
#include "tm4c1294ncpdt.h"
#include "VL53L1X_api.h"

#define I2C_MCS_ACK             0x00000008
#define I2C_MCS_DATACK          0x00000008
#define I2C_MCS_ADRACK          0x00000004
#define I2C_MCS_STOP            0x00000004
#define I2C_MCS_START           0x00000002
#define I2C_MCS_ERROR           0x00000002
#define I2C_MCS_RUN             0x00000001
#define I2C_MCS_BUSY            0x00000001
#define I2C_MCR_MFE             0x00000010

#define STEPS_PER_REV_FULLSTEP   512U
#define NUM_SCAN_POINTS          32U
#define STEPS_PER_SCAN           (STEPS_PER_REV_FULLSTEP / NUM_SCAN_POINTS)
#define ANGLE_PER_SCAN_DEG       11.25f
#define STEPPER_DELAY_MS         4U
#define SENSOR_SETTLE_MS         120U
#define NUM_TOTAL_SCANS          3U

static const uint8_t FULLSTEP_SEQ[4] = {0x03, 0x06, 0x0C, 0x09};

uint16_t dev = 0x29;
int status = 0;

void I2C_Init(void){
    SYSCTL_RCGCI2C_R |= SYSCTL_RCGCI2C_R0;
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R1;
    while((SYSCTL_PRGPIO_R & 0x0002) == 0){}

    GPIO_PORTB_AFSEL_R |= 0x0C;
    GPIO_PORTB_ODR_R   |= 0x08;
    GPIO_PORTB_DEN_R   |= 0x0C;
    GPIO_PORTB_PCTL_R   = (GPIO_PORTB_PCTL_R & 0xFFFF00FF) + 0x00002200;

    I2C0_MCR_R  = I2C_MCR_MFE;
    I2C0_MTPR_R = 0b0000000000000101000000000111011;
}

void PortG_Init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R6;
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R6) == 0){}

    GPIO_PORTG_DIR_R   &= ~0x01;
    GPIO_PORTG_AFSEL_R &= ~0x01;
    GPIO_PORTG_DEN_R   |= 0x01;
    GPIO_PORTG_AMSEL_R &= ~0x01;
}

void VL53L1X_XSHUT(void){
    GPIO_PORTG_DIR_R  |= 0x01;
    GPIO_PORTG_DATA_R &= ~0x01;
    SysTick_Wait10ms(1);
    GPIO_PORTG_DIR_R  &= ~0x01;
}

void PortH_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x80;
    while((SYSCTL_PRGPIO_R & 0x80) == 0){}

    GPIO_PORTH_DIR_R   |= 0x0F;
    GPIO_PORTH_AFSEL_R &= ~0x0F;
    GPIO_PORTH_DEN_R   |= 0x0F;
    GPIO_PORTH_AMSEL_R &= ~0x0F;
    GPIO_PORTH_DATA_R  &= ~0x0F;
}

//PJ1 pushbutton init 
void PortJ_Init(void){
    SYSCTL_RCGCGPIO_R |= 0x100;
    while((SYSCTL_PRGPIO_R & 0x100) == 0){}

    GPIO_PORTJ_DIR_R   &= ~0x02;   // PJ1 input
    GPIO_PORTJ_AFSEL_R &= ~0x02;
    GPIO_PORTJ_DEN_R   |= 0x02;
    GPIO_PORTJ_AMSEL_R &= ~0x02;
    GPIO_PORTJ_PUR_R   |= 0x02;    // enable pull-up
}

 // PJ1 press and release
void WaitForPJ1Press(void){
    UART_printf("Waiting for PJ1 press to begin scans...\r\n");

    // Wait until button is pressed (active low)
    while((GPIO_PORTJ_DATA_R & 0x02) != 0){
    }

    VL53L1_WaitMs(dev, 20); // debounce

    // Wait until button is released
    while((GPIO_PORTJ_DATA_R & 0x02) == 0){
    }

    VL53L1_WaitMs(dev, 20); // debounce
}

void Stepper_Output(uint8_t pattern){
    GPIO_PORTH_DATA_R = (GPIO_PORTH_DATA_R & ~0x0F) | (pattern & 0x0F);
}

void Stepper_Off(void){
    GPIO_PORTH_DATA_R &= ~0x0F;
}

void Stepper_RotateCW(uint16_t steps){
    uint16_t s;
    uint8_t idx;

    for(s = 0; s < steps; s++){
        for(idx = 0; idx < 4; idx++){
            Stepper_Output(FULLSTEP_SEQ[idx]);
            VL53L1_WaitMs(dev, STEPPER_DELAY_MS);
        }
    }
}

void Stepper_RotateCCW(uint16_t steps){
    uint16_t s;
    int8_t idx;

    for(s = 0; s < steps; s++){
        for(idx = 3; idx >= 0; idx--){
            Stepper_Output(FULLSTEP_SEQ[idx]);
            VL53L1_WaitMs(dev, STEPPER_DELAY_MS);
        }
    }
    Stepper_Off();
}

void WaitForToFBoot(void){
    uint8_t sensorState = 0;

    while(sensorState == 0){
        status = VL53L1X_BootState(dev, &sensorState);
        VL53L1_WaitMs(dev, 10);
    }
}

uint16_t ToF_ReadDistance(uint8_t *rangeStatus){
    uint8_t dataReady = 0;
    uint16_t distance = 0;

    while(dataReady == 0){
        status = VL53L1X_CheckForDataReady(dev, &dataReady);
        VL53L1_WaitMs(dev, 5);
    }

    status = VL53L1X_GetRangeStatus(dev, rangeStatus);
    status = VL53L1X_GetDistance(dev, &distance);
    status = VL53L1X_ClearInterrupt(dev);

    return distance;
}

//One full scan, then return motor to home position - will repeat 3 times
void PerformSingleScan(uint8_t scanNumber){
    uint8_t i;
    uint8_t rangeStatus;
    uint16_t distance;
    float angleDeg;

    sprintf(printf_buffer, "\r\nStarting scan %u of %u\r\n", scanNumber, NUM_TOTAL_SCANS);
    UART_printf(printf_buffer);

    UART_printf("SCAN_START\r\n");
    UART_printf("index,angle_deg,range_status,distance_mm\r\n");

    for(i = 0; i < NUM_SCAN_POINTS; i++){
        angleDeg = i * ANGLE_PER_SCAN_DEG;
        distance = ToF_ReadDistance(&rangeStatus);

        sprintf(printf_buffer, "%u,%.2f,%u,%u\r\n",
                i,
                angleDeg,
                rangeStatus,
                distance);
        UART_printf(printf_buffer);

        FlashLED2(1);	//Measurement Status LED
        VL53L1_WaitMs(dev, 50);

        // Rotate only between points, not after the final point 
        if(i < NUM_SCAN_POINTS - 1){
            Stepper_RotateCW(STEPS_PER_SCAN);
            VL53L1_WaitMs(dev, SENSOR_SETTLE_MS);
        }
    }

    UART_printf("SCAN_END\r\n");
    FlashLED3(1);	//Additional LED - End of Scan

    // Return to initial/home position after this scan 
    Stepper_RotateCCW(STEPS_PER_SCAN * (NUM_SCAN_POINTS - 1));
    VL53L1_WaitMs(dev, SENSOR_SETTLE_MS);

    sprintf(printf_buffer, "Scan %u complete. Returned to initial position.\r\n", scanNumber);
    UART_printf(printf_buffer);
}

int main(void){
    uint8_t scanNum;

    PLL_Init();
    SysTick_Init();
    onboardLEDs_Init();
    I2C_Init();
    UART_Init();
    PortH_Init();
    PortG_Init();
    PortJ_Init();

    UART_printf("Program Begins\r\n");
    UART_printf("System initializing only. No scan on reset.\r\n");

    WaitForToFBoot();
    UART_printf("ToF Booted\r\n");

    status = VL53L1X_ClearInterrupt(dev);
    status = VL53L1X_SensorInit(dev);
    Status_Check("SensorInit", status);

    status = VL53L1X_StartRanging(dev);
    Status_Check("StartRanging", status);

    FlashLED4(3);	//UART Tx LED

    // Do nothing until PJ1 is pressed 
    WaitForPJ1Press();

    UART_printf("PJ1 pressed. Beginning 3-scan routine.\r\n");

    for(scanNum = 1; scanNum <= NUM_TOTAL_SCANS; scanNum++){
        PerformSingleScan(scanNum);
    }

    status = VL53L1X_StopRanging(dev);
    Status_Check("StopRanging", status);

    UART_printf("All 3 scans complete. Motor returned home after each scan.\r\n");
    UART_printf("Program finished.\r\n");

    while(1){}
}



/*

// BUS CLOCK TEST PROGRAM
// Toggles PK0 every 18,000,000 bus cycles using SysTick_Wait.
// If bus clock = 36 MHz, output frequency should be about 1 Hz.

#include <stdint.h>
#include "tm4c1294ncpdt.h"
#include "PLL.h"
#include "SysTick.h"

void PK0_TestPin_Init(void){
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R9;            // enable Port K clock
    while((SYSCTL_PRGPIO_R & SYSCTL_PRGPIO_R9) == 0){}  // wait until ready

    GPIO_PORTK_DIR_R   |= 0x01;     // PK0 output
    GPIO_PORTK_AFSEL_R &= ~0x01;    // GPIO
    GPIO_PORTK_DEN_R   |= 0x01;     // digital enable
    GPIO_PORTK_AMSEL_R &= ~0x01;    // no analog
    GPIO_PORTK_PCTL_R  &= ~0x0000000F;
}

int main(void){
    PLL_Init();      // should set bus clock to 32 MHz
    SysTick_Init();
    PK0_TestPin_Init();

    while(1){
        GPIO_PORTK_DATA_R ^= 0x01;   // toggle PK0
        SysTick_Wait(18000000);      // 18,000,000 bus cycles
    }
}
*/