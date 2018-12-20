//
// Created by lamar on 19.12.18.
//

#ifndef CYCLICQUEUE_QUEUE_H
#define CYCLICQUEUE_QUEUE_H


#include <mutex>
#include <atomic>
#include <vector>
#include <thread>
#include <cassert>
#include <iostream>
#include <shared_mutex>


class Tmp {
    int *l{nullptr};
public:
    Tmp(int k=0) {
        l = new int(k);
    }
    ~Tmp() {
        delete l;
    }
    Tmp(const Tmp&) = delete;
    Tmp(Tmp &&arg) noexcept {
        l = arg.l;
        arg.l = nullptr;
    }
    Tmp& operator=(Tmp &&arg) noexcept {

        if (this != &arg) {
            delete l;
            l = arg.l;
            arg.l = nullptr;
        }
        return *this;
    }
    int get() const {
        assert(l != nullptr);
        return *l;
    }
};

std::ostream& operator<<(std::ostream& s, const Tmp& arg);

class Queue {
private:
    std::size_t read{0};
    std::size_t written{0};

    std::vector<Tmp> container;
    std::size_t size;

    std::mutex m;

    std::condition_variable_any signal_read;
    std::condition_variable_any signal_write;
    std::shared_timed_mutex signal_mutex_read;
    std::shared_timed_mutex signal_mutex_write;

    bool can_write() const {
        auto readM = static_cast<size_t>(read % size);
        auto writtenM = static_cast<size_t>(written % size);
        return readM > writtenM || (readM == writtenM && written == read) || (readM < writtenM && written > read);
    }
public:
    Queue(std::size_t size):size(size) {
        container.resize(size);
    }

    void push(Tmp item) {
        std::unique_lock<std::mutex> l(m);
        if (this->can_write()) {
            container[written++ % size] = std::move(item);
            signal_read.notify_one();
            return;
        }

        l.unlock();
        std::shared_lock<std::shared_timed_mutex> s(signal_mutex_write);

        while (true) {
            signal_write.wait(s);

            l.lock();
            if (this->can_write()) {
                container[written++ % size] = std::move(item);
                signal_read.notify_one();
                return;
            }
            l.unlock();
        }
    }
    Tmp pull() {
        std::unique_lock<std::mutex> l(m);
        if (read < written) {
            signal_write.notify_one();
            return std::move(container[read++ % size]);
        }

        l.unlock();

        std::shared_lock<std::shared_timed_mutex> s(signal_mutex_read);

        while (true) {
            signal_read.wait(s);

            l.lock();
            if (read < written) {
                signal_write.notify_one();
                return std::move(container[read++ % size]);
            }
            l.unlock();
        }

    }
};

#endif //CYCLICQUEUE_QUEUE_H
