/*
 * Prjct_PWM_Ex
 * Example of using two PWMs using TI DriverLib in
 * preparation for the robot project.
 *
 * Specifically, use PWm to control brightness of the RGB LED on the dev board.
 *
 */

/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

typedef enum RGB_Opt { RED, GREEN, BLUE} RGB_Opt;


Timer_A_PWMConfig pwmConfigRed =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 512,
 TIMER_A_CAPTURECOMPARE_REGISTER_1,
 TIMER_A_OUTPUTMODE_RESET_SET,
 0
};

Timer_A_PWMConfig pwmConfigGreen =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 512,
 TIMER_A_CAPTURECOMPARE_REGISTER_2,
 TIMER_A_OUTPUTMODE_RESET_SET,
 0
};

Timer_A_PWMConfig pwmConfigBlue =
{
 TIMER_A_CLOCKSOURCE_SMCLK,
 TIMER_A_CLOCKSOURCE_DIVIDER_1,
 512,
 TIMER_A_CAPTURECOMPARE_REGISTER_3,
 TIMER_A_OUTPUTMODE_RESET_SET,
 0
};

int main(void)
{
    /* Stop Watchdog  */
    MAP_WDT_A_holdTimer();

    // Configure peripheral clock.
    MAP_CS_setReferenceOscillatorFrequency(CS_REFO_128KHZ);
    MAP_CS_initClockSignal(CS_SMCLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    // Map pins for LED to timer signals.
    // https://e2e.ti.com/support/microcontrollers/msp430/f/166/t/427197
    // Configure Pins. I need Pin2.0, Pin2.1, and Pin2.3 to be set to PWM.
    // Using CCR0A broke things. I have a feeling that generate PWM has clock in up mode?
    const uint8_t port_mapping[] = {  PM_TA0CCR1A, PM_TA0CCR2A, PM_TA0CCR3A, PM_NONE, PM_NONE, PM_NONE, PM_NONE, PM_NONE };
    MAP_PMAP_configurePorts((const uint8_t *) port_mapping, PMAP_P2MAP, 1, PMAP_DISABLE_RECONFIGURATION);
    MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN0|GPIO_PIN1|GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

    // Setup interrupts on buttons.
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN1 | GPIO_PIN4);
    MAP_Interrupt_enableInterrupt(INT_PORT1);

    // Use LED2 for Debug
    MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);
    MAP_GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);

    // Start base PWM
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigBlue);
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigGreen);
    MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigRed);

    MAP_Interrupt_enableMaster();

      while(1){};
}

void PORT1_IRQHandler(void)
{
    static RGB_Opt opt = BLUE;
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P1);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P1, status);

    if (status & GPIO_PIN1)
    {
        MAP_GPIO_toggleOutputOnPin(GPIO_PORT_P1, GPIO_PIN0);
    }



    switch(status)
    {
    case GPIO_PIN1:
        // increment option
        opt = (opt + 1) % 3;
        break;
    case GPIO_PIN4:
        // Increment duty by 10.
        switch(opt)
        {
        case RED:
            pwmConfigRed.dutyCycle = (pwmConfigRed.dutyCycle + 20) % 512;
            break;
        case GREEN:
            pwmConfigGreen.dutyCycle = (pwmConfigGreen.dutyCycle + 20) % 512;
            break;
        case BLUE:
            pwmConfigBlue.dutyCycle = (pwmConfigBlue.dutyCycle + 20) % 512;
            break;
        }

        MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigBlue);
        MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigGreen);
        MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigRed);
        break;
    }
}
