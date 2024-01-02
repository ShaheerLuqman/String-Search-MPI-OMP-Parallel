#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <cstring>
#include <mpi.h>

// mpic++ SSearchMPI.cpp -o SSearchMPI && mpirun -n 4 ./SSearchMPI

using namespace std;

vector<int> computePrefixFunction(const char *pattern)
{
    int m = std::strlen(pattern); // Use std::strlen instead of strlen
    vector<int> lps(m, 0);
    int len = 0;
    int i = 1;

    while (i < m)
    {
        if (pattern[i] == pattern[len])
        {
            len++;
            lps[i] = len;
            i++;
        }
        else
        {
            if (len != 0)
            {
                len = lps[len - 1];
            }
            else
            {
                lps[i] = 0;
                i++;
            }
        }
    }

    return lps;
}

int searchAndPrintLines(const int &fileNum, const char *searchWord, const string &outputFilePath)
{
    string filePath = "dataset/dataset_" + to_string(fileNum) + ".txt";
    ifstream file(filePath);
    ofstream outputFile(outputFilePath, ios::app);

    if (file.is_open() && outputFile.is_open())
    {
        string line;
        int lineNumber = 0;
        int linesWithWord = 0;

        vector<int> lps = computePrefixFunction(searchWord);

        while (getline(file, line))
        {
            lineNumber++;

            int m = std::strlen(searchWord); // Use std::strlen instead of strlen
            int n = line.length();
            int i = 0;
            int j = 0;

            while (j < n)
            {
                if (searchWord[i] == line[j])
                {
                    i++;
                    j++;
                }

                if (i == m)
                {
                    outputFile << setfill('0') << setw(2) << fileNum << "|" << setw(6) << lineNumber << " : " << line << endl;
                    linesWithWord++;
                    i = lps[i - 1];
                    break;
                }
                else if (j < n && searchWord[i] != line[j])
                {
                    if (i != 0)
                    {
                        i = lps[i - 1];
                    }
                    else
                    {
                        j++;
                    }
                }
            }
        }

        file.close();
        outputFile.close();
        return linesWithWord;
    }
    else
    {
        cerr << "Unable to open files: " << filePath << " or " << outputFilePath << endl;
        return -1;
    }
}
int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char searchWord[256];

    if (rank == 0)
    {
        cout << "Enter the word to search: ";
        // const char *tempWord = "Beautiful";
        // std::strcpy(searchWord, tempWord);
        cin.getline(searchWord, sizeof(searchWord));
    }

    MPI_Bcast(searchWord, sizeof(searchWord), MPI_CHAR, 0, MPI_COMM_WORLD);

    string outputFilePath = "output/" + string(searchWord) + "_output_file.txt ";

    double start_time = MPI_Wtime();
    double computation_time = 0.0;

    int linesWithWord = 0;
    MPI_Barrier(MPI_COMM_WORLD); // Synchronize processes before starting work

    double local_computation_time = MPI_Wtime();
    for (int i = rank + 1; i <= 83; i += size)
    {
        linesWithWord += searchAndPrintLines(i, searchWord, outputFilePath);
    }
    local_computation_time = MPI_Wtime() - local_computation_time;

    MPI_Reduce(&local_computation_time, &computation_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD); // Synchronize processes before measuring communication time

    int totalLinesWithWord;
    MPI_Reduce(&linesWithWord, &totalLinesWithWord, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    double end_time = MPI_Wtime();
    double execution_time = end_time - start_time;
    double communication_time = execution_time - computation_time;

    if (rank == 0)
    {
        cout << "---------------------------------------------" << endl;
        cout << "Search Report for Word: '" << searchWord << "'" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Total lines containing the word: " << totalLinesWithWord << endl;
        cout << "Results stored in the file: " << outputFilePath << endl;
        cout << "Number of Processes Used: " << size << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Parallelism Type: MPI (Message Passing Interface)" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Performance Metrics:" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Execution Time: " << fixed << setprecision(6) << execution_time << " seconds" << endl;
        cout << "Computation Time: " << fixed << setprecision(6) << computation_time << " seconds" << endl;
        cout << "Communication Time: " << fixed << setprecision(6) << communication_time << " seconds" << endl;
        cout << "Parallelism Type: ";
        if (computation_time > communication_time)
        {
            cout << "Coarse-Grained" << endl;
        }
        else
        {
            cout << "Fine-Grained" << endl;
        }
        cout << "---------------------------------------------" << endl;
    }

    MPI_Finalize();

    return 0;
}