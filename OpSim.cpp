/*      Eric Neiman
*       CMSC 312 Intro to Operating Systems
*       Operating System Simulation Project Main file
*/

#include <iostream>
#include <thread>
#include <mutex>
#include <fstream>
#include <string>
#include <queue>
#include <stdlib.h>
#include <unistd.h>
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
    int inputOutput; // cycle location where the io interrupt is

    // Process constructor
	Process(int p, int tc, string name, int pr, int cs, int cl, int io) {
        pid = p;
        totalCycles = tc;
        remainingCycles = tc; // remainingCycles is always the same as totalCycles at creation
        pState = newP;
        processName = name;
        priority = pr;
        criticalStart = cs;
        criticalLength = cl;
        memory = 1;
        inputOutput = io;
    }

    void printProcess() {
        cout << "\nProcess ID: " << pid;
        cout << "\nProcess Name: " << processName;
        cout << "\nRemaining Cycles: " << remainingCycles;
        cout << "\nState: " << pState;
        cout << "\nPriority: " << priority;
        cout << "\nCritical Start: " << criticalStart;
        cout << "\nCritical Length: " << criticalLength;
    }
}; // end of the process class


// Memory management class
class Memory {
    public:
        int memory[4] = {-1}; // initializes the memory (RAM) 4 pages
        int memoryUsage = 0; // keeps track of the overall memory usage
        
        Memory() {
            cout << "\nInitiating Memory";
        }

        void addProcess(Process p) {
            if (memoryUsage < 4) { // if there is room in memory for the process we put it in there
                for (int i = 0; i < 4; i++) {
                    if (memory[i] == -1) {
                        p.pState = ready; // process is now ready
                        memory[i] = p.pid; // the momory now holds the pid for the process
                        memoryUsage = memoryUsage + 1; // incrememnt memory usage
                        cout << "\nAdding Process " << p.processName << " to the memory.";
                        // cout << "\n" << 4 - memoryUsage << "MB free";
                        break;
                    }
                } 
            } else { // we will have to manage the memory/storage and remove something
                memory[0] = memory[1];
                memory[1] = memory[2];
                memory[2] = memory[3];
                memory[3] = p.pid; // the last location in the memory now holds the pid for the process
                p.pState = ready; // the process is now ready
                cout << "\nMemory full. Adding Process " << p.processName << " to the memory.";
                // cout << "\n" << 4 - memoryUsage << "MB free";   
            }
        }

        bool checkMemory(Process p) {
            bool inMemory = false;
            for (int i = 0; i <= 4; i++) {
                if (memory[i] == p.pid) {
                    inMemory = true; // if the process was found in the memory
                    break;
                }
            }
            if (inMemory == true) {
                cout << "\nProcess " << p.processName << " is already in memory.";
            } else {
                cout << "\nProcess " << p.processName << " is not in memory.";
            }
            return inMemory;
        }

        void removeProcess(Process p) {
            for (int i =0; i < 4; i++) {
                if (memory[i] == p.pid) {
                    memory[i] = -1; // if the process is found in memory we remove it
                    memoryUsage--; // decrememnt memory usage
                    cout << "\nRemoving Process " << p.processName << " from the memory.";
                    // cout << "\n" << 4 - memoryUsage << "MB free";
                    break;
                }
            }
        }
};


// global variables
    int numberOfProcesses = 0; // keeps track of the number of process created thus far so the pids don't overlap
    queue<Process> readyQueue; // empty readyQueue for processes
    Memory MainMemory = Memory();
    mutex mtx;

void helpMenu() {
    cout << "\nList of commands: help";
    cout << "\nAdd a job: add <path to jobFile>";
    cout << "\nCreate a process: create process";
    cout << "\nGenerate processes: generate";
    cout << "\nRun the round robin: run round";
    cout << "\nRun the priority: run priority";
}

void generateProcesses(int number) {
    int pid;
    int totalCycles;
    string name;
    int priority;
    int criticalStart;
    int criticalLength;
    int inputOutput;
    for (int i = 0; i < number; i++) {
        pid = numberOfProcesses;
        name = "generated";
        numberOfProcesses++;
        totalCycles = rand() % 200 + 51; // random number from 50 to 250
        priority = rand() % 3; // random number from 0 to 2
        criticalStart = rand() % (totalCycles - 31) + 31; // random number from 30 to to 210
        criticalLength = rand() % 40 + 21; // random number from 20 to 60 
        inputOutput = rand() & (totalCycles - 31) + 31; 
        Process p = Process(pid, totalCycles, name, priority, criticalStart, criticalLength, inputOutput);
        readyQueue.push(p);
    }
}

