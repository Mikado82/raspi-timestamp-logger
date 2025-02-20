// Taken and adapted from:
// https://github.com/mvorbrodt/blog/blob/master/src/lesson_queue_how_to.cpp

#ifndef SYNCED_QUEUE_INCLUDED
#define SYNCED_QUEUE_INCLUDED

#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>

template<typename T>
class synced_queue
{
public:
	synced_queue() = default;
	~synced_queue()
	{
		done();
	};

	void push(const T& item)
	{
		{
			std::unique_lock guard(m_queue_lock);
			m_queue.push(item);
		}
		m_condition.notify_one();
	}

	void push(T&& item)
	{
		{
			std::unique_lock guard(m_queue_lock);
			m_queue.push(std::move(item));
		}
		m_condition.notify_one();
	}

	bool wait_for_items()
	{
		std::unique_lock guard(m_queue_lock);
		m_condition.wait(guard, [&]() { return !m_queue.empty() || m_done; });
		return m_done;
	}

	bool pop(T& item)
	{
		std::unique_lock guard(m_queue_lock);
		if(m_queue.empty()) return false;
		item = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	std::size_t size() const
	{
		std::unique_lock guard(m_queue_lock);
		return m_queue.size();
	}

	bool empty() const
	{
		std::unique_lock guard(m_queue_lock);
		return m_queue.empty();
	}

	void done()
	{
		{
			std::unique_lock guard(m_queue_lock);
			m_done = true;
		}
		m_condition.notify_all();
	}

	bool is_done()
	{
		std::unique_lock guard(m_queue_lock);
		return m_done;
	}

private:
	using queue_t = std::queue<T>;
	queue_t m_queue;
	mutable std::mutex m_queue_lock;
	std::condition_variable m_condition;
	bool m_done = false;
};

#endif