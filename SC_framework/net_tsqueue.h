#ifndef __THREAD_SAFE_QUEUE_H__
#define __THREAD_SAFE_QUEUE_H__

#include "net_common.h"

namespace net
{
    template<typename T>
    class tsqueue
    {
    public :
        tsqueue() = default; // default constructor
        tsqueue(const tsqueue<T>&) = delete; // do not create copying constructor, becuase it has mutex in it
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

            // stop waiting for 'cvBlocking' condition variable
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }
        void push_back(const T& item)
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            deqQueue.emplace_back(std::move(item));

            // stop waiting for 'cvBlocking' condition variable
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.notify_one();
        }

        bool empty()
        {
            std::scoped_lock lock(muxQueue); // mutex lock
            return deqQueue.empty();
        }

        size_t count()
        {
            std::scoped_lock lock(muxQueue);
            return deqQueue.size();
        }
        void clear()
        {
            std::scoped_lock lock(muxQueue);
            deqQueue.clear();
        }


        T pop_front()
        {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(deqQueue.front()); // cache the item
            deqQueue.pop_front(); // remove the item
            return t; // return the item
        }
        T pop_back()
        {
            std::scoped_lock lock(muxQueue);
            auto t = std::move(deqQueue.front()); // cache the item
            deqQueue.pop_back(); // remove the item
            return t; // return the item
        }
        /*
        * when it's called, it suspends calling object until the new message is written to the queue
        */
        void wait()
        {
            /*
            * check whether the queue is empty or not with tight loop
            * if queue is empty, process that calls this function will be locked in the while loop below
            * when some other thread wrote a message in queue, the block will be released
            */
            while (empty())
            {
                std::unique_lock<std::mutex> ul(muxBlocking);
                cvBlocking.wait(ul); 
                /* 
                * thread wait here until something signals condition variable to wake up
                * 1) our cell can wake up this thread
                * 2) phenomena called "spurious wakeup" can wake up thread
                * this will save CPU usage
                */
            }
        }
    protected:
        std::mutex muxQueue;
        std::deque<T> deqQueue;

        std::condition_variable cvBlocking; // to make thread sleep
        std::mutex muxBlocking; // to make thread-safe 'cvBlocking'
    };
}

#endif
