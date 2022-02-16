//================= PIC CONFIGURATIONS =====================================
#pragma config OSC = HS 		//set osc mode to HS
#pragma config WDT = OFF 		// set watchdog timer off
#pragma config LVP = OFF 		// Low Voltage Programming Off
#pragma config DEBUG = OFF 		// Compile without extra Debug compile Code
#pragma config PBADEN = OFF  		// Analog I/O port B dissabled
#pragma config MCLRE  = ON		// Master Clear enabled

//================= INCLUDED LIBRARIES =====================================
#include <p18f4520.h> 				
#include <XLCD.h>	                		
#include <delays.h> 
#include <adc.h> 
#include <stdlib.h>
#include <math.h>	
#include <usart.h>
#include<timers.h>	

//==================== DEFINITIONS =========================================
#define CLR_LCD 1			// Clear LCD command code
#define HOME_LCD 2			// Cursor home command code
#define LINE2_LCD 0xC0			// Position cursor on line 2 command
#define CURS_OFF 0x0c			// Turn Cursor off
#define CURS_L 0x10			// Move Cursor left
#define CURS_R 0x14			// Move Cursor right
#define SHIFT_L 0x18			// Scroll display left
#define SHIFT_R 0x1C			// Scroll display right

#define VAULT PORTCbits.RC4
#define LED_YELLOW PORTCbits.RC0
#define LED_ORANGE PORTCbits.RC1
#define LED_RED PORTCbits.RC2
#define BUZZER PORTCbits.RC5

//==================== VARIABLES =========================================
char inputPin[4], attemptsInChar;
int attempts = 0, n = 0, x = 0, i = 0,  b, c, d, e;
unsigned int a = 0, int_extract;
unsigned char pin1, pin2, pin3, termChar;
unsigned char addr1, addr2, addr3, data1, data2, data3;
char ASCII_dig1, ASCII_dig2, ASCII_dig3, ASCII_dig4;
float result;

//==================== FLAGS ==============================================
int PinIsSet = 0, isUnlocked = 0, isTerminal = 0, isOwner = 0, isOnFire = 0, isLocked = 0;

//==================== FUNCTION DECLARATIONS ==============================
unsigned char EEPROM_READ(unsigned char address);
void EEPROM_Write(unsigned char address, unsigned char datatype);
void firstActivation(void);
void setPIN(int num);
void storePIN(void);
void readPIN(void);
void checkPIN(void);
void failedAttempt(int attempts);
void showTemp(void);
void termInpPIN(void);
void triggerTerminal(void);
void timer5Sec(void);
void TMR0_ISR(void);
void test_isr(void);
void curPos(void);
void onFire(void);
void calTemp(void);

// ----- Bellow we test for the source of the interrupt ----          
     
#pragma code my_interrupt = 0x08  // High Interrupt vector @ 0x08      
void my_interrupt (void)          // At location 0x08 instruction GOTO  
   {  _asm     			  // This is the assembly code at vector location 
      GOTO test_isr 
      _endasm 
   } 
#pragma code 
   
#pragma interrupt test_isr     
void test_isr (void)    
   {  if (INTCONbits.TMR0IF == 1)  // Was interrupt caused by Timer 0? 
   { 
      TMR0_ISR ();                 // Yes , execute TMR0 ISR program
   }  
   }   

