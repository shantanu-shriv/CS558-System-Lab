#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <fstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <sstream>

using namespace std;

std::vector<std::string> split_string_by_newline(const std::string& str)
{
    std::vector<std::string> result;
    std::stringstream ss(str);

    std::string line;
    while (std::getline(ss, line, '\n'))
        result.push_back(line);

    return result;
}


int main() {
    int pipe1[2], pipe2[2], n;
    pid_t pid1, pid2;
    pipe(pipe1);
    pipe(pipe2);
    pid1 = fork();
    if(pid1 == 0){
        // Child 1 process
        struct dirent *d;

        struct stat dst;

        vector<string> files;
        vector<string> file_contents;

        DIR *dr;

        string path = "./d1";

        dr = opendir(path.c_str());


        // Iterate through directory files
        if(dr != NULL) {
            for(d = readdir(dr); d != NULL; d = readdir(dr)) {
                string filename = d -> d_name;
                if(filename == "." || filename == "..")
                    continue;
                else {
                    string s;
                    string contents = "";
                    // add filename to filename vector
                    files.push_back(filename);
                    filename = path + "/" + filename;
                    ifstream in(filename);
                    // store contents of files
                    while(getline(in, s)) {
                        contents += s;
                        contents += '\n';
                    }
                    file_contents.push_back(contents);
                }
            }
        }

        closedir(dr);

        close(pipe1[0]);

        string s = to_string(files.size());


        // write number of files to pipe
        write(pipe1[1], s.c_str(), 1);
        write(pipe1[1], "\n", 1);


        // write filenames to pipe
        for (const auto& s : files) {
            write(pipe1[1], s.c_str(), s.size());
            write(pipe1[1], "\n", 1);
        }


        // write contents of files to pipe
        for (const auto& s : file_contents) {
            write(pipe1[1], s.c_str(), s.size());
            write(pipe1[1], "\n", 1);
        }

        close(pipe1[1]);
    


        close(pipe2[1]);
        char buf[1024];

        ssize_t n = read(pipe2[0], buf, sizeof(buf));
        string t(buf, n);
        string received_string = t;

        close(pipe2[0]);

        // split string received from pipe by newline
        vector<string> received_strings = split_string_by_newline(received_string);

        int files_count = stoi(received_strings[0]);

        vector<string> filenames;
        vector<string> file_contents_read;

        // store filenames to vector
        for(int i = 1; i <= files_count; i++)
            filenames.push_back(received_strings[i]);

        // store file contents to vector
        for(int i = files_count + 1; i <= 4 * files_count; i++)
            file_contents_read.push_back(received_strings[i]);
	
	
        //string path = "./d1";

        string filename;
        
        for(int i = 0; i < files_count; i++) {
                filename = path + "/" + filenames[i];
                ofstream file_;
                file_.open(filename);
                for(auto j:file_contents_read)
                	file_ << j <<endl;
                file_.close();
            }
        
    
    }
    else {

        pid2 = fork();

        if(pid2 == 0) {

            struct dirent *d;

            struct stat dst;

            vector<string> files;
            vector<string> file_contents;

            DIR *dr;

            string path = "./d2";

            dr = opendir(path.c_str());

            if(dr != NULL) {
                for(d = readdir(dr); d != NULL; d = readdir(dr)) {
                    string filename = d -> d_name;
                    if(filename == "." || filename == "..")
                        continue;
                    else {
                        string s;
                        string contents = "";
                        files.push_back(filename);
                        filename = path + "/" + filename;
                        ifstream in(filename);
                        while(getline(in, s)) {
                            contents += s;
                            contents += '\n';
                        }
                        file_contents.push_back(contents);
                    }
                }
            }
            
            
            //cout<<file_contents[0];

            closedir(dr);

            close(pipe2[0]);

            string s = to_string(files.size());

            write(pipe2[1], s.c_str(), 1);
            write(pipe2[1], "\n", 1);

            for (const auto& s : files) {
                write(pipe2[1], s.c_str(), s.size());
                write(pipe2[1], "\n", 1);
            }

            for (const auto& s : file_contents) {
                write(pipe2[1], s.c_str(), s.size());
                write(pipe2[1], "\n", 1);
            }

            close(pipe2[1]);
            //_exit(0);


            close(pipe1[1]);
            char buf[1024];

            ssize_t n = read(pipe1[0], buf, sizeof(buf));
            string t(buf, n);
            string received_string = t;

            close(pipe1[0]);

            vector<string> received_strings = split_string_by_newline(received_string);
	    	
		//cout<<received_strings[3];
	
            int files_count = stoi(received_strings[0]);

            vector<string> filenames;
            vector<string> file_contents_read;

            for(int i = 1; i <= files_count; i++)
                filenames.push_back(received_strings[i]);
                
		
            for(int i = files_count + 1; i <= 4 * files_count; i++)
            {
                file_contents_read.push_back(received_strings[i]);
	     }

            //string path = "./d2";

            string filename;
            
             for(int i = 0; i < files_count; i++) {
                filename = path + "/" + filenames[i];
                ofstream file_;
                file_.open(filename);
                for(auto j:file_contents_read)
                	file_ << j <<endl;
                file_.close();
            }
            
       
    }
                     
        

    }
}
