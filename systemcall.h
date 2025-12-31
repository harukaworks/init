#ifndef SYSTEMCALL_H
#define SYSTEMCALL_H

extern char **environ;

int write(int fd, const void *buf, unsigned long count);
long read(int fd, void *buf, unsigned long count);
long fork(void);
long execve(const char *filename, char *const argv[], char *const envp[]);
void _exit(long status);
long wait(long *status);
long chdir(const char *path);
long pipe(int *fd);
long dup2(int oldfd, int newfd);
long close(int fd);

#endif
