#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <dirent.h>

typedef struct {
    char *name;
    int (*func)(double, double, double*);
    void *handle;
} Operation;

Operation *ops = NULL;
int num_ops = 0;

void add_operation(const char *name, int (*func)(double, double, double*), void *handle) {
    ops = realloc(ops, (num_ops + 1) * sizeof(Operation));
    ops[num_ops].name = strdup(name);
    ops[num_ops].func = func;
    ops[num_ops].handle = handle;
    num_ops++;
}

void load_library(const char *path) {
    void *handle = dlopen(path, RTLD_LAZY);
    if (!handle) {
        return;
    }
    dlerror();

    const char **name_ptr = dlsym(handle, "op_name");
    if (dlerror() != NULL) {
        dlclose(handle);
        return;
    }
    const char *name = *name_ptr;

    int (*func)(double, double, double*) = dlsym(handle, "op_func");
    if (dlerror() != NULL) {
        dlclose(handle);
        return;
    }

    add_operation(name, func, handle);
    printf("Loaded operation: %s from %s\n", name, path);
}

void load_operations_from_dir(const char *dir) {
    DIR *d = opendir(dir);
    if (!d) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL) {
        const char *name = entry->d_name;
        size_t len = strlen(name);
        if (len > 3 && strcmp(name + len - 3, ".so") == 0) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, name);
            load_library(path);
        }
    }
    closedir(d);
}

int main(int argc, char **argv) {
    const char *lib_dir = (argc > 1) ? argv[1] : ".";
    load_operations_from_dir(lib_dir);

    if (num_ops == 0) {
        printf("No operations loaded. Exiting.\n");
        return 1;
    }

    double result, a, b;
    int choice;

    while (1) {
        printf("\nChoose your calculator operation.\n");
        for (int i = 0; i < num_ops; i++) {
            printf("%d. %s\n", i + 1, ops[i].name);
        }
        printf("%d. Exit\n", num_ops + 1);
        printf("> ");

        scanf("%d", &choice);

        if (choice == num_ops + 1) {
            break;
        }

        if (choice < 1 || choice > num_ops) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }

        printf("Type arguments a and b with space between them: ");
        scanf("%lf %lf", &a, &b);

        int status = ops[choice - 1].func(a, b, &result);
        if (status != 0) {
            printf("Error: operation failed (division by zero or invalid arguments).\n");
        } else {
            printf("Result: %lf\n", result);
        }
    }

    for (int i = 0; i < num_ops; i++) {
        free(ops[i].name);
        dlclose(ops[i].handle);
    }
    free(ops);

    return 0;
}