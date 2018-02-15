# Platypus Camp Badge

**In RECEIVE Mode:**
 - Green LEDs flash
 - Waits for flag to be received by IR sensor
 - If flag is received, switchs to SEND Mode
 - If corrupted flag is received, flashes red once

**In SEND Mode:**
 - Red LEDs flash
 - Every 10 seconds, sends flag via IR LED
 - After about 13 minutes, deletes flag from memory and switchs to RECEIVE Mode

**Parts list:**
 - 3 red LEDs
 - 3 green LEDs
 - 1 IR LED
 - 1 Attiny85
 - Resistors for each LED: 100ohm for IR and Red, 68ohm for green
 - IR sensor TSOP4838
 - CR2032 Battery holder
 - 3V CR2032 Battery
