#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <omp.h>

using namespace std;

vector<int> computePrefixFunction(const string &pattern)
{
    int m = pattern.length();
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

int searchAndPrintLines(const int &fileNum, const string &searchWord, const string &outputFilePath)
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

            // Check if the line contains the search word using KMP algorithm
            int m = searchWord.length();
            int n = line.length();
            int i = 0; // index for the search word
            int j = 0; // index for the line

            while (j < n)
            {
                if (searchWord[i] == line[j])
                {
                    i++;
                    j++;
                }

                if (i == m)
                {
                    // Line contains the search word
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
int main()
{
    string filePath;

    cout << "Enter the word to search: ";
    string searchWord = "Beautiful";
    cin >> searchWord;

    string outputFilePath = "output/" + searchWord + "_output_file.txt ";

    int numThreads = 4;
    omp_set_num_threads(numThreads);

    double start_time = omp_get_wtime();
    double max_computation_time = 0.0;
    double communication_time = 0.0;

    int linesWithWord = 0;

#pragma omp parallel
    {
        double thread_start_time = omp_get_wtime();

#pragma omp for schedule(dynamic) reduction(+ : linesWithWord)
        for (int i = 1; i <= 83; i++)
        {
            linesWithWord += searchAndPrintLines(i, searchWord, outputFilePath);
        }

        double thread_end_time = omp_get_wtime();
        double thread_computation_time = thread_end_time - thread_start_time;

#pragma omp critical
        {
            if (thread_computation_time > max_computation_time)
            {
                max_computation_time = thread_computation_time;
            }
        }
    }

    double end_time = omp_get_wtime();
    double execution_time = end_time - start_time;
    communication_time = execution_time - max_computation_time;

    if (linesWithWord >= 0)
    {
        cout << "\n---------------------------------------------" << endl;
        cout << "Search Report for Word: '" << searchWord << "'" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Total lines containing the word: " << linesWithWord << endl;
        cout << "Results stored in the file: " << outputFilePath << endl;
        cout << "Number of Threads Used: " << numThreads << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Performance Metrics:" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Execution Time: " << fixed << setprecision(6) << execution_time << " seconds" << endl;
        cout << "Computation Time: " << fixed << setprecision(6) << max_computation_time << " seconds" << endl;
        cout << "Communication Time: " << fixed << setprecision(6) << communication_time << " seconds" << endl;
        cout << "---------------------------------------------" << endl;
        cout << "Parallelism Type: ";
        if (max_computation_time > communication_time)
        {
            cout << "Coarse-Grained" << endl;
        }
        else
        {
            cout << "Fine-Grained" << endl;
        }
        cout << "---------------------------------------------" << endl;
    }

    return 0;
}