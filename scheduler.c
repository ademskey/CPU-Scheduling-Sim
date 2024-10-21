/****************** Headers *********************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
/***************** structs and Typedefs ***********************/
typedef enum {false, true} bool;        // Allows boolean types in C
/* Defines a job struct */
typedef struct Process {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time required
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read
    uint8_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated
    int32_t finishingTime;              // The cycle when the the process finishes (initially -1)
    uint32_t currentCPUTimeRun;         // The amount of time the process has already run (time in running state)
    uint32_t currentIOBlockedTime;      // The amount of time the process has been IO blocked (time in blocked state)
    uint32_t currentWaitingTime;        // The amount of time spent waiting to be run (time in ready state)
    uint32_t IOBurst;                   // The amount of time until the process finishes being blocked
    uint32_t CPUBurst;                  // The CPU availability of the process (has to be > 1 to move to running)
    int32_t quantum;                    // Used for schedulers that utilise pre-emption
    bool isFirstTimeRunning;            // Used to check when to calculate the CPU burst when it hits running mode
    struct Process* nextInBlockedList;  // A pointer to the next process available in the blocked list
    struct Process* nextInReadyQueue;   // A pointer to the next process available in the ready queue
    struct Process* nextInReadySuspendedQueue; // A pointer to the next process available in the ready suspended queue
} _process;

/***************** Global Variables ***********************/
uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
uint32_t TOTAL_CREATED_PROCESSES = 0;   // The total number of processes constructed
uint32_t TOTAL_STARTED_PROCESSES = 0;   // The total number of processes that have started being simulated
uint32_t TOTAL_FINISHED_PROCESSES = 0;  // The total number of processes that have finished running
uint32_t TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; // The total cycles in the blocked state
int32_t QUANTUM = 2;                   // The quantum for the Round Robin (RR) scheduler
uint32_t TOTAL_NUMBER_OF_PROCESSESS = 0; // The total number of processes in the process list
const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";
const uint32_t SEED_VALUE = 200;  // Seed value for reading from file

/***************** Function Prototypes ***********************/
uint32_t randomOS(uint32_t upper_bound, uint32_t process_indx, FILE* random_num_file_ptr);
void start_process(_process* process, FILE* random_num_file_ptr);
void block_process(_process* process, FILE* random_num_file_ptr);
void unblock_process(_process* process, FILE* random_num_file_ptr);
void terminate_process(_process* process, FILE* random_num_file_ptr);
void printStart(_process process_list[]);
void printFinal(_process finished_process_list[]);
void printProcessSpecifics(_process process_list[]);
void printSummaryData(_process process_list[]);
void sim_fcfs(_process plist[], _process finished_plist[], FILE* frandom);
void sim_rr(_process plist[], _process finished_plist[], FILE* frandom);
void sim_sjf(_process plist[], _process finished_plist[], FILE* frandom);
void sjf_add_to_ready(_process **ready, _process *newready);
void fcfs_add_to_ready(_process **ready, _process *newready);
void fcfs_add_arrivals(_process process_list[], _process** ready);
void add_blocked(_process **blocked, _process *newblock);
void parce_file(FILE* input, char* line, _process plist[]);
_process* create_process_list(FILE* input);
void reset_process_list(_process process_list[]);
void reset_counters();
uint32_t find_total_processes(char line[]);
_process init_process(uint32_t A, uint32_t B, uint32_t C, uint32_t M, uint32_t processId);
void print_reset(_process plist[], _process finished_plist[]);
void printStart(_process process_list[]);
void printFinal(_process finished_process_list[]);
void printProcessSpecifics(_process process_list[]);
void printSummaryData(_process process_list[]);
void print_reset(_process plist[], _process finished_plist[]);
uint32_t getRandNumFromFile(uint32_t line, FILE* random_num_file_ptr);

/********************* Random Number Helpers **************************/
/**
 * Reads a random non-negative integer X from a file with a given line named random-numbers (in the current directory)
 */
