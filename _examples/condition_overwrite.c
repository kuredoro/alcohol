#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(4 * sizeof(int));
    int* y = NULL;
    if (x != NULL)
    {
        y = x + 1;
    }
    else
    {
        y = x + 2;
    }

    x = NULL;

    free(y);

    return 0;
}
