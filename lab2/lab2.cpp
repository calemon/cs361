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
char *const UART_DLM = UART_BASE + 0;

void init();
char read_char();
void write_char(char c);
void write_string(const char *out);
void write_stringln(const char *out);
void extract_two_numbers(int &left, int &right, bool echo);
void to_string(char *dest, int value);

void init(){
    return;
}

char read_char(){
    return 'c';
}

void write_char(char c){
    return;
}

void write_string(const char *out){
    return;
}

void write_stringln(const char *out){
    return;
}

void extract_two_numbers(int &left, int &right, bool echo){
    return;
}

void to_string(char *dest, int value){
    return;
}