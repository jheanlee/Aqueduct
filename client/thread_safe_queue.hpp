//
// Created by Jhean Lee on 2024/12/9.
//

#ifndef TUNNEL_THREAD_SAFE_QUEUE_HPP
  #define TUNNEL_THREAD_SAFE_QUEUE_HPP

  #include <queue>
  #include <mutex>
  #include <condition_variable>


  template<typename T>
  class ThreadSafeQueue {
  private:
    std::queue<T> queue;
    std::mutex queue_mutex;
    std::condition_variable cv;
  public:
    void push(T value) {
      std::lock_guard<std::mutex> lock(queue_mutex);
      queue.push(value);
      cv.notify_one();
    }

    void pop() {
      std::unique_lock<std::mutex> lock(queue_mutex);
      cv.wait(lock, [this]() { return !queue.empty(); });
      queue.pop();
    }

    T pop_rtn() {
      std::unique_lock<std::mutex> lock(queue_mutex);
      cv.wait(lock, [this]() { return !queue.empty(); });
      T value = queue.front();
      queue.pop();
      return value;
    }

    T front() {
      std::unique_lock<std::mutex> lock(queue_mutex);
      cv.wait(lock, [this]() { return !queue.empty(); });
      T value = queue.front();
      return value;
    }


    bool empty() {
      std::lock_guard<std::mutex> lock(queue_mutex);
      return queue.empty();
    }
  };

#endif //TUNNEL_THREAD_SAFE_QUEUE_HPP