uint32_t getRandNumFromFile(uint32_t line, FILE* random_num_file_ptr){
    uint32_t end, loop;
    char str[512];

    rewind(random_num_file_ptr); // reset to be beginning
    for(end = loop = 0;loop<line;++loop){
        if(0==fgets(str, sizeof(str), random_num_file_ptr)){ //include '\n'
            end = 1;  //can't input (EOF)
            break;
        }
    }
    if(!end) {
        return (uint32_t) atoi(str);
    }

    // fail-safe return
    return (uint32_t) 1804289383;
}
/**
 * Reads a random non-negative integer X from a file named random-numbers.
 * Returns the CPU Burst: : 1 + (random-number-from-file % upper_bound)
 */
uint32_t randomOS(uint32_t upper_bound, uint32_t process_indx, FILE* random_num_file_ptr)
{
    char str[20];
    uint32_t unsigned_rand_int = (uint32_t) getRandNumFromFile(SEED_VALUE+process_indx, random_num_file_ptr);
    uint32_t returnValue = 1 + (unsigned_rand_int % upper_bound);

    return returnValue;
} 
/********************* SOME PRINTING HELPERS *********************/
/**
 * Prints to standard output the original input
 * process_list is the original processes inputted (in array form)
 */
void printStart(_process process_list[])
{
    printf("The original input was: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", process_list[i].A, process_list[i].B,
               process_list[i].C, process_list[i].M);
    }
    printf("\n");
} 
/**
 * Prints to standard output the final output
 * finished_process_list is the terminated processes (in array form) in the order they each finished in.
 */
void printFinal(_process finished_process_list[])
{
    printf("The (sorted) input is: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_FINISHED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", finished_process_list[i].A, finished_process_list[i].B,
               finished_process_list[i].C, finished_process_list[i].M);
    }
    printf("\n");
} // End of the print final function
/**
 * Prints out specifics for each process.
 * @param process_list The original processes inputted, in array form
 */
void printProcessSpecifics(_process process_list[])
{
    uint32_t i = 0;
    printf("\n");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf("Process %i:\n", process_list[i].processID);
        printf("\t(A,B,C,M) = (%i,%i,%i,%i)\n", process_list[i].A, process_list[i].B,
               process_list[i].C, process_list[i].M);
        printf("\tFinishing time: %i\n", process_list[i].finishingTime);
        printf("\tTurnaround time: %i\n", process_list[i].finishingTime - process_list[i].A);
        printf("\tI/O time: %i\n", process_list[i].currentIOBlockedTime);
        printf("\tWaiting time: %i\n", process_list[i].currentWaitingTime);
        printf("\n");
    }
} // End of the print process specifics function
/**
 * Prints out the summary data
 * process_list The original processes inputted, in array form
 */
void printSummaryData(_process process_list[])
{
    uint32_t i = 0;
    double total_amount_of_time_utilizing_cpu = 0.0;
    double total_amount_of_time_io_blocked = 0.0;
    double total_amount_of_time_spent_waiting = 0.0;
    double total_turnaround_time = 0.0;
    uint32_t final_finishing_time = CURRENT_CYCLE - 1;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        total_amount_of_time_utilizing_cpu += process_list[i].currentCPUTimeRun;
        total_amount_of_time_io_blocked += process_list[i].currentIOBlockedTime;
        total_amount_of_time_spent_waiting += process_list[i].currentWaitingTime;
        total_turnaround_time += (process_list[i].finishingTime - process_list[i].A);
    }
    // Calculates the CPU utilisation
    double cpu_util = total_amount_of_time_utilizing_cpu / final_finishing_time;
    // Calculates the IO utilisation
    double io_util = (double) TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED / final_finishing_time;
    // Calculates the throughput (Number of processes over the final finishing time times 100)
    double throughput =  100 * ((double) TOTAL_CREATED_PROCESSES/ final_finishing_time);
    // Calculates the average turnaround time
    double avg_turnaround_time = total_turnaround_time / TOTAL_CREATED_PROCESSES;
    // Calculates the average waiting time
    double avg_waiting_time = total_amount_of_time_spent_waiting / TOTAL_CREATED_PROCESSES;
    // Print the summary data
    printf("Summary Data:\n");
    printf("\tFinishing time: %i\n", CURRENT_CYCLE - 1);
    printf("\tCPU Utilisation: %6f\n", cpu_util);
    printf("\tI/O Utilisation: %6f\n", io_util);
    printf("\tThroughput: %6f processes per hundred cycles\n", throughput);
    printf("\tAverage turnaround time: %6f\n", avg_turnaround_time);
    printf("\tAverage waiting time: %6f\n", avg_waiting_time);
} // End of the print summary data function

