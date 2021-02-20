#include "socket/default-socket.hh"

#include "misc/socket.hh"

namespace http
{
    DefaultSocket::DefaultSocket(int domain, int type, int protocol)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
    {}

    DefaultSocket::DefaultSocket(const misc::shared_fd &sfd)
        : Socket(sfd)
    {}

    ssize_t DefaultSocket::recv(void *, size_t)
    {
        /* FIXME */
        return -1;
    }

    ssize_t DefaultSocket::send(const void *, size_t)
    {
        /* FIXME */
        return -1;
    }

    ssize_t DefaultSocket::sendfile(misc::shared_fd &, off_t &, size_t)
    {
        /* FIXME */
        return -1;
    }

    void DefaultSocket::bind(const sockaddr *addr, socklen_t addrlen)
    {
        sys::bind(fd_->fd_, addr, addrlen);
    }

    void DefaultSocket::listen(int backlog)
    {
        sys::listen(fd_->fd_, backlog);
    }

    void DefaultSocket::setsockopt(int level, int optname, int optval)
    {
        sys::setsockopt(fd_->fd_, level, optname, &optval, sizeof(int));
    }

    void DefaultSocket::getsockopt(int, int, int &)
    {
        /* FIXME */
    }

    shared_socket DefaultSocket::accept(sockaddr *addr, socklen_t *len)
    {
        auto cfd = std::make_shared<misc::FileDescriptor>(
            sys::accept(fd_->fd_, addr, len));
        return std::make_shared<DefaultSocket>(cfd);
    }

    void DefaultSocket::connect(const sockaddr *, socklen_t)
    {
        /* FIXME */
    }

} // namespace http
