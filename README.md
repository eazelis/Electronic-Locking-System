# ELS

This C program is a secure lock system for a vault controlled by a PIC18F4520 microcontroller. The system has the following features:

<b>Configuration Settings: </b> The configuration settings define the system's behavior, such as disabling the watchdog timer, setting the oscillator mode, and enabling Master Clear.

<b>Definitions and Variables: </b> Constants and variables needed for the program are defined. These include various control commands for the LCD display, pin assignments for the vault lock and LEDs, and flags to track the status of the system (e.g., whether the pin is set, the vault is locked/unlocked, etc.).

<b>Function Declarations: </b> A set of functions are declared that will be used to control the system's behavior. These include reading from and writing to EEPROM, setting and checking the PIN, handling failed attempts, displaying temperature, etc.

<b>Main Function: </b> The main function initializes the system, sets up the ADC and USART, and enters an infinite loop where it continuously checks the temperature, handles inputs from a terminal, and processes keypad inputs.

<b>Function Definitions: The various functions declared earlier are defined here. For instance, calTemp() converts the ADC reading to a temperature and displays it on the LCD, triggerTerminal() handles the terminal inputs, EEPROM_READ() and EEPROM_WRITE() read from and write to EEPROM, respectively, and so on.

In summary, this program implements a secure lock for a vault, with a keypad for entering a PIN, an LCD display for feedback, and a terminal for additional commands. It also has an overheating protection feature that unlocks the vault and raises an alarm if the temperature exceeds a certain threshold.

![image](https://user-images.githubusercontent.com/95705759/154262027-6ddfba34-a43e-44f9-afd6-7d0bc270dfc9.png)
PCB layout
![image](https://user-images.githubusercontent.com/95705759/154262155-e1f2c1e3-5cae-445d-ab0f-43e38266b0b8.png)
