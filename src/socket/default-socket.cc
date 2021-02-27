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

    ssize_t DefaultSocket::recv(void *buffer, size_t size)
    {
        return sys::recv(fd_->fd_, buffer, size, MSG_DONTWAIT);
    }

    ssize_t DefaultSocket::send(const void *buffer, size_t size)
    {
        return sys::send(fd_->fd_, buffer, size, 0);
    }

    ssize_t DefaultSocket::sendfile(misc::shared_fd &s, off_t &offset,
                                    size_t len)
    {
        return sys::sendfile(fd_->fd_, s->fd_, &offset, len);
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

		// Get current socket flags
		int flags = fcntl(cfd->fd_, F_GETFL);

        fcntl(cfd->fd_, F_SETFL, flags | O_NONBLOCK);
        return std::make_shared<DefaultSocket>(cfd);
    }

    void DefaultSocket::connect(const sockaddr *, socklen_t)
    {
        /* FIXME */
    }

} // namespace http
