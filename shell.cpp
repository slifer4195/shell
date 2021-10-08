#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

using namespace std;

string trim (string input){
    int i = 0;
    while (i < input.size() && input[i] == ' ')
        i++;
    if (i < input.size())
        input = input.substr(i);
    else{
        return "";
    }

    i = input.size() - 1;
    while (i >= 0 && input[i] == ' ')
        i--;
    if (i >= 0)
        input = input.substr(0, i+1);
    else
        return "";

    return input;
}


char** vec_to_char_array (vector<string>& parts){
    char** result = new char * [parts.size() + 1];
    for (int i = 0; i < parts.size(); i++){
        result [i] = (char* ) parts [i].c_str();
    }
    result[parts.size()] = NULL;
    return result;
}

vector<string> split(string line, string seperators = " "){
    string newline = trim(line);
    vector<string> splVec;
    int index;
    int count = 0;
    for (int i = 0; i < newline.length(); i++){
        if (newline[i] == ' '){
            count += 1;
        }
    }
}

int main(){
    vector<int> bgs;
    while (true){
        for (int i = 0 ; i < bgs.size();i++){
            if (waitpid (bgs [i], 0,WNOHANG) < 0){
                ;
            }else{
                bgs.erase (bgs.begin() + i);
                i--;
            }
        }
        cout << "My shells$: ";
        string inputline;
        getline(cin, inputline);
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }
        int pid = fork();
        bool bg = false;
        inputline = trim(inputline);

        if (inputline[inputline.size()-1] == '&'){
            cout << "BG process found" << endl;
            bg = true;
            inputline = inputline.substr (0, inputline.size() - 1);
        }

        if (pid == 0){
            vector<string> parts = split(inputline);
            char** args = vec_to_char_array(parts);
            execvp(args [0], args);
        }else{
            if (!bg)
                waitpid(pid, 0,0);
            else{
                bgs.push_back(pid);
            }
        }

    }
    return 0;
}