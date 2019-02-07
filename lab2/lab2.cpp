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

void init();
char read_char();
void write_char(char c);
void write_string(const char *out);
void write_stringln(const char *out);
void extract_two_numbers(int &left, int &right, bool echo);
void to_string(char *dest, int value);
int to_int(char *number_string, int length);

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
        /*
        while(((*UART_LSR >> 6) & 1) != 1);
        *UART_THR = out[i];
        */
        write_char(out[i]);
    }
    return;
}

void write_stringln(const char *out){
    for(int i = 0; out[i] != '\0'; i++){
        /*
        while(((*UART_LSR >> 6) & 1) != 1);
        *UART_THR = out[i];
        */
        write_char(out[i]);
    }
    write_char('\r');
    write_char('\n');
    /*
    while(((*UART_LSR >> 6) & 1) != 1);
    *UART_THR = '\r';
    while(((*UART_LSR >> 6) & 1) != 1);
    *UART_THR = '\n';
    */

    return;
}

void extract_two_numbers(int &left, int &right, bool echo){
    char first, second;
    char left_number_string[12], right_number_string[12];
    int left_number_index = 0, right_number_index = 0;
    bool negative = false, digit_read = false;

    while((first = read_char()) != '\n'){
        if(first >= '0' && first <= '9'){
            if(echo) write_char(first);
            digit_read = true;
            left_number_string[left_number_index++] = first;
        } else if(first == '-' && digit_read == false){
            if(echo) write_char(first);
            negative = !negative;
        } else if(first == ' ' && digit_read == true){
            if(echo) write_char(first);
            digit_read = false;
            negative = false;
            left_number_string[left_number_index] = '\0';
            break;
        } else if(first == ' '){
            if(echo) write_char(first);
        }
    }

    digit_read = false;
    negative = false;
    while((second = read_char()) != '\r'){
        if(second >= '0' && second <= '9'){
            if(echo) write_char(second);
            digit_read = true;
            right_number_string[right_number_index++] = second;
        } else if(second == '-' && digit_read == false ){
            if(echo) write_char(second);
            negative = !negative;
        } else if(second == ' '){
            if(echo) write_char(first);
        } else if(second == '\n' && digit_read == true){
            digit_read = false;
            negative = false;
            right_number_string[right_number_index] = '\0';
            break;
        }
    }

    left = to_int(left_number_string, left_number_index);
    right = to_int(right_number_string, right_number_index);

    return;
}

void to_string(char *dest, int value){
    int i = 0, length;
    char temp;
    while(value > 0){
        dest[i] = (value % 10) + '0';
        value /= 10;
        i++;
    }

    length = i;
    for(int j = 0; j < i; j++, i--){
        temp = dest[j];
        dest[j] = dest[i-1];
        dest[i-1] = temp;
    }

    dest[length] = '\0';

    return;
}

int to_int(char *number_string, int length){
    int multiplier = 1, sum = 0, temp;

    for(int i = length - 1; i >= 0; i--){
        temp = (number_string[i] - '0') * multiplier;
        sum += temp;
        multiplier *= 10;
    }

    return sum;
}