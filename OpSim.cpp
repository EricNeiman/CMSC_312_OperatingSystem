/*      Eric Neiman
*       CMSC 312, Intro to Operating Systems
*       Operating System Simulation Project Main file
*/

#include <iostream>
#include <fstream>
#include <string>
#include <queue>
using namespace std;

class Process{
public:
    int pid; // process ID number
    int totalCycles; // total number of cycles it takes to finish a process
    int remainingCycles; // number of cycles remaining before a process is complete
    string processName; // name of the process
    string state; // state the process is in [new, running, waiting, ready, terminated]

    // Process constructor
	Process(int p, int tc, string name) {
        pid = p;
        totalCycles = tc;
        remainingCycles = tc; // remainingCycles is always the same as totalCycles at creation
        processName = name;
    }

    void printProcess() {
        cout << "\nProcess ID: " << pid;
        cout << "\nProcess Name: " << processName;
        cout << "\nRemaining Cycles: " << remainingCycles;
    }
};

// global variable
    int numberOfProcesses = 0; // keeps track of the number of process created thus far so the pids don't overlap

void helpMenu() {
    cout << "\nList of commands";
    cout << "\nAdd a job: add <path to jobFile>";
    cout << "\nCreate a process: create process";
    cout << "\nRun the processes: run";
}

queue<Process> roundRobin(queue<Process> rq) {
    int cycles = 50; // number of cycles before switching to the next process
    while (!rq.empty()) {
        Process current = rq.front(); // sets current to the front of the queue
        rq.pop(); // removes current from the queue
        if (current.remainingCycles < cycles) {
            cout << "\nFinishing process " << current.processName << " pid: " << current.pid;
        } else {
            current.remainingCycles = current.remainingCycles - 50;
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
    cout << "Creating a process: " << name;
    Process p = Process(pid, totalCycles, name);
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
        while (line != "EXE") {
            jobFile >> line;
            if (line == "NAME") {
                jobFile >> name;
            }
            if (line == "LOAD") {
                jobFile >> cycleString;
                cycles = stoi(cycleString, nullptr, 10);
            }
            if (name != "default" && cycles >= 0) {
                cout << "\nCreating Process from: " << path;
                cout << "\nProcess Name: " << name;
                cout << "\nNumber of Cycles: " << cycleString;
                Process jobProcess = Process(numberOfProcesses, cycles, name);
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
        else if (command == "run") {
            readyQueue = roundRobin(readyQueue);
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
