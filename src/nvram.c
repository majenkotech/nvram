#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <alloca.h>

#define NVRAM_START 0x1d0fc000

#define min(A, B) (((A) < (B)) ? (A) : (B))

int main(int argc, char **argv) {

    int all = 0;
    int rd = 0;
    int usage = 0;

    char *key = NULL;

    int opt;

    while ((opt = getopt(argc, argv, "har:")) != -1) {
        switch (opt) {
            case 'a':
                all++;
                break;
            case 'r':
                rd++;
                key = optarg;
                break;
            case 'h':
                usage++;
                break;
            default:
                usage++;
                break;
        }
    }

    if (!all && !rd) usage++;

    if (usage) {
        printf("nvram <options>\n");
        printf("  Options:\n");
        printf("    -r <key>    : Read the value from one key\n");
        printf("    -a          : Read the values of all keys\n");
        printf("    -h          : This help text\n");
        return 0;
    }


    // Obtain handle to physical memory
    int fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("Unable to open /dev/mem: %s\n", strerror(errno));
        return -1;
    }

    // Map a page of memory to the physical address
    int32_t nvram_base = (int32_t) mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, NVRAM_START);
    if (nvram_base < 0) {
        printf("Mmap failed: %s\n", strerror(errno));
        return -1;
    }


    if (all) {
        char *data = (char *)(nvram_base + 4);
        while (data[0] != 0) {
            printf("%s\n", data);
            data += (strlen(data) + 1);
        }
    } else if (rd) {
        char *data = (char *)(nvram_base + 4);
        char *keyeq = alloca(strlen(key) + 2);
        strcpy(keyeq, key);
        strcat(keyeq, "=");
        while (data[0] != 0) {
            if (strncmp(data, keyeq, min(strlen(keyeq), strlen(data))) == 0) {
                printf("%s\n", data + strlen(keyeq));
            }
            data += (strlen(data) + 1);
        }
    }

    return 0;
}
