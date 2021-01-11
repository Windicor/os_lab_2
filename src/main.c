#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

void child_work(int from, int to) {
  char buf[1];
  while (read(from, buf, 1) > 0) {
    char c = buf[0];
    if (c != 'a' && c != 'e' && c != 'i' && c != 'o' && c != 'u' && c != 'y' &&
        c != 'A' && c != 'E' && c != 'I' && c != 'O' && c != 'U' && c != 'Y') {
      write(to, buf, 1);
    }
  }
  close(to);
  close(from);
}

void parrent_work(int child1, int child2) {
  char buf[1];
  int is_even = 0;
  while (read(STDIN_FILENO, buf, 1) > 0) {
    if (!is_even) {
      write(child1, buf, 1);
    } else {
      write(child2, buf, 1);
    }
    if (buf[0] == '\n') {
      is_even = !is_even;
    }
  }

  close(child1);
  close(child2);
}

int read_name_and_open_file() {
  const size_t FILE_NAME_SIZE = 64;
  char f_name[FILE_NAME_SIZE];
  char buf[1];
  int idx = 0;
  while (idx < FILE_NAME_SIZE && read(STDIN_FILENO, buf, 1) > 0) {
    if (buf[0] == '\n') {
      break;
    }
    f_name[idx++] = buf[0];
  }
  f_name[idx++] = '\0';
  return open(f_name, O_WRONLY | O_TRUNC);
}

void error(char* buf, size_t size) { write(STDERR_FILENO, buf, size); }
void check_file_id(int id) {
  if (id == -1) {
    error("File not found\n", 15);
    exit(-1);
  }
}
void check_pipe_creation(int* pipefd) {
  if (pipe(pipefd) == -1) {
    error("Cannot create pipe\n", 19);
    exit(-2);
  }
}
int check_fork() {
  int fd = fork();
  if (fd == -1) {
    error("Cannot create process\n", 22);
    exit(-3);
  }
  return fd;
}

int main(int argc, char* argv[]) {
  int f1 = read_name_and_open_file();
  check_file_id(f1);
  int f2 = read_name_and_open_file();
  check_file_id(f2);

  int pipefd1[2];
  check_pipe_creation(pipefd1);

  int child1 = check_fork();
  if (child1 == 0) {
    close(pipefd1[1]);
    child_work(pipefd1[0], f1);
    return 0;
  }
  close(pipefd1[0]);

  int pipefd2[2];
  check_pipe_creation(pipefd2);

  int child2 = check_fork();
  if (child2 == 0) {
    close(pipefd1[1]);
    close(pipefd2[1]);
    child_work(pipefd2[0], f2);
    return 0;
  }
  close(pipefd2[0]);

  parrent_work(pipefd1[1], pipefd2[1]);

  return 0;
}