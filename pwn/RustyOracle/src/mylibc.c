#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

void vuln_code(char *line, size_t size)
{
    char buffer[1024];
    snprintf(buffer, size, "%s", line);
    fprintf(stdout, "%s\n", buffer);
}
