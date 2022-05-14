#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(2 * sizeof(int));
    if (!x)
        return 1;

    x[0] = 1;
    x[1] = 3;

    x = 0;

    return 0;
}
