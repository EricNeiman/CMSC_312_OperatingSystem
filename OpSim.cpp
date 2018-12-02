/*      Eric Neiman
*       CMSC 312 Intro to Operating Systems
*       Operating System Simulation Project Main file
*/

#include <iostream>
#include <fstream>
#include <string>
#include <queue>
#include <semaphore.h>

using namespace std;
enum state { newP, running, waiting, ready, terminated };
// newP: the process is being created
// running: instructions are being executed
// waiting: the process is waiting for some event to occur
// ready: the process is waiting to be assigned a to a processor (first time the process goes into memory))
// terminated: the process has finished executing


class Process {
public:
    int pid; // process ID number
    int totalCycles; // total number of cycles it takes to finish a process
    int remainingCycles; // number of cycles remaining before a process is complete
    string processName; // name of the process
    state pState; // state the process is in [ running, waiting, ready, terminated ] 
    int priority; // priority of the process: 0 = low, 1 = medium, 2 = high
    int criticalStart; // critical section has a start cycle
    int criticalLength; // critical section has a length in cycles
    int memory; // memory usage in MB

    // Process constructor
	Process(int p, int tc, string name, int pr, int cs, int cl, int m) {
        pid = p;
        totalCycles = tc;
        remainingCycles = tc; // remainingCycles is always the same as totalCycles at creation
        pState = newP;
        processName = name;
        priority = pr;
        criticalStart = cs;
        criticalLength = cl;
        memory = m;
    }

    void printProcess() {
        cout << "\nProcess ID: " << pid;
        cout << "\nProcess Name: " << processName;
        cout << "\nRemaining Cycles: " << remainingCycles;
        cout << "\nState: " << pState;
        cout << "\nPriority: " << priority;
        cout << "\nCritical Start: " << criticalStart;
        cout << "\nCritical Length: " << criticalLength;
        cout << "\nMemory Usage in MB: " << memory;
    }
}; // end of the process class



// global variables
    int numberOfProcesses = 0; // keeps track of the number of process created thus far so the pids don't overlap
    int memory[256][8] = {-1}; // initializes the memory (RAM)
    sem_t semaphores[256][8]; // 2d array of semaphores for locking down the corresponding memory locations
    queue<Process> readyQueue; // empty readyQueue for processes

void helpMenu() {
    cout << "\nList of commands: help";
    cout << "\nAdd a job: add <path to jobFile>";
    cout << "\nCreate a process: create process";
    cout << "\nRun the round robin: run round";
}


void roundRobin() {
    int cycles = 20; // number of cycles before switching to the next process
    while (!readyQueue.empty()) {
        Process current = readyQueue.front(); // sets current to the front of the queue
        readyQueue.pop(); // removes current from the queue
        current.pState = running; // the current process is now running

        for (int i = 0; i < cycles; i++) { // for loop simulates running a cycle on the CPU
            current.remainingCycles = current.remainingCycles - 1;
            current.criticalStart = current.criticalStart -1; // if critical section gets to 0 the critical section has started
            if (current.criticalStart == 0) {
                cout << "\nCritical Section Started for process " << current.processName << ".";
                break; // when the critical section starts we break from the normal cycle
            }
        }

        if (current.criticalStart == 0) { // checks of critical start point has been reached and if there are still cycles left in the critical section
            for (int i = 0; i < current.criticalLength; i++) {
                    current.remainingCycles = current.remainingCycles - 1;
            }
        }

        if (current.remainingCycles < 0) { // checks if the process has finished
            cout << "\nFinishing process " << current.processName << " pid: " << current.pid;
            current.pState = terminated; // sets the processes state to terminated
        } else {
            current.pState = ready; // the process is being put back into the ready queue
            cout << "\nRunning " << current.processName << " pid: " << current.pid << " has " << current.remainingCycles << " cycles left before it completes.";
            readyQueue.push(current); // puts the current process at the back of the queue to wait for its turn again
        }
    }
    return;
}


void addUserProcess(int numProc) {
    int pid = numProc;
    cout << "\nEnter the amount of cycles this process takes: ";
    int totalCycles;
    cin >> totalCycles;
    cout << "\nEnter the process name: ";
    string name;
    cin >> name;
    cout << "\nEnter the process' priority: ";
    int priority;
    cin >> priority;
    cout << "\nEnter the start cycle for the critical section: ";
    int criticalStart;
    cin >> criticalStart;
    cout << "\nEnter the length of the critical section in cycles: ";
    int criticalLength;
    cin >> criticalLength;
    cout << "\nEnter the amount of memory in MB that the process will need: ";
    int memory;
    cin >> memory;
    cout << "\nCreating a process: " << name;
    Process p = Process(pid, totalCycles, name, priority, criticalStart, criticalLength, memory);
    readyQueue.push(p);
    return;
}


void addFile(string path) {
    ifstream jobFile; // creates input file stream
    jobFile.open(path); // points jobFile to the path given
    if (jobFile.is_open()) {
        cout << "\n" << path << " opened";
        string line;
        string name = "default";
        string cycleString;
        int cycles = -1;
        string priorityString;
        int priority = -1;
        string criticalStartString;
        int criticalStart = -1;
        string criticalLengthString;
        int criticalLength = 0;
        string memoryString;
        int memory = 1;
        while (line != "EXE") {
            jobFile >> line;
            if (line == "NAME") {
                jobFile >> name;
            }
            if (line == "LOAD") {
                jobFile >> cycleString;
                cycles = stoi(cycleString, nullptr, 10);
            }
            if (line == "PRIORITY") {
                jobFile >> priorityString;
                priority = stoi(priorityString, nullptr, 10);
            }
            if (line == "CRITICALS") {
                jobFile >> criticalStartString;
                criticalStart = stoi(criticalStartString, nullptr, 10);
            }
            if (line == "CRITICALL") {
                jobFile >> criticalLengthString;
                criticalLength = stoi(criticalLengthString, nullptr, 10);
            }
            if (line == "MEMORY") {
                jobFile >> memoryString;
                memory = stoi(memoryString, nullptr, 10);
            }
            if (line == "-") {
                cout << "\n\nCreating Process from: " << path;
                Process jobProcess = Process(numberOfProcesses, cycles, name, priority, criticalStart, criticalLength, memory);
                jobProcess.printProcess();
                readyQueue.push(jobProcess);
                numberOfProcesses++;
                name = "default";
                cycles = -1;
            }
        }
    } else {
        cout << "\nFile not found"; // the path did not point to a file
    }
    jobFile.close(); // closes jobFile
    return;
}


int main(int argc, char* argv[]) {
    bool running = true; // is the operating system running
    string command = "";

    helpMenu(); // calls the help menu at the start of the application
    while (running) {
        cout << "\nPlease enter a command: ";
        getline(cin, command);
        if (command == "help") {
            helpMenu();
        }
        else if (command.compare(0, 4, "add ") == 0) {
            string pathToJob = command.substr(4, command.length());
            addFile(pathToJob);
        }
        else if (command == "create process") {
            addUserProcess(numberOfProcesses);
            numberOfProcesses++;
        }
        else if (command == "run round") {
            roundRobin();
        }
        // pritority queue goes here
        else if (command.compare(0, 4, "exit") ==  0) {
            cout << "\nExiting the Operating System\n";
            running = false;
        }
        else {
            cout << "Not a valid input, try: help";
        }
    }
    exit(0); // exiting
}
