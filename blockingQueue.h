//
// Taken from module 3 helper resources
//

#ifndef NETWORKSYSTEMS_BLOCKINGQUEUE_H
#define NETWORKSYSTEMS_BLOCKINGQUEUE_H

#include <queue>
#include <thread>
#include <list>
#include <iostream>
#include <mutex>
#include <condition_variable>


template<class T>
class BlockingQueue
{

private:
    std::queue<T> theQueue;
    std::mutex mutex_queue;
    std::condition_variable cond;

public:
    BlockingQueue<T>() {}

    T pop() {
        std::unique_lock<std::mutex> lk(mutex_queue);
        if ( theQueue.empty() ) {
            cond.wait(lk);
        }
        T ret = theQueue.front();
        theQueue.pop();
        lk.unlock();
        return ret;
    }

    void push(T object) {
        mutex_queue.lock();
        theQueue.push(object);
        mutex_queue.unlock();
        cond.notify_one();
    }

};

#endif //NETWORKSYSTEMS_BLOCKINGQUEUE_H
