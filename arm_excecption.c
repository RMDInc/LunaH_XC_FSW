#include <stdio.h>
#include "xil_exception.h"
static void DataAbortIntrHandler(void *CallBackRef);

int main()
{
    xil_printf("Zynq data abort example \n");
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
                                                                                (Xil_ExceptionHandler)DataAbortIntrHandler,
                                                                                NULL);
    Xil_In32(0xc0000000);
    return 0;
}

static void DataAbortIntrHandler(void *CallBackRef)
{

unsigned int addr;
                __asm__ __volatile__("\n\ldr %0, [sp,#44]"  : "=r" (addr));
                xil_printf("instruction address causing data abort is %x\n",addr-8);
}
