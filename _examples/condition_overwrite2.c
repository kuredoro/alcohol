#include <stdlib.h>
#include <stdio.h>

int main()
{
    // Simulate statically undecidable condition
    int dummy;
    scanf("%d", &dummy);

    int* x = (int*)malloc(4 * sizeof(int));
    if (!x) return 0;

    int* y = (int*)malloc(4 * sizeof(int));
    if (dummy == 0)
    {
        // y is forgotten
        y = x + 1;
    }
    else
    {
        // y is forgotten
        y = x + 2;
    }

    x = NULL;

    // Bad free
    free(y);

    return 0;
}
