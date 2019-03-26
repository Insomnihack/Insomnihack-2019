#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>
#include <err.h>
#include <malloc.h>
#include <sys/types.h>
#include <time.h>

#define show(msg) printf("%s\n", msg)
#define clear_screen() printf("\033[H\033[J");

void alarm_handler(int sig)
{
    pid_t pid;
    pid = getpid();
    printf("Bye bye ...\n");
    printf("Exiting from pid %d\n", pid);
    kill(pid, 15);
    exit(0);
}

void trigger_alarm(int timeout)
{
    signal(SIGALRM, alarm_handler);
    alarm(timeout);
}

size_t x_strlcpy(char *__restrict dest, const char *__restrict src, size_t dest_len)
{
    if (dest_len == 0) {
        return strlen(src);
    } else {
        char *const dest_str_end = dest + dest_len - 1;
        char *__restrict d = dest;
        const char *__restrict s = src;

        while (d != dest_str_end) {
            if ((*d++ = *s++) == '\0') {
                return (d - 1 - dest);
            }
        }
        *d = '\0';
        return (dest_len - 1) + strlen(s);
    }
}
