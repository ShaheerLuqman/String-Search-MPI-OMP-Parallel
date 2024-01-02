#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <chrono>

using namespace std;
using namespace std::chrono;

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
    string searchWord;
    cin >> searchWord;

    string outputFilePath = "output/" + searchWord + "_output_file.txt ";

    auto start_time = high_resolution_clock::now();

    int linesWithWord = 0;

    for (int i = 1; i <= 83; i++)
    {
        linesWithWord += searchAndPrintLines(i, searchWord, outputFilePath);
    }

    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end_time - start_time);

    if (linesWithWord >= 0)
    {
        cout << "Total lines containing the word '" << searchWord << "': " << linesWithWord << endl;
        cout << "Results have been stored in the file: " << outputFilePath << endl;
        cout << "Execution time: " << duration.count() / 1000.0 << " seconds" << endl;
    }

    return 0;
}