/***************** Simulation Helper Functions ***********************/
// Finds the total number of processes
uint32_t find_total_processes(char* line){
    char *token;
    char *line_copy = strdup(line);
    const char delim[2] = " ";
    token = strtok(line_copy, delim);
    return atoi(token);
}

// Initializes a process
_process init_process(uint32_t A, uint32_t B, uint32_t C, uint32_t M, uint32_t processId) {
    // Create a new process and set variables
    _process newprocess;
    newprocess.A = A; // Set arrival time
    newprocess.B = B; // Set CPU burst upper bound
    newprocess.C = C; // Set total CPU time
    newprocess.M = M; // Set burst time multiplier
    newprocess.status = 0; // Set the status to unstarted
    newprocess.processID = processId;
    newprocess.finishingTime = -1; // Set the finishing time to -1
    newprocess.currentCPUTimeRun = 0; // Set the current CPU time run to 0
    newprocess.currentIOBlockedTime = 0; // Set the current IO blocked time to 0
    newprocess.currentWaitingTime = 0; // Set the current waiting time to 0
    newprocess.IOBurst = 0; // Set the IO burst to 0
    newprocess.CPUBurst = 0; // Set the CPU burst to 0
    newprocess.isFirstTimeRunning = true;
    newprocess.quantum = QUANTUM; // Set the quantum for the process (used for Round Robin)
    newprocess.nextInBlockedList = NULL; 
    newprocess.nextInReadyQueue = NULL;
    newprocess.nextInReadySuspendedQueue = NULL;
    return newprocess;
}

// Parces the file
void parce_file(FILE* input, char* line, _process plist[]){
    // File input will be in form "1 (0 1 5 1)" and must ignore comments after (One Line)
    const char delim[4] = "() "; // Deliminator
    char* token;
    uint32_t val[4];

    // skip over count prefix (avoids bad input and segfault)
    token = strtok(line, " ");
    token = strtok(NULL, delim);

    if (token == NULL){
        printf("Error: No token found after count prefix\n");
        exit(1);
    }
    for (uint32_t i = 0; i < TOTAL_CREATED_PROCESSES; i++){
        for (int j = 0; j < 4; j++) {
            if(token != NULL){
                val[j] = atoi(token);
            }else {
                printf("%s\n", token);
                printf("Error: Invalid input\n");
                exit(1);
            }
            token = strtok(NULL, delim);
        }
        plist[i] = init_process(val[0], val[1], val[2], val[3], i);
    }
}

// Fills process list with processess from file
_process* create_process_list(FILE* input){
    char* line = NULL;
    size_t len = 0;

    // Check if file exists
    if (!input) {
        printf("Error: File not found\n");
        exit(1);
    }
    // Read the file
    getline(&line, &len, input);

    // Get the total number of processes and allocate memory
    TOTAL_CREATED_PROCESSES = find_total_processes(line);
    _process* plist = (_process*) malloc(TOTAL_CREATED_PROCESSES * sizeof(_process));
    parce_file(input, line, plist);
    return plist;
}

// Resets the process list
void reset_process_list(_process process_list[]){
    for (uint32_t i = 0; i < TOTAL_CREATED_PROCESSES; i++){
        process_list[i].status = 0;
        process_list[i].finishingTime = -1;
        process_list[i].currentCPUTimeRun = 0;
        process_list[i].currentIOBlockedTime = 0;
        process_list[i].currentWaitingTime = 0;
        process_list[i].IOBurst = 0;
        process_list[i].CPUBurst = 0;
        process_list[i].quantum = QUANTUM;
        process_list[i].isFirstTimeRunning = true;
        process_list[i].nextInBlockedList = NULL;
        process_list[i].nextInReadyQueue = NULL;
        process_list[i].nextInReadySuspendedQueue = NULL;
    }
}

// Resets the counters
void reset_counters(){
    CURRENT_CYCLE = 0;
    TOTAL_STARTED_PROCESSES = 0;
    TOTAL_FINISHED_PROCESSES = 0;
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0;
}

void print_reset(_process plist[], _process finished_plist[]){
    // Print results
    printFinal(finished_plist);
    printProcessSpecifics(plist);
    printSummaryData(plist);

    // Do counter/list resets
    reset_counters();
    reset_process_list(plist);
    reset_process_list(finished_plist);
}

