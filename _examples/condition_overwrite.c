#include <stdlib.h>
#include <stdio.h>

int main()
{
    // Simulate statically undecidable condition
    int dummy;
    scanf("%d", &dummy);

    int* x = (int*)malloc(4 * sizeof(int));
    if (!x) return 0;

    int* y = NULL;
    if (dummy == 0)
    {
        y = x + 1;
    }
    else
    {
        y = x + 2;
    }

    // x and x+1 are forgotten
    // (...Not really...)
    x = NULL;

    // Bad free
    free(y);

    return 0;
}
