#pragma once
#include <vector>
#include <mutex>
#include <atomic>
#include <algorithm>
#include <functional>
#include <assert.h>

template <typename T>
class ThreadedQueue
{
public:
	ThreadedQueue() {};
	~ThreadedQueue() {};

	void Push(T const& data) 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		m_data.push_back(data);
	}

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

	bool Pop(T & data) 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		if (m_data.empty())
			return false;

		data = (m_data.front());
		m_data.erase(m_data.begin());
		return true;
	}

	bool IsEmpty() 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		return m_data.empty();
	}

	void Clear() 
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		m_data.empty();
	}

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

	size_t Size()
	{
		std::lock_guard<std::mutex> lock(m_dataMutex);
		return m_data.size();
	}

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