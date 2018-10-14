/*      Eric Neiman
*       CMSC 312, Intro to Operating Systems
*       Operating System Simulation Project Main file
*/

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define maxRam = 2048; // maximum amount of ram (int Mb) available to the operating system


void helpMenu() {
    cout << "\nList of commands";
    cout << "\nRun a program: run <program name>";
    cout << "\n2";
}

int main(int argc, char* argv[]) {
    bool running = true; // is the operating system running
    string command = "";
    while (running) {
        cout << "\nPlease enter a command: ";
        cin >> command;
        cout << "\nExecuting " << command << "...";

        if (command == "help") {
            helpMenu();
        }
    }

}

// States of a process
// new: the process is being created
// running: instructions are being executed
// waiting: the process is waiting for some event to occur
// ready: the process is waiting to be assigned to a processor
// terminated: the process has nished execution
