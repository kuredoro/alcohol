#include <stdlib.h>

int main()
{
    int* x = (int*)malloc(6 * sizeof(int));
    for (int i = 0; i < 6; i++)
        x[i] = i + 1;

    int* y = (int*)malloc(6 * sizeof(int));
    for (int i = 0; i < 6; i++)
        y[i] = i + 1;

    free(x);
    free(y);

    return 0;
}
