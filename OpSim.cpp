#include <iostream>
#include <fstream>
#include <string>
using namespace std;

void helpMenu() {
    cout << "\nList of commands";
    cout << "\n1";
    cout << "\n2";
}

int main(int argc, char* argv[]) {

    string command = "";
    while (true) {
        cout << "\nPlease enter a command: ";
        cin >> command;
        cout << "\nExecuting " << command;

        if (command == "help") {
            helpMenu();
        }

    }

}
