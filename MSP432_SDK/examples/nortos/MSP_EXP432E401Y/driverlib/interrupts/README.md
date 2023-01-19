# Interrupts

This example application demonstrates the interrupt preemption and
tail-chaining capabilities of Cortex-M4 microprocessor and NVIC.  Nested
interrupts are synthesized when the interrupts have the same priority,
increasing priorities, and decreasing priorities.  With increasing
priorities, preemption will occur; in the other two cases tail-chaining
will occur.  The currently pending interrupts and the currently executing
interrupt will be displayed on the UART; GPIO pins B3, L1 and L0 (the
GPIO on jumper J27 on the left edge of the board) will be asserted upon
interrupt handler entry and de-asserted before interrupt handler exit so
that the off-to-on time can be observed with a scope or logic analyzer to
see the speed of tail-chaining (for the two cases where tail-chaining is
occurring).
