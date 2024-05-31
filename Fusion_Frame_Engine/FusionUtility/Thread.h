#pragma once
#include "Log.h"
#include <functional>
#include <queue>
#include <thread>
#include <mutex>
#include <future>

namespace FUSIONUTIL
{
	static std::mutex Lock;
	static std::vector<std::shared_future<void>> shared_futures;
	static std::vector<std::shared_ptr<std::promise<void>>> promises;

	inline void ExecuteFunctionsAsync(std::vector<std::function<void()>> Functions)
	{
		promises.clear();
		shared_futures.clear();

		for (size_t i = 0; i < Functions.size(); i++)
		{
			auto promise = std::make_shared<std::promise<void>>();
			auto future = promise->get_future();

			{
				std::lock_guard<std::mutex> lock(Lock);
				promises.push_back(promise);
				shared_futures.push_back(std::move(future));
			}

			std::thread([func = Functions[i], promise]() mutable {
				func();
				promise->set_value();
				}).detach();
		}
	}

	class ThreadPool
	{
	public:

		ThreadPool(size_t numberOfThreads, size_t maxTaskCount)
		{
			this->maxTaskCount = maxTaskCount;

			for (size_t i = 0; i < numberOfThreads; i++)
			{
				threads.emplace_back([this]
				{
					while (true)
					{
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(mutex);
							condition.wait(lock, [this] { return !tasks.empty() || stop; });
							
							if (stop && tasks.empty())
							{
								workingThreads--;
								return;
							}

							task = std::move(tasks.front());
							tasks.pop();
							
							if (firstTime)
							{
								firstTime = false;
								workingThreads += 2;
							}
							else
							{
								workingThreads++;
							}
						}

						try
						{
							task();
						}
						catch (const std::exception& ex)
						{
							LOG_ERR("Task ended with an exception: :: " << ex.what() << typeid(task).name());
						}

						{
							std::unique_lock<std::mutex> lock(mutex);
							workingThreads--;
							if (workingThreads == 0)
							{
								allow = true;
								//LOG("Allowed :: " << allow);
								condition.notify_all();
							}
							//LOG("Thread executed the task!");
							//LOG("workingThreads: " << workingThreads);
						}
					}
				});
			}

			LOG_INF("Thread pool initialized!");
		}

		template<class F>
		void enqueue(F&& task)
		{
			if (tasks.size() <= maxTaskCount)
			{
				{
					std::unique_lock<std::mutex> lock(mutex);
					tasks.emplace(std::forward<F>(task));
				}
				condition.notify_one();
			}
			else
			{
				LOG_WARN("Reached the maximum count of tasks in queue!");
			}
		}

		~ThreadPool()
		{
			{
				std::unique_lock<std::mutex> lock(mutex);
				stop = true;
			}

			condition.notify_all();

			int Index = 0;
			for (auto& thread : threads)
			{
				thread.join();
				LOG_INF("Thread " << Index << " is distrupted!");
				Index++;
			}
		}

		void wait()
		{
			{
			  std::unique_lock<std::mutex> lock(mutex);
			  condition.wait(lock, [this]() { return allow; });
			  //LOG_INF("Main thread released wait");
			  //LOG_INF("Main thread finished waiting");
			  firstTime = true;
			  workingThreads = -1;
			  allow = false;
			}
		}


	private:

		std::vector<std::thread> threads;
		std::queue<std::function<void()>> tasks;
		std::mutex mutex;
		std::condition_variable condition;
		std::condition_variable IsFinishedcondition;
		bool stop = false;
		size_t maxTaskCount;
		int workingThreads = -1;
		bool firstTime = true;

		bool allow = false;
	};
}
