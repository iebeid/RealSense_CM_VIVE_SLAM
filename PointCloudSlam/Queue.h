#pragma once


#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

//https://github.com/juanchopanza/cppblog

template <typename T>
class Queue
{
public:

	T pop()
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while (queue_.empty())
		{
			cond_.wait(mlock);
		}
		auto val = queue_.front();
		queue_.pop_front();
		return val;
	}

	void pop(T& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		while (queue_.empty())
		{
			cond_.wait(mlock);
		}
		item = queue_.front();
		queue_.pop_front();
	}

	void push(const T& item)
	{
		std::unique_lock<std::mutex> mlock(mutex_);
		queue_.push_back(item);
		mlock.unlock();
		cond_.notify_one();
	}

	T get(int id){
		return queue_[id];
	}
	Queue() = default;
	Queue(const Queue&) = delete;            // disable copying
	Queue& operator=(const Queue&) = delete; // disable assignment

private:
	std::deque<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;
};