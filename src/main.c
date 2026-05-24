#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

typedef enum {
    DecodeTypeStr,
    DecodeTypeInt,
    DecodeTypeList,
    DecodeTypeDict,
    DecodeTypeMax,
} DecodeType;

struct decode_entry;

typedef struct decode_entry {
    DecodeType type;
    union {
        char *decode_str;
        struct {
            struct decode_entry *sub_e[16];
            int sub_e_len;
        };
    };
    int cus_len;
} DecodeEntry;

void free_decode_entry(DecodeEntry *e) {
    switch (e->type) {
        case DecodeTypeList:
        case DecodeTypeDict:
            for (int i = 0; i < e->sub_e_len; i++) {
                free_decode_entry(e->sub_e[i]);
            }
            break;
        case DecodeTypeStr:
        case DecodeTypeInt:
            free(e->decode_str);
            break;
        default:
            break;
    }
    free(e);
}

void print_decode_entry(DecodeEntry *e) {
    switch (e->type) {
        case DecodeTypeStr:
            printf("\"%s\"", e->decode_str);
            break;
        case DecodeTypeInt:
            printf("%s", e->decode_str);
            break;
        case DecodeTypeList:
            putchar('[');
            for (int i = 0; i < e->sub_e_len; i++) {
                print_decode_entry(e->sub_e[i]);
                if (i != e->sub_e_len - 1) {
                    putchar(',');
                }
            }
            putchar(']');
            break;
        case DecodeTypeDict:
            putchar('{');
            for (int i = 0; i < e->sub_e_len; i += 2) {
                print_decode_entry(e->sub_e[i]);       // key
                putchar(':');
                print_decode_entry(e->sub_e[i + 1]);   // value
                if (i + 2 < e->sub_e_len) putchar(',');
            }
            putchar('}');
            break;
        default:
            break;
    }
    return;
}

DecodeEntry* decode_bencode(const char* bencoded_value) {
    DecodeEntry *e = calloc(1, sizeof(*e));
    e->type = DecodeTypeMax;

    if (is_digit(bencoded_value[0])) {
        int length = atoi(bencoded_value);
        const char* colon = strchr(bencoded_value, ':');
        if (colon == NULL) {
            fprintf(stderr, "Invalid encoded value\n");
            exit(1);
        }
        const char* start = colon + 1;
        char* decoded_str = malloc(length + 1);
        memcpy(decoded_str, start, length);
        decoded_str[length] = '\0';
        e->type = DecodeTypeStr;
        e->decode_str = decoded_str;
        e->cus_len = (start + length) - bencoded_value;
    } else if (bencoded_value[0] == 'i') {
        const char *p = bencoded_value + 1;
        if (*p == '-') p++;
        while (is_digit(*p)) p++;
        // p now points to 'e'
        int len = p - (bencoded_value + 1);
        char *decoded_str = calloc(1, len + 1);
        memcpy(decoded_str, bencoded_value + 1, len);
        e->type = DecodeTypeInt;
        e->decode_str = decoded_str;
        e->cus_len = (p + 1) - bencoded_value;
    } else if (bencoded_value[0] == 'l') {
        e->type = DecodeTypeList;
        int offset = 1; // skip 'l'
        while (bencoded_value[offset] != 'e') {
            DecodeEntry *sub_e = decode_bencode(bencoded_value + offset);
            e->sub_e[e->sub_e_len++] = sub_e;
            offset += sub_e->cus_len;
        }
        e->cus_len = offset + 1; // +1 for trailing 'e'
    } else if (bencoded_value[0] == 'd') {
        e->type = DecodeTypeDict;
        int offset = 1; // skip 'd'
        while (bencoded_value[offset] != 'e') {
            DecodeEntry *sub_e = decode_bencode(bencoded_value + offset);
            e->sub_e[e->sub_e_len++] = sub_e;
            offset += sub_e->cus_len;
        }
        e->cus_len = offset + 1; // +1 for trailing 'e'
    } else {
        fprintf(stderr, "Only strings, integers, lists and dicts are supported\n");
        exit(1);
    }

    return e;
}

int main(int argc, char* argv[]) {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: your_program.sh <command> <args>\n");
        return 1;
    }

    const char* command = argv[1];

    if (strcmp(command, "decode") == 0) {
    	// You can use print statements as follows for debugging, they'll be visible when running tests.
        fprintf(stderr, "Logs from your program will appear here!\n");
            
        // TODO: Uncomment the code below to pass the first stage
        const char* encoded_str = argv[2];
        DecodeEntry *e = decode_bencode(encoded_str);
        print_decode_entry(e);
        putchar('\n');
        free_decode_entry(e);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}
