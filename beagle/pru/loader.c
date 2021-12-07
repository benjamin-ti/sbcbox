// Loads a PRU text.bin (and optionally data.bin) file,
// executes it, and waits for completion.
// This version enables interrupt event 15 (ecap timer)
//
// Usage:
// $ ./loader text.bin [data.bin]
//
// Compile with:
// gcc -o loader loader.c -lprussdrv
//
// Based on https://credentiality2.blogspot.com/2015/09/beaglebone-pru-gpio-example.html

#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define PRUSS_INTC_CUSTOM {   \
  { PRU0_PRU1_INTERRUPT, PRU1_PRU0_INTERRUPT, PRU0_ARM_INTERRUPT, PRU1_ARM_INTERRUPT, ARM_PRU0_INTERRUPT, ARM_PRU1_INTERRUPT,  15, (char)-1  },  \
  { {PRU0_PRU1_INTERRUPT,CHANNEL1}, {PRU1_PRU0_INTERRUPT, CHANNEL0}, {PRU0_ARM_INTERRUPT,CHANNEL2}, {PRU1_ARM_INTERRUPT, CHANNEL3}, {ARM_PRU0_INTERRUPT, CHANNEL0}, {ARM_PRU1_INTERRUPT, CHANNEL1}, {15, CHANNEL0}, {-1,-1}},  \
  {  {CHANNEL0,PRU0}, {CHANNEL1, PRU1}, {CHANNEL2, PRU_EVTOUT0}, {CHANNEL3, PRU_EVTOUT1}, {-1,-1} },  \
  (PRU0_HOSTEN_MASK | PRU1_HOSTEN_MASK | PRU_EVTOUT0_HOSTEN_MASK | PRU_EVTOUT1_HOSTEN_MASK) /*Enable PRU0, PRU1, PRU_EVTOUT0 */ \
}

int main(int argc, char **argv) {
  if (argc != 2 && argc != 3) {
    printf("Usage: %s loader text.bin [data.bin]\n", argv[0]);
    return 1;
  }

  prussdrv_init();
  if (prussdrv_open(PRU_EVTOUT_0) == -1) {
    printf("prussdrv_open() failed\n");
    return 1;
  }

  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_CUSTOM;
  prussdrv_pruintc_init(&pruss_intc_initdata);

  printf("Executing program and waiting for termination\n");
  if (argc == 3) {
    if (prussdrv_load_datafile(0 /* PRU0 */, argv[2]) < 0) {
      fprintf(stderr, "Error loading %s\n", argv[2]);
      exit(-1);
    }
  }
  if (prussdrv_exec_program(0 /* PRU0 */, argv[1]) < 0) {
    fprintf(stderr, "Error loading %s\n", argv[1]);
    exit(-1);
  }

  // Wait for the PRU to let us know it's done
  prussdrv_pru_wait_event(PRU_EVTOUT_0);
  printf("All done\n");

  prussdrv_pru_disable(0 /* PRU0 */);
  prussdrv_exit();

  return 0;
}
