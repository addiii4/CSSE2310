// Wordle Helper CSSE2310 Assignment 1 
// Aditya Singh Cheema 46538127

//Header files
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

//Defining environment variables
#define WORDLE_DICTIONARY "WORDLE_DICTIONARY"
#define DEFAULT "/usr/share/dict/words"
#define BUFFER_SIZE 80

int alpha = 0;
int best = 0;
int len = 5;
char* with;
char* without;
int pattern = 0;

// Function prototypes : List of all functions present in code
void argument_validate (int, char**);
void exit_validate_error_1 ();
void alpha_best_validate (int, char**);
void len_validate (int, char**);
int digit_check (char*);
char* with_validate (int, char**);
char* without_validate (int, char**);
char* get_environment_variable ();
char* dictionary_open (FILE*);
void exit_dict_error_3 (char*);

// READING DICTIONARY USING ENVIRONMENT VARIABLE

/* get_environment_variable()
 * --------------------------
 * Retrieves the filepath mentioned to the dictionary
 * If no filepath is mentioned, DEFAULT filepath ,i.e. 
 * /usr/share/dict/words is assigned
 *
 * Returns: Dictionary filepath as a string
 *
 * Errors: None
 **/
char* get_environment_variable() {
    char* var = getenv("WORDLE_DICTIONARY");
    if (var == NULL) {
	var = DEFAULT; 
    }
    return var;
}

/* dictionary_open(FILE*)
 * ----------------------
 * This function reads characters from the dictionary file passed to it
 * It returns a pointer string of all the characters present in the file
 *
 * *dictionary: dictionary file passed by the main function
 *		this file is returned by the environment variable
 *
 * Returns: A pointer string with all characters in the file in uppercase
 *
 * Errors: If dictionary file is NULL, invokes exit_dict_error_3()
 *  
 * Reference: This code is inspired from Ed platform Week 3.2 C exercises
 **/
char* dictionary_open (FILE* dictionary) {
    if (dictionary == NULL) {
	exit_dict_error_3(DEFAULT);
    }
    int bufferSize = BUFFER_SIZE;
    char* buffer = malloc(sizeof(char*) * bufferSize);
    int num = 0;
    int next;

    if (feof(dictionary)) {
	return NULL;
    }

    while (1) {
	next = toupper(fgetc(dictionary));
	if (next == EOF && num == 0) {
	    free(buffer);
	    return NULL;
	}
	if (num == bufferSize - 1) {
	    bufferSize *= 2;
	    buffer = realloc(buffer, sizeof(char*) * bufferSize);
	}
	if (next == EOF) { 
	    buffer[num] = '\0';
	    break;
	}
	buffer[num++] = next;
    }
    return buffer;
}

/* line_distinguish(char*, char)
 * -----------------------------
 * This function separates each word in the string using in a newline
 *
 * *buffer: pointer array which contains characters read from the file
 *
 * Returns: Returns pointer array with each word separated by a newline
 *
 * Errors: None
 *  
 * Reference: This code is inspired from Ed platform Week 3.2 C exercises
 **/
char** line_distinguish (char* array, char delim) {
    char** index = malloc(sizeof(char*));
    index[0] = NULL;

    char delimiter[] = {delim, '\0'};
    int numIndex = 0;
    char* token = strtok(array, delimiter);

    while (token) {
	index[numIndex] = strdup(token);
	numIndex++;
	index = realloc(index, sizeof(char*) * (numIndex + 1));
	index[numIndex] = NULL;

	token = strtok(NULL, delimiter);
    }
    return index;
}

//ERROR HANDLING

/* exit_validate_error_1()
 * -----------------------
 * Invoked when commandline arguments are invalid
 * When invoked, prints to standard error and exits with value 1
 * 
 * Returns: 1 and exits the program
 *
 * Errors: None
 **/
void exit_validate_error_1 () {
    fprintf(stderr, "Usage: wordle-helper ");
    fprintf(stderr, "[-alpha|-best] [-len len] [-with letters] ");
    fprintf(stderr, "[-without letters] [pattern]\n");
    exit(1);
}

/* exit_dict_error_3(char*)
 * ------------------------
 * Invoked when dictionary cannot be open with filepath
 * When invoked, prints to standard error and exits with value 3
 * 
 * *location: Stores the filepath of the dictionary which cannot be opened
 *
 * Returns: 3 and exits the program
 *
 * Errors: None
 **/
