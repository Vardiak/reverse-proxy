#include "events/event-loop.hh"

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

    void EventLoop::operator()() const
    {
        while (run)
        {
            ev_loop(loop, 0);
        }
    }
} // namespace http
