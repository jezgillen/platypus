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

