#include "grep.h"

#include <getopt.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pattern_add(arguments *arg, char *pattern) {
  int n = strlen(pattern);

  if (arg->len_pattern != 0) {
    strcat(arg->pattern + arg->len_pattern, "|");
    arg->len_pattern++;
  }
  arg->len_pattern += sprintf(arg->pattern + arg->len_pattern, "(%s)", pattern);
}

void add_reg_from_file(arguments *arg, char *filepath) {
  FILE *f = fopen(filepath, "r");

  if (f == NULL) {
    if (!arg->s) {
      perror(filepath);
      exit(1);
    }
  }
  char *line = NULL;
  size_t memlen = 0;
  int read = getline(&line, &memlen, f);
  while (read != -1) {
    if (line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }
    pattern_add(arg, line);
    read = getline(&line, &memlen, f);
  }
  free(line);
  fclose(f);
}

arguments argument_parser(int argc, char **argv) {
  arguments arg = {0};
  int opt;

  while ((opt = getopt(argc, argv, "e:ivclnhsf:o")) != -1) {
    switch (opt) {
    case 'e':
      arg.e = 1;
      pattern_add(&arg, optarg);
      break;
    case 'i':
      arg.i = REG_ICASE;
      break;
    case 'v':
      arg.v = 1;
      break;
    case 'c':
      arg.c = 1;
      break;
    case 'l':
      arg.l = 1;
      break;
    case 'n':
      arg.n = 1;
      break;
    case 'h':
      arg.h = 1;
      break;
    case 'f':
      arg.f = 1;
      add_reg_from_file(&arg, optarg);
      break;
    case 'o':
      arg.o = 1;
      break;
    default:;
      break;
    }
  }
  if (arg.len_pattern == 0) {
    pattern_add(&arg, argv[optind]);
    optind++;
  }
  if (argc - optind == 1) {
    arg.h = 1;
  }
  return arg;
}

void output_line(char *line, int n) {
  for (int i = 0; i < n; i++) {
    putchar(line[i]);
  }
  if (line[n - 1] != '\n') {
    putchar('\n');
  }
}

void print_math(regex_t *re, char *line) {
  regmatch_t math;
  int offset = 0;
  while (1) {
    int result = regexec(re, line + offset, 1, &math, 0);
    if (result != 0) {
      break;
    }
    for (int i = math.rm_so; i < math.rm_eo; i++) {
      putchar(line[i]);
    }
    putchar('\n');
    offset += math.rm_eo;
  }
}

void processLine(arguments arg, char *path, regex_t *reg) {
  FILE *f = fopen(path, "r");
  if (f == NULL) {
    if (!arg.s) {
      perror(path);
      exit(1);
    }
  }

  char *line = NULL;
  size_t memlen = 0;
  int read = 0;
  int line_count = 1;
  int c = 0;
  read = getline(&line, &memlen, f);
  while (read != -1) {
    int result = regexec(reg, line, 0, NULL, 0);

    if ((result == 0 && !arg.v) || (arg.v && result != 0)) {
      if (!arg.c && !arg.l) {
        if (!arg.h) {
          printf("%s:", path);
        }
        if (arg.n) {
          printf("%d:", line_count);
        }
        if (arg.o) {
          print_math(reg, line);
        } else {
          output_line(line, read);
        }
      }
      c++;
    }
    read = getline(&line, &memlen, f);
    line_count++;
  }
  free(line);
  if (arg.c && !arg.l) {
    if (!arg.h) {
      printf("%s:", path);
    }
    printf("%d\n", c);
  }
  if (arg.l && c > 0) {
    printf("%s\n", path);
  }
  fclose(f);
}

void output(arguments arg, int argc, char **argv) {
  regex_t re;
  int error = regcomp(&re, arg.pattern, REG_EXTENDED | arg.i);
  if (error) {
    perror("Error");
    exit(1);
  }

  for (int i = optind; i < argc; i++) {
    processLine(arg, argv[i], &re);
  }
  regfree(&re);
}

int main(int argc, char **argv) {
  arguments arg = argument_parser(argc, argv);
  output(arg, argc, argv);
  return 0;
}