void update_waiting(_process *ready){
    _process *tracker;
    
    for (tracker = ready; tracker; tracker = tracker->nextInReadyQueue) {
        tracker->currentWaitingTime++;
    }
    // Increment cycle and quantam count
    CURRENT_CYCLE++;
}
/***************** FCFS ***********************/
void fcfs_add_to_ready(_process **ready, _process *newready){
    _process *tracker = *ready;
    newready->status = 1;

    // ready queue is empty
    if (tracker == NULL) {
        *ready = newready;
        return;
    }
    // iterate to end of ready queue
    while (tracker->nextInReadyQueue != NULL) {
        tracker = tracker->nextInReadyQueue;
    }
    // add current process to end of ready queue
    tracker->nextInReadyQueue = newready;
}

// Function to add arrivals
void fcfs_add_arrivals(_process process_list[], _process** ready){
    // iterate over process list
    for (uint32_t i = 0; i < TOTAL_CREATED_PROCESSES; i++) {
        if (process_list[i].A == CURRENT_CYCLE) {
            fcfs_add_to_ready(ready, &process_list[i]);
        }
    }
}

// Function to add to blocked list
void add_blocked(_process **blocked, _process *newblock){
    _process *tracker = *blocked;
    newblock->status = 3;

    // blocked list is empty
    if (tracker == NULL) {
        *blocked = newblock;
        return;
    }
    // add to front of blocked list
    if (newblock->IOBurst < tracker->IOBurst) {
        newblock->nextInBlockedList = *blocked;
        *blocked = newblock;
        return;
    }
    // iterate to end of blocked list
    while (tracker->nextInBlockedList != NULL) {
        // Sort by IO burst time
        if(tracker->nextInBlockedList->IOBurst > newblock->IOBurst){
            newblock->nextInBlockedList = tracker->nextInBlockedList;
            tracker->nextInBlockedList = newblock;
            return;
        }
        tracker = tracker->nextInBlockedList; // Move to next in blocked list
    }
    // add to end of blocked list
    tracker->nextInBlockedList = newblock;
}

// Function to start a process
void start_process(_process* process, FILE* random_num_file){
    // Varaibles and setting process new status
    uint32_t burst, remaining_time;
    process->status = 2;

    // Cold start or resuming process
    if(process->CPUBurst == 0){ // Check if the CPU burst is 0
        remaining_time = process->C - process->currentCPUTimeRun; // Calculate the remaining time
        burst = randomOS(process->B, process->processID, random_num_file);

        //If burst greater than remainnig CPU time
        if(burst > remaining_time){
            process->CPUBurst = remaining_time; // Set the CPU burst to the remaining time
        } else {
            process->CPUBurst = burst;
            process->IOBurst = burst * process->M; // Set the IO burst to burst * M
        }
    }
    if (process->isFirstTimeRunning) {
        TOTAL_STARTED_PROCESSES++;
        process->isFirstTimeRunning = false;
    }
    // clear now running process pointers to avoid confusion
    process->nextInBlockedList = NULL;
    process->nextInReadyQueue = NULL;
}

// First Come First Serve Scheduler
void sim_fcfs(_process plist[], _process finished_plist[], FILE* frandom){
    // Initialize the process lists for FCFS
    _process *running = NULL, *ready = NULL, *blocked = NULL, *tracker = NULL;

    // Run loop for simulation to create all processess
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        // Manage blocked processes
        if (blocked){
            tracker = blocked;
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            tracker->IOBurst--; // Decrement the IO burst
            tracker->currentIOBlockedTime++; // Increment the IO blocked time

            while(tracker) { 
                if (tracker->IOBurst == 0) { // Check if IO burst is done
                    //tracker -> currentIOBlockedTime++;
                    fcfs_add_to_ready(&ready, tracker);
                    blocked = blocked->nextInBlockedList; // Move ahead in blocked list
                    tracker = blocked; // Set tracker for next iteration
                }else{
                    tracker = tracker->nextInBlockedList;
                }
            }
        }

        // Add arrivals to ready queue
        fcfs_add_arrivals(plist, &ready); // Add arrivals to ready queue

        // Run the current process
        if (running) {
            running->currentCPUTimeRun++; // Increment the CPU run time
            running -> CPUBurst--; // Decrement the CPU burst

            if (running->currentCPUTimeRun == running->C) {

                running->status = 4;
                running->finishingTime = CURRENT_CYCLE;
                finished_plist[TOTAL_FINISHED_PROCESSES++] = *running;
                running = NULL;
            } else if (running->CPUBurst == 0){ 
                // Check if CPU burst is done
                // Block for IO burst
                add_blocked(&blocked, running); // Add to blocked list
                running = NULL;
            }
        }

        // Switch to next ready process if not running
        if (!running && ready) {
            running = ready;
            ready = ready->nextInReadyQueue;
            start_process(running, frandom);
        }
        // Update waiting times for ready processes
        update_waiting(ready);
    }
}

