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
        return events_.erase(ew) != 0;
    }

} // namespace http
