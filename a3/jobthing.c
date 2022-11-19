//Aditya Singh Cheema CSSE2310 A3
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <csse2310a3.h>

//Character array to store input file
char* inputFile = NULL;
//Character array to job file
char* jobFile = NULL;
//Integer counter to store verbose mode
int verbose = 0;
//Integer counter to store presence of input file
int iFile = 0;
//Integer counter to store presence of job file
int job = 0;

void read_jobfile();
void read_inputfile();
void job_checker(char**, int);
void job_handler(int, int, char*, char*, char** args);

/* exit_validate_error_1()
 * -----------------------
 * Invoked when commandline arguments are invalid
 * When invoked, prints to standard error and exit with value 1
 *
 * Returns: Exits with value 1
 *
 * Errors: None
 **/
void exit_validate_error_1(){
    fprintf(stderr, "Usage: jobthing [-v] [-i inputfile] jobfile\n");
    exit(1);
}

/* validate_arguments(int, char**)
 * -------------------------------
 * Validates presence of correct commandline arguments
 *
 * n: Number of commandline arguments argc
 * **array: Character array to store commandline arguments **argv
 * inputFile: Globally declared character array to store input file
 * jobFile: Globally declared character array to job file
 * verbose: Globally declared integer counter to store verbose mode
 * iFile: Globally declared integer counter to store presence of input file
 * job: Globally declared integer counter to store presence of job file
 * 
 * Returns: Void
 *
 * Errors: Invokes exit_validate_error_1() and exits with 1
 *	   jobFile is not present
 *	   verbose, iFile or jobFile occurs more than once
 *	   -i not followed by inputFile
 *	   -v or -i used as inputFile
 **/
void validate_arguments(int n, char** array){
    if (n > 5 || n < 2) { 
	exit_validate_error_1();
    }
    for (int i = 1; i < n; i++) {
	if (array[i][0] == '-') {
	    if (strcmp(array[i], "-v") != 0 && strcmp(array[i], "-i") != 0) {
		exit_validate_error_1();
	    } 
	    if (strcmp(array[i], "-v") == 0) {
		verbose++;
	    } 
	    if (strcmp(array[i], "-i") == 0) {
		if (array[i + 1] == NULL) {
		    exit_validate_error_1();
		} else {
		    if (array[i + 1][0] != '-') {
			inputFile = array[i + 1];
			iFile++;
		    } else if (array[i + 1][1] == 'i') {
			fprintf(stderr, "Error: Unable to read input file\n");
			exit(3);
		    }
		}
	    }
	} 
	if (array[i - 1][0] != '-' && array[i - 1][1] != 'i') {
	    if (array[i][0] != '-') {
		jobFile = array[i];
		job++;
	    }
	}
    } 
    if (jobFile == NULL) {
	exit_validate_error_1();
    } 
    if (verbose > 1 || iFile > 1 || job > 1) {
	exit_validate_error_1();
    } 
    if (inputFile != NULL && iFile == 1) {
	read_inputfile();
    } 
    if (jobFile != NULL && job == 1) {
	read_jobfile();
    }
}

/* read_jobfile()
 * --------------
 * Reads jobFile, stores all jobfiles and invokes job_checker() to validate
 * job files.
 *
 * file: stream to store file contents from fopen
 * jobFile: Globally declared character array to job file
 * reader: Character to store contents from getc(file)
 * lineCounter: To count the number of lines in jobFile
 * line: store each line read from jobFile
 * jobList: 2D array to store all valid jobs
 * jobNumber: Integer to store total number of jobs
 *
 * Returns: Void
 *
 * Errors: If file cant be opened, prints to stderr and exits with 2
 **/
void read_jobfile(){
    FILE* file = fopen(jobFile, "r");
    if (file == NULL) {
	fprintf(stderr, "Error: Unable to read job file\n");
	exit(2);
    } else {
	char reader;
	int lineCounter = 0;
	for (reader = getc(file); reader != EOF; reader = getc(file)) {
	    if (reader == '\n') {
		lineCounter++;
	    }
	}
	fclose(file);
	file = fopen(jobFile, "r");
	char* line = read_line(file);
	char** jobList;
	int jobNumber = 0;
	jobList = malloc(sizeof(char*) * lineCounter);
	for (int i = 0; i < lineCounter; i++) {
	    if (line[0] != '#' && strlen(line) != 0) {
	    	jobList[jobNumber] = malloc(sizeof(char) * 100);
		strcpy(jobList[jobNumber], line);
		jobNumber++;
		line = read_line(file);
	    } else {
		line = read_line(file);
	    }
	}
    	job_checker(jobList, jobNumber);
    }
    fclose(file);
}

