#ifndef QUEUE_H
#define QUEUE_H
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>


/**
 *  Queue : A thread safe Queue
 */
namespace Utils
{
    template <typename Item>
    class Queue
    {
    public:
        Queue()
        {

        }

        Queue( Queue const& other)
        {
            std::lock_guard<std::mutex>locker(other.itemMutex);
            items = other.items;
        }

        Queue(Queue&& other)
        {
            std::lock_guard<std::mutex>locker(other.itemMutex);
            items = std::move(other.items);
            other.clear();
        }

        Queue& operator =(Queue&& other)
        {
            std::lock(itemMutex, other.itemMutex);
            std::lock_guard<std::mutex> this_lock(itemMutex,std::adopt_lock);
            std::lock_guard<std::mutex> other_lock(other.itemMutex,std::adopt_lock);
            items = std::move(other.items);
            other.clear();
            return *this;

        }

        Queue& operator =(Queue& other)
        {
            std::lock(itemMutex, other.itemMutex);
            std::lock_guard<std::mutex> this_lock(itemMutex,std::adopt_lock);
            std::lock_guard<std::mutex> other_lock(other.itemMutex,std::adopt_lock);
            items = std::move(other.items);
            return *this;

        }

        void push(Item item)
        {
            std::lock_guard<std::mutex>locker(itemMutex);
            items.push(item);
            itemCond.notify_one();
        }
        // wait until the queue is not empty
        // the poped Item is put the argument item.
        void wait_and_pop(Item& item)
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            item = items.front();
            items.pop();
        }
        // wait until the queue is not empty
        // the poped Item is return as a pointer.
        std::shared_ptr<Item> wait_and_pop()
        {
            std::unique_lock<std::mutex> locker(itemMutex);
            itemCond.wait(locker,[this]{return !items.empty();});
            std::shared_ptr<Item> result (std::make_shared<Item> (items.front()));
            items.pop();
            return result;
        }

        bool try_pop(Item& item)
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return false;
            item = items.front();
            items.pop();
            return true;
        }

        std::shared_ptr<Item> try_pop()
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            if(items.empty())
                return std::shared_ptr<Item>();
            std::shared_ptr<Item> result (std::make_shared<Item> (items.front()));
            items.pop();
            return result;
        }

        bool empty()const
        {
            std::lock_guard<std::mutex> locker(itemMutex);
            return items.empty();
        }

        int size()const
        {
            return items.size();
        }

        void clear()
        {

            items = std::queue<Item>();

        }
        std::queue<Item> getContainer()
        {
            return items;
        }

    private:
        std::queue<Item> items;
        mutable std::mutex itemMutex;
        std::condition_variable itemCond;
    };
}

#endif // QUEUE_H
