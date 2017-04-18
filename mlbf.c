#include <stdio.h>
#include <stdlib.h>

const size_t STDIN_ALLOC_SIZE = 64;

/**
 * Reads a file and returns a string containing the contents. A size must be
 * specified which allows the caller to control allocation size.
 */
char *read_file(FILE *fp, size_t size)
{
    char *str;      // Will contain file contents when the function completes.
    int ch;         // Holder for last character read from the file.
    size_t len = 0; // Current length of the string (not allocated size).

    str = malloc(sizeof(char) * size);
    if (!str) goto fail;

    // Read bytes from the file copying them into the string. The string will
    // be dynamically reallocated as needed.
    while ((ch = getc(fp)) != EOF) {
        str[len++] = ch;

        if (len >= size) {
            size += STDIN_ALLOC_SIZE;
            char *nstr = realloc(str, sizeof(char) * size);
            if (!nstr) goto fail; // Failed realloc, cleanup.
            str = nstr;
        }
    }
    str[len++] = '\0';

    return str;

fail:
    free(str);
    return NULL;
}

int main(void)
{
    char *src = read_file(stdin, STDIN_ALLOC_SIZE);
    printf("%s\n", src);

    free(src);
    return 0;
}
