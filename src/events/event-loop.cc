#include "events/event-loop.hh"

namespace http
{
    EventLoop::EventLoop()
    {
        loop = ev_default_loop(0);
    }

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
} // namespace http
