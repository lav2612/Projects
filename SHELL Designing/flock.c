#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



int main() {
    int fd;
    pid_t pid;

    fd = open("file.txt", O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // child process
        struct flock lock;

        lock.l_type = F_WRLCK;// F_WRLCK for write lock
        lock.l_whence = SEEK_SET;// SEEK_SET for start of file
        lock.l_start = 0;
        lock.l_len = 0;
        lock.l_pid = getpid();

        if (fcntl(fd, F_SETLKW, &lock) == -1) {// this function will block until the lock is acquired
            perror("fcntl");
            return 1;
        }

        while (1) {
            write(fd, "child process\n", strlen("child process\n"));
            sleep(1);
        }
    } else {
        // parent process
        while (1) {
            write(fd, "parent process\n", strlen("parent process\n"));
            sleep(1);
        }
    }

    return 0;
}
