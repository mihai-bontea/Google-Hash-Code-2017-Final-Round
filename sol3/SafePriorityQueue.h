#pragma once

#include <set>
#include <mutex>
#include <queue>

#include "Definitions.h"

class ThreadSafePriorityQueue{
public:
    using triple = std::pair<int, Point>;

    struct cmp{
        bool operator()(const triple& a, const triple& b) const
        {
            return a.first < b.first; // max-heap
        }
    };

    void push(const triple& t)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (seen.insert(t).second)
        {
            pq.push(t);
        }
    }

    bool pop(triple& out)
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!pq.empty())
        {
            out = pq.top(); pq.pop();
            return true;
        }
        return false;
    }

    bool empty()
    {
        std::lock_guard<std::mutex> lock(mtx);
        return pq.empty();
    }

private:
    std::priority_queue<triple, std::vector<triple>, cmp> pq;
    std::set<triple> seen;
    std::mutex mtx;
};