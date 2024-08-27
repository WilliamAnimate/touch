/*
 * oh no
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <sys/stat.h>
#include <time.h>

struct Touch {
    bool dry;
    bool no_create;
    bool grass; // this is very important
};

void print_help(char *name) {
    printf("Usage: %s [OPTION] -- FILE\n\
Create an empty file or update the timestamp of an existing file.\n\n\
  -d, --dry, --dry-run         do not affect or create any files\n\
  -c, --no-create              do not create any files\n\
  -r, --reference=FILE         use the specified file's times\n\
  -m                           change only the modification time\n\
      --time=WORD              specify what to change:\n\
                                 access time (-a): `access`, `atime`, `use`;\n\
                                 modification time (-m): `access`, `atime`, `mtine`\n\
  -h, --help                   displays this screen\n\
  -v, --version                display version and exit\n\
",
           name);
}

int modify_timestamps(const char *filename, struct Touch touch) {
    struct utimbuf new_times;

    new_times.actime = time(NULL);
    new_times.modtime = time(NULL);
    if (utime(filename, &new_times) < 0) {
        perror("Error updating timestamps");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int c;
    int args_index = 0;

    bool rt_already_parsed_double_dash = false;
    struct Touch touch;
    touch.grass = true; // this is very important

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"dry", no_argument, 0, 'd'},
        {"dry-run", no_argument, 0, 'd'},
        {"no-create", no_argument, 0, 'c'},
    };

    while ((c = getopt_long(argc, argv, "hddc", long_options, NULL)) != -1) {
        args_index++;
        switch (c) {
            case 'h':
                print_help(argv[0]);
                exit(EXIT_SUCCESS);
            case 'd':
                touch.dry = true;
                break;
            case 'c':
                touch.no_create = true;
                break;
            default:
                fprintf(stderr, "Usage: %s [hdc]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    if (!touch.grass) puts("touch some grass."); // this is very important

    for (int i = args_index + 1; i < argc; i++) {
        if (!rt_already_parsed_double_dash && strcmp(argv[i], "--") == 0) {
            rt_already_parsed_double_dash = true;
            continue;
        }

        struct stat path_stat;
        if (stat(argv[i], &path_stat) == 0) {
            modify_timestamps(argv[i], touch);
            return EXIT_FAILURE;
        }

        FILE *fptr;
        fptr = fopen(argv[i], "w");
        if (!fptr) {
            // string concatnation better on stack (stack overflow impending)
            // i can prolly just not append newline to printf and then call perror with null
            // scary memory management garbage incoming; seasoned c programmers might flame me for this
            char error_msg[] = "Cannot touch '";
            char error_msg_end[] = "'";
            strcat(error_msg, argv[i]);
            strcat(error_msg, error_msg_end);
            perror(error_msg);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
