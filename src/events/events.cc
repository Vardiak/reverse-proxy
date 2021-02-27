#include "events/events.hh"

#include <iostream>

#include "events/register.hh"

namespace http
{
    EventWatcher::EventWatcher(int fd, int flags)
    {
        ev_io_init(&watcher_, EventWatcher::event_callback, fd, flags);
        watcher_.data = reinterpret_cast<void *>(this);
    }

    void EventWatcher::event_callback(struct ev_loop *, ev_io *w, int)
    {
        auto ew = reinterpret_cast<EventWatcher *>(w->data);
        auto shared_ew = event_register.at(ew).value();
        std::cout << "before\n";
        (*shared_ew)();
        std::cout << "after\n\n";
    }
} // namespace http