void roundRobin() {
    int cycles = 20; // number of cycles before switching to the next process
    while (!readyQueue.empty()) {
        mtx.lock(); // locks when the thread is going to access the ready queue
        Process current = readyQueue.front(); // sets current to the front of the queue
        bool inMemory = MainMemory.checkMemory(current); 
        if (inMemory == false) { // checks if the current process is in memory and adds it to the memory if it isn't
            MainMemory.addProcess(current); // adds the process to the memory if it is not already in it
        }
        readyQueue.pop(); // removes current from the queue
        current.pState = running; // the current process is now running
        mtx.unlock(); // unlocks after the thread has accessed the queue
        for (int i = 0; i < cycles; i++) { // for loop simulates running a cycle on the CPU
            current.remainingCycles--;
            current.criticalStart--; // if critical section gets to 0 the critical section has started
            current.inputOutput--;
            if (current.criticalStart == 0) {
                cout << "\nCritical Section Started for process " << current.processName << " pid: " << current.pid;
                break; // when the critical section starts we break from the normal cycle
            }
            if (current.inputOutput == 0){
                cout << "\nIO INTERUPT in process: " << current.processName << " pid: " << current.pid;
                usleep(1000);
            }
        }

        if (current.criticalStart == 0) { // checks of critical start point has been reached and if there are still cycles left in the critical section
            for (int i = 0; i < current.criticalLength; i++) {
                current.remainingCycles--;
                current.inputOutput--;
                if (current.inputOutput == 0){
                    cout << "\nIO INTERUPT in process: " << current.processName << " pid: " << current.pid;
                    usleep(1000);
                }
            }
        }

        if (current.remainingCycles < 0) { // checks if the process has finished
            mtx.lock();
            MainMemory.removeProcess(current); // removes a process from the memory when it is being terminated
            cout << "\nFinishing process " << current.processName << " pid: " << current.pid;
            current.pState = terminated; // sets the processes state to terminated
            mtx.unlock();
        } else {
            mtx.lock();
            current.pState = ready; // the process is being put back into the ready queue
            cout << "\nRunning " << current.processName << " pid: " << current.pid << " has " << current.remainingCycles << " cycles left before it completes.";
            readyQueue.push(current); // puts the current process at the back of the queue to wait for its turn again
            mtx.unlock();
        }
    }
    return;
}

void priorityRobin() {
    int cycles = 20; // number of cycles before switching to the next process
    int runningCycles = 20;
    while (!readyQueue.empty()) {
        mtx.lock(); // locks to access the ready queue
        Process current = readyQueue.front(); // sets current to the front of the queue
        bool inMemory = MainMemory.checkMemory(current); 
        if (inMemory == false) { // checks if the current process is in memory and adds it to the memory if it isn't
            MainMemory.addProcess(current); // adds the process to the memory if it is not already in it
        }
        readyQueue.pop(); // removes current from the queue
        mtx.unlock(); // unlocks once the queue has been accessed
        current.pState = running; // the current process is now running
        if (current.priority == 0) { // low priority
            runningCycles = 20;
        } else if (current.priority == 1) { // medium priority
            runningCycles = 25;
        } else if (current.priority == 2) { // high priority
            runningCycles = 30;
        }
        for (int i = 0; i < runningCycles; i++) { // for loop simulates running a cycle on the CPU
            current.remainingCycles--;
            current.criticalStart--; // if critical section gets to 0 the critical section has started
            current.inputOutput--;
            if (current.criticalStart == 0) {
                cout << "\nCritical Section Started for process " << current.processName << " pid: " << current.pid;
                break; // when the critical section starts we break from the normal cycle
            }
            if (current.inputOutput == 0) {
                cout << "\nIO INTERUPT in process: " << current.processName << " pid: " << current.pid;
                usleep(1000);
            }
        }

        if (current.criticalStart == 0) { // checks of critical start point has been reached and if there are still cycles left in the critical section
            for (int i = 0; i < current.criticalLength; i++) {
                current.remainingCycles--;
                current.inputOutput--;
                if (current.inputOutput == 0){
                    cout << "\nIO INTERUPT in process: " << current.processName << " pid: " << current.pid;
                    usleep(1000);
                }
            }
        }

        if (current.remainingCycles < 0) { // checks if the process has finished
            mtx.lock();
            MainMemory.removeProcess(current); // removes a process from the memory when it is being terminated
            cout << "\nFinishing process " << current.processName << " pid: " << current.pid;
            current.pState = terminated; // sets the processes state to terminated
            mtx.unlock();
        } else {
            current.pState = ready; // the process is being put back into the ready queue
            mtx.lock();
            cout << "\nRunning " << current.processName << " pid: " << current.pid << " has " << current.remainingCycles << " cycles left before it completes.";
            readyQueue.push(current); // puts the current process at the back of the queue to wait for its turn again
            mtx.unlock();
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
    cout << "\nEnter the location of the inputOutput: ";
    int inputOutput;
    cin >> inputOutput;
    cout << "\nCreating a process: " << name << ".";
    Process p = Process(pid, totalCycles, name, priority, criticalStart, criticalLength, inputOutput);
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
        string ioString;
        int inputOutput = -1;
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
            if (line == "IO") {
                jobFile >> ioString;
                inputOutput = stoi(ioString);
            }
            if (line == "-") {
                cout << "\n\nCreating Process from: " << path;
                Process jobProcess = Process(numberOfProcesses, cycles, name, priority, criticalStart, criticalLength, inputOutput);
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
            thread one (roundRobin);
            thread two (roundRobin);
            one.join();
            two.join();
        }
        else if (command == "run priority") {
            priorityRobin();
        }
        else if (command == "generate") {
            cout << "\nEnter the number of processes to be generated. ";
            int processes;
            cin >> processes;
            generateProcesses(processes);
        }
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