/*job_checker(char**, int)
 *------------------------
 * Checks validity of all jobs in jobList. If jobs are valid, job_handler() 
 * is invoked.
 *
 * numRestarts: Integer to store how many times a job has to be restarted
 * input: Character array to store input file
 * output: Character array to store output file 
 * args: Character array to store command line arguments
 * tempJob: 2D character array to store each line after split_line() is 
 * invoked.
 * size: Integer to store size of tempJob array
 * num: Store numtoken from split_space_not_quote()
 * spaced: 2D character array to store args after separation
 * pid: store parent ID
 *
 * Returns: Void
 *
 * Errors: When no more viable workers remaining, prints to stderr
 *	   When invalid job specificationm prints to stderr
 **/
void job_checker(char** jobs, int count) {
    int numRestarts = 0;
    char* input = malloc(sizeof(char*) * 20);
    char* output = malloc(sizeof(char*) * 20);
    char* args = malloc(sizeof(char*) * 20);
    for (int i = 0; i < count; i++) {
	char** tempJob = split_line(jobs[i], ':');
	int size = 0;
	while (tempJob[size]) {
	    size++;
	}
	if (size == 4 && !(tempJob[3][0] == ' ' || strlen(tempJob[3]) == 0)
		&& tempJob[3] != NULL) {
	    numRestarts = atoi(tempJob[0]);
	    strcpy(input, tempJob[1]);
	    strcpy(output, tempJob[2]);
	    strcpy(args, tempJob[3]);
	    int num = 0;
	    char** spaced = split_space_not_quote(args, &num);
	    pid_t pid = fork();
	    if (pid == 0) {
		execvp(spaced[0], spaced);
	    } else {
		wait(NULL);
		fprintf(stderr, "No more viable workers, exiting\n");
	    }
	    if ((numRestarts > -1) && (args[i] != ' ')) { 
		//job_handler(i, numRestarts, input, output, spaced);
	    } else {
		if (verbose == 1) {
		    fprintf(stderr, "Error: invalid job specification");
		    fprintf(stderr, ": %s\n", jobs[i]);
		}
	    }
	}
    }
}

/* read_inputfile()
 * ----------------
 * Reads input file. If input file is not present, takes stdin.
 * Gets invoked by validate_arguments
 *
 * in: File descripter when inputFile is opened
 * inputFile: Globally declared character array to store input file
 * iFile: Globally declared integer counter to store presence of input file
 * 
 * Returns: Void
 *
 * Errors: Prints to stderr and exits with 3 when file descripter < 0
 *	   exits with 99 when execlp statement is not working
 **/
void read_inputfile(){
    int in = open(inputFile, O_RDONLY);
    if (in < 0) {
	fprintf(stderr, "Error: Unable to read input file\n");
	exit(3);
    }
    if (iFile == 0) {
    	while ((inputFile = read_line(stdin))) {
	    int p[2];
	    if (pipe(p) < 0) {
		exit(4);
	    }
	    if (!fork()) {
		close(p[1]);
		dup2(p[0], 0);
		close(p[0]);
		execlp("echo", "echo", NULL);
		exit(99);
	    } else {
		close(p[0]);
		write(p[1], inputFile, strlen(inputFile));
		close(p[1]);
		wait(0);
	    }
	}
    }
    close(in);
}

/* job_handler(int, int, char*, char*, char**)
 * -------------------------------------------
 * Recieves parsed joblist and spawns processes. 
 *
 * pipeNode: File descriptor which acts as reading and writing end of pipe
 * input: File descriptor for opening input file
 * output: File descriptor for opening output file
 *
 * Returns: Void
 *
 * Errors: When pipe not created, exits with error 4
 * */
void job_handler(int jobNo, int num, char* in, char* out, char** args) {
    int pipeNode[2];
    if (pipe(pipeNode) < 0) {
	printf("Pipe not created\n");
	exit(4);
    } 
    if (verbose == 1) {
	fprintf(stdout, "Spawning worker %d\n", jobNo);
	fflush(stdout);
    }
    int input = open(in, O_RDONLY);
    if (input < 0) {
	fprintf(stderr, "Error: unable to open \"%s\" for reading\n", in);
    }	
    dup2(input, STDIN_FILENO);
    int output = open(out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (output < 0) {
	fprintf(stderr, "Error: unable to open \"%s\" for writing\n", out);
    }	
    dup2(output, STDOUT_FILENO);
    if (fork()) {
	//Parent process
	close(pipeNode[0]);     //Reading node closed in parent process
	char* buffer = read_line(stdin);
	write(pipeNode[1], buffer, strlen(buffer));
	close(pipeNode[1]);     //Closed write node in parent process
	wait(NULL);		//Wait for any child process to end
	exit(0);
    } else {
	//Child process
	close(pipeNode[1]);     //Writing node closed in child process
	//Changing child's stdin to point to pipeNode[0] for reading
	dup2(pipeNode[0], 0);
	close(pipeNode[0]);
	execvp(args[0], args);
	exit(99);
    }
}

int main(int argc, char** argv) {
    validate_arguments(argc, argv);
    return 0;
}
