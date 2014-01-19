# HWreset

'HWreset' is a little ATmega8 microcontroller which uses the V-USB software stack to connect to your PC.
Its equipped with two low voltage singaling relays to emulate your power and reset switches on a secondary SUT (system under test).

I use this little tool to speed up my compile & debug cycle for x86 os development with [MetalSVM] (http://www.lfbs.rwth-aachen.de/content/765)

## Hardware

You might want to use an existing [USBasp] (http://www.fischl.de/usbasp/) AVR Programmer and reflash it with the provided firmware. The USBasp is also based on the V-USB stack and has pretty much everything we need conserning the hardware.
The firmware is compatible with the USBasp pin assignments.



