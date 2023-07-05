#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <stdio.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/stdc++.h>
#include <signal.h>
#include <glob.h>
#include <dirent.h>
#include <errno.h>
#include <fstream>
#include <fcntl.h>
#include <readline/readline.h>
#include <fstream>
#include <sstream>

#define MAX_LEN 1024
using namespace std;



string strip(string s)
{
    for (int i = 0; i < s.size(); i++)
        if (s[i] != ' ')
        {
            s = s.substr(i);
            break;
        }
    for (int i = 0; i < s.size(); i++)
        if (s[s.size() - i - 1] != ' ')
        {
            s = s.substr(0, s.size() - i);
            break;
        }
    return s;
}

vector<string> split(string s, char delim)//splitting the string based on the delimiter
{
    vector<string> res;
    string elem = "";
    for (char c : s)
    {
        if (c == delim)
        {
            if (delim != ' ' || elem != "")
                res.push_back(elem);
            elem = "";
        }
        else
            elem.push_back(c);
    }
    res.push_back(elem);
    return res;
}

void redirect_io(string inp, string out)//redirecting the input and output to the files specified by the user 
{
    // Open input redirecting file
    if (inp.size())
    {
        int inp_fd = open(inp.c_str(), O_RDONLY); // Open in read only mode
        int status = dup2(inp_fd, 0);             // Redirect input 
        if (inp_fd < 0)
        {
            cout << "virtual-bash: " << inp << ": No such file or directory" << endl;
            exit(EXIT_FAILURE);
        }
        if (status < 0)
        {
            cout << "Input redirecting error" << endl;
            exit(EXIT_FAILURE);
        }
    }
    // Open output redirecting file
    if (out.size())
    {
        int out_fd = open(out.c_str(), O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // Open in create and truncate mode with read write execute permissions for user only 
        int status = dup2(out_fd, 1);                                          // Redirect output 
        if (status < 0)
        {
            cout << "Output redirecting error" << endl;
            exit(EXIT_FAILURE);
        }
    }
}

void exec_cmd(string cmd)//executing the command 
{
    vector<string> parsed = split(cmd, ' ');     // splitting the command based on spaces and storing it in a vector
    char **args = new char *[parsed.size() + 1]; // creating a char array to store the arguments of the command 
    for (int i = 0; i < parsed.size(); i++)      // storing the arguments in the char array 
        args[i] = (char *)parsed[i].c_str();     // converting the string to char array   
    args[parsed.size()] = NULL;
    execvp(args[0], args);      // executing the command
    perror("virtual-bash: command not found");
    exit(EXIT_FAILURE);
}

vector<string> split_io(string cmd) // function to split the input and output files from the command
{
    vector<string> parsed;
    string output, input, command;
    bool lsign = true, rsign = true;

    if (cmd.find('>') == string::npos) // check if output file does not exist
    {
        output = "";
        rsign = false;// if output file does not exist then set rsign to false
    }
    else
        output = split(cmd, '>').back(); // find the output file if exists

    output = strip(output);// strip the output string to remove the spaces
    output = split(output, '<')[0];// split the output string based on "<" sign
    if (output[0] == '\"')
        output = "\"" + split(output, '\"')[1] + "\"";
    else
        output = split(output, ' ')[0];
    output = strip(output);

    if (cmd.find('<') == string::npos) // check if input file does not exist
    {
        input = "";
        lsign = false;
    }
    else
        input = split(cmd, '<').back(); // find the input file if exists

    input = strip(input);
    input = split(input, '>')[0];
    if (input[0] == '\"')
        input = "\"" + split(input, '\"')[1] + "\"";
    else
        input = split(input, ' ')[0];
    input = strip(input);

    // handling and returning syntax errors for these cases :

    // case 1 : same input and output file , Treating this as error to avoid overwriting the input file
    if (input == output && (lsign || rsign))
    {
        cout << "virtual-bash: syntax error"
             << "\n";
        return {};
    }

    // case 2 : no input or outfile files corresponding to input and output symbols
    if ((lsign && input == "") || (rsign && output == ""))
    {
        cout << "virtual-bash: syntax error"
             << "\n";
        return {};
    }

    /* using regular expressions to extract the command, by erasing
    names of input output files and the "<" and ">" signs */
    regex re_op(output);
    regex re_inp(input);
    regex re_lsign("<");
    regex re_rsign(">");

    command = regex_replace(cmd, re_inp, "");// erasing the input file name from the command
    command = regex_replace(command, re_op, "");
    command = regex_replace(command, re_lsign, "");
    command = regex_replace(command, re_rsign, "");
    command = strip(command);

    input.erase(remove(input.begin(), input.end(), '\"'), input.end());
    output.erase(remove(output.begin(), output.end(), '\"'), output.end());
    parsed.push_back(command); // parsed[0] = command
    parsed.push_back(input);   // parsed[1] = input file
    parsed.push_back(output);  // parsed[2] = output file

    return parsed;
}

int check_chdir(string cmd)// function to check if the command is cd
{
    cmd = strip(cmd);
    if (split(cmd, ' ')[0] != "cd")
    {
        return -1;
    }
    cmd.replace(0, 3, "");
    cmd.erase(remove(cmd.begin(), cmd.end(), '\"'), cmd.end());

    if (!cmd.size())
    {
        chdir(getenv("HOME"));
        return 1;
    }
    cmd = strip(cmd);
    int status = chdir(cmd.c_str());
    if (status == -1)
    {
        cout << "virtual-bash: cd: " + cmd + ": No such file or directory\n";
    }
    return 1;
}

void exec_pipe(string cmd, int status, bool bg)// function to execute the pipe command 
{
    vector<string> parsed = split(cmd, '|');
    int num_pipes = parsed.size() - 1;
    int pipefds[2 * num_pipes];
    int pid;
    for (int i = 0; i < num_pipes; i++)
    {
        if (pipe(pipefds + i * 2) < 0)
        {
            perror("Pipe error");
            exit(EXIT_FAILURE);
        }
    }
    int inp = 0;
    for (int i = 0; i < parsed.size(); i++)
    {
        pid = fork();
        if (pid == 0)
        {
            if (i != 0)
            {
                if (dup2(pipefds[(i - 1) * 2], 0) < 0)
                {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
            }
            if (i != parsed.size() - 1)
            {
                if (dup2(pipefds[i * 2 + 1], 1) < 0)
                {
                    perror("dup2 error");
                    exit(EXIT_FAILURE);
                }
            }
            for (int j = 0; j < 2 * num_pipes; j++)
            {
                close(pipefds[j]);
            }
            vector<string> parsed_io = split_io(parsed[i]);
            if (parsed_io.size() == 0)
                exit(EXIT_FAILURE);
            string cmd = parsed_io[0];
            string inp = parsed_io[1];
            string out = parsed_io[2];
            redirect_io(inp, out);
            exec_cmd(cmd);
        }
    }
    for (int i = 0; i < 2 * num_pipes; i++)
    {
        close(pipefds[i]);
    }
    if (!bg)// if the command is not a background command
    {
        for (int i = 0; i < num_pipes + 1; i++)// wait for all the child processes to finish execution 
        {
            waitpid(pid, &status, 0);
        }
    }
}

void check_exit(string cmd)// function to check if the command is exit
{
    if (split(cmd, ' ')[0] == "exit")
    {
        cout << "Exiting shell ...\n";
        exit(EXIT_FAILURE);
    }
}

void sigint_handler(int signum)// function to handle ctrl-c signal 
{
    printf("\nInterrupt signal caught.\n");
    printf("virtual-bash: Enter command: ");
    fflush(stdout);
}

pid_t fg_pid = 0;
pid_t child_pid = 0;

void sigtstp_handler(int signum)// function to handle ctrl-z signal
{
    printf("\nStopped signal caught.\n");
    child_pid = fork();
    if (child_pid > 0)
    {
        waitpid(child_pid, NULL, 0);
    }

    // remove the fork if ctrl-c is printed twice

    printf("virtual-bash: Enter command: ");
    fflush(stdout);
}

string join_cmd(vector<string> tokens) // function to join the tokens of the command
{
    string cmd;
    for (int i = 0; i < tokens.size(); i++)
    {
        cmd += tokens[i];
        if (i < tokens.size() - 1)
            cmd += " ";
    }
    return cmd;
}

string expand_wildcards(string cmd)// function to expand wildcards
{
    vector<string> tokens = split(cmd, ' ');
    for (int i = 0; i < tokens.size(); i++)
    {
        string token = tokens[i];
        if (token.find("*") == string::npos && token.find("?") == string::npos)
            continue;

        vector<string> matching_files;
        glob_t glob_result;
        int glob_return = glob(token.c_str(), GLOB_TILDE, NULL, &glob_result);
        if (glob_return != 0)
        {
            perror("Error expanding wildcards");
            continue;
        }
        for (int j = 0; j < glob_result.gl_pathc; j++)
            matching_files.push_back(string(glob_result.gl_pathv[j]));

        globfree(&glob_result);
        tokens.erase(tokens.begin() + i);
        for (int j = matching_files.size() - 1; j >= 0; j--)
            tokens.insert(tokens.begin() + i, matching_files[j]);
    }
    return join_cmd(tokens);
}

vector<int> get_pids_with_file(const std::string &file_path)// function to get the pids of the processes that have the file open 
{
    vector<int> pids;
    DIR *proc_dir = opendir("/proc"); 
    if (!proc_dir)
    {
        std::cerr << "Failed to open /proc directory" << std::endl;
        return pids;
    }

    struct dirent *entry;
    while ((entry = readdir(proc_dir)) != NULL)// iterate over all the processes
    {
        int pid = atoi(entry->d_name);
        if (pid <= 0)
        {
            continue;
        }
        std::string fd_path = "/proc/" + std::to_string(pid) + "/fd"; // get the file descriptors of the process 
        DIR *fd_dir = opendir(fd_path.c_str()); // open the file descriptors of the process
        if (!fd_dir)
        {
            continue;
        }

        struct dirent *fd_entry;
        while ((fd_entry = readdir(fd_dir)) != NULL)// iterate over all the file descriptors of the process 
        {
            std::string link_path = fd_path + "/" + fd_entry->d_name;
            char buffer[1024];
            ssize_t len = readlink(link_path.c_str(), buffer, sizeof(buffer) - 1);
            if (len == -1)
            {
                continue;
            }

            buffer[len] = '\0';
            if (file_path == buffer)
            {
                pids.push_back(pid);
                break;
            }
        }

        closedir(fd_dir);
    }

    closedir(proc_dir);
    return pids;
}
void delep(string file_path)// function to delete the file
{
    vector<int> pids_with_file = get_pids_with_file(file_path);
    cout << "pids with file :" << endl;
    for (int i = 0; i < pids_with_file.size(); i++)
    {
        cout << pids_with_file[i] << endl;
    }
    cout << "Do you want to kill process "
         << " ? (y/n)" << endl;
    char c;
    cin >> c;

    if (c == 'y')
    {
        for (int i = 0; i < pids_with_file.size(); i++)
        {
            kill(pids_with_file[i], SIGKILL);
        }
        remove(file_path.c_str());
    }
    else
    {
        cout << "File not deleted" << endl;
    }
}

const int HISTORY_SIZE = 1000;
int current_command = -1;
vector<string> history;
string current_input;
int my_cool_readline_func(int count, int key) // function to handle the up and down arrow keys
{
    switch (key)
    {
    case 65: // up arrow
        if (current_command > 0)
        {
            current_command--;
            rl_replace_line(history[current_command].c_str(), 0);
            rl_end_of_line(0, 0);
            rl_redisplay();
        }
        break;
    case 66: // down arrow
        if (current_command < HISTORY_SIZE - 1 && current_command + 1 < history.size())
        {
            current_command++;
            rl_replace_line(history[current_command].c_str(), 0);
            rl_end_of_line(0, 0);
            rl_redisplay();
        }
        break;
    case 1: // ctrl + a
        rl_beg_of_line(0, 0);
        break;
    case 5: // ctrl + e
        rl_end_of_line(0, 0);
        break;
    default:
        break;
    }
    return 0;
}

map<int, int> childCount;

int getParentPID(int pid)
{
    stringstream filePath;
    filePath << "/proc/" << pid << "/stat";
    ifstream statFile(filePath.str());
    if (statFile.is_open())
    {
        string line;
        getline(statFile, line);
        statFile.close();

        // Parsing the line to get the parent process ID
        istringstream linestream(line);
        vector<string> values;
        string value;
        while (getline(linestream, value, ' '))
        {
            values.push_back(value);
        }
        try
        {
            int parentPID = stoi(values[3]);
            return parentPID;
        }
        catch (const std::invalid_argument &e)
        {
            return -1;
        }
    }
    return -1;
}
void sb(int pid)
{
    cout << "Process Tree:" << endl;
    while (pid != 0)
    {
        cout << pid << " -> ";
        childCount[pid]++;
        pid = getParentPID(pid);
    }
    cout << "Root" << endl;
    for (int i = 1; i < 100000; i++)
    {
        int parentPID = getParentPID(i);
        if (childCount.find(parentPID) != childCount.end())
            childCount[parentPID] += 1;
    }
    int max = 0;
    int max_pid = 0;
    for (auto it = childCount.begin(); it != childCount.end(); it++)
    {
        if (it->second > max)
        {
            max = it->second;
            max_pid = it->first;
        }
    }
    cout << "Process with PID " << max_pid << " has spawned the most number of children: " << max << endl;
}

/// @brief 
/// @return 
int main()
{
    string cmd;
    int status = 0;
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    rl_bind_keyseq("\033[A", my_cool_readline_func); // up arrow
    rl_bind_keyseq("\033[B", my_cool_readline_func); // down arrow
    rl_bind_key('\001', my_cool_readline_func);      // ctrl + a
    rl_bind_key('\005', my_cool_readline_func);      // ctrl + e

    ifstream history_file("history.txt");
    if (history_file.is_open())
    {
        string line;
        while (getline(history_file, line))
        {
            history.push_back(line);
        }
        history_file.close();
    }

    current_command = history.size();
    while (true)
    {
        char *str_pt;
        char curr_dir[MAX_LEN] = "";
        str_pt = getcwd(curr_dir, sizeof(curr_dir));
        if (str_pt == NULL)
        {
            perror("Couldn't fetch current working directory!");
            continue;
        }
        bool bg = false;
        history.push_back(" ");
        string temp = "MYSHELL: " + string(curr_dir) + "$ ";
        char *input = readline(temp.c_str());
        if (!input)
        {
            break;
        }
        current_input = strdup(input);
        cmd = strdup(input);
        free(input);
        history[history.size() - 1] = cmd;
        current_command = history.size();
        ofstream history_file("history.txt", ios::app);
        if (history_file.is_open())
        {
            history_file << cmd << endl;
            history_file.close();
        }

        //remove the readline and add getline(cin,cmd) here incase to test the & part properly
        cmd = strip(cmd);
        check_exit(cmd);
        fg_pid = getpid();
        int x = check_chdir(cmd);
        if (x == -1)
        {
            if (cmd.substr(0, 6) == "delep ")
            {
                string file_path = cmd.substr(6);
                delep(file_path);
                continue;
            }
            if (cmd.back() == '&')
            {
                bg = true, cmd.back() = ' ';
            }
            if (cmd.substr(0, 3) == "sb ")
            {
                string pid = cmd.substr(3);
                pid = strip(pid);
                sb(stoi(pid));
                continue;
            }
            cmd = expand_wildcards(cmd);
            exec_pipe(cmd, status, bg);
        }
    }
}


