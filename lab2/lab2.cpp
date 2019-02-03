/*
* Lab 2 - UART Driver
* Date: 15 February 2018
* Author: Casey Lemon
*/

char *const UART_BASE = (char *) 0x10000000;
const char *const UART_RBR = UART_BASE + 0;
char *const UART_THR = UART_BASE + 0;
char *const UART_LCR = UART_BASE + 3;
const char *const UART_LSR = UART_BASE + 5;
char *const UART_DLL = UART_BASE + 0;
char *const UART_DLM = UART_BASE + 1;

const int OSC = 18000000;
const int BAUD = 115200;
const char* STR_LITERAL = "\r\n";

void init();
char read_char();
void write_char(char c);
void write_string(const char *out);
void write_stringln(const char *out);
void extract_two_numbers(int &left, int &right, bool echo);
void to_string(char *dest, int value);

void init(){
    /* Calculate divisor */
    int divisor = (OSC / BAUD) - 1;

    /* Set DLAB to 1 */
    *UART_LCR |= 1 << 7;

    /* Set DLL and DLM */
    *UART_DLL = divisor & 0xff;
    *UART_DLM = (divisor >> 8) & 0xff;

    /* Clear DLAB to 0 */
    *UART_LCR &= ~(1 << 7);

    return;
}

char read_char(){
    char ret_char;
    /* Check if DR (Data Ready) bit at index 0 is set to 1 */
    if(((*UART_LSR >> 0) & 1) == 1){
        ret_char = *UART_RBR;
        return ret_char;
    }

    /* Return '\0' when UART RX FIFO is empty */
    return '\0';
}

void write_char(char c){
    /* Loop until TEMT (Transmitter Empty) bit at index 6 is set to 1 */
    while(((*UART_LSR >> 6) & 1) != 1);
    
    /* Write to the transmitter */
    *UART_THR = c;

    return;
}

void write_string(const char *out){
    for(int i = 0; out[i] != '\0'; i++){
        write_char(out[i]);
    }
    return;
}

void write_stringln(const char *out){
    write_string(out);
    write_string("\r\n");
    return;
}

void extract_two_numbers(int &left, int &right, bool echo){
    return;
}

void to_string(char *dest, int value){
    return;
}