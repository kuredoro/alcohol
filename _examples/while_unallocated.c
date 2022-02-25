#include <stdlib.h>
//#include <stdio.h>

int main()
{
    int dummy;
    //scanf("%d", &dummy);

    int* x = NULL;
    while (dummy != 0)
    {
        x = (int*)malloc(2 * sizeof(int));
        if (!x)
            return 1;

        x[0] = 1;
        x[1] = 2;
    }

    free(x);

    return 0;
}