void exit_dict_error_3 (char* location) {
    fprintf(stderr, "wordle-helper: dictionary file ");
    fprintf(stderr,"\"%s\" cannot be opened\n", location);
    exit(3);
}

// VALIDATION OF COMMANDLINE ARGUMENTS

/*alpha_best_validate(int, char**)
 * -------------------------------
 * Checks for presence of -alpha or -best in commandline arguments
 *
 * n : Number of command line arguments argc
 * ** array: character array to store command line arguments **argv
 *
 * alpha global variable incremented to 1 when -alpha is present
 * best global variable incremented to 1 when -best is present
 * Returns: VOID
 *
 * Errors: Invokes exit_validate_error_1() to exit with error 1 
 *	   when -alpha or -best are present more than once
 *	   when -alpha and -best both are present
 **/
void alpha_best_validate (int n, char** array) {
    for (int i = 1; i < n; i++) {
	if (strcmp(array[i], "-alpha") == 0){
	    alpha++;
	} else if (strcmp(array[i], "-best") == 0) {
	    best++;
	} else {
	    continue;
	}
    }
    if (alpha > 1 || best > 1) {
	exit_validate_error_1();	
    }
    if (alpha == 1 && best == 1) {
	exit_validate_error_1();
    }
}

/*len_validate(int, char**)
 * ------------------------
 * Checks for presence of -len in commandline arguments
 *
 * n : Number of command line arguments argc
 * ** array: character array to store command line arguments **argv
 * checkRepeat: counter variable to check for repeated value of -len
 * 
 * len global variable given value of digit followed by -len
 *
 * Invokes: digit_check(char*) to check if -len is followed by a number
 *
 * Returns: VOID
 *
 * Errors: Invokes exit_validate_error_1() to exit with error 1
 *	   when -len is not followed by a digit
 *	   when digit following -len is less than 4 or greater than 9
 *	   when -len is repeated
 **/   
void len_validate (int n, char** array) {
    int checkRepeat = 0;
    for (int i = 1; i < n; i++) {
	if (strcmp(array[i], "-len") == 0) {
	    if (i + 1 == n) { //If -len is the last argument
		exit_validate_error_1();
	    } else if (digit_check(array[i + 1]) != 0) {
	        exit_validate_error_1();
	    } else {
	        len = atoi(array[i + 1]);
	        checkRepeat++;
	    }	    
	}
    }
    if (!len) { // 0 is not checked in isdigit()
	exit_validate_error_1();
    }
    if (len < 4 || len > 9) {
        exit_validate_error_1();
    }
    if (checkRepeat > 1) {
	exit_validate_error_1();
    }
}

/*digit_check(char*)
 * -----------------
 * Checks for presence of numbers after -len 
 *
 * *string: to store the argument followed by -len
 *
 * Invoked by: len_validate(int, char*)
 *
 * Returns: 1 if string is a number
 *	    2 if string is not a number
 **/
int digit_check (char* string){
    for (int i = 0; string[i] != '\0'; i++) {
	if (!isdigit(string[i])) {
	    return 1;
	}
    }
    return 0;
}

/*with_validate(int, char**)
 * -------------------------
 * Checks for presence of -with followed by letters in commandline arguments
 *
 * n : Number of command line arguments argc
 * ** array: character array to store command line arguments **argv
 *
 * Returns: letters stored after -with command
 *	    if -with command is not present, "" is returned
 *
 * Errors: Invokes exit_validate_error_1() to exit with error 1 
 *	   when -with is not followed by letters
 *	   when -with is present more than once
 **/
char* with_validate (int n, char** array) {
    int checkRepeat = 0;
    char* letters;
    for (int i = 1; i < n; i++) {
	if (strcmp(array[i], "-with") == 0) {
	    if (i + 1 == n) { //with is the last argument
		exit_validate_error_1();	    
	    }
	    checkRepeat++;
	    letters = array[i + 1];
	    for (int i = 0; i < strlen(letters); i++) {
		if (!isalpha(letters[i])) {
		    exit_validate_error_1();
		} else if (letters[i] == '\0') {
		    exit_validate_error_1();
		}
	    }
	}
    }
    if (checkRepeat > 1) {
	exit_validate_error_1();
    }
    if (checkRepeat == 0) {
	return letters = "";
    }
    //Loop to make letter uppercase
    for (int i = 0; letters[i] != '\0'; ++i) {
	letters[i] = toupper((unsigned char) letters[i]);
    }
    return letters;
}

