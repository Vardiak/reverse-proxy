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
        (*shared_ew)();
    }

    EventTimer::EventTimer(float time, std::function<void()> callback)
        : callback_(callback)
    {
        ev_timer_init(&timer_, EventTimer::event_callback, time, 0.);
        timer_.data = reinterpret_cast<void *>(this);
    }

    void EventTimer::event_callback(EV_P_ ev_timer *w, int)
    {
        auto et = reinterpret_cast<EventTimer *>(w->data);
        auto shared_et = event_register.at(et).value();
        shared_et->callback_();
    }
} // namespace http