//==================== MAIN FUNCTION =====================================
void main (void)
{
TRISA = 0xFF;
ADCON1 = 0xFF; 			// All ports I/Ps set to digital
INTCON2bits.RBPU = 0; 		// Set Internal port B Pull Ups
LATD = 0x00; 			// Initialise Port D
LATC = 0x00;			// Initialise Port C
TRISB = 0x0F; 			// Most Significant Nibble of Port B as toutputs, and Least to inputs
TRISD = 0x00; 			// Port D as output port
TRISC = 0x80;			// RC0-RC6 as output port and RC7 as input

// Below are memory locations for the three-digit pin
addr1 = 0x01;
addr2 = 0x02;
addr3 = 0x03;

OpenXLCD(FOUR_BIT & LINES_5X7);		// Use 4 bit Data, 5x7 pixel per char
while(BusyXLCD());			// Wait for LCD to finish
WriteCmdXLCD(CURS_OFF);			// Turn cursor OFF
while(BusyXLCD());			// Wait for LCD to finish processing
WriteCmdXLCD(SHIFT_DISP_LEFT);		// Shift Cursor Display Left
while(BusyXLCD());			// Wait for LCD to finish processing

OpenADC( ADC_FOSC_32 & ADC_RIGHT_JUST &	ADC_20_TAD, ADC_CH0 & ADC_INT_OFF & ADC_VREFPLUS_EXT & ADC_VREFMINUS_VSS, 0b1011);
OpenUSART(USART_TX_INT_OFF & USART_RX_INT_OFF &	USART_ASYNCH_MODE &USART_EIGHT_BIT &USART_CONT_RX & USART_BRGH_HIGH, 25);

CloseTimer0 ();  // Close The Timer 0  if it was previously Open  
// Configure the Timer 0 with Interrupt, ON, 16 bits internal clock source Prescaler of 1:1
OpenTimer0 (TIMER_INT_ON & T0_16BIT & T0_SOURCE_INT & T0_PS_1_1);  
WriteTimer0 (0);

INTCONbits.GIE = 1;          // Enable global interrupts  
INTCONbits.TMR0IE = 1;       // Enable Timer 0 Interrupt  
INTCONbits.TMR0IF = 0;

if(PinIsSet == 0){ isOwner = 1; }

firstActivation();
  
while(1)
{
calTemp();
if(ASCII_dig1 > '0'){ isOnFire = 1; }

termChar = getcUSART();
if(termChar == 't' || termChar == 'T'){ isTerminal = 1; triggerTerminal(); }
else{ termChar = getcUSART(); }

// Read Port B Input switches and display on LEDs at Port D

/*------------------ COLUMN 1 ----------------------------*/
LATB = 0b11010000; 		// Column 1 firstly set to zero
Delay1KTCYx(1); 		// Debounce delay
if(PORTBbits.RB0==0){ char num = '1'; setPIN(num); }
if(PORTBbits.RB1==0){ char num = '4'; setPIN(num); }
if(PORTBbits.RB2==0){ char num = '7'; setPIN(num); }
if(PORTBbits.RB3==0){ char num = '*';
   if(n > 0){ n--; }
   WriteCmdXLCD(CLR_LCD);
   while(BusyXLCD());
   curPos();
   Delay10KTCYx(100);
}      

/*------------------ COLUMN 2 ----------------------------*/
LATB = 0b10110000; 		// Column 2 set to zero
Delay1KTCYx(1); 		// Debounce delay
if(PORTBbits.RB0==0){ char num = '2'; setPIN(num); }
if(PORTBbits.RB1==0){ char num = '5'; setPIN(num); }
if(PORTBbits.RB2==0){ char num = '8'; setPIN(num); }
if(PORTBbits.RB3==0){ char num = '0'; setPIN(num); }
 
/*------------------ COLUMN 3 ----------------------------*/
PORTB = 0b01110000;		// Column 3 set to zero
Delay1KTCYx(1); 		// Debounce delay
if(PORTBbits.RB0==0){ char num = '3'; setPIN(num); }
if(PORTBbits.RB1==0){ char num = '6'; setPIN(num); }
if(PORTBbits.RB2==0){ char num = '9'; setPIN(num); }
if(PORTBbits.RB3==0){ char num = '#';
   putcXLCD(num); while(BusyXLCD());
   if(isUnlocked == 1) { 
      VAULT = 0; 
      WriteCmdXLCD(CLR_LCD); putrsXLCD("LOCKED!"); Delay10KTCYx(100);
      putrsUSART("LOCKED!"); while(BusyUSART());
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("PIN:");
      isUnlocked = 0; } 
   else if(PinIsSet == 0){ storePIN(); }
   else{ checkPIN(); }        
   Delay10KTCYx(100); 
   n = 0; }
}
}

//==================== FUNCTION DEFINITIONS ==============================
void timer5Sec(void){
   calTemp();
   b++;
   if(isOnFire == 1){ d++; e = e + c; }
   if(isOnFire == 1){ onFire(); }
   if(c == 1){ c = 0; WriteCmdXLCD(LINE2_LCD); putrsXLCD("                "); WriteCmdXLCD(HOME_LCD); curPos(); }
   if(b == 6){ b = 0; c = 1; if(isOnFire == 0){ showTemp(); }}  
}

