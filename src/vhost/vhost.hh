/**
 * \file vhost/vhost.hh
 * \brief VHost declaration.
 */

#pragma once

#include <openssl/ssl.h>

#include "config/config.hh"
#include "error/not-implemented.hh"
#include "misc/addrinfo/addrinfo.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "socket/default-socket.hh"

namespace http
{
    struct Connection;

    /**
     * \class VHost
     * \brief Abstract class representing a VHost.
     */
    class VHost
    {
    public:
        /**
         * \brief Create a VHost from its configuration.
         *
         * \param conf VHostConfig virtual host configuration.
         */
        explicit VHost(const VHostConfig &);

        VHost() = delete;
        VHost(const VHost &) = delete;
        VHost &operator=(const VHost &) = delete;
        VHost(VHost &&) = delete;
        VHost &operator=(VHost &&) = delete;
        virtual ~VHost() = default;

        /**
         * \brief Send response depending on the VHost type.
         *
         * \param req Request.
         * \param conn Connection.
         */
        virtual void respond(shared_req, std::shared_ptr<Connection>) = 0;

        inline const VHostConfig &conf_get() const noexcept
        {
            return conf_;
        }

        shared_socket create_and_bind(const misc::AddrInfo &addrinfos,
                                      bool ssl);
        shared_socket prepare_socket(bool ssl);
        static int sni_callback(SSL *ssl, int *, void *arg);

        bool check_auth(shared_req, std::shared_ptr<Connection>, bool);
        static std::string base64_decode(const std::string &in);

    protected:
        /**
         *  \brief VHost configuration.
         */
        VHostConfig conf_;

        /**
         * \brief VHost's SSL context.
         *
         * From ssl(3):
         *
         * SSL_CTX (SSL Context)
         *   This is the global context structure which is created by a server
         *   or client once per program life-time and which holds mainly default
         *   values for the SSL structures which are later created for the
         *   connections.
         *
         * Warning: with this unique_ptr syntax, you'll need to instanciate the
         * pointer with both a value and a Deleter function.
         */
        std::unique_ptr<SSL_CTX, decltype(SSL_CTX_free) *> ssl_ctx_{
            nullptr, SSL_CTX_free
        };

        static bool openssl_loaded;
    };

    using shared_vhost = std::shared_ptr<VHost>;
} // namespace http
