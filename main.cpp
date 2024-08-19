#include "SPSCLockFreeQueue.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace std;

constexpr uint32_t BUFFER_SIZE = 1000 * 1000; 
constexpr uint32_t NUM_OPERATIONS = 1000000; 

struct LargeStruct
{
    int data[100];
};

template <typename Queue>
void producer(Queue &queue)
{
    LargeStruct item;
    for (uint32_t i = 0; i < NUM_OPERATIONS; ++i)
    {
		queue.push( item );
    }
}

template <typename Queue>
void consumer(Queue &queue)
{
    uint32_t count = 0;
    LargeStruct item;
    while (count < NUM_OPERATIONS)
    {
        if (queue.pop(item))
        {
            count++;
        }
    }
}

template <typename Queue>
void benchmark(const std::string &name, Queue &queue)
{
    auto start = std::chrono::high_resolution_clock::now();

    std::thread prod(producer<Queue>, std::ref(queue));
    std::thread cons(consumer<Queue>, std::ref(queue));

    prod.join();
    cons.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << name << " - Elapsed time: " << elapsed.count() << " seconds" << std::endl;
}

int main()
{
	for (size_t i = 0; i < 10; i++)
	{
		SPSCLockFreeQueue<LargeStruct> queue;
		queue.Allocate(BUFFER_SIZE);

		benchmark("SPSCLockFreeQueue", queue);
	}

	return 0;
}
