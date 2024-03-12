#include <chrono>
#include <functional>
#include <list>
#include <mutex>
#include <thread>
#include <vector>
#include <unistd.h>
#include <iostream>

using namespace std;
typedef  function<void()> Task;

class TimerWheel
{
public:
	TimerWheel(int maxTaskNum ,int  waitTime) :
		maxTaskNum(maxTaskNum),
		waitTime(waitTime),
		taskVector(maxTaskNum),
		current_index_(0),
		running(false)
	{

	}

	void static threadFunction(TimerWheel* timer) {
		while (timer->running) {
			std::this_thread::sleep_for(std::chrono::milliseconds(timer->waitTime));
			timer->Tick();
		}
		std::cout << "timer oooops!" << std::endl;
	}

	void start()
	{
		if (running)
		{
			cout << "already runing" << endl;
			return;
		}

		running = true;

		thread_ = thread(threadFunction,this);

		thread_.detach();
	}

	void stop()
	{
		if (!running)
		{
			cout << "already stop" << endl;
			return;
		}

		running = false;

		if (thread_.joinable())
			thread_.join();
	}

	void AddTask(int wait, Task task)
	{
		std::lock_guard<std::mutex> lock(mutex_);

		int addNum = wait / waitTime;
		int addto = (current_index_ + addNum) % maxTaskNum;
		int allindex = addto;
		for (int i = 1; allindex < maxTaskNum; i++)
		{
			allindex = addto * i;
			if (allindex >= maxTaskNum)
				break;
			taskVector[allindex].push_back(task);
		}

	}

private:

	void Tick()
	{
		std::lock_guard<std::mutex> lock(mutex_);

		int n = taskVector[current_index_].size();

		for (int i = 0; i < n; i++)
		{
			Task task = taskVector[current_index_].back();
			taskVector[current_index_].pop_back();
			task();
		}

		current_index_ = (current_index_ + 1) % maxTaskNum;
	}

	int maxTaskNum;
	int waitTime;
	int current_index_;
	vector<vector<Task>> taskVector;
	thread thread_;
	bool running;
	mutex mutex_;
};

int main()
{
	TimerWheel timer(10, 1000);

	timer.start();

	timer.AddTask(2000, []() {std::cout << "Task 1" << std::endl; });
	timer.AddTask(3000, []() {std::cout << "Task 2" << std::endl; });

	sleep(5);

	timer.stop();
}