/*************************** Round Robin (RR) *****************************/
// Run the round robin simulation
void sim_rr(_process plist[], _process finished_plist[], FILE* frandom){
    // Initialize the process lists for RR
    _process *running = NULL, *ready = NULL, *blocked = NULL, *tracker = NULL;
    int quantam_count = QUANTUM;

    // Run loop for simulation to create all processess
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        // Manage blocked processes
        if (blocked){
            tracker = blocked;
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            tracker->IOBurst--; // Decrement the IO burst
            tracker->currentIOBlockedTime++; // Increment the IO blocked time

            while(tracker) { 
                if (tracker->IOBurst == 0) { // Check if IO burst is done
                    //tracker -> currentIOBlockedTime++;
                    fcfs_add_to_ready(&ready, tracker);
                    blocked = blocked->nextInBlockedList; // Move ahead in blocked list
                    tracker = blocked; // Set tracker for next iteration
                }else{
                    tracker = tracker->nextInBlockedList;
                }
            }
        }

        // Add arrivals to ready queue
        fcfs_add_arrivals(plist, &ready); // Add arrivals to ready queue

        // Run the current process
        if (running) {
            running->currentCPUTimeRun++;
            running -> CPUBurst--;

            if (running->currentCPUTimeRun == running->C) {
                running->status = 4;
                running->finishingTime = CURRENT_CYCLE;
                finished_plist[TOTAL_FINISHED_PROCESSES++] = *running;
                running = NULL;
            } else if (running->CPUBurst == 0){ 
                // Check if CPU burst is done & block for IO burst
                add_blocked(&blocked, running); // Add to blocked list
                running = NULL;
            } else if (running->quantum == 0) {
                // Check if quantum is done
                fcfs_add_to_ready(&ready, running); // Add to ready queue
                running = NULL;
            }
        }
        // switch contexts if quantum is done or process done running
        if(!running || quantam_count == 0) {
            quantam_count = QUANTUM;

            // If running process is not done, add to ready queue
            if(running) {
                fcfs_add_to_ready(&ready, running);
                running = NULL;
            }
            // Switch to next ready process if not running
            if(ready) {
                running = ready;
                if(running->nextInReadyQueue){
                    ready = ready->nextInReadyQueue;
                } else {
                    ready = NULL;
                }
                start_process(running, frandom);
            }
        }
        update_waiting(ready);
        quantam_count--;
    }
}

/*************************** Shortest Job First (SJF) *****************************/
// Function to add shortest job first to ready queue
void sjf_add_to_ready(_process **ready, _process *newready) {
    _process *tracker = *ready;

    // Calculate remaining time for new and current process
    uint32_t remaining_time_newproc = newready->C - newready->currentCPUTimeRun;

    // If the ready queue is empty add the new process
    if (tracker == NULL) {
        newready->status = 1;
        *ready = newready;
        return;
    }
    // Calculate remaining time for current process
    uint32_t remaining_time_current = (*ready)->C - (*ready)->currentCPUTimeRun;

    // If the new process has a shorter remaining time than the current process
    if (remaining_time_newproc < remaining_time_current) {
        newready->nextInReadyQueue = *ready;
        newready->status = 1;
        *ready = newready;
        return;
    }
    // Iterate to end of ready queue
    while (tracker->nextInReadyQueue != NULL) {
        remaining_time_current = tracker->nextInReadyQueue->C - tracker->nextInReadyQueue->currentCPUTimeRun;

        // If the new process has a shorter remaining time than the current process
        if (remaining_time_newproc < remaining_time_current) {
            // Swap processes
            newready->nextInReadyQueue = tracker->nextInReadyQueue;
            tracker->nextInReadyQueue = newready;
            newready->status = 1;
            return;
        }
        tracker = tracker->nextInReadyQueue;
    }
    // If not found in while loop (It is the longest), Add to end of ready queue
    tracker->nextInReadyQueue = newready;
    newready->status = 1;
}