void onFire(void){  
   isUnlocked = 1;
   VAULT = 1;
   if(isOnFire == 1){
      if(d == 1){ BUZZER = 1; WriteCmdXLCD(CLR_LCD); putrsXLCD("T100! UNLOCKING!"); putrsUSART("T100! UNLOCKING!\r\n"); }
      if(d > 1){ d = 0; BUZZER = 0; WriteCmdXLCD(CLR_LCD); }
      if(e == 1){ isOnFire = 0; e = 0; d = 0;}
   }
   if(isOnFire == 0){d = 0; BUZZER = 0;}
}

void calTemp(void){
   ConvertADC();
   while(BusyADC( ));
   result = ReadADC(); 
   int_extract = (result / 5.115) * 10;
   ASCII_dig1 = (int_extract / 1000) + 0x30;; 			// e.g. 4235/1000 = 4
   ASCII_dig2 = ((int_extract % 1000) / 100) + 0x30; 		// e.g. (4235 % 1000) / 100 = 2
   ASCII_dig3 = (((int_extract % 1000) % 100) / 10) + 0x30;	// e.g. ((4235 % 1000) % 100) / 10 = 3
   ASCII_dig4 = (((int_extract % 1000) % 100) % 10) + 0x30;	// e.g. ((4235 % 1000) % 100) % 10 = 5
}

void triggerTerminal(void){
   putrsUSART("Type 'n' for new PIN or type 'o' to open the vault or 'e' to exit!\r\n");
   while(!DataRdyUSART());
   termChar = getcUSART();
   switch (termChar){
      case 'n': if(isOwner == 1){ PinIsSet = 0; termInpPIN();}
		else{ putrsUSART("*** Ownership is not verified! ***\r\n"); triggerTerminal(); } break;
      case 'o': if(PinIsSet == 0){putrsUSART("Create a PIN!");}; termInpPIN(); break;
      case 'e': isTerminal = 0; putrsUSART("Goodbye!\r\n"); break;
      case 'l': if(isUnlocked == 1){ putrsUSART("LOCKED!\r\n"); WriteCmdXLCD(CLR_LCD); while(BusyXLCD());
	        putrsXLCD("PIN:"); isUnlocked = 0; VAULT = 0; } else{ putrsUSART("ALREADY LOCKED!\r\n"); }; break;
   }
   termChar = ' ';
}

void termInpPIN(void){
   putrsUSART("\r\n"); while(BusyUSART());
   if(PinIsSet == 0){ putrsUSART("New PIN:"); }
   else{ putrsUSART("PIN:"); }
   for(i = 0; i < 3; i++){
      while(BusyUSART());
      while(!DataRdyUSART());
      inputPin[i] = getcUSART();
      putcUSART('*'); while(BusyUSART());
      n++;
   }
   putcUSART('#');
   putrsUSART("\r\n");
   if(PinIsSet == 0){ storePIN(); termInpPIN(); }
   else{ checkPIN(); }  
}

void showTemp(void){
   WriteCmdXLCD(LINE2_LCD);
   putrsXLCD("T");
   if(ASCII_dig1 == '0'){} else{ putcXLCD(ASCII_dig1); }
   putcXLCD(ASCII_dig2);
   putcXLCD(ASCII_dig3);
   putcXLCD('.');
   putcXLCD(ASCII_dig4);
   putrsXLCD(" DEGREE C");
   WriteCmdXLCD(HOME_LCD);
   curPos();
}

void curPos(void){
   if(isLocked == 0){
      if(isUnlocked == 0){
	 if(isOnFire == 0){
	    if(PinIsSet == 0){ putrsXLCD("New PIN:"); }
	    if(PinIsSet == 1){ putrsXLCD("PIN:"); }
	    switch(n){
	       case 1: 
	       while(BusyXLCD()); putrsXLCD("*"); break;
	       case 2: 
	       while(BusyXLCD()); putrsXLCD("**"); break;
	       case 3:
	       while(BusyXLCD()); putrsXLCD("***"); break;
	    }
	 }
      }
   }
}

void firstActivation(void){
   putrsXLCD("WELCOME!"); while(BusyXLCD());
   putrsUSART("Welcome! Type 'T' to activate terminal!\r\n");
   while(BusyUSART()) ;
   Delay10KTCYx(100);
   WriteCmdXLCD(CLR_LCD);
   if(PinIsSet == 0){ putrsXLCD("New PIN:"); while(BusyXLCD()); }
   else{ putrsXLCD("PIN:"); }  
}

void storePIN(void){
   if(n == 3){
   data1 = inputPin[0];
   EEPROM_Write(addr1, data1);
   data2 = inputPin[1];   
   EEPROM_Write(addr2, data2);
   data3 = inputPin[2];   
   EEPROM_Write(addr3, data3);
   Delay10KTCYx(100); 
   PinIsSet = 1;
   WriteCmdXLCD(CLR_LCD);
   putrsXLCD("PIN:");
   n = 0;
   }
   else {
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("New PIN:");
   }
}

void checkPIN(void){
   readPIN();
   if(pin1 == inputPin[0] && pin2 == inputPin[1] && pin3 == inputPin[2] && n == 3){
      n = 0;
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("UNLOCKING!");
      putrsUSART("UNOCKING!"); while(BusyUSART());
      Delay10KTCYx(100);
      VAULT = 1;
      Delay10KTCYx(100); 
      putrsUSART(" Press 'l' to LOCK!\r\n"); while(BusyUSART());
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("Press # to lock!");
      isUnlocked = 1;
      Delay10KTCYx(100);
      if(isTerminal == 1){ triggerTerminal(); }
      attempts = 0;
      isOwner = 1;
   }
   else{
      attempts++;   
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("ATTEMPT NO. ");
      attemptsInChar = attempts+ '0';
      putcXLCD(attemptsInChar);
      putrsUSART("ATTEMPT NO. "); while(BusyUSART());
      putcUSART(attemptsInChar); while(BusyUSART());
      putrsUSART("\r\n");
      if(isTerminal == 0){ putrsUSART("\r\n"); }
      failedAttempt(attempts);
      Delay10KTCYx(100); 
      WriteCmdXLCD(CLR_LCD);
      putrsXLCD("PIN:");
      if(attempts == 3){
	 WriteCmdXLCD(CLR_LCD);
	 isLocked = 1;
	 putrsXLCD("LOCKING KEYPAD!");
	 putrsUSART("\r\nLOCKING KEYPAD!\r\n"); while(BusyUSART());
	 failedAttempt(4);
	 WriteCmdXLCD(CLR_LCD);
	 putrsXLCD("KEYPAD LOCKED!");
	 putrsUSART("KEYPAD LOCKED!\r\n"); 
	 for(i =0; i<50; i++){ Delay10KTCYx(100); }
	 isLocked = 0;
	 WriteCmdXLCD(CLR_LCD);
	 putrsXLCD("PIN:"); 
	 putrsUSART("KEYPAD UNLOCKED!\r\n"); 
	 attempts = 0;
	 if(isTerminal == 1){ triggerTerminal(); }
      }
   } 
}



void setPIN(int num){
    putcXLCD('*'); 
    while(BusyXLCD());   
    inputPin[n] = num;
    n++;
    Delay10KTCYx(100);
}

void readPIN(void){
   pin1 = EEPROM_READ(addr1);        		
   pin2 = EEPROM_READ(addr2);             	
   pin3 = EEPROM_READ(addr3);
}

