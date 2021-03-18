#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME "/Users/siyual.park/Project/soongsil/soongsil-system-programing/assignments_1/input"

struct Array {
    void* value;
    size_t size;
    size_t count;
};


size_t get_file_size(char *name) {
    FILE *fp = fopen(name, "r");

    fseek(fp, 0L, SEEK_END);
    size_t size = ftell(fp);

    fclose(fp);

    return size;
}

void load_values(FILE *fp, char *buffer, size_t size) {
    size_t str_buffer_size = size * sizeof(char) * 8;

    char *str_buffer = calloc(str_buffer_size, sizeof(char));
    fread(str_buffer, sizeof(char), str_buffer_size, fp);

    unsigned char byte = 0;
    for (size_t i = 0; i < str_buffer_size; i++) {
        byte = byte << 1;

        char currnet = str_buffer[i];
        if (currnet == '1') {
            byte += 1;
        }

        if (((i + 1) % 8) == 0) {
            char *desk = buffer + size - ((i + 1) / 8);
            memcpy(desk, &byte, sizeof(char));
            byte = 0;
        }
    }

    free(str_buffer);
}

void *load_data(FILE *fp, size_t size, size_t unit_size) {
    size_t buffer_size = size / sizeof(char) * 8;
    
    char *buffer = malloc(buffer_size);
    char *unit_buffer = malloc(unit_size);

    memset(buffer, 0, buffer_size);

    char *desk = buffer;
    while (feof(fp) == 0) {
        memset(unit_buffer, 0, unit_size);
        load_values(fp, unit_buffer, unit_size);
        memcpy(desk, unit_buffer, unit_size);

        desk += unit_size;
    }

    free(unit_buffer);

    return buffer;
}

struct Array load(char *name, size_t unit_size) {
    size_t size = get_file_size(NAME);
    size_t data_size = size / 8 / unit_size;

    FILE *fp = fopen(NAME, "r");
    
    void *data = load_data(fp, size, unit_size);

    fclose(fp);
    
    return (struct Array) { .value = data, .size = unit_size, .count = data_size };
}


void print_data() {
    struct Array data = load(NAME, sizeof(char));
    size_t count = data.count;
    char *value = data.value;

    char maxPow = (char)(1 << (8-1));

    for (size_t i = 0; i < count; i++) {
        char current = value[i];
        for (size_t j = 0; j < 8; ++j) {
            printf("%u", current & maxPow ? 1 : 0);
            current = current << 1;
        }
        printf(" ");
    }
    printf("\n");

    free(value);
}

void print_signed_char() {
    struct Array data = load(NAME, sizeof(char));
    size_t count = data.count;
    char *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%d", value[i]);
    }
    printf("\n");

    free(value);
}

void print_unsigned_char() {
    struct Array data = load(NAME, sizeof(unsigned char));
    size_t count = data.count;
    unsigned char *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%u", value[i]);
    }
    printf("\n");

    free(value);
}

void print_ascii() {
    struct Array data = load(NAME, sizeof(char));
    size_t count = data.count;
    char *value = data.value;

    for (size_t i = 0; i < count; i++) {
        char current = value[i];
        if (current >= 0 && current <= 127) {
            printf("%c", value[i]);
        } else {
            printf(".");
        }
    }
    printf("\n");

    free(value);
}

void print_signed_int() {
    struct Array data = load(NAME, sizeof(int));
    size_t count = data.count;
    int *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%d", value[i]);
    }
    printf("\n");

    free(value);
}

void print_unsigned_int() {
    struct Array data = load(NAME, sizeof(unsigned int));
    size_t count = data.count;
    int *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%u", value[i]);
    }
    printf("\n");
    
    free(value);
}

void print_signed_float() {
    struct Array data = load(NAME, sizeof(float));
    size_t count = data.count;
    float *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%0.4f", value[i]);
    }
    printf("\n");
    
    free(value);
}

// ??
void print_unsigned_float() {
    struct Array data = load(NAME, sizeof(float));
    size_t count = data.count;
    float *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%0.4f", value[i]);
    }
    printf("\n");

    free(value);
}

void print_signed_double() {
    struct Array data = load(NAME, sizeof(double));
    size_t count = data.count;
    double *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%0.4lf", value[i]);
    }
    printf("\n");
    
    free(value);
}

void print_unsigned_double() {
    struct Array data = load(NAME, sizeof(double));
    size_t count = data.count;
    double *value = data.value;

    for (size_t i = 0; i < count; i++) {
        printf("%0.4lf", value[i]);
    }
    printf("\n");
    
    free(value);
}

int main() {
    print_data();

    print_signed_char();
    print_ascii();
    print_unsigned_char();

    print_signed_int();
    print_unsigned_int();

    print_signed_float();
    print_unsigned_float();

    print_signed_double();
    print_unsigned_double();

    return 0;
}
