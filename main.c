#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define MAX_BUFFER 2048
#define MAX_SCALES 10

// Global lookup arrays for sub-100 values
const char* units[] = {
    "", "One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine",
    "Ten", "Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", 
    "Seventeen", "Eighteen", "Nineteen"
};

const char* tens[] = {
    "", "", "Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety"
};

// Step 1: Define the Decoupled Locale Structure
typedef struct {
    const char* locale_id;
    const char* locale_name;
    const char* scale_names[MAX_SCALES];
    long long int initial_modulus;   // e.g., 1000 for hundreds chunk
    long long int repeating_modulus; // e.g., 100 for Indian pairs, 1000 for Western millions
} LocaleConfig;

// Helper to convert 1 or 2 digit chunks into words
void convertTwoDigits(int num, char* result) {
    if (num >= 20) {
        strcat(result, tens[num / 10]);
        if (num % 10 > 0) {
            strcat(result, " ");
            strcat(result, units[num % 10]);
        }
    } else if (num > 0) {
        strcat(result, units[num]);
    }
}

// Helper to convert up to 3 digit chunks (needed for the initial hundreds group)
void convertThreeDigits(int num, char* result) {
    if (num >= 100) {
        strcat(result, units[num / 100]);
        strcat(result, " Hundred ");
        num %= 100;
    }
    convertTwoDigits(num, result);
}

// Step 3: Write the Pure Engine (Zero hardcoded strings or formatting boundaries)
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
    for (int i = 0; i < MAX_SCALES; i++) groupsBuffer[i][0] = '\0';
    
    int groupIndex = 0;

    // 1. Process Initial Base Chunk (e.g., hundreds place)
    long long int current_mod = config->initial_modulus;
    int chunk = num % current_mod;
    if (chunk != 0) {
        convertThreeDigits(chunk, groupsBuffer[groupIndex]);
        strcat(groupsBuffer[groupIndex], " ");
    }
    num /= current_mod;
    groupIndex++;

    // 2. Process Repeating Structural Scale Chunks (e.g., Millions vs Lakhs/Crores)
    current_mod = config->repeating_modulus;
    while (num > 0 && groupIndex < MAX_SCALES) {
        chunk = num % current_mod;
        if (chunk != 0) {
            char chunkStr[256] = "";
            // Handle chunk words safely depending on size mapping
            if (current_mod >= 1000) {
                convertThreeDigits(chunk, chunkStr);
            } else {
                convertTwoDigits(chunk, chunkStr);
            }
            strcat(chunkStr, " ");
            
            // Append dynamic scale name from structural config map
            if (strlen(config->scale_names[groupIndex]) > 0) {
                strcat(chunkStr, config->scale_names[groupIndex]);
                strcat(chunkStr, " ");
            }
            strcpy(groupsBuffer[groupIndex], chunkStr);
        }
        num /= current_mod;
        groupIndex++;
    }

    // 3. Reverse build string buffer into final left-to-right format
    for (int i = groupIndex - 1; i >= 0; i--) {
        if (strlen(groupsBuffer[i]) > 0) {
            strcat(finalResult, groupsBuffer[i]);
        }
    }

    // Strip trailing space
    int len = strlen(finalResult);
    if (len > 0 && finalResult[len - 1] == ' ') {
        finalResult[len - 1] = '\0';
    }

    printf("%s\n", finalResult);
}

// Input data sanitization framework
int sanitizeAndValidate(const char* input, char* output) {
    int i = 0, j = 0, has_digits = 0;
    if (input[i] == '-' || input[i] == '+') output[j++] = input[i++];
    while (input[i] != '\0') {
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
    // Step 2: Instantiate Diverse Context Profiles
    LocaleConfig profiles[] = {
        {
            "w", "western", 
            {"", "Thousand", "Million", "Billion", "Trillion", "Quadrillion"}, 
            1000, 1000
        },
        {
            "h", "hindi", 
            {"", "Thousand", "Lakh", "Crore", "Arab", "Kharab", "Nil"}, 
            1000, 100
        },
        {
            "t", "tamil", 
            {"", "Aayiram", "Ilatcham", "Kodi", "Nirbudham", "Vindhai"}, 
            1000, 100
        }
    };
    int profileCount = sizeof(profiles) / sizeof(profiles[0]);
    
    // Default system fallback configuration profile index (Western)
    const LocaleConfig* activeConfig = &profiles[0]; 
    char* numStrRaw = NULL;

    if (argc < 2 || argc > 3) {
        printHelp(argv[0], profiles, profileCount);
        return 1;
    }

    // Parsing command arguments and flags
    if (argc == 2) {
        numStrRaw = argv[1];
    } else {
        int profileFound = 0;
        for (int i = 0; i < profileCount; i++) {
            char longFlag[64];
            sprintf(longFlag, "--%s", profiles[i].locale_name);
            char shortFlag[8];
            sprintf(shortFlag, "-%s", profiles[i].locale_id);

            if (strcmp(argv[1], shortFlag) == 0 || strcmp(argv[1], longFlag) == 0) {
                activeConfig = &profiles[i];
                profileFound = 1;
                break;
            }
        }
        if (!profileFound) {
            fprintf(stderr, "Error: Unknown locale identifier switch configuration '%s'.\n", argv[1]);
            printHelp(argv[0], profiles, profileCount);
            return 1;
        }
        numStrRaw = argv[2];
    }

    char sanitizedStr[MAX_BUFFER];
    if (!sanitizeAndValidate(numStrRaw, sanitizedStr)) {
        fprintf(stderr, "Error: Standard parsing layer rejected '%s' due to invalid string formatting characters.\n", numStrRaw);
        return 1;
    }

    char* endptr;
    long long int inputNum = strtoll(sanitizedStr, &endptr, 10);
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Critical math validation failure during string layout transformation.\n");
        return 1;
    }

    // Execute engine processing pass
    UniversalNumberConverter(inputNum, activeConfig);

    return 0;
}
