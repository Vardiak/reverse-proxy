#include "events/event-loop.hh"

#include <iostream>

namespace http
{
    EventLoop::EventLoop()
        : loop(ev_default_loop(0))
        , run(true)
    {}

    EventLoop::EventLoop(struct ev_loop *l)
    {
        loop = l;
    }

    EventLoop::~EventLoop()
    {
        ev_loop_destroy(loop);
    }

    void EventLoop::register_watcher(EventWatcher *watcher)
    {
        ev_io_start(loop, &watcher->watcher_get());
    }

    void EventLoop::unregister_watcher(EventWatcher *watcher)
    {
        ev_io_stop(loop, &watcher->watcher_get());
    }

    void EventLoop::register_sigint_watcher(ev_signal *signal) const
    {
        ev_signal_start(loop, signal);
    }

    void signal_handler(struct ev_loop *l, ev_signal *, int)
    {
        std::cout << "Gracefully shutdowned..." << std::endl;
        ev_break(l, EVBREAK_ALL);
    }

    void EventLoop::operator()() const
    {
        ev_signal signal_watcher;

        ev_signal_init(&signal_watcher, signal_handler, SIGINT);
        register_sigint_watcher(&signal_watcher);

        ev_run(loop, 0);
    }
} // namespace http
