#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>

using namespace std;

int main(int, char**){
    string pyFile = "python Voicerecognition.py";
    FILE * in = popen(pyFile.c_str(), "r");
    if(in){
        char buf[1024];
        while(fgets(buf, sizeof(buf), in))
        {
           printf("%s\n", buf);
           // Code to parse out values from the text in (buf) could go here
        }
        pclose(in);
    }

    
    //cout << status;
}