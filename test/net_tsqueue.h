#ifndef __THREAD_SAFE_QUEUE_H__
#define __THREAD_SAFE__QUEUE_H__

#include "net_common.h"

namespace net
{
    template<typename T>
    class TSQueue
    {
    public :
        TSQueue() = default; // default constructor
        TSQueue(const tsqueue<T>&) = delete; // do not create copying constructor, becuase it has mutex in it
        virtual ~tsqueue() { clear(); } // desctructor that clear queue

        /* add guarding rules to the standard functions provided to dequeue */
        const T& front()
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            return deqQueue.front();
        }
        const T& back()
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            return deqQueue.back();
        }
        void push_front(const T& item)
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            deqQueue.emplace_front(std::move(item));
        }
        void push_back(const T& item)
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            deqQueue.emplace_back(std::move(item));
        }

        void empty()
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            return deqQueue.empty();
        }

        size_t count()
        {
            std::scope_lock lock(muxQueue);
            return deqQueue.size();
        }
        void clear()
        {
            std::scope_lock lock(muxQueue);
            deqQueue.clear();
        }


        T pop_front()
        {
            std::scope_lock lock(muxQueue);
            auto t = std::move(deqQueue.front()); // cache the item
            deqQueue.pop_front(); // remove the item
            return t; // return the item
        }
        T pop_back()
        {
            std::scope_lock lock(muxQueue);
            auto t = std::move(deqQueue.front()); // cache the item
            deqQueue.pop_back(); // remove the item
            return t; // return the item
        }
    protected:
        std::mutex muxQueue;
        std::deque<T> deqQueue;
    }
}

#endif
