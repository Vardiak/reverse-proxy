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

    EventTimer::EventTimer(EventWatcher *watcher, float time, float repeat,
                           std::function<bool()> callback)
        : callback_(callback)
        , watcher_(watcher)
    {
        ev_timer_init(&timer_, EventTimer::event_callback, time, repeat);
        timer_.data = reinterpret_cast<void *>(this);
    }

    void EventTimer::event_callback(EV_P_ ev_timer *w, int)
    {
        auto et = reinterpret_cast<EventTimer *>(w->data);
        auto shared_et = event_register.at(et).value();
        if (shared_et->callback_())
            event_register.unregister_ew(shared_et->watcher_);
    }

    std::shared_ptr<EventTimer>
    EventTimer::start(EventWatcher *watcher, float time, float repeat,
                      std::function<bool()> callback)
    {
        return event_register.register_timer<EventWatcher *, float, float,
                                             std::function<bool()>>(
            std::move(watcher), std::move(time), std::move(repeat),
            std::move(callback));
    }
} // namespace http
