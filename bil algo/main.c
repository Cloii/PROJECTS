#include <stdio.h>
#include <time.h>

extern void radixSort(int arr[], int n);
extern void cocktailSort(int arr[], int n);
extern void bozoSort(int arr[], int n);
extern void bubbleSort(int arr[], int n);

void printArray(int arr[], int n) {
    printf("Sorted array: ");
    for (int i = 0; i < n; i++)
        printf("%d ", arr[i]);
    printf("\n");
}

int main() {
    int arr[6];

    printf("========================================\n");
    printf("Welcome to Sorting Algorithm Selection\n");
    printf("========================================\n\n");

    printf("Enter 6 elements to sort:\n");
    for (int i = 0; i < 6; i++)
        scanf("%d", &arr[i]);

    printf("\nChoose a sorting algorithm:\n");
    printf("1. Radix Sort\n");
    printf("2. Cocktail Sort\n");
    printf("3. Bozo Sort\n");
    printf("4. Bubble Sort\n");

    int choice;
    printf("Enter your choice (1-4): ");
    scanf("%d", &choice);

    clock_t start, end;
    double cpu_time_used;

    start = clock(); // Start measuring time

    switch (choice) {
        case 1:
            radixSort(arr, 6);
            printf("\nSorted array using Radix Sort:\n");
            break;
        case 2:
            cocktailSort(arr, 6);
            printf("\nSorted array using Cocktail Sort:\n");
            break;
        case 3:
            bozoSort(arr, 6);
            printf("\nSorted array using Bozo Sort:\n");
            break;
        case 4:
            bubbleSort(arr, 6);
            printf("\nSorted array using Bubble Sort:\n");
            break;
        default:
            printf("\nInvalid choice. Please enter a number between 1 and 4.\n");
            return 1;
    }

    end = clock(); // End measuring time

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("Time taken: %.6f seconds\n", cpu_time_used);

    printArray(arr, 6);
    return 0;
}
