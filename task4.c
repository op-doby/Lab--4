#include <stdio.h>
int count_digits(const char *str) {
    int count = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            count++;
        }
        str++;
    }
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <string>\n", argv[0]);
        return 1;
    }
    int digit_count = count_digits(argv[1]);
    printf("Digit count: %d\n", digit_count);
    return 0;
}
