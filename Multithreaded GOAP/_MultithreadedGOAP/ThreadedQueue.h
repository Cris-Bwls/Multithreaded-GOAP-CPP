#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <functional>
#include <assert.h>

/*
	Templated Threaded Queue
*/
template <typename T>
class ThreadedQueue
{
public:
	ThreadedQueue() {};
	~ThreadedQueue() {};

	/*
		Adds data to queue (simple PushBack)
	*/
	void Push(T const& data) 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		m_data.push_back(data);
	}

	/*
		Adds data to queue and sorts it according to given function
	*/
	void PushSorted(T const& data)
	{
		assert(sortSet || "Sort Function not set in ThreadedQueue, tried to PushSorted()");

		std::lock_guard<std::mutex> lock(m_dataMutex);
		if (!isHeap)
		{
			isHeap = true;
			std::make_heap(m_data.begin(), m_data.end(), m_SortFunc);
		}

		m_data.push_back(data);
		std::push_heap(m_data.begin(), m_data.end(), m_SortFunc);
	}

	/*
		gives the front value as data and pops it from queue, returns true if there was data
	*/
	bool Pop(T & data) 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		if (m_data.empty())
			return false;

		data = (m_data.front());
		m_data.erase(m_data.begin());
		return true;
	}

	/*
		Returns true if queue is empty
	*/
	bool IsEmpty() 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		return m_data.empty();
	}

	/*
		Clears the queue
	*/
	void Clear() 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		m_data.empty();
	}

	/*
		Sorts queue according to given Sort Function
	*/
	void Sort()
	{
		assert(sortSet || "Sort Function not set in ThreadedQueue, tried to Sort()");

		std::lock_guard<std::mutex> lock(m_dataMutex);
		if (!isHeap)
		{
			isHeap = true;
			std::make_heap(m_data.begin(), m_data.end(), m_SortFunc);
		}
		std::sort_heap(m_data.begin(), m_data.end(), m_SortFunc);
	}

	/*
		returns the length/size of the queue
	*/
	size_t Size()
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		return m_data.size();
	}

	/*
		Sets the sort function (required before attempting PushSorted or Sort)
	*/
	void SetSortFunc(std::function<bool(T const&, T const&)> SortFunc)
	{
		m_SortFunc = SortFunc;
		sortSet = true;
	}

private:
	std::function<bool(T const&, T const&)> m_SortFunc;
	std::vector<T> m_data;
	bool isHeap = false;
	bool sortSet = false;
	std::mutex m_dataMutex;
};