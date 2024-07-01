#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_BUF_SIZE 10000

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size; // 1, 2, or 4 bytes
    unsigned char mem_buf[MEM_BUF_SIZE]; // to store data read into memory
    size_t mem_count;
    int display_flag; // 0 for decimal, 1 for hexadecimal
    // Additional fields if needed
} state;

void toggle_debug_mode(state* s) { // Switch the debug flag on and off
    if (s->debug_mode) {
        printf("Debug flag now off\n");
        s->debug_mode = 0;
    } else {
        printf("Debug flag now on\n");
        s->debug_mode = 1;
    }
}

void set_file_name(state* s) {
    printf("Enter file name: ");
    fgets(s->file_name, 128, stdin);
    s->file_name[strcspn(s->file_name, "\n")] = 0;  // Remove newline character
    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state* s) {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    getchar();  // Remove newline character left by scanf
    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("ERROR: Invalid size\n");
    }
}

void load_into_memory(state *s) {
    if (strcmp(s->file_name, "") == 0) {
        printf("ERROR: No file name provided\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    if (!file) {
        printf("ERROR: Cannot open file %s\n", s->file_name);
        return;
    }

    char input[100];
    unsigned int location;
    int length;

    printf("Please enter <location> <length>\n");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode) {
        printf("Debug: file_name=%s, location=0x%x, length=%d\n", s->file_name, location, length);
    }

    fseek(file, location, SEEK_SET); // Move file pointer to location
    size_t units_read = fread(s->mem_buf, s->unit_size, length, file); // Read length units of size unit_size into memory buffer
    s->mem_count = units_read * s->unit_size;
    fclose(file);

    printf("Loaded %d units into memory\n", length);
}

void toggle_display_mode(state *s) {
    if (s->display_flag) {
        s->display_flag = 0;
        printf("Display flag now off, decimal representation\n");
    } else {
        s->display_flag = 1;
        printf("Display flag now on, hexadecimal representation\n");
    }
}

void memory_display(state *s) {
    unsigned int addr; // address to start displaying from (if addr !=0)
    int u; // number of units to display
    char input[100];

    printf("Enter address and length\n> ");
    fgets(input, sizeof(input), stdin);
    sscanf(input, "%x %d", &addr, &u);

    unsigned char *start_addr = (addr == 0) ? s->mem_buf : (unsigned char *)addr;
    
    if (s->display_flag) { // Hexadecimal Representation
        printf("Hexadecimal\n===========\n");
        for (int i = 0; i < u; ++i) {
            if (s->unit_size == 1) {
                printf("0x%02X\n", *((unsigned char *)(start_addr + i * s->unit_size)));
            } else if (s->unit_size == 2) {
                printf("0x%04X\n", *((unsigned short *)(start_addr + i * s->unit_size)));
            } else if (s->unit_size == 4) {
                printf("0x%08X\n", *((unsigned int *)(start_addr + i * s->unit_size)));
            }
        }
    } else {
        printf("Decimal\n=======\n"); // Decimal Representation
        for (int i = 0; i < u; ++i) {
            if (s->unit_size == 1) {
                printf("%d\n", *((unsigned char *)(start_addr + i * s->unit_size)));
            } else if (s->unit_size == 2) {
                printf("%d\n", *((unsigned short *)(start_addr + i * s->unit_size)));
            } else if (s->unit_size == 4) {
                printf("%d\n", *((unsigned int *)(start_addr + i * s->unit_size)));
            }
        }
    }
}

void save_into_file(state* s) {
    unsigned int source_address, target_location;
    int length;
    printf("Please enter <source-address> <target-location> <length>\n");
    scanf("%x %x %d", &source_address, &target_location, &length);
    // Handle special case for source address
    unsigned char* source = (source_address == 0) ? s->mem_buf : (unsigned char*) source_address;
    if (source < s->mem_buf || source >= s->mem_buf + MEM_BUF_SIZE) {
        printf("ERROR: Source address is out of bounds\n");
        return;
    }
    // Open the file for writing (not truncating)
    FILE* file = fopen(s->file_name, "r+");
    if (!file) {
        perror("ERROR: couldn't open the file");
        return;
    }
    // Move the file pointer to the target location
    if (fseek(file, target_location, SEEK_SET) != 0) {
        perror("ERROR: couldn't move the file pointer to the target location");
        fclose(file);
        return;
    }
    // Write the specified length of data to the file
    size_t written = fwrite(source, s->unit_size, length, file);
    if (written < length) {
        perror("ERROR: couldn't write the specified length of data to the file");
    }
    // Close the file
    fclose(file);
    if (s->debug_mode) {
        printf("Debug: Written %d units of size %d from address %p to file %s at offset %x\n", 
               length, s->unit_size, source, s->file_name, target_location);
    }
}

void memory_modify(state* s) {
    unsigned int location, val;
    // Prompt the user for location and val
    printf("Please enter <location> <val>\n");
    scanf("%x %x", &location, &val);
    if (s->debug_mode) {
        printf("Debug: Location = 0x%x, Value = 0x%x\n", location, val);
    }
    // Check if the location is valid given the current unit size
    if (location + s->unit_size > s->mem_count) {
        printf("ERROR: Location out of bounds\n");
        return;
    }
    // Replace a unit at location in the memory with the value given by val
    switch (s->unit_size) {
        case 1:
            *(unsigned char*)(s->mem_buf + location) = (unsigned char)val; 
            break;
        case 2:
            *(unsigned short*)(s->mem_buf + location) = (unsigned short)val;
            break;
        case 4:
            *(unsigned int*)(s->mem_buf + location) = (unsigned int)val;
            break;
        default:
            printf("ERROR: Invalid unit size\n");
            return;
    }
}

void quit(state* s) {
    if (s->debug_mode) {
        printf("quitting\n");
    }
    exit(0);
}

void menu(state* s) {
    char* menu[] = {
        "0-Toggle Debug Mode",
        "1-Set File Name",
        "2-Set Unit Size",
        "3-Load Into Memory",
        "4-Toggle Display Mode",
        "5-Memory Display",
        "6-Save Into File",
        "7-Memory Modify",
        "8-Quit",
        NULL
    };

    void (*functions[])(state*) = {
        toggle_debug_mode,
        set_file_name,
        set_unit_size,
        load_into_memory,
        toggle_display_mode,  
        memory_display,  
        save_into_file,
        memory_modify,
        quit
    };

    while (1) {
        printf("Choose action:\n");
        for (int i = 0; menu[i] != NULL; i++) {
            printf("%s\n", menu[i]);
            //printf("%d-%s\n", i, menu[i]);
        }

        int choice;
        scanf("%d", &choice);
        getchar();  // Remove newline character left by scanf
        if (choice >= 0 && choice <= 8) {
                functions[choice](s); // like switch case
        } else {
            printf("Invalid choice\n");
        }
        if (s->debug_mode) {
            fprintf(stderr, "Debug: unit_size=%d, file_name='%s', mem_count=%zu\n", s->unit_size, s->file_name, s->mem_count);
        }
    }
}


int main() {
    state s = {0, "", 1, {0}, 0}; // debug mode - off ,file name - "", unit size - 1, memory buffer - {0}, memory count - 0
    menu(&s);
    return 0;
}
