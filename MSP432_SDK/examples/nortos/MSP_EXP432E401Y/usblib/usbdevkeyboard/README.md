---
# usbDevKeyboard

---

## Example Summary

 This example application turns the evaluation board into a USB keyboard
 supporting the Human Interface Device class.  When the push button is
 pressed, a sequence of key presses is simulated to type a string in an opened
 text file. Care
 should be taken to ensure that the active window can safely receive the
 text; enter is not pressed at any point so no actions are attempted by the
 host if a terminal window is used (for example).  The status LED is used to
 indicate the current Caps Lock state and is updated in response to any
 other keyboard attached to the same USB host system.

 The device implemented by this application also supports USB remote wakeup
 allowing it to request the host to reactivate a suspended bus.  If the bus
 is suspended (as indicated on the terminal window), pressing the push
 button will request a remote wakeup assuming the host has not specifically
 disabled such requests.

## Example Usage

>__Important:__ Adding the define USE_ULPI to the pre-processor defines allows the use of an external USB High Speed PHY.
