#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <cjson/cJSON.h>

char* current_char; //Current char of the standard input string we are reading
long long accumulator = 0; //Number of the 64 bit integer.

void error(){
    printf("ERROR.\n");
    exit(1);
}

void consume(){
    current_char++;
}

void skip_space(){
    while (*current_char == ' ' || *current_char == '\t' || *current_char == '\n' || *current_char == '\r'){
        consume();
    }
}

bool preshot_symbol(char* symbol){
    char* current_char_copy = current_char;
    skip_space();
    char buffer[100];
    for(int i=0; i<strlen(symbol); i++){
        if (*current_char == '\0'){
            return false;
        }
        buffer[i] = *current_char;
        consume();
    }
    buffer[strlen(symbol)] = '\0';
    current_char = current_char_copy; // Put back the current_char to its original position
    if(strcmp(buffer, symbol) == 0){ // If the buffer is equal to the symbol we are looking for
        return true;
    }
    return false;
}

//sign == -1 for negative, sign == 1 for positive
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
    if (*current_char == '-'){
        consume();
        signed_int(-1);  
    }
    else{
        signed_int(1);
    }
}

long long number(cJSON* number){
    return number->valuedouble;
}

long long identifier(char* identifier){
    if (strcmp(identifier, "x")==0){
        return 10;
    }
    else if (strcmp(identifier, "v")==0){
        return 5;
    }
    else if (strcmp(identifier, "i")==0){
        return 1;
    }
    else if (strcmp(identifier, "true")==0){
        return 1;
    }
    else if (strcmp(identifier, "false")==0){
        return 0;
    }
    else{
        error();
    }
}


long long application(char* identifier, long long arg1, long long arg2){
    if(strcmp(identifier, "add") == 0){
        return arg1 + arg2;
    }
    else if(strcmp(identifier, "sub") == 0){
        return arg1 - arg2;
    }
    else{
        error();
    }
}

//Go through json key value pairs recursively
long long readjsonrecursive(cJSON* json) {
    //print json object
    char* json_string = cJSON_Print(json);

    // Create a current cJSON to go through the key value pairs
    cJSON* current = NULL;

    if (cJSON_IsNumber(json)){
        return number(json);
    }

    // Go through the key value pairs of the json object
    cJSON_ArrayForEach(current, json){
        if (strcmp(current->string, "Application")==0){
            //An application contains AN ARRAY with an indentifier and two arguments
            //For now, I shall only allow applications with 2 arguments. That is why there is a counter that goes up to 2.
            int counter = 0;
            char identifier[100]; 
            long long arg1 = 0;
            long long arg2 = 0;
            cJSON* child = NULL;
            cJSON_ArrayForEach(child, current){
                if(counter > 2){
                    printf("ERROR: Too many arguments in the application.\n");
                    exit(1);
                }
                //Identifier of the application
                if(counter == 0){
                    cJSON* identifier_item = cJSON_GetObjectItem(child, "Identifier");
                    strcpy(identifier, identifier_item->valuestring);
                }
                //Argument 1 of the application
                else if (counter == 1){
                    arg1 = readjsonrecursive(child);
                }
                //Argument 2 of the application
                else{
                    arg2 = readjsonrecursive(child);
                }
                counter++;
            }
            return application(identifier, arg1, arg2);
        }
        else if (strcmp(current->string, "Identifier")==0){
            return identifier(current->valuestring);
        }
        else if (strcmp(current->string, "Cond")==0){
            //{"Cond":[{"Clause":[{"Identifier": "false"},-1]},{"Clause":[{"Identifier": "true"},5]}]}
            //The cond object contains an array of clauses. Each clause contains a 1st argument that should evaluate to either 0 or 1 and a 2nd argument that is the value returned when 1.
            cJSON* clause = NULL;
            //printf("Condition: %s\n", cJSON_Print(current));
            cJSON_ArrayForEach(clause, current){
                //Clause will be {Clause : [{arg1 (the condition), arg2 (the value)}]}
                //We want it without the curly brackets, Clause : [{arg1 (the condition), arg2 (the value)}]
                cJSON* clauseArray = cJSON_GetArrayItem(clause, 0);
                //printf("clauseArray: %s\n", cJSON_Print(clauseArray));
                cJSON *item1 = cJSON_GetArrayItem(clauseArray, 0);
                cJSON *item2 = cJSON_GetArrayItem(clauseArray, 1);
                if(readjsonrecursive(item1) == 0){
                    //printf("False\n");
                }
                else if(readjsonrecursive(item1) == 1){
                    //printf("True\n");
                    return readjsonrecursive(item2);
                }
                else{
                    printf("ERROR: The condition is not a boolean value.\n");
                    error();
                }
            }
            //If no clause is true, return 0
            return 0;
        }
        else{
            error();
        }
    }
}

void readjson(){
    cJSON* json = cJSON_Parse(current_char);
    if (json == NULL){
        error();
    }
    // Go through the json object recursively
    long long result = readjsonrecursive(json);
    accumulator = result;
}

bool isNumberStart(char c){
    return (('0' <= c) && (c <= '9')) || (c == '-');
}

void interpreter(){
    //JSON object reader. Analyzes most code like:
    //{"Application":[{"Identifier":"add"},{"Identifier":"x"},{"Identifier":"v"}]} => 'add(x, v)'
    //{"Identifier":"x"} => 'x'
    if (*current_char == '{'){
        printf("reading json\n");
        readjson();
        printf("FINAL OUTPUT: %lld\n", accumulator);
    }
    //String reader with double quotes. Analyzes most code like:
    //"Hello World"
    //64 bit integer reader. Analyzes most code like:
    //123456789
    else if (isNumberStart(*current_char)){
        printf("reading int\n");
        int64();
        //Make sure there are no strange characters at the end of the code.
        if (*current_char != '\0') {
            error();
        }
        printf("FINAL OUTPUT: %lld\n", accumulator);
    }
    else{
        printf("reading string\n");
        if(strlen(current_char) == 0){
            error();
        }
        printf("FINAL OUTPUT: %s\n", current_char);
    }
}

int main(int argc, char const *argv[])
{
    printf("\n");
    // Check if the program takes exactly one argument, that is to say the string to be interpreted.
    if (argc != 2){
        printf("ERROR: The program takes exactly one argument.\n");
        exit(1);
    }

    printf("The code is: %s\n", argv[1]);   

    // Copy the code given in stdin to a newly created string called code. Also, set the current_char to the beginning of the code.
    char* code = malloc(strlen(argv[1]) * sizeof(char));
    strcpy(code, argv[1]);
    current_char = code;

    //Run the interpreter
    interpreter();
}


// Basic grammar of the program
// <interpreter> ::= <json> | <signed_int>
// <json> ::= '{' {<json_content> ','} '}'

// <json_content> ::= 