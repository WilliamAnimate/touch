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

#define VERSION_MAJOR 0
#define VERSION_MINOR 1
#define VERSION_PATCH 0

struct Touch {
    bool dry;
    bool no_create;
    bool only_access;
    bool only_modify;
};

void print_version(char *name) {
    printf("%s v%i.%i.%i\n",
           name, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}

void print_help(char *name) {
    printf("Usage: %s [OPTION] -- FILE\n\
Create an empty file or update the timestamp of an existing file.\n\n\
  -a                           only affect the access time\n\
  -d, --dry-run                do not affect or create any files\n\
  -c, --no-create              do not create any files\n\
  -r, --reference=FILE         use the specified file's times\n\
  -m                           only affect the modificaton time\n\
      --time=WORD              specify what to change:\n\
                                 access time (-a): `access`, `atime`, `use`;\n\
                                 modification time (-m): `access`, `atime`, `mtime`\n\
  -h, --help                   displays this screen\n\
  -v, --version                display version and exit\n\
",
           name);
}

int modify_timestamps(const char *filename, const char *ref, struct Touch touch) {
    struct utimbuf new_times;
    struct stat source_stat;
    struct stat current_stat;

    if (ref != NULL && stat(ref, &source_stat) != 0) {
        perror("Cannot get reference file timestamps");
        return -1;
    }

    // if we aren't only touching one value (setting it to what it currently is)
    // then don't expend unneeded effort finding it.
    if ((touch.only_modify || touch.only_access) && stat(filename, &current_stat) != 0) {
        perror("Cannot get current file timestamps");
        return -1;
    }

    if (touch.dry) {
        printf("Would modify timestamp to latest for file: '%s'\n", filename);
        return 0;
    }

    // if the *ref var is NULL. if it is, then we're in reference mode. otherwise, we're in update mode
    // then we check if we're only updating the modify time or access time, or both
    if (ref != NULL) {
        new_times.actime = touch.only_access ? current_stat.st_atime: source_stat.st_atime;
        new_times.modtime = touch.only_modify ? current_stat.st_mtime : source_stat.st_mtime ;
    } else {
        new_times.actime = touch.only_access ? current_stat.st_atime : time(NULL);
        new_times.modtime = touch.only_modify ? current_stat.st_mtime : time(NULL);
    }

    if (utime(filename, &new_times) != 0) {
        perror("Error updating timestamps");
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

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {"dry-run", no_argument, 0, 'd'},
        {"no-create", no_argument, 0, 'c'},
        {"reference", no_argument, 0, 'r'},
        {"version", no_argument, 0, 'v'},
    };

    while ((c = getopt_long(argc, argv, "hdr:camv", long_options, NULL)) != -1) {
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
            case 'a':
                touch.only_access = true;
                break;
            case 'm':
                touch.only_modify = true;
                break;
            case 'v':
                print_version(argv[0]);
                break;
            default:
                fprintf(stderr, "Usage: %s [hdc]\n", argv[0]);
                return EXIT_FAILURE;
        }
    }

    for (int i = args_index + 1; i < argc; i++) {
        if (!rt_already_parsed_double_dash && strcmp(argv[i], "--") == 0) {
            rt_already_parsed_double_dash = true;
            continue;
        }

        struct stat path_stat;
        if (stat(argv[i], &path_stat) == 0) {
            // FIXME: reference mode breaks if we dont do this
            return (reference_file_to_modify == NULL) ?
                modify_timestamps(argv[i], NULL, touch) :
                modify_timestamps(reference_file_to_modify, argv[i], touch);
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
            char concat[strlen(error_msg) + strlen(argv[i]) + strlen(error_msg_end) + 1];
            strcpy(concat, error_msg);
            strcat(concat, argv[i]);
            strcat(concat, error_msg_end);
            perror(concat);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

