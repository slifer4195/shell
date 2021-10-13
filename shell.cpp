#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <fcntl.h>
#include <sstream>
#include <iomanip>

using namespace std;


bool isQuote(char a){
  return a == '\'' || a == '\"';
}

void splitPipes(string inputline, vector<string>& c){
  stringstream ss(inputline);
  string line;
  string total;

  while (!ss.eof()){
    char a = ss.peek();
    bool isquote = isQuote(a);

    if (isquote){
      ss >> quoted(line, a);
      line = a + line + a;

    }
    else{
      ss >> line;
      if (line == "|"){
        c.push_back(total);
        total.clear();
        continue;
      }
    }
    ss >> ws;
    if (total.size() != 0){
      total += " " + line;
    }
    else{
      total = line;
    }
    
  }

  if (total.size() > 0){
    c.push_back(total);
  }
}

void resetSS(stringstream& ss, size_t curPos, vector<string>& parts, string& inputline, string line, char redir){
  string prev = line.substr(0, line.find(redir));
  if (prev.size() > 0){
    parts.push_back(prev);
  }
  size_t newlinePos = curPos + prev.size();
  string newline = inputline.substr(newlinePos);
  newline = newline.substr(0, 1) + " " + newline.substr(1);
  inputline = newline;
  ss.str(inputline);
  ss.clear();  
}


void parseString(string inputline, vector<string>& parts, string& InputFile, string&OutputFile){
  stringstream ss(inputline);
  string line;

  while(!ss.eof()){

    char a = ss.peek();
    bool isquote = isQuote(a);

    if (isquote){
      ss >> quoted(line, a);
    }
    else{

      size_t curPos = ss.tellg();

      ss >> line;
      
      if (line == "<"){
        ss >> ws >> InputFile;
        continue;
      }
      else if (line == ">"){
        ss >> ws >> OutputFile;
        continue;
      }

      if (line.find('<') != string::npos){
        resetSS(ss, curPos, parts, inputline, line, '<');
        continue;
      }
      else if (line.find('>') != string::npos){
        resetSS(ss, curPos, parts, inputline, line, '>');
        continue;
      }
    }
    ss >> ws;
    parts.push_back(line);
  }
}



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

void execute(vector<string> parts) {
    char** args = vec_to_char_array(parts);
    int fd;
    execvp(args [0], args);
}

void directOutput(string Outputfile){
    char* output = (char*)Outputfile.c_str();
    int fd = open(output,O_CREAT|O_WRONLY|O_TRUNC,
    S_IRUSR| S_IWUSR | S_IRGRP |S_IROTH);
    dup2(fd, 1);
    close(fd);
}

void directInput(string Inputfile){

    char* output = (char*)Inputfile.c_str();
    int fd = open(output,O_CREAT|O_RDONLY,
    S_IRUSR|S_IRGRP |S_IROTH);
    dup2(fd, 0);
    close(fd);
}

void outputInput(string Inputfile, string Outputfile){

    char* input = (char*)Inputfile.c_str();  // a
    char* output = (char*)Outputfile.c_str();  // a

    int fd1 = open(input,O_CREAT|O_RDONLY, 
    S_IRUSR|S_IRGRP |S_IROTH);

    int fd2 = open(output, O_CREAT|O_WRONLY|O_TRUNC,
    S_IRUSR| S_IWUSR | S_IRGRP |S_IROTH);
    dup2(fd1, 0);
    dup2(fd2, 1);
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
        splitPipes(inputline, c);

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

                string InputFile = "", Outputfile = "";
                vector<string> parts;
                parseString(c[i], parts, InputFile, Outputfile);

                if (InputFile != "" && Outputfile != "") {
                    //redirect input and output
                    outputInput(InputFile, Outputfile);
                    execute(parts);
                }
                else if (InputFile != ""){
                    //redirect input
                    directInput(InputFile);
                    execute(parts);
                }
                else if (Outputfile != ""){
                    //redirect output
                    directOutput(Outputfile);
                    execute(parts);
                }
                else{
                    execute(parts);
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