// Function to add arrivals to ready queue
// Same as FCFS but with shortest job first
void sjf_add_arrivals(_process process_list[], _process** ready){
    // iterate over process list and add if scheduled to arrive
    for (uint32_t i = 0; i < TOTAL_CREATED_PROCESSES; i++) {
        if (process_list[i].A == CURRENT_CYCLE) {
            sjf_add_to_ready(ready, &process_list[i]);
        }
    }
}

// Run the shortest job first simulation
void sim_sjf(_process plist[], _process finished_plist[], FILE* frandom){
    // Initialize the process lists for SJF
    _process *running = NULL, *ready = NULL, *blocked = NULL, *tracker = NULL;

    // Run loop for simulation to create all processess and manage them
    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        // Manage blocked processes
        if (blocked){
            tracker = blocked;
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            tracker->IOBurst--; // Decrement the IO burst
            tracker->currentIOBlockedTime++; // Increment the IO blocked time

            while(tracker) { 
                if (tracker->IOBurst == 0) { // Check if IO burst is done
                    //tracker -> currentIOBlockedTime++;
                    sjf_add_to_ready(&ready, tracker);
                    blocked = blocked->nextInBlockedList; // Move ahead in blocked list
                    tracker = blocked; // Set tracker for next iteration
                }else{
                    tracker = tracker->nextInBlockedList;
                }
            }
        }
        // Add potential arrivals to ready queue
        sjf_add_arrivals(plist, &ready);
        // Run the current process
        if (running) {
            running->currentCPUTimeRun++;
            running -> CPUBurst--;

            if (running->currentCPUTimeRun == running->C) { // If process done
                running->status = 4;
                running->finishingTime = CURRENT_CYCLE;
                finished_plist[TOTAL_FINISHED_PROCESSES++] = *running;
                running = NULL;
            } else if (running->CPUBurst == 0){ // Check if CPU burst is done & block for IO burst
                add_blocked(&blocked, running); // Add to blocked list
                running = NULL;
            }
        }
        // Switch to next ready process if nothing running
        if (!running && ready) {
            running = ready;
            if(running->nextInReadyQueue){
                ready = ready->nextInReadyQueue;
            } else {
                ready = NULL;
            }
            start_process(running, frandom);
        }
        // Update waiting times for ready processes
        update_waiting(ready);
    }
}

int main(int argc, char *argv[])
{
    // Read the file and fill the process list
    FILE* input = fopen(argv[1], "r");
    FILE* rand_file = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    _process* plist = create_process_list(input);
    _process* finished_plist = (_process*) malloc(TOTAL_CREATED_PROCESSES* sizeof(_process));

    // Print the start
    printf("Starting simulation with file: %s\n", argv[1]);

    // Run Simulation (for each scheduler)
    for (int scheduler = 0; scheduler < 3; scheduler++) {
        //Chose schedulers
        if (scheduler == 0) {
            printf("###############    Starting FSFS    ###############\n");
            printf("The scheduling algorithm used is First Come First Serve\n");
            printStart(plist);
            sim_fcfs(plist, finished_plist, rand_file);
            print_reset(plist, finished_plist); // Print results and reset counters
            printf("###############    Finished FSFS    ###############\n");
        } else if (scheduler == 1) {
            printf("###############    Starting Round Robin    ##############\n");
            printf("The scheduling algorithm used is Round Robin\n");
            printf("Quantum is %d\n", QUANTUM);
            printStart(plist);
            sim_rr(plist, finished_plist, rand_file);
            print_reset(plist, finished_plist); // Print results and reset counters
            printf("###############    Finished Round Robin    ##############\n");
        } 
        else {
            printf("###############    Starting Shortest Job First  ###############\n");
            printf("The scheduling algorithm used is Shortest Job First\n");
            printStart(plist);
            sim_sjf(plist, finished_plist, rand_file);
            print_reset(plist, finished_plist); // Print results and reset counters
            printf("###############    Finished Shortest Job First  ###############\n");
        }
    }
    return 0;
} 
