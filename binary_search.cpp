#include <iostream>

int binarySearch(int arr[], int low, int high, int target) {
    if (low > high) {
        return -1; // Base case: target is not in the array
    }

    int mid = low + (high - low) / 2; // Calculate the midpoint

    if (arr[mid] == target) {
        return mid; // Target found at mid
    } else if (arr[mid] > target) {
        return binarySearch(arr, low, mid - 1, target); // Search in the left half
    } else {
        return binarySearch(arr, mid + 1, high, target); // Search in the right half
    }
}

int main() {
    int arr[8] = {1, 3, 5, 7, 9, 11, 13, 15};
    int target = 7;
    int result = binarySearch(arr, 0, sizeof(arr) - 1, target);

    if (result != -1) {
        std::cout << "Target found at index: " << result << std::endl;
    } else {
        std::cout << "Target not found." << std::endl;
    }

    return 0;
}
