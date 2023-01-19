# Bit-Banding

This example application demonstrates the use of the bit-banding
capabilities of the Cortex-M4F microprocessor.  All of SRAM and all of the
peripherals reside within bit-band regions, meaning that bit-banding
operations can be applied to any of them.  In this example, a variable in
SRAM is set to a particular value one bit at a time using bit-banding
operations (it would be more efficient to do a single non-bit-banded write;
this simply demonstrates the operation of bit-banding).
