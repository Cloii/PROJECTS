#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int isSorted(int arr[], int n) {
    for (int i = 0; i < n - 1; i++)
        if (arr[i] > arr[i + 1])
            return 0;
    return 1;
}

void bozoSort(int arr[], int n) {
    srand(time(0));
    while (!isSorted(arr, n)) {
        int i = rand() % n;
        int j = rand() % n;
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}
