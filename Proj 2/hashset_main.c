#include "hashset.h"
#define COMMAND_SIZE 256

int main(int argc, char *argv[])
{
    hashset_t hs;
    char inputLine[COMMAND_SIZE];
    char key[128];
    char fileName[COMMAND_SIZE];
    int num, echoMode = 0;

    // Check for echo mode flag.
    if (argc > 1 && strcmp(argv[1], "-echo") == 0)
    {
        echoMode = 1;
    }

    // Initialize the hash set with the default table size.
    hashset_init(&hs, HASHSET_DEFAULT_TABLE_SIZE);

    printf("Hashset Application\n");
    printf("Commands:\n");
    printf("  hashcode <data>  : prints out the numeric hash code for the given key (does not change the hash set)\n");
    printf("  contains <data>  : prints FOUND if data is in the set NOT PRESENT otherwise\n");
    printf("  add <data>       : inserts the given data into the hash set, reports existing data\n");
    printf("  print            : prints all data in the hash set in the order they were addded\n");
    printf("  structure        : prints detailed structure of the hash set\n");
    printf("  clear            : reinitializes hash set to be empty with default size\n");
    printf("  save <file>      : writes the contents of the hash set to the given file\n");
    printf("  load <file>      : clears the current hash set and loads the one in the given file\n");
    printf("  next_prime <int> : if <int> is prime, prints it, otherwise finds the next prime and prints it\n");
    printf("  expand           : expands memory size of hash set to reduce its load factor\n");
    printf("  quit             : exit the program\n");
    // Main command processing loop.
    while (1)
    {
        if (fgets(inputLine, COMMAND_SIZE, stdin) == NULL)
        {
            break;
        }

        // Optionally echo the command.
        if (echoMode)
        {
            printf("%s", inputLine);
        }

        // Check each command with sscanf or strncmp.
        if (sscanf(inputLine, "hashcode %127s", key) == 1)
        {
            printf("%d\n", hashcode(key));
        }
        else if (sscanf(inputLine, "contains %127s", key) == 1)
        {
            if (hashset_contains(&hs, key))
            {
                printf("FOUND: %s\n", key);
            }
            else
            {
                printf("NOT PRESENT\n");
            }
        }
        else if (sscanf(inputLine, "add %127s", key) == 1)
        {
            int status = hashset_add(&hs, key);
            if (status == 0)
            {
                printf("Data already present, no changes made\n");
            }
        }
        else if (strncmp(inputLine, "print", 5) == 0)
        {
            hashset_write_data_ordered(&hs, stdout);
        }
        else if (strncmp(inputLine, "structure", 9) == 0)
        {
            hashset_show_structure(&hs);
        }
        else if (strncmp(inputLine, "clear", 5) == 0)
        {
            hashset_free_fields(&hs);
            hashset_init(&hs, HASHSET_DEFAULT_TABLE_SIZE);
        }
        else if (sscanf(inputLine, "save %127s", fileName) == 1)
        {
            hashset_save(&hs, fileName);
        }
        else if (sscanf(inputLine, "load %127s", fileName) == 1)
        {
            if (!hashset_load(&hs, fileName))
            {
                printf("load failed\n");
            }
        }
        else if (sscanf(inputLine, "next_prime %d", &num) == 1)
        {
            printf("%d\n", next_prime(num));
        }
        else if (strncmp(inputLine, "expand", 6) == 0)
        {
            hashset_expand(&hs);
        }
        else if (strncmp(inputLine, "quit", 4) == 0)
        {
            break;
        }
    }

    hashset_free_fields(&hs);
    return 0;
}