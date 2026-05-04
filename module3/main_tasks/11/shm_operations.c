#include "shm_operations.h"


int create_or_open_shm(const char *name, size_t size) {
    int fd = shm_open(name, O_CREAT | O_RDWR, 0644);
    if (fd == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Устанавливаем размер объекта разделяемой памяти
    if (ftruncate(fd, size) == -1) {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }

    return fd;
}

shared_data_t *map_shm(int fd, size_t size) {
    shared_data_t *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    return addr;
}

void close_and_unlink_shm(int fd, shared_data_t *addr, size_t size, const char *name) {
    if (munmap(addr, size) == -1) {
        perror("munmap");
    }
    if (close(fd) == -1) {
        perror("close");
    }
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
    }
}