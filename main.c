/*
 * oh no
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <string.h>
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

    if (touch.dry) {
        printf("Would modify timestamp to latest for file: '%s'\n", filename);
        return 0;
    }
    new_times.actime = time(NULL);
    new_times.modtime = time(NULL);
    if (utime(filename, &new_times) < 0) {
        perror("Error updating timestamps");
        return -1;
    }

    return 0;
}

int modify_timestamp_from_reference(const char *newfile, const char *ref, struct Touch touch) {
    struct stat source_stat;
    if (stat(ref, &source_stat) != 0) {
        perror("Cannot get reference file timestamps");
        return -1;
    }

    struct utimbuf new_times = {source_stat.st_atime, source_stat.st_mtime};
    if (utime(newfile, &new_times) != 0) {
        perror("Error updating target file timestamps");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int c;
    int args_index = 0;

    bool rt_already_parsed_double_dash = false;
    char *reference_file_to_modify = NULL;
    struct Touch touch;
    touch.grass = true; // this is very important

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"dry", no_argument, 0, 'd'},
        {"dry-run", no_argument, 0, 'd'},
        {"no-create", no_argument, 0, 'c'},
        {"reference", no_argument, 0, 'r'},
    };

    while ((c = getopt_long(argc, argv, "hddr:c", long_options, NULL)) != -1) {
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
            case 'r':
                // SAFETY: getopt won't call this code if argument isn't suppied; this is marked as a must-have-parameter, therefore, args_index + 2 will always exist.
                reference_file_to_modify = argv[args_index + 2];
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
            int ret;
            if (reference_file_to_modify) {
                ret = modify_timestamp_from_reference(reference_file_to_modify, argv[i], touch);
            } else {
                ret = modify_timestamps(argv[i], touch);
            }
            printf("%i\n", ret);
            return ret;
        }

        if (touch.dry) {
            printf("Would create file '%s'\n", argv[i]);
            return 0;
        } else if (touch.no_create) {
            printf("Not creating file '%s'\n", argv[i]);
            return 0;
        }
        FILE *fptr;
        fptr = fopen(argv[i], "w");
        if (!fptr) {
            // string concatnation better on stack (stack overflow impending)
            // scary memory management mess; hallilo please dont murder me im trying my best >~<
            char error_msg[] = "Cannot touch '";
            char error_msg_end[] = "'";
            char concat[sizeof(error_msg) + sizeof(argv[i]) + sizeof(error_msg_end)];
            strcpy(concat, error_msg);
            strcat(concat, argv[i]);
            strcat(concat, error_msg_end);
            perror(concat);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

