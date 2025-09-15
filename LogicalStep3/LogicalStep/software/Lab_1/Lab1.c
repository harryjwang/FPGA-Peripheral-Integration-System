/*
 * "Hello World" example.
 *
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example
 * designs. It runs with or without the MicroC/OS-II RTOS and requires a STDOUT
 * device in your system's hardware.
 * The memory footprint of this hosted application is ~69 kbytes by default
 * using the standard reference design.
 *
 * For a reduced footprint version of this template, and an explanation of how
 * to reduce the memory footprint for a given application, see the
 * "small_hello_world" template.
 *
 */

#include "system.h"                   // includes system configuration for NIOS II processor.
#include <stdio.h>                     // includes standard input/output functions for printing.
#include "sys/alt_irq.h"               // includes interrupt handling functions.
#include <io.h>                        // provides I/O access functions for PIO registers.

//data that will be outputted to the console
static int PULSE_PERIOD;                     // variable for storing pulse period.
static int PULSE_WIDTH;                // variable for pulse width (half of PULSE_PERIOD).
static int BG_TASK_CALLS_RUN = 0;      // counts number of background tasks executed.
static alt_u16 AVERAGE_LATENCY = 0;        // tracks average latency for each pulse.
static alt_u16 MISSED_PULSES = 0;      // counts missed pulses.
static alt_u16 MULTIPLE_PULSES = 0;       // counts multiple pulses detected simultaneously.

static int NUM_OF_BACKGROUND_TASKS = 0;   // tracks background tasks in polling.

/*
 * runBackgroundTask(): simulates workload, toggles LED 0, and increments task count.
 */
int runBackgroundTask() {
    alt_u32 masked_value = 0;
    masked_value = IORD(LED_PIO_BASE, 0) | (1);    // set LED 0 (task indicator).
    IOWR(LED_PIO_BASE, 0, masked_value);           // write updated LED state.

    int j;                                         // loop variable for task simulation.
    int x = 0;                                     // dummy return variable.
    int grainsize = 4;                             // task load size.
    int g_taskProcessed = 0;                       // counter for processing tasks.

    for(j = 0; j < grainsize; j++) {               // loop simulates task workload.
        g_taskProcessed++;                         // increment task counter.
    }
    ++BG_TASK_CALLS_RUN;                           // increment background task count.

    masked_value = IORD(LED_PIO_BASE, 0) & 0xFFFFFFFE; // clear LED 0.
    IOWR(LED_PIO_BASE, 0, masked_value);           // write updated LED state.
    return x;                                      // return dummy variable.
}

/*
 * handleInterrupt(void* context, alt_u32 id): handles interrupt by toggling LED 2 and sending response pulse.
 */
static void handleInterrupt (void* context, alt_u32 id) {
    alt_u32 masked_value = 0;
    masked_value = IORD(LED_PIO_BASE, 0) | (1 << 2);   // set LED 2 to on (interrupt indicator).
    masked_value = masked_value & 0xFFFFFFFE;          // ensure LED 0 is off.
    IOWR(LED_PIO_BASE, 0, masked_value);               // write updated LED state by sending statuses of LEDs 0 and 2

    IOWR(RESPONSE_OUT_BASE, 0, 1);                     // send to response signal with pulse high.
    IOWR(RESPONSE_OUT_BASE, 0, 0);                     // send to response signal with pulse low.

    masked_value = masked_value ^ (1 << 2);            // clear LED 2.
    IOWR(LED_PIO_BASE, 0, masked_value);               // write updated LED state.

    IOWR(STIMULUS_IN_BASE, 3, 0);                      // clear interrupt request.
}

/*
 * runInterruptTest(int period): runs interrupt-driven test by processing pulses and background tasks.
 */
int runInterruptTest(int period) {
    IOWR(EGM_BASE, 0, 0);                              // disable EGM for setup.

    PULSE_PERIOD = period;                                   // set current period.
    PULSE_WIDTH = period / 2;                          // calculate pulse width (half period).

	//writing to registers
    IOWR(EGM_BASE, 2, PULSE_PERIOD);                         // set EGM period.
    IOWR(EGM_BASE, 3, PULSE_WIDTH);                    // set EGM pulse width.
    IOWR(EGM_BASE, 0, 1);                              // enable EGM.

    while(1) {
        if (IORD(EGM_BASE, 1) == 0) {                  // check if EGM pulse train is done (no longer busy).
            break;                                     // exit loop if complete.
        }
        runBackgroundTask();                                  // run background tasks while waiting.
    }

	//get info from registers on EGM core
    AVERAGE_LATENCY = IORD(EGM_BASE, 4);                   // read average latency from EGM.
    MISSED_PULSES = IORD(EGM_BASE, 5);                 // read missed pulses from EGM.
    MULTIPLE_PULSES = IORD(EGM_BASE, 6);                  // read multiple pulse counts from EGM.

    IOWR(EGM_BASE, 0, 0);                              // disable EGM after test for next test

    printf("%d,%d,%d,%d,%d,%d\n", PULSE_PERIOD, PULSE_WIDTH, BG_TASK_CALLS_RUN, AVERAGE_LATENCY, MISSED_PULSES, MULTIPLE_PULSES);
    return 0;                                          // print results and return.
}

/*
 * characterize_background_tasks(): determines the maximum number of background tasks that can be executed within one pulse cycle without missing pulses.
 */
