#pragma once

#include "events/register.hh"

namespace http
{
    template <typename EventWatcher, typename... Args>
    std::shared_ptr<EventWatcher>
    EventWatcherRegistry::register_event(Args &&...args)
    {
        auto ew = std::make_shared<EventWatcher>(std::forward<Args>(args)...);

        events_[ew.get()] = ew;
        loop_.register_watcher(ew.get());

        return ew;
    }

    template <typename... Args>
    std::shared_ptr<EventTimer>
    EventWatcherRegistry::register_timer(Args &&...args)
    {
        auto et = std::make_shared<EventTimer>(std::forward<Args>(args)...);

        event_timers_[et.get()] = et;
        loop_.register_timer(et.get());

        return et;
    }
} // namespace http
