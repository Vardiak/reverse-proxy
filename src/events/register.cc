#include "events/register.hh"

#include "events/event-loop.hh"

namespace http
{
    EventWatcherRegistry event_register;

    std::optional<std::shared_ptr<EventWatcher>>
    EventWatcherRegistry::at(EventWatcher *ev)
    {
        if (events_.find(ev) == events_.end())
            return std::nullopt;
        return events_[ev];
    }

    bool EventWatcherRegistry::unregister_ew(EventWatcher *ew)
    {
        loop_.unregister_watcher(ew);
        return events_.erase(ew) != 0;
    }

    std::optional<std::shared_ptr<EventTimer>>
    EventWatcherRegistry::at(EventTimer *ev)
    {
        if (event_timers_.find(ev) == event_timers_.end())
            return std::nullopt;
        return event_timers_[ev];
    }

    bool EventWatcherRegistry::unregister_et(EventTimer *et)
    {
        loop_.unregister_timer(et);
        return event_timers_.erase(et) != 0;
    }

} // namespace http
