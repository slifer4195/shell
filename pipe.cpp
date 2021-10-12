#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <fcntl.h>
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

    for (int j = 0; j < count + 1; j++){
        index = newline.find(seperators);
        splVec.push_back(newline.substr(0, index));
        newline =newline.substr(index+1);
    }
    return splVec;
}

bool input(string inputline ){
     vector<string> parts = split(inputline);
    for (int i = 0; i < parts.size(); i++){
        if (parts.at(i) == "<") return true;
    }
    return false;
}

bool output(string inputline){
     vector<string> parts = split(inputline);
    for (int i = 0; i < parts.size(); i++){
        if (parts.at(i) == ">") return true;
    }
    return false;
}

void execute(string inputline) {
    vector<string> parts = split(inputline);
    char** args = vec_to_char_array(parts);
    int fd;
    execvp(args [0], args);
}

string directOutput(string inputline){
    string seperator = ">";
    vector<string> parts = split(inputline, seperator);
    string command = parts.at(0);
    char* output = (char*)parts.at(1).c_str();
    int fd = open(output,O_CREAT|O_WRONLY|O_TRUNC,
    S_IRUSR| S_IWUSR | S_IRGRP |S_IROTH);
    dup2(fd, 1);
    close(fd);
    return command;
}

string directInput(string inputline){
    string seperator = "<";
    vector<string> parts = split(inputline, seperator);
    string command = parts.at(0);
    char* output = (char*)parts.at(1).c_str();
    int fd = open(output,O_CREAT|O_RDONLY,
    S_IRUSR|S_IRGRP |S_IROTH);
    dup2(fd, 0);
    close(fd);
    return command;
}

string outputInput(string inputline){
    string seperatorOutput = ">";
    vector<string> parts = split(inputline, seperatorOutput);//parts(0) grep rcu < a parts(1)  b

    string seperatorInput = "<";
    vector<string> line = split(parts.at(0), seperatorInput);//parts(0) grep rcu  parts(1) a

    char* input = (char*)line.at(1).c_str();  // a
    char* output = (char*)parts.at(1).c_str();  // a

    int fd1 = open(input,O_CREAT|O_RDONLY, 
    S_IRUSR|S_IRGRP |S_IROTH);

    int fd2 = open(output, O_CREAT|O_WRONLY|O_TRUNC,
    S_IRUSR| S_IWUSR | S_IRGRP |S_IROTH);
    dup2(fd1, 0);
    dup2(fd2, 1);
    execute(line.at(0));
    
    string command = line.at(0);
    return command;
}

int main(){
    vector<int> bgs;
    while (true){
        for (int i = 0 ; i < bgs.size();i++){
            if (waitpid (bgs [i], 0,WNOHANG) == bgs[i]){
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
        // int pid = fork();
        bool bg = false;
        inputline = trim(inputline);

        vector<string> c = split(inputline, "|");

        for (int i = 0; i < c.size() ; i++){
            // int fd [2];
            // pipe(fd);
            int pid = fork();

            // if (i < c.size() - 1){
            //     dup2(fd[1],1);
            // }
            
            if (inputline[inputline.size()-1] == '&'){
                // cout << "BG process found" << endl;
                bg = true;
                inputline = inputline.substr (0, inputline.size() - 1);
            }

            if (pid == 0){
                // execute() 
                string command;
                if (input(inputline) && output(inputline)) {
                    //redirect input and output
                    command = outputInput(inputline);
                    execute(command);
                }
                else if (input(inputline)){
                    //redirect input
                    command = directInput(inputline);
                    execute(command);
                }
                else if (output(inputline)){
                    //redirect input
                    command = directOutput(inputline);
                    execute(command);
                }
                else{
                    execute(c[i]);
                }
                // if (i < c.size() - 1){
                //     dup2(fd[1], 1);
                // }
                // execute(c[i]);

            }else{
                if (!bg)
                    waitpid(pid, 0,0);
                else{
                    bgs.push_back(pid);
                }
            }
        }
    }

    return 0;
}