void failedAttempt(int attempts){
   switch(attempts){
      case 1: 
      LED_YELLOW = 1; BUZZER = 1; Delay1KTCYx(200);LED_YELLOW = 0; BUZZER = 0;Delay1KTCYx(200);
      LED_YELLOW = 1; BUZZER = 1; Delay1KTCYx(200);LED_YELLOW = 0; BUZZER = 0; Delay1KTCYx(200); break;
      case 2: 
      LED_YELLOW = 1; LED_ORANGE = 1 ; BUZZER = 1; Delay1KTCYx(200);LED_YELLOW = 0; LED_ORANGE = 0; BUZZER = 0; Delay1KTCYx(200);
      LED_YELLOW = 1; LED_ORANGE = 1 ; BUZZER = 1; Delay1KTCYx(200);LED_YELLOW = 0; LED_ORANGE = 0; BUZZER = 0; Delay1KTCYx(200); break;
      case 3: 
      LED_YELLOW = 1; LED_ORANGE = 1; LED_RED = 1; BUZZER = 1; Delay1KTCYx(200); LED_YELLOW = 0; LED_ORANGE = 0; LED_RED = 0; BUZZER = 0; Delay1KTCYx(200);
      LED_YELLOW = 1; LED_ORANGE = 1; LED_RED = 1; BUZZER = 1; Delay1KTCYx(200); LED_YELLOW = 0; LED_ORANGE = 0; LED_RED = 0; BUZZER = 0; Delay1KTCYx(200); break;
      case 4: 
      while(x<10){
	 x++; LED_YELLOW = 1; LED_ORANGE = 1; LED_RED = 1; BUZZER = 1; Delay1KTCYx(200); LED_YELLOW = 0; LED_ORANGE = 0; LED_RED = 0; BUZZER = 0; Delay1KTCYx(200); }
	 break;     
   }
}

unsigned char EEPROM_READ(unsigned char address)
{                                
    EEADR = address;                // Load EEADR with first passed argument
    EECON1bits.EEPGD = 0; 	    // 0 = Access data EEPROM memory        
    EECON1bits.CFGS  = 0; 	    // 0 = Access DATA EEPROM memory        
    EECON1bits.RD    = 1; 	    // Select EEPROM Read operations        
    return EEDATA;       	    // Fetch data from the specified address
}   

void EEPROM_Write(unsigned char address, unsigned char data) 
{                                
    unsigned char INTCON_STORE;                                     
    EEADR  = address;               // Load EEADR with first passed argument
    EEDATA = data;                  // Load EEDATA with second argument     
    EECON1bits.EEPGD = 0;           // 0 = Access data EEPROM memory        
    EECON1bits.CFGS  = 0; 	    // 0 = Access Flash memory              
    EECON1bits.WREN  = 1; 	    // Enable writing to internal EEPROM    
    INTCON_STORE = INTCON;          // Save INTCON register constants       
    INTCON = 0x00;                  // Disable interrupts                   
	                            //                                      
    EECON2 = 0x55;		    // Sequence for write to internal EEPROM
    EECON2 = 0xAA;		    // Sequence for write to internal EEPROM
                                    //                                      
    EECON1bits.WR = 1;  	    // Begin writing the data to EEPROM     
    INTCON = INTCON_STORE;          // Re-enable interrupts                 
    Nop();                          // Waste a single instruction cycle     
    while(PIR2bits.EEIF == 0){      // Wait till write operation complete   
        Nop();                      // Waste a single instruction cycle     
    }                               //                                      
    EECON1bits.WREN = 0;            // Disable writes to EEPROM on write    
    PIR2bits.EEIF   = 0;            // Clear EEPROM write complete flag.    
}

//================ CUSTOM DELAY FUNCTION 1 =================================
void DelayFor18TCY(void)            // Delay for 18 Instr. cycles using NOPs
{
_asm NOP _endasm		    
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
_asm NOP _endasm
}

//=================== CUSTOM DELAY FUNCTION 2 ==============================
void DelayPORXLCD(void)
{				// DelayPORXLCD = ~15 ms. LCD required
	Delay1KTCYx(75);
}

//=================== CUSTOM DELAY FUNCTION 2 ==============================
void DelayXLCD(void)
{               		// DelayXLCD = ~5 ms.     LCD required
	Delay1KTCYx(25);
}

//=================== TMR0_ISR FUNCTION ====================================
void TMR0_ISR(void)       	// This is the interrupt service routine    
   {  INTCONbits.TMR0IE = 0;    // Disable any interrupt within interrupts  
      a++;
      if(a > 73){ timer5Sec(); a= 0; }
      WriteTimer0 (0);    	// Clear Timer 
      INTCONbits.TMR0IE = 1;    // Re-enable TMR0 interrupts
      INTCONbits.TMR0IF = 0;    // Clear TMR0 flag before returning to main program
   } 