void characterize_background_tasks() {
    runBackgroundTask();  // run one background task for characterization.
    NUM_OF_BACKGROUND_TASKS = 1;  // start background task count.

    while (IORD(EGM_BASE, 1) == 1 && IORD(STIMULUS_IN_BASE, 0) == 1) {
        runBackgroundTask();
        ++NUM_OF_BACKGROUND_TASKS;
    }
    while (IORD(EGM_BASE, 1) == 1 && IORD(STIMULUS_IN_BASE, 0) == 0) {
        runBackgroundTask();
        ++NUM_OF_BACKGROUND_TASKS;
    }
    IOWR(RESPONSE_OUT_BASE, 0, 1);  // send response pulse high.
    IOWR(RESPONSE_OUT_BASE, 0, 0);  // send response pulse low.
    --NUM_OF_BACKGROUND_TASKS;  // adjust task count.
}

/*
 * runPollingTest(int period): runs polling-based test by counting tasks in pulse cycle and processing pulses.
 */
int runPollingTest(int period) {
    IOWR(EGM_BASE, 0, 0);                              // disable EGM for setup.

    PULSE_PERIOD = period;                                   // set current period.
    PULSE_WIDTH = period / 2;                          // calculate pulse width.

    IOWR(EGM_BASE, 2, PULSE_PERIOD);                         // set EGM period.
    IOWR(EGM_BASE, 3, PULSE_WIDTH);                    // set EGM pulse width.
    IOWR(EGM_BASE, 0, 1);                              // enable EGM for test

    while(IORD(EGM_BASE, 1) == 1 && IORD(STIMULUS_IN_BASE, 0) == 0) {
		// wait for the first pulse start
	}

	//acknowledge the first pulse
    IOWR(RESPONSE_OUT_BASE, 0, 1);                     // send response pulse high.
    IOWR(RESPONSE_OUT_BASE, 0, 0);                     // send response pulse low.

	// Start of characterization cycle - call existing function
	characterize_background_tasks();

	//run background tasks until the EGM is no longer busy
    while(IORD(EGM_BASE, 1) == 1) {                    // continue running background tasks.
        for (int i = 0; i < NUM_OF_BACKGROUND_TASKS; ++i) {
            runBackgroundTask();                              // run calculated background task count.
        }
        while(IORD(EGM_BASE, 1) == 1 && IORD(STIMULUS_IN_BASE, 0) == 0) {} // wait for next pulse.
        IOWR(RESPONSE_OUT_BASE, 0, 1);                 // send response pulse high.
        IOWR(RESPONSE_OUT_BASE, 0, 0);                 // send response pulse low.
    }

	//read important metrics from registers on the EGM core
    AVERAGE_LATENCY = IORD(EGM_BASE, 4);                   // read average latency from EGM.
    MISSED_PULSES = IORD(EGM_BASE, 5);                 // read missed pulses from EGM.
    MULTIPLE_PULSES = IORD(EGM_BASE, 6);                  // read multiple pulse counts.

    IOWR(EGM_BASE, 0, 0);                              // disable EGM after test as per the manual

    printf("%d,%d,%d,%d,%d,%d\n", PULSE_PERIOD, PULSE_WIDTH, BG_TASK_CALLS_RUN, AVERAGE_LATENCY, MISSED_PULSES, MULTIPLE_PULSES);
    return 0;                                          // print results and return.
}

/*
 * main(): runs polling or interrupt test based on SW0 switch and loops through periods from 2 to 5000.
 */
int main() {
    int mode = 0;                                      // mode variable, 0 = interrupt, 1 = polling.
    mode = IORD(SWITCH_PIO_BASE, 0) & 1;               // read mode from switch SW0.

    if (mode) {
        printf("Polling method selected.\n");          // output polling mode selection.
    } else {
        alt_irq_register( STIMULUS_IN_IRQ, (void *) 0, handleInterrupt); // register ISR for interrupt mode.
        IOWR(STIMULUS_IN_BASE, 2, 1);                  // enable interrupt requests at offset 2.
        printf("Interrupt method selected.\n");        // output interrupt mode selection.
    }

    printf("Period, Pulse_Width, BG_Tasks Run, Latency, Missed, Multiple\n"); // column headers.
    printf("Press PB0 to start.\n");                   // wait for button press to start.

    while ((IORD(PUSH_PIO_BASE, 0) & 1) != 0) {}       // wait for button PB0 to be pressed once

	//PB0 is now pressed, start with the test (either interrupt or polling mode)
    for (int i = 2; i <= 5000; i += 2) {               // loop from period 2 to 5000 in steps of 2.
        BG_TASK_CALLS_RUN = 0;                         // reset background task counter.
        AVERAGE_LATENCY = 0;                               // reset latency counter.
        MISSED_PULSES = 0;                             // reset missed pulse counter.
        MULTIPLE_PULSES = 0;                              // reset multiple pulse counter.

        alt_u32 masked_value = 0;

		//blink LED 1 once for every test
        masked_value = IORD(LED_PIO_BASE, 0) | (1 << 1); // set LED 1 for mode indication.
        IOWR(LED_PIO_BASE, 0, masked_value);           // write updated LED state.
        masked_value = IORD(LED_PIO_BASE, 0) ^ (1 << 1); // clear LED 1.
        IOWR(LED_PIO_BASE, 0, masked_value);           // write updated LED state.

        if (mode) {
            runPollingTest(i);                           // run polling test.
        } else {
            runInterruptTest(i);                         // run interrupt test.
        }
    }

    return 0;
}

