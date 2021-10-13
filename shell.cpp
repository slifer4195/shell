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

    vector<string> vec;
    int index;
    int cnt = 0;
    for (int i = 0; i < newline.length(); i++){
        if (newline[i] == seperators.at(0)){
            cnt += 1;
        }
    }
    for (int j = 0; j < cnt + 1; j++){
        index = newline.find(seperators);
        vec.push_back(newline.substr(0, index));
        newline =newline.substr(index+1);
    }
    return vec;
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


void quote(string inputline, int & start, int & end, bool single){
    if (single){
        start = inputline.find("'", 0);
        end = inputline.find("'", start+1);
    }
    else{
        start = inputline.find("\"", 0);
        end = inputline.find("\"", start+1);
    }
}
void printVector(vector<string> line){
    for (int i = 0; i < line.size(); i++){
        cout << line.at(i) << "--";
    }
    cout << endl;
}


bool singleQuote(string inputline){
    for (int i = 0; i < inputline.size(); i++){
        if (inputline.at(i) == '\'') return true;
    }
    return false;
}

bool doubleQuote(string inputline){
    for (int i = 0; i < inputline.size(); i++){
        if (inputline.at(i) == '\"') return true;
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

void changeDirectory(vector<string> command){
    char** array = vec_to_char_array(command);
    char s[100];
    char* word = array[1];
    chdir(word);
    char* name = get_current_dir_name();
}


int main(){
    vector<int> bgs;
    int in_def = dup(0);
    int out_def = dup(1);
  
    const string username = getenv("USER");
    while (true){
        dup2(in_def, 0);
        dup2(out_def, 1);
        for (int i = 0 ; i < bgs.size();i++){
            if (waitpid (bgs [i], 0,WNOHANG) == bgs[i]){
                bgs.erase (bgs.begin() + i);
                i--;
            }
        }
        time_t timeNow;
        struct tm* timeInformation;

        time(&timeNow);
        timeInformation = localtime(&timeNow);
        string showTime = asctime(timeInformation);
        showTime.pop_back();
        string prompt = username + ": " + showTime + " $";
        cout << prompt;
        string inputline; 
        getline(cin, inputline);
        if (inputline == string("exit")){
            cout << "Bye!! End of shell" << endl;
            break;
        }
        bool bg = false;
        inputline = trim(inputline);
        bool change = false;

        if (inputline[inputline.size()-1] == '&'){
            // cout << "BG process found" << endl;
            bg = true;
            inputline = inputline.substr (0, inputline.size() - 1);
        }
        vector<string> changeDirectoryVec = split(inputline);
        if (changeDirectoryVec.at(0) == "cd") {
            change = true;
            changeDirectory(changeDirectoryVec);
        }
        vector<string> c;
        c = split(inputline, "|");

        // if (change == false){
        for (int i = 0; i < c.size(); i++){
            int pid;
            int fd[2];
            if (change == false){
                pipe(fd);
                pid = fork();
            }
            if (pid == 0){
                if (i < c.size() - 1){
                    dup2(fd[1], 1);
                }
                string command;

                if (input(c[i]) && output(c[i])) {
                    //redirect input and output
                    command = outputInput(c[i]);
                    execute(command);
                }
                else if (input(c[i])){
                    //redirect input
                    command = directInput(c[i]);
                    execute(command);
                }
                else if (output(inputline)){
                    //redirect output
                    command = directOutput(c[i]);
                    execute(command);
                }
                else{
                    execute(c[i]);
                }
            }else{
                if (!bg)
                    if (i < c.size() - 1){
                        dup2(fd[0], 0);
                        close(fd[1]);
                    }else{
                        waitpid(pid, 0,0);
                    }
                else{
                    bgs.push_back(pid);
                }
            }
        }
    }

    return 0;
}