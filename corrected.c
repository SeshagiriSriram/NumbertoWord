#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_BUFFER 2048
#define MAX_SCALES 10

// Global lookup arrays with explicit array sizes declared for runtime safety
const char* units[20] = {
    "", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine",
    "Ten", "Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", 
    "Seventeen", "Eighteen", "Nineteen"
};

const char* tens[10] = {
    "", "", "Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety"
};

typedef struct {
    const char* locale_id;
    const char* locale_name;
    const char* scale_names[MAX_SCALES];
    long long int initial_modulus;   
    long long int repeating_modulus; 
} LocaleConfig;

// --- DEFENSIVE FIX 1: snprintf bounds tracking ---
// Replaced dangerous strcat loops with strict array length calculations.
void convertTwoDigits(int num, char* result, size_t max_len) {
    if (num >= 20) {
        size_t current_len = strlen(result);
        snprintf(result + current_len, max_len - current_len, "%s", tens[num / 10]);
        
        if (num % 10 > 0) {
            current_len = strlen(result);
            snprintf(result + current_len, max_len - current_len, " %s", units[num % 10]);
        }
    } else if (num > 0) {
        size_t current_len = strlen(result);
        snprintf(result + current_len, max_len - current_len, "%s", units[num]);
    }
}

void convertThreeDigits(int num, char* result, size_t max_len) {
    if (num >= 100) {
        size_t current_len = strlen(result);
        snprintf(result + current_len, max_len - current_len, "%s Hundred ", units[num / 100]);
        num %= 100;
    }
    convertTwoDigits(num, result, max_len);
}

void UniversalNumberConverter(long long int num, const LocaleConfig* config) {
    if (num == 0) {
        printf("Zero\n");
        return;
    }

    if (num < 0) {
        printf("Minus ");
        if (num == -9223372036854775807LL - 1LL) {
            fprintf(stderr, "Error: Absolute integer boundary limit exceeded.\n");
            return;
        }
        num = -num;
    }

    char finalResult[MAX_BUFFER] = "";
    char groupsBuffer[MAX_SCALES][256];
    
    // Explicitly initialize every memory block to clear compiler warnings
    for (int i = 0; i < MAX_SCALES; i++) {
        groupsBuffer[i][0] = '\0';
    }
    
    int groupIndex = 0;

    // 1. Process Initial Base Chunk
    long long int current_mod = config->initial_modulus;
    int chunk = num % current_mod;
    if (chunk != 0) {
        convertThreeDigits(chunk, groupsBuffer[groupIndex], sizeof(groupsBuffer[groupIndex]));
        
        size_t len = strlen(groupsBuffer[groupIndex]);
        if (len < sizeof(groupsBuffer[groupIndex]) - 1) {
            groupsBuffer[groupIndex][len] = ' ';
            groupsBuffer[groupIndex][len + 1] = '\0';
        }
    }
    num /= current_mod;
    groupIndex++;

    // 2. Process Repeating Structural Scale Chunks
    current_mod = config->repeating_modulus;
    while (num > 0 && groupIndex < MAX_SCALES) {
        chunk = num % current_mod;
        
        // --- DEFENSIVE FIX 2: Bounds Checking Scale Access ---
        // Prevents out-of-bounds array reads if the scale configuration map is smaller than MAX_SCALES
        if (chunk != 0 && config->scale_names[groupIndex] != NULL) {
            char chunkStr[256] = "";
            
            if (current_mod >= 1000) {
                convertThreeDigits(chunk, chunkStr, sizeof(chunkStr));
            } else {
                convertTwoDigits(chunk, chunkStr, sizeof(chunkStr));
            }
            
            size_t current_len = strlen(chunkStr);
            snprintf(chunkStr + current_len, sizeof(chunkStr) - current_len, " ");
            
            if (strlen(config->scale_names[groupIndex]) > 0) {
                current_len = strlen(chunkStr);
                snprintf(chunkStr + current_len, sizeof(chunkStr) - current_len, "%s ", config->scale_names[groupIndex]);
            }
            
            snprintf(groupsBuffer[groupIndex], sizeof(groupsBuffer[groupIndex]), "%s", chunkStr);
        }
        num /= current_mod;
        groupIndex++;
    }

    // 3. Reverse build string buffer into final left-to-right format
    for (int i = groupIndex - 1; i >= 0; i--) {
        if (strlen(groupsBuffer[i]) > 0) {
            size_t current_len = strlen(finalResult);
            snprintf(finalResult + current_len, sizeof(finalResult) - current_len, "%s", groupsBuffer[i]);
        }
    }

    // Strip trailing space safely
    int len = strlen(finalResult);
    if (len > 0 && finalResult[len - 1] == ' ') {
        finalResult[len - 1] = '\0';
    }

    printf("%s\n", finalResult);
}

