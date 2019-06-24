#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>

int main()
{
    unsigned char buffer[64] = {0};
    while (fread(buffer, 1, 64, stdin) == 64)
    {
        uint64_t size;
        memcpy(&size, &buffer[56], 8);
        size /= 8;
        fwrite(buffer, 1, size, stdout);
        printf("\n");
    }
}