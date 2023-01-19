# MSP432E4 Example Project to demonstrate GPIO rising edge DMA Request.

In this example the GPIO Port J Pin-0 is configured to generate a DMA Request on
 rising edge detect. When the user releases the USR_SW1 the GPIO for LED D2 is 
 toggled by the DMA writing to the GPIO Port N Data Register.
