// vim: ts=4 sw=4 et ai
#include "server/fcgi-threaded.hpp"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#include "fcgi-server.hpp"


namespace fcgi {
namespace threaded {
namespace {
class Semaphore {
public:
    Semaphore()
        : count(0) {}

    void
    add()
    {
        std::unique_lock<std::mutex> lock(mutex);
        ++count;
        condition.notify_one();
    }

    void
    acquire()
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition.wait(lock, [&]() { return count != 0; });
        --count;
    }

private:
  std::mutex mutex;
  std::condition_variable condition;
  std::atomic<int> count;
};

class SocketQueue {
public:
    SocketQueue()
        : spin(ATOMIC_FLAG_INIT) {}

    int
    get()
    {
        semaphore.acquire();
        lock();
        int fd = fds.front();
        fds.pop_front();
        unlock();

        return fd;
    }

    void
    set(int fd)
    {
        lock();
        fds.push_back(fd);
        unlock();
        semaphore.add();
    }

private:
    Semaphore semaphore;
    std::atomic_flag spin;
    std::list<int> fds;

    void
    lock()
    { while (spin.test_and_set(std::memory_order_acquire)); }

    void
    unlock()
    { spin.clear(std::memory_order_release); }
};

void
clientLoop(SocketQueue* queue, MasterServer* server)
{
    do {
        int sock = queue->get();
        server->handleInboundSocket(sock);
    } while (true);
}
} // ns


void
eventLoop(MasterServer* master, ServerConfig config, int socket)
{
    ::signal(SIGPIPE, SIG_IGN);

    std::vector<std::thread> threads;
    SocketQueue queue;
    for (size_t i = 0; i < config.childCount; ++i) {
        threads.emplace_back(
            std::thread(std::bind(clientLoop, &queue, master)));
    }

    do {
        struct sockaddr_in address;
        socklen_t          address_len;
        int client = accept4(socket, (struct sockaddr*)&address,
                             &address_len, SOCK_CLOEXEC);
        if (client < 0) {
            continue;
        } else {
            queue.set(client);
        }
    } while (true);
}

} // ns fcgi
} // ns fcgi
