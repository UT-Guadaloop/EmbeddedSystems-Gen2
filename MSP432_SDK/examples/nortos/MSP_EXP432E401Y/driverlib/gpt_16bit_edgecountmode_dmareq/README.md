# MSP432E4 Example project for Configuring a timer in 16-bit Edge Count mode with DMA Request

In this example, the timer is configured in Event Time mode. The GPIO port J Pull Up is enabled
 so that a switch SW2 press causes an event to be generated. The timer captures the falling edge
 and counts the event down from the load value to 0. When the counter reaches 0, it generates a
 DMA Request. The DMA request is used to copy data from one SRAM buffer to another in Basic Mode.