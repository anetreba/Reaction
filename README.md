# Reaction🌡️💡

##Devices
LAUNCHXL-CC1352R1, BOOSTXL-SENSORS

##Description
It is a program that filter(moving average filter) a raw data from sensors OPT3001 and BME280. If the temperature is lower than 30 degrees - red led is off, if the temperature is higher that 34 - red led is on, if the lux is lower that 15 - green led is on, if the lux is higher that 25 - green led is off.

##Building the program
1. Download/Clone the source code.
2. Open CodeComposerStudio.
3. Build the program.
4. Connect device.
5. Open UnitFlash program and find device.
6. In UnitFlash (find button Program) -> Flash Image -> Browse -> i2copt3001_CC1352R1_LAUNCHXL_tirtos_ccs (dir) -> Debug (dir) -> choose i2copt3001_CC1352R1_LAUNCHXL_tirtos_ccs.out(file).
7. In UnitFlash Load Image
8. Well done & good luck!😎