/*without_validate(int, char**)
 * ----------------------------
 * Checks for presence of -without followed by letters in commandline args
 *
 * n : Number of command line arguments argc
 * ** array: character array to store command line arguments **argv
 *
 * Returns: letters stored after -without command
 *	    if -without command is not present, "" is returned
 *
 * Errors: Invokes exit_validate_error_1() to exit with error 1 
 *	   when -without is not followed by letters
 *	   when -without is present more than once
 **/
char* without_validate (int n, char** array) {
    int checkRepeat = 0;
    char* letters;
    for (int i = 1; i < n; i++) {
	if (strcmp(array[i], "-without") == 0) {
	    if (i + 1 == n) { //without is the last argument
		exit_validate_error_1();	    
	    }
	    checkRepeat++;
	    letters = array[i + 1];
	    for (int i = 0; i < strlen(letters); i++) {
		if (!isalpha(letters[i])) {
		    exit_validate_error_1();
		} else if (letters[i] == '\0') {
		    exit_validate_error_1();
		}
	    }
	}
    }
    if (checkRepeat > 1) {
	exit_validate_error_1();
    }
    if (checkRepeat == 0) {
	return letters = "";
    }
    //Loop to make letter uppercase
    for (int i = 0; letters[i] != '\0'; ++i) {
	letters[i] = toupper((unsigned char) letters[i]);
    }
    return letters;
}

/*argument_validate(int, char**)
 * ----------------------------
 * Validates presence of correct commandline arguments
 *
 * n: Number of command line arguments argc
 * ** array: character array to store command line arguments **argv
 * with: globally declared pointer which stores letters after -with
 * without: globally declared pointer which stores letters after -without
 *
 * Invokes: alpha_validate()
 *	    len_validate()
 *	    with_validate()
 *	    without_validate()
 *	    exit_validate_error_1()
 *
 * Returns: letters stored after -without command
 *	    if -without command is not present, "" is returned
 *
 * Errors: Invokes exit_validate_error_1() to exit with error 1 
 *	   when -without is not followed by letters
 *	   when -without is present more than once
 **/
void argument_validate (int n, char** array) {
    for (int i = 1; i < n ; i++) {
	if (array[i][0] == '-') {
	    if (strcmp(array[i], "-alpha") != 0 &&
		strcmp(array[i], "-best" ) != 0 &&
		strcmp(array[i], "-len") != 0 &&
		strcmp(array[i], "-with") != 0 &&
		strcmp(array[i], "-without") != 0) {
		exit_validate_error_1();
	    }
	} 
	if (strcmp(array[i], "pattern")) {
	    pattern = 1;
	} else {
	    exit_validate_error_1();
	}
    }
    alpha_best_validate(n, array);
    len_validate(n, array);
    with = with_validate(n, array);
    without = without_validate(n, array);
}

// MAIN FUNCTION
/* main(int, char**)
 * -----------------
 * The main function of the program which reads commandline arguments
 * It opens dictionary file, performs functions provided by user
 * Prints the words as required by the user
 *
 * invokes: argument_validate()
 *	    get_environment_variable()
 *	    dictionary_open()
 *	    line_ditinguish()
 *
 * *env: stores filepath given by environment variable
 * *file: the dictionary file
 * *dict: pointer array to store characters read by dictionary_open
 * **distinct: pointer array to store dictionary words in new line
 * count: counter variable to count the number of words in the dictionary
 *  
 * returns: 0 when program complied successfully
 *
 * errors: none
 **/ 
int main(int argc, char** argv) {
    argument_validate(argc, argv);
    char* env = get_environment_variable();
    FILE* file = fopen(env, "r");
    char* dict = dictionary_open(file);
    char** distinct = line_distinguish(dict, '\n');
    long count = 0;
    while (distinct[count] != NULL) {
	count++;
    } 
    // Printing dictionary words
    for (long i = 0; i < count - 1; i++) {  
	if (strlen(distinct[i]) == len){	
	    if (strstr(distinct[i], with) != NULL) { 
		printf("%s\n", distinct[i]);
	    }
	    if (strstr(distinct[i], without) == NULL) {
		printf("%s\n", distinct[i]);
	    }
	}
    }
    fclose(file);
    return 0;
}

// END OF CODE
