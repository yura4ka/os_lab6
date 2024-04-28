#include <iostream>
#include <random>
#include <thread>
#include <vector>
#include <chrono>
#include <fstream>

using std::vector;

typedef vector<vector<int>> Matrix;

const bool PRINT_VALUES = false;
const bool PRINT_RESULT = true;

int generateRandomNumber(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}

void fillRandomMatrix(Matrix &m) {
    for (auto &i: m)
        for (int &j: i)
            j = generateRandomNumber(-10, 10);
}

void calculateBlock(const Matrix &A, const Matrix &B, Matrix &C,
                    const int &start, const int &end, bool printValues) {
    for (int pos = start; pos < end; pos++) {
        int i = pos / C[0].size();
        int j = pos % C[0].size();
        C[i][j] = 0;
        for (int k = 0; k < B.size(); k++)
            C[i][j] += A[i][k] * B[k][j];

        if (printValues)
            std::cout << "(" << i << ", " << j << "): " << C[i][j] << "\n";
    }
}

void printMatrix(const Matrix &m) {
    for (auto &i: m) {
        for (auto &j: i)
            std::cout << j << " ";
        std::cout << "\n";
    }
    std::cout << "\n";
}

void printMatrixWolfram(const Matrix &m) {
    int r = m.size();
    int c = m[0].size();
    std::cout << "{";
    for (int i = 0; i < r - 1; i++) {
        std::cout << "{";
        for (int j = 0; j < c - 1; j++)
            std::cout << m[i][j] << ",";
        std::cout << m[i][c - 1] << "},";
    }
    std::cout << "{";
    for (int j = 0; j < c - 1; j++)
        std::cout << m[r - 1][j] << ",";
    std::cout << m[m.size() - 1][m[0].size() - 1] << "}}\n";
}

void multiplyMatrices(const Matrix &A, const Matrix &B, Matrix &C, const int threadsCount) {
    vector<std::thread> threads;
    int totalElements = A.size() * C[0].size();
    int perThread = 1 + ((totalElements - 1) / threadsCount);

    for (int i = 0; i < threadsCount; i++) {
        int start = i * perThread;
        int end = std::min(start + perThread, totalElements);
        threads.emplace_back(calculateBlock, ref(A), ref(B), ref(C), start, end, PRINT_VALUES);
    }
    for (auto &t: threads)
        t.join();
}

void benchmark(const int &size) {
    unsigned int n = std::thread::hardware_concurrency();
    std::cout << n << " concurrent threads are supported.\n";

    Matrix A(size, vector<int>(size));
    Matrix B(size, vector<int>(size));
    Matrix C(size, vector<int>(size));

    fillRandomMatrix(A);
    fillRandomMatrix(B);

    std::ofstream results("results.csv");

    for (int threadsCount = 1; threadsCount <= 16; threadsCount++) {
        std::cout << "Benchmarking with " << threadsCount << " threads... ";
        auto startTime = std::chrono::high_resolution_clock::now();
        multiplyMatrices(A, B, C, threadsCount);
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        std::cout << "    DURATION " << duration.count() << "ms\n";
        results << threadsCount << "," << duration.count() << "\n";
    }
}

int main() {
    int type;
    std::cout << "Perform benchmark (1) or demonstrate program (2) : ";
    std::cin >> type;
    if (type == 1) {
        benchmark(1000);
        return 0;
    }

    int n, m, k, threadsCount;
    std::cout << "Enter n m k: ";
    std::cin >> n >> m >> k;
    std::cout << "Enter the number of threads: ";
    std::cin >> threadsCount;

    Matrix A(n, vector<int>(m));
    Matrix B(m, vector<int>(k));
    Matrix C(n, vector<int>(k));

    fillRandomMatrix(A);
    fillRandomMatrix(B);

    vector<std::thread> threads;
    int totalElements = n * k;
    int perThread = 1 + ((totalElements - 1) / threadsCount);

    for (int i = 0; i < threadsCount; i++) {
        int start = i * perThread;
        int end = std::min(start + perThread, totalElements);
        threads.emplace_back(calculateBlock, ref(A), ref(B), ref(C), start, end, PRINT_VALUES);
    }
    for (auto &t: threads)
        t.join();

    if (PRINT_RESULT) {
        printMatrixWolfram(A);
        printMatrixWolfram(B);
        printMatrix(C);
    }
    return 0;
}
