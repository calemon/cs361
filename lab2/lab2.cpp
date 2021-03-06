/*
* Lab 2 - UART Driver
* Date: 15 February 2018
* Author: Casey Lemon
*/

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;

char *const UART_BASE = (char *) 0x10000000;
const char *const UART_RBR = UART_BASE + 0;
char *const UART_THR = UART_BASE + 0;
char *const UART_LCR = UART_BASE + 3;
const char *const UART_LSR = UART_BASE + 5;
char *const UART_DLL = UART_BASE + 0;
char *const UART_DLM = UART_BASE + 1;

const uint32_t OSC = 18000000;
const uint32_t BAUD = 115200;

void init();
char read_char();
void write_char(char c);
void write_string(const char *out);
void write_stringln(const char *out);
void extract_two_numbers(int &left, int &right, bool echo);
void to_string(char *dest, int value);
int to_int(char *number_string, int length, bool is_negative);

void init(){
    /* Calculate divisor */
    uint32_t divisor = (OSC / BAUD) - 1;

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
    for(int i = 0; out[i] != '\0'; i++) write_char(out[i]);
    return;
}

void write_stringln(const char *out){
    for(int i = 0; out[i] != '\0'; i++) write_char(out[i]);
    write_char('\r');
    write_char('\n');

    return;
}

void extract_two_numbers(int &left, int &right, bool echo){
    char first, second;
    char left_number_string[12], right_number_string[12];
    int left_number_index = 0, right_number_index = 0;
    bool negative = false, digit_read = false;

    /* Begin reading the first number */
    while((first = read_char()) != '\n'){
        if(first >= '0' && first <= '9'){
            /* Only digits will be saved onto the appropiate char array */
            if(echo) write_char(first);
            digit_read = true;
            left_number_string[left_number_index++] = first;
        } else if(first == '-' && digit_read == false){
            /* A negative will be tracked if a digit has not been read yet */
            if(echo) write_char(first);
            negative = true;
        } else if(first == ' ' && digit_read == true){
            /* Once a space is read and at least one digit has been read, then break */
            if(echo) write_char(first);
            left_number_string[left_number_index] = '\0';
            break;
        } else if(first == ' '){
            if(echo) write_char(first);
        }
    }

    /* Convert the first entered number from a char array to an integer value */
    left = to_int(left_number_string, left_number_index, negative);

    /* Reset and do same procedure for second number */
    digit_read = false;
    negative = false;
    while((second = read_char()) != '\r'){
        if(second >= '0' && second <= '9'){
            if(echo) write_char(second);
            digit_read = true;
            right_number_string[right_number_index++] = second;
        } else if(second == '-' && digit_read == false){
            if(echo) write_char(second);
            negative = true;
        } else if(second == ' '){
            if(echo) write_char(first);
        } else if(second == '\n' && digit_read == true){
            right_number_string[right_number_index] = '\0';
            break;
        }
    }

    /* Convert right number to actual integer value */
    right = to_int(right_number_string, right_number_index, negative);

    return;
}

void to_string(char *dest, int value){
    int i = 0, length;
    char temp;
    bool is_negative = false;

    /* If the value is negative, set the flag and make value positive */
    if(value < 0){
        is_negative = true;
        value *= -1;
    } else if(value == 0){
        dest[i++] = '0';
    }
    
    /* Get each digit by taking the mod 10 of the value as a digit then divide value by 10 */
    while(value > 0){
        dest[i++] = (value % 10) + '0';
        value /= 10;
    }

    /* The above algorithm produces the number in reverse order, so reverse again to correct the order */
    length = i;
    for(int j = 0; j < i; j++, i--){
        temp = dest[j];
        dest[j] = dest[i-1];
        dest[i-1] = temp;
    }

    /* If the value was originally negative, then shift each character
     to the right one and set the first character to '-' */
    if(is_negative){
        for(int j = length; j >= 0; j--){
            dest[j+1] = dest[j];
        }
        dest[0] = '-';
        length++;
    }

    dest[length] = '\0';
    return;
}

/* Helper function to convert a number in a string format to it's integer equivalent */
int to_int(char *number_string, int length, bool is_negative){
    int multiplier = 1, sum = 0, temp;
    /* Going from the back to the front, multiply the digit at the current index
    by the multiplier (a factor of 10) and continuously sum up the result */
    for(int i = length - 1; i >= 0; i--){
        temp = (number_string[i] - '0') * multiplier;
        sum += temp;
        multiplier *= 10;
    }

    /* If the value was originally negative, make it negative again */
    if(is_negative) sum *= -1;
    return sum;
}