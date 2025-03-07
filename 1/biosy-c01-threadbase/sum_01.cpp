#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <cmath>

using namespace std;

double sum(const size_t &start, const size_t &end)
{
    double output = 0.0;
    for (size_t i = start; i < end; i++)
    {
        double tmp = (sqrt(i + 1) + 1) / (sqrt(pow(i, 2) + i + 1));
        output += tmp;
    }
    return output;
}

//-------------------------------------------
class ThreadClass // <--- #1
{
public:
    ThreadClass(const size_t &num_of_threads) : outputs(num_of_threads, 0), m_num_of_threads(num_of_threads)
    {
    }

    void Solve(int tid, const size_t &m)
    {
        // printf("Thread %d: Start\n", tid);
        size_t start = (tid * m) / m_num_of_threads;
        size_t end = ((tid + 1) * m) / m_num_of_threads;
        outputs[tid] = sum(start, end);
        // printf("Thread %d: Stop\n", tid);
    }
    double Finalize()
    {
        double output = 0;
        for (size_t i = 0; i < outputs.size(); i++)
        {
            output += outputs[i];
        }
        return output;
    }

private:
    vector<double> outputs;
    size_t m_num_of_threads;
};

//-------------------------------------------
int main(int argc, char *argv[])
{
    vector<thread> threads;
    int threadNum;
    size_t number;

    // Check arguments
    if (argc != 3 || sscanf(argv[1], "%zu", &number) != 1 || sscanf(argv[2], "%d", &threadNum) != 1 ||
        number < 0 || number > 100000000 || threadNum < 0 || threadNum > 1024)
    {
        printf("Usage: %s number(0-100000000) number_of_threads(0-1024)\n", argv[0]);
        return 1;
    }

    ThreadClass threadObj(threadNum);

    printf("Main:     Start\n");

    // Create threads
    /*
    Like telling threadNum number of athletes to start running to a nearby tree and return
    without waiting for them to return. We just iterate through them, add them to the
    */
    for (int i = 0; i < threadNum; i++)
    {
        // printf("Main:     Creating thread %d\n", i);
        threads.push_back(thread(&ThreadClass::Solve, &threadObj, i, number)); // <--- #2
    }

    // Wait for threads
    for (int i = 0; i < threadNum; i++)
        threads[i].join();

    double final = threadObj.Finalize();
    printf("Main:     Stop\n");
    printf("Answer: %f\n", final);
    return 0;
}
