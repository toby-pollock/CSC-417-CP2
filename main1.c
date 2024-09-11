#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>

char* current_char; //Current char of the standard input string we are reading
long long accumulator = 0; //Number of the 64 bit integer.

void error(){
    printf("ERROR: not a signed integer.\n");
    exit(1);
}

void consume(){
    current_char++;
}

//-1 for negative, 1 for positive
void signed_int(int sign){
    //There has to be at least one digit
    if (('0' <= *current_char) && (*current_char <= '9')){
        int temp = (*current_char - '0')* sign;
        accumulator += temp;
        consume();
    }
    else{
        error();
    }
    //Check for more digits while making sure it does not pass the 64 bit integer limit. I used long long type as a 64 bit integer.
    while (('0' <= *current_char) && (*current_char <= '9'))
    {
        if (accumulator > LLONG_MAX / 10 || accumulator < LLONG_MIN / 10){
            error();
        }
        accumulator *= 10;
        int temp = (*current_char - '0');
        if ((sign > 0 && accumulator > LLONG_MAX - temp)){
            error();
        }
        else if(sign < 0 && accumulator < (LLONG_MIN + temp)){
            error();
        }
        accumulator += temp * sign;
        consume();
    }
    
}

void int64(){
    bool negative = false;
    if (*current_char == '-'){
        consume();
        negative = true;
    }
    if (negative){
        signed_int(-1);    
    }
    else{
        signed_int(1);
    }
}

int main(int argc, char const *argv[])
{
    char* code = malloc(20 * sizeof(char));
    printf("Enter a signed 64 bit integer: ");
    scanf("%s", code);
    if(strlen(code) == 0){
        printf("ERROR: The number cannot be an empty string.\n");
        exit(1);
    }
    current_char = code;
    int64();
    //Make sure there are no strange characters in the integer we are reading.
    if (*current_char != '\0') {
        error();
    }
        
    printf("The number is: %lld\n", accumulator);

}
