#include <stdlib.h>

int main()
{
    int* x = NULL;
    while (x != (int*)25)
    {
        x = (int*)malloc(2 * sizeof(int));
        x[0] = 1;
        x[1] = 2;
    }

    free(x);

    return 0;
}
