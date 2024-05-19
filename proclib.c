#include <proclib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int get1LineFromPipe(const char *argv[], char* buf, size_t bufsize) {
	int p[2];
	if (pipe(p) < 0) {
		fprintf(stderr, "pipe error\n");
		return -1;
	}

	pid_t pid = fork();
	if (pid < 0) {
		fprintf(stderr, "fork error\n");
		return -1;
	}

	if (0 == pid) {
		close(p[0]);
		dup2(p[1], STDOUT_FILENO);
		execvp(argv[0], (char * const *)argv);
		fprintf(stderr, "Error executing %s: %s\n", argv[0], strerror(errno));
	}

	close(p[1]);

	int st;
	waitpid(pid, &st, 0);

	memset(buf, 0, bufsize);
	if (WIFEXITED(st) && WEXITSTATUS(st) == 0) {
		if (read(p[0], buf, bufsize) > 0) {
			char* pn = strchr(buf, '\n');
			if (pn)
				*pn = 0;
		}
	}

	close(p[0]);
	return 0;
}
