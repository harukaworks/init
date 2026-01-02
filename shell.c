#include "str.h"
#include "systemcall.h"

static const char *path = 0;
static const char *home = 0;

void build_full_path(char *buf, const char *dir, const char *cmd) {
    while (*dir) *buf++ = *dir++;
    if (dir > buf && dir[-1] != '/') *buf++ = '/';
    while (*cmd) *buf++ = *cmd++;
    *buf = '\0';
}

void tokenize(char *line, char **argv, int max_args) {
    int i = 0;
    char *p = line;
    while (*p && i < max_args - 1) {
        p += spnstr(p, " \t\n", 0);
        if (!*p) break;
        argv[i++] = p;
        p = p + spnstr(p, " \t\n", 1);
        *p++ = '\0';
    }
    argv[i] = (char*)0;
}

const char* get_env_value(const char *prefix) {
    for (int i = 0; environ[i] != 0; i++) if (powstr(environ[i], prefix) == environ[i]) return environ[i] + lenstr(prefix);
    return 0;
}

int exec_with_path(const char *cmd, char *const argv[], char *const envp[]) {
    if (chrstr(cmd, '/')) {
        execve(cmd, argv, envp);
        return -1;
    }
    char path_buf[256];
    int i;
    for (i = 0; path[i] && i < 255; i++) path_buf[i] = path[i];
    path_buf[i] = '\0';
    char *dir = path_buf;
    char full[256];
    while (dir) {
        char *next = dir;
        while (*next && *next != ':') {
            next++;
        }
        if (*next == ':') {
            *next = '\0';
            next++;
        } else {
            next = 0;
        }
        if (*dir == '\0') {
            dir = next;
            continue;
        }
        build_full_path(full, dir, cmd);
        execve(full, argv, envp);
        dir = next;
    }
    return -1;
}

void exec_pipeline(char *commands[], int ncmds) {
    if (ncmds == 1) {
        char *argv[64];
        tokenize(commands[0], argv, 64);
        if (!argv[0]) return;
        if (cmpstr(argv[0], "cd") == 0) {
            const char *path = argv[1] ? argv[1] : home;
            if (chdir(path) != 0) {
                write(2, "gsh: cd: No such file or directory", 34);
                write(2, path, lenstr(path));
                write(2, "\n", 1);
            }
            return;
        }
        long pid = fork();
        if (pid == 0) {
            exec_with_path(argv[0], argv, environ);
            write(2, "gsh: Command not found\n", 23);
            _exit(127);
        } else if (pid > 0) {
            wait(0);
        } else {
            write(2, "gsh: Fork failed\n", 17);
        }
        return;
    }
    int pipes[16][2];
    for (int i = 0; i < ncmds - 1; i++) {
        if (pipe(pipes[i]) != 0) {
            write(2, "gsh: Pipe failed\n", 17);
            return;
        }
    }
    for (int i = 0; i < ncmds; i++) {
        long pid = fork();
        if (pid == 0) {
            if (i > 0) {
                dup2(pipes[i-1][0], 0);
                close(pipes[i-1][0]);
                close(pipes[i-1][1]);
            }
            if (i < ncmds - 1) {
                dup2(pipes[i][1], 1);
                close(pipes[i][0]);
                close(pipes[i][1]);
            }

            for (int j = 0; j < ncmds - 1; j++) {
                if (j != i && j != i-1) {
                    close(pipes[j][0]);
                    close(pipes[j][1]);
                }
            }

            char *argv[64];
            tokenize(commands[i], argv, 64);
            if (!argv[0]) _exit(0);

            exec_with_path(argv[0], argv, environ);
            write(2, "gsh: Command not found\n", 23);
            _exit(127);
        }
    }
    for (int i = 0; i < ncmds - 1; i++) {
        close(pipes[i][0]);
        close(pipes[i][1]);
    }

    for (int i = 0; i < ncmds; i++) {
        wait(0);
    }
}

int split_by_pipe(char *line, char *commands[], int max) {
    int n = 0;
    char *p = line;
    while (*p && n < max) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;
        commands[n++] = p;
        while (*p && *p != '|') p++;
        if (*p) *p++ = '\0';
    }
    return n;
}

int main() {
    char line[512];
    char *commands[16];
    path = get_env_value("PATH=");
    path = path != 0 ? path : "/bin:/usr/bin:/sbin";
    home = get_env_value("HOME=");
    home = home !=0 ? home : "/";
    while (1) {
        write(1, "# ", 2);
        long n = read(0, line, sizeof(line) - 1);
        if (n <= 0) break;
        line[n] = '\0';
        if (n > 0 && line[n-1] == '\n') line[n-1] = '\0';
        while (n >= 2 && line[n-2] == '\\') {
            write(1, "> ", 2);
            long more = read(0, line + n - 2, sizeof(line) - (n - 1));
            if (more <= 0) break;
            n += more - 1;
            line[n] = '\0';
            if (line[n-1] == '\n') line[--n] = '\0';
        }

        if (cmpstr(line, "exit") == 0) {
            _exit(0);
        }

        int ncmds = split_by_pipe(line, commands, 16);
        if (ncmds > 0) {
            exec_pipeline(commands, ncmds);
        }
    }
    _exit(0);
}
