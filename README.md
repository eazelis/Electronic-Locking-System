# ELS
Electronic Locking System in Proteus with C language firmware

Documentation for main.c in the ELS Repository
This is the documentation for the main.c file in the ELS repository. The program in this file appears to be written for a microcontroller (possibly the PIC18F4520) and involves working with an LCD display, reading input from switches, temperature sensing, and communicating through a USART terminal.

Configuration and Global Variables

At the top of the file, the program sets up PIC configurations, such as oscillator mode, watchdog timer, low voltage programming, and debug mode. There are several included libraries for interfacing with specific microcontroller peripherals and performing various functions.

The program also defines commands for interacting with an LCD display, defines port assignments for various hardware components (like LEDs and a buzzer), and declares several global variables and flags for use throughout the program​1​.

Main Function

The main function first sets up the microcontroller's port configurations and initializes the LCD and ADC peripherals. It also initializes the USART for serial communication and sets up Timer0 for interrupt generation.

The function then enters an infinite loop where it checks the temperature, checks for USART inputs, and reads inputs from switches connected to Port B​1​.

Function Definitions

There are several function definitions in the code, each performing specific tasks:

timer5Sec(): This function checks the temperature and performs some actions if the system is on fire.

onFire(): This function handles the situation when the system is on fire. It unlocks the vault, triggers the buzzer, and displays messages on the LCD and USART terminal.

calTemp(): This function reads ADC values, converts them into a temperature reading, and stores the result as a series of ASCII digits.

triggerTerminal(): This function handles USART terminal interactions. It prompts the user for actions and handles the input commands accordingly.

termInpPIN(): This function handles PIN input from the USART terminal. It can either set a new PIN or check an entered PIN against a stored one.

showTemp(): This function displays the current temperature on the LCD

EEPROM_READ(): This function reads data from a specified address in the EEPROM

EEPROM_Write(): This function writes data to a specified address in the EEPROM

![image](https://user-images.githubusercontent.com/95705759/154262027-6ddfba34-a43e-44f9-afd6-7d0bc270dfc9.png)
PCB layout
![image](https://user-images.githubusercontent.com/95705759/154262155-e1f2c1e3-5cae-445d-ab0f-43e38266b0b8.png)
