#include <iostream>
#include <algorithm>

using namespace std;

int fibonacci(int n, int *cache) {
    if (n == 0) { return 0; }
    else if (n == 1) { return 1; }
    else if (cache[n] != -1) {
        return cache[n];
    } else {
        cache[n] = fibonacci(n - 1, cache) + fibonacci(n - 2, cache);
        return cache[n];
    }
}

int main() {
    int global_cache[50];
    fill(global_cache, global_cache + 50, -1);
    cout << fibonacci(40, global_cache);
    return 0;
}