int sanitizeAndValidate(const char* input, char* output) {
    int i = 0, j = 0, has_digits = 0;
    if (input[i] == '-' || input[i] == '+') output[j++] = input[i++];
    while (input[i] != '\0' && j < (MAX_BUFFER - 1)) {
        if (input[i] == ',') { i++; continue; }
        if (!isdigit((unsigned char)input[i])) return 0;
        has_digits = 1;
        output[j++] = input[i++];
    }
    output[j] = '\0';
    return has_digits;
}

void printHelp(const char* progName, const LocaleConfig profiles[], int count) {
    fprintf(stderr, "Usage: %s [flag] <number>\n\n", progName);
    fprintf(stderr, "Available Locale Flags:\n");
    for (int i = 0; i < count; i++) {
        fprintf(stderr, "  -%s, --%-10s (%s format)\n", 
                profiles[i].locale_id, profiles[i].locale_name, profiles[i].locale_name);
    }
}

int main(int argc, char* argv[]) {
    // --- DEFENSIVE FIX 3: Explicit Null Pointer Map Termination ---
    // Added trailing explicit NULL pointers to ensure safe initialization boundaries.
    LocaleConfig profiles[] = {
        {
            "w", "western", 
            {"", "Thousand", "Million", "Billion", "Trillion", "Quadrillion", NULL, NULL, NULL, NULL}, 
            1000, 1000
        },
        {
            "h", "hindi", 
            {"", "Thousand", "Lakh", "Crore", "Arab", "Kharab", "Nil", NULL, NULL, NULL}, 
            1000, 100
        },
        {
            "t", "tamil", 
            {"", "Aayiram", "Ilatcham", "Kodi", "Nirbudham", "Vindhai", NULL, NULL, NULL, NULL} , 
            1000, 100
        }
    };
    int profileCount = sizeof(profiles) / sizeof(profiles[0]);
    const LocaleConfig* activeConfig = &profiles[0]; 
    char* numStrRaw = NULL;

    if (argc < 2 || argc > 3) {
        printHelp(argv[0], profiles, profileCount);
        return 1;
    }

    if (argc == 2) {
        numStrRaw = argv[1];
    } else {
        int profileFound = 0;
        for (int i = 0; i < profileCount; i++) {
            char longFlag[64];
            snprintf(longFlag, sizeof(longFlag), "--%s", profiles[i].locale_name);
            char shortFlag[8];
            snprintf(shortFlag, sizeof(shortFlag), "-%s", profiles[i].locale_id);

            if (strcmp(argv[1], shortFlag) == 0 || strcmp(argv[1], longFlag) == 0) {
                activeConfig = &profiles[i];
                profileFound = 1;
                break;
            }
        }
        if (!profileFound) {
            fprintf(stderr, "Error: Unknown locale switch '%s'.\n", argv[1]);
            printHelp(argv[0], profiles, profileCount);
            return 1;
        }
        numStrRaw = argv[2];
    }

    char sanitizedStr[MAX_BUFFER];
    if (!sanitizeAndValidate(numStrRaw, sanitizedStr)) {
        fprintf(stderr, "Error: Invalid string formatting characters in '%s'.\n", numStrRaw);
        return 1;
    }

    char* endptr;
    long long int inputNum = strtoll(sanitizedStr, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Critical math validation failure during transformation.\n");
        return 1;
    }

    UniversalNumberConverter(inputNum, activeConfig);
    return 0;
}
