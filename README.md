# Platypus Camp Badge

**In RECEIVE Mode (default):**
 - Green LEDs flash
 - Waits for flag to be received by IR sensor
 - If flag is received, switch to SEND Mode
 - If corrupted flag is received, flashes red once

**In SEND Mode:**
 - Red LEDs flash
 - Every 10 seconds, sends flag via IR LED
 - After about 7 minutes, delete flag from memory and switch to RECEIVE Mode

