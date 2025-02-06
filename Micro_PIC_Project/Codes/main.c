#include <xc.h>
#include <stdio.h>
#include <string.h>
#include "lcd.h" // Include the LCD header file
#include "config.h"

#define _XTAL_FREQ 8000000 // Assuming 8MHz crystal frequency, adjust as needed

// Define keypad and LCD pins
#define KEYPAD_PORT PORTD
#define KEYPAD_DDR TRISD
#define R1 PORTDbits.RD4
#define R2 PORTDbits.RD5
#define R3 PORTDbits.RD6
#define R4 PORTDbits.RD7
#define C1 PORTDbits.RD0
#define C2 PORTDbits.RD1
#define C3 PORTDbits.RD2

// Define keypad constants
#define ROWS 4
#define COLS 3
char keys[ROWS][COLS] = {
    {'1', '2', '3'},
    {'4', '5', '6'},
    {'7', '8', '9'},
    {'*', '0', '#'}
};

char password[] = "1234"; // Change this to your desired password
char enteredPassword[5];   // Buffer to store entered password

void Initialize_UART(void);
void UART_send_char(char bt);
void UART_send_string(const char* st_pt);
void checkPassword();
char initKeypad();

void main() {
    OSCCON = 0x73;    // Set oscillator frequency
    TRISC = 0x80;     // RC6=0(TX)output, RC7=1(RX)input.
    Lcd_Init();       // Initialize the LCD
    TRISD = 0xF0;
    TRISBbits.RB3 = 0;
    TRISBbits.RB4 = 0;
    Initialize_UART();
    PORTBbits.RB3 = 0;
    PORTBbits.RB4 = 0;

    while (1) {
        checkPassword();
    }
}

// Initialize UART module
void Initialize_UART(void) {
    TRISC6 = 0;                         // TX pin as output
    TRISC7 = 1;                         // RX pin as input

    SPBRG = ((_XTAL_FREQ/16) / 9600) - 1; // Assuming baud rate of 9600

    BRGH = 1;                           // High baud rate select
    SYNC = 0;                           // Asynchronous mode
    SPEN = 1;                           // Enable serial port pins
    TXEN = 1;                           // Enable transmission
    CREN = 1;                           // Enable reception
    TX9 = 0;                            // 8-bit transmission
    RX9 = 0;                            // 8-bit reception
}

// Send a single character via UART
void UART_send_char(char bt) {
    while (!TXIF);      // Wait for transmit buffer empty
    TXREG = bt;         // Transmit data
}

// Send a string via UART
void UART_send_string(const char* st_pt) {
    while (*st_pt) {
        UART_send_char(*st_pt++);
    }
}

// Initialize keypad and return pressed key
char initKeypad() {
    C1 = 1;
    if (R1 == 1) {
        return '1';
    }
    if (R2 == 1) {
        return '4';
    }
    if (R3 == 1) {
        return '7';
    }
    if (R4 == 1) {
        return '*';
    }
    C1 = 0; C2 = 1;
    if (R1 == 1) {
        return '2';
    }
    if (R2 == 1) {
        return '5';
    }
    if (R3 == 1) {
        return '8';
    }
    if (R4 == 1) {
        return '0';
    }
    C1 = 0; C2 = 0; C3 = 1;
    if (R1 == 1) {
        return '3';
    }
    if (R2 == 1) {
        return '6';
    }
    if (R3 == 1) {
        return '9';
    }
    if (R4 == 1) {
        return '#';
    }

    return '\0'; // No key pressed
}

// Check entered password
void checkPassword() {
    int attemptCount = 3; // Number of allowed attempts

    while (attemptCount > 0) {
        Lcd_Clear();
        Lcd_Set_Cursor(1, 1);
        Lcd_Write_String("Enter Password:");

        // Move to second line
        memset(enteredPassword, 0, sizeof(enteredPassword)); // Clear entered password

        int enteredIndex = 0; // Index for entered password buffer
        char enteredChar;      // Temporary variable to store entered character

        while (enteredIndex < sizeof(enteredPassword) - 1) {
            enteredChar = initKeypad(); // Get pressed key
            C1 = 0;
            C2 = 0;
            C3 = 0;

            if (enteredChar != '\0') {
                enteredPassword[enteredIndex] = enteredChar;
                Lcd_Set_Cursor(2, enteredIndex + 1);
                Lcd_Write_Char('*'); // Display asterisk instead of key
                enteredIndex++;
            }
            __delay_ms(300); // Delay for debouncing
        }

        // Check entered password
        if (strcmp(enteredPassword, password) == 0) {
            Lcd_Clear(); // Clear the entire LCD before displaying success message
            Lcd_Set_Cursor(1, 1);
            Lcd_Write_String("Access Granted!");
            PORTDbits.RD3 = 1;
            PORTBbits.RB3 = 1;
            __delay_ms(100);
            PORTBbits.RB3 = 0;
            __delay_ms(100);
            PORTBbits.RB3 = 1;
            __delay_ms(100);
            PORTBbits.RB3 = 0;
            __delay_ms(5000);
            PORTDbits.RD3 = 0;

            __delay_ms(100);
            return; // Exit the function if access granted
        } else {
            Lcd_Clear();
            Lcd_Set_Cursor(1, 1);
            Lcd_Write_String("Access Denied!");
            char attemptsLeft[2]; // String to display remaining attempts
            sprintf(attemptsLeft, "%d", --attemptCount); // Decrement attempts and convert to string
            Lcd_Set_Cursor(1, 1);
            Lcd_Write_String("Attempts left: ");
            if (attemptCount > 0) {
                PORTBbits.RB3 = 1;
                __delay_ms(200);
                PORTBbits.RB3 = 0;
            } else {
                PORTBbits.RB3 = 1;
            }

            Lcd_Write_String(attemptsLeft);
            __delay_ms(2000);
        }
    }

    Lcd_Clear();
    Lcd_Set_Cursor(1, 1);
    Lcd_Write_String("Out of Attempts!");
    PORTBbits.RB4 = 1;
    __delay_ms(1000);
    PORTBbits.RB4 = 0;
    __delay_ms(6000);
    UART_send_string("Send Email");
    PORTBbits.RB3 = 0;
}
