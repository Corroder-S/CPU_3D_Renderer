#pragma once
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

class WorkerPool
{
public:
    explicit WorkerPool(int numThreads = std::max(1, (int)std::thread::hardware_concurrency() - 1))
    {
        for (int i = 0; i < numThreads; i++)
            m_threads.emplace_back([this] { loop(); });
    }

    ~WorkerPool()
    {
        { std::lock_guard lk(m_mutex); m_stop = true; }
        m_wake.notify_all();
        for (auto& t : m_threads) t.join();
    }

    void parallelFor(uint32_t count, const std::function<void(uint32_t)>& fn)
    {
        {
            std::lock_guard lk(m_mutex);
            m_fn = &fn;
            m_count = count;
            m_next = 0;
            m_busy = (int)m_threads.size();
            ++m_generation;
        }
        m_wake.notify_all();

        run(); // le main thread travaille aussi

        std::unique_lock lk(m_mutex);
        m_done.wait(lk, [&] { return m_busy == 0; });
    }

private:
    void run()
    {
        uint32_t i;
        while ((i = m_next.fetch_add(1)) < m_count)
            (*m_fn)(i);
    }

    void loop()
    {
        uint64_t seen = 0;
        while (true)
        {
            {
                std::unique_lock lk(m_mutex);
                m_wake.wait(lk, [&] { return m_stop || m_generation != seen; });
                if (m_stop) return;
                seen = m_generation;
            }
            run();
            {
                std::lock_guard lk(m_mutex);
                if (--m_busy == 0) m_done.notify_one();
            }
        }
    }

    std::vector<std::thread> m_threads;
    std::mutex m_mutex;
    std::condition_variable m_wake, m_done;
    const std::function<void(uint32_t)>* m_fn = nullptr;
    uint32_t m_count = 0;
    std::atomic<uint32_t> m_next{ 0 };
    int m_busy = 0;
    uint64_t m_generation = 0;
    bool m_stop = false;
};