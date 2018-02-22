<a name="power"></a>

Modify empeg's power connection to car if needed
==============================================================================
I recommend that for this installation, you connect the empeg's wiring harness
to your car differently than if it were a standard car installation.

This design is intended for the empeg to be a secondary Bluetooth input
into a modern car stereo, rather than the car's main stereo system. In this
situation, the empeg's sleep mode, the empeg's "memory" power connection, the
way the BlueGigaEmpeg module gets its power from the empeg tuner connector,
and the way the car stereo issues Bluetooth commands when you shut off the
ignition, all combine to cause some interesting problems with power state
transitions. At least they did in my car, your mileage may vary.

I found that if I connected the empeg to my car via the regular method (i.e.
car accessory power to the orange ignition wire on the sled, and constant 12v
power to the yellow memory wire on the sled) then there were certain unusual
states that it could get into. Sometimes I would shut off my car, but the
empeg would come back up out of sleep mode and play tracks silently to an
unconnected Bluetooth module, draining my car's battery.

If that problem happens to you, then I recommend connecting power from the
car to the empeg like this instead:

- Orange "ignition" wire from empeg: Connect to car 12v accessory power.
- Yellow "memory" wire from empeg: Also connect to car 12v accessory power (instead
  of the usual constant power).
- Do *not* connect your car's constant 12v power to any part of the empeg.
- White "lights on" wire on empeg: Connect to car dash illumination power.
- Connect ground to ground, of course.
- Tuner module and serial plug connectors go to the BlueGigaEmpeg module.

That should be all you need. No audio or amplifier connections because the
Bluetooth takes care of that now. No cellphone mute wire because the Bluetooth
takes care of that now. No amp remote wire because there is no amp connected
directly to the empeg. No aux connection to the empeg, etc.

In this situation, the empeg does not receive a 12v constant power source at
all, and will completely lose all power when you turn off the ignition, rather
than going into sleep mode. The empeg and its attached BlueGigaEmpeg module
will always power on and off at the same time as the car stereo, and nothing
will get confused about power state. You might lose the date/time information
on the empeg sometimes, but your modern car likely has a perfectly functional
clock of its own.


