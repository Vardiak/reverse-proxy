#include "ssl-socket.hh"

#include "misc/openssl/ssl.hh"

namespace http
{
    SSLSocket::SSLSocket(int domain, int type, int protocol, SSL_CTX *ssl_ctx)
        : Socket{ std::make_shared<misc::FileDescriptor>(
            sys::socket(domain, type, protocol)) }
        , ssl_(SSL_new(ssl_ctx), SSL_free)
    {
        // ???
    }

    SSLSocket::SSLSocket(const misc::shared_fd &fd, SSL_CTX *ssl_ctx)
        : Socket(fd)
        , ssl_(SSL_new(ssl_ctx), SSL_free)
    {
        // ???
    }

    ssize_t SSLSocket::recv(void *buffer, size_t size)
    {
        return ssl::read(ssl_.get(), buffer, size);
    }

    ssize_t SSLSocket::send(const void *buffer, size_t size)
    {
        return ssl::write(ssl_.get(), buffer, size);
    }

    ssize_t SSLSocket::sendfile(misc::shared_fd &file, off_t &offset,
                                size_t size)
    {
        char *buffer = new char[size];

        sys::lseek(file->fd_, offset, SEEK_SET);
        size = sys::read(file->fd_, buffer, size);

        size = ssl::write(ssl_.get(), buffer, size);

        offset += size;
        delete buffer;
        return size;
    }

    void SSLSocket::bind(const sockaddr *addr, socklen_t addrlen)
    {
        sys::bind(fd_->fd_, addr, addrlen);
    }

    void SSLSocket::listen(int backlog)
    {
        sys::listen(fd_->fd_, backlog);
    }

    void SSLSocket::setsockopt(int level, int optname, int optval)
    {
        sys::setsockopt(fd_->fd_, level, optname, &optval, sizeof(int));
    }

    void SSLSocket::getsockopt(int, int, int &)
    {
        /* FIXME */
    }

    shared_socket SSLSocket::accept(sockaddr *addr, socklen_t *len)
    {
        auto cfd = std::make_shared<misc::FileDescriptor>(
            sys::accept(fd_->fd_, addr, len));

        auto ssl_ctx = SSL_get_SSL_CTX(ssl_.get());

        auto res = std::make_shared<SSLSocket>(cfd, ssl_ctx);

        SSL_set_fd(res->ssl_.get(), cfd->fd_);

        ssl::accept(res->ssl_.get());

        return res;
    }

    void SSLSocket::connect(const sockaddr *, socklen_t)
    {
        /* FIXME */
    }
} // namespace http
