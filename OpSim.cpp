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

    // Process constructor
	Process(int p, int tc, string name, int pr, int cs, int cl) {
        pid = p;
        totalCycles = tc;
        remainingCycles = tc; // remainingCycles is always the same as totalCycles at creation
        pState = newP;
        processName = name;
        priority = pr;
        criticalStart = cs;
        criticalLength = cl;
    }

    void printProcess() {
        cout << "\nProcess ID: " << pid;
        cout << "\nProcess Name: " << processName;
        cout << "\nRemaining Cycles: " << remainingCycles;
        cout << "\nState: " << pState;
        cout << "\nPriority: " << priority;
    }
}; // end of the process class



// global variables
    int numberOfProcesses = 0; // keeps track of the number of process created thus far so the pids don't overlap
    int memory[256][8] = {-1}; // initializes the memory (RAM)
    sem_t semaphores[256][8]; // 2d array of semaphores for locking down the corresponding memory locations

void helpMenu() {
    cout << "\nList of commands: help";
    cout << "\nAdd a job: add <path to jobFile>";
    cout << "\nCreate a process: create process";
    cout << "\nRun the round robing: run round";
}


queue<Process> roundRobin(queue<Process> rq) {
    int cycles = 50; // number of cycles before switching to the next process
    while (!rq.empty()) {
        Process current = rq.front(); // sets current to the front of the queue
        rq.pop(); // removes current from the queue
        current.pState = running; // the current process is now running
        if (current.remainingCycles < cycles) {
            cout << "\nFinishing process " << current.processName << " pid: " << current.pid;
            current.pState = terminated; // sets the processes state to terminated
        } else {
            for (int i = 0; i < cycles; i++) {
                current.remainingCycles = current.remainingCycles - 1; // accounts for the number of cycles that were just run
            }
            current.pState = ready; // the process is being put back into the ready queue
            cout << "\nRunning " << current.processName << " pid: " << current.pid << " has " << current.remainingCycles << " cycles left before it completes.";
            rq.push(current); // puts the current process at the back of the queue to wait for its turn again
        }
    }
    return rq; // returns and empty queue once the list is empty
}


queue<Process> addUserProcess(queue<Process> rq, int numProc) {
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
    cout << "\nCreating a process: " << name;
    Process p = Process(pid, totalCycles, name, priority, criticalStart, criticalLength);
    rq.push(p);
    return rq;
}


queue<Process> addFile(queue<Process> rq, string path) {
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
        int criticalStart = 0;
        string criticalLengthString;
        int criticalLength = 0;
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
            if (line == "CRITICAL") {
                
            }
            if (name != "default" && cycles >= 0) {
                cout << "\nCreating Process from: " << path;
                cout << "\nProcess Name: " << name;
                cout << "\nNumber of Cycles: " << cycleString;
                Process jobProcess = Process(numberOfProcesses, cycles, name, priority, criticalStart, criticalLength);
                rq.push(jobProcess);
                numberOfProcesses++;
                name = "default";
                cycles = -1;
            }
        }
    } else {
        cout << "\nFile not found"; // the path did not point to a file
    }
    jobFile.close(); // closes jobFile
    return rq;
}


int main(int argc, char* argv[]) {
    bool running = true; // is the operating system running
    string command = "";
    queue<Process> readyQueue; // empty readyQueue for processes

    helpMenu(); // calls the help menu at the start of the application
    while (running) {
        cout << "\nPlease enter a command: ";
        getline(cin, command);
        if (command == "help") {
            helpMenu();
        }
        else if (command.compare(0, 4, "add ") == 0) {
            string pathToJob = command.substr(4, command.length());
            readyQueue = addFile(readyQueue, pathToJob);
        }
        else if (command == "create process") {
            readyQueue = addUserProcess(readyQueue, numberOfProcesses);
            numberOfProcesses++;
        }
        else if (command == "run round") {
            readyQueue = roundRobin(readyQueue);
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
