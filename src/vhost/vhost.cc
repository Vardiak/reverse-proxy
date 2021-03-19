#include "vhost.hh"

#include <iostream>
#include <regex>

#include "dispatcher.hh"
#include "error/init-error.hh"
#include "error/request-error.hh"
#include "events/listener.hh"
#include "events/send-response.hh"
#include "misc/openssl/ssl.hh"
#include "socket/ssl-socket.hh"

namespace http
{
    bool VHost::openssl_loaded = false;

    VHost::VHost(const VHostConfig &config)
        : conf_(config)
    {
        if (!config.ssl_cert.empty())
        {
            if (!openssl_loaded)
            {
                SSL_load_error_strings();
                OpenSSL_add_ssl_algorithms();
                openssl_loaded = true;
            }

            ssl_ctx_ = std::unique_ptr<SSL_CTX, decltype(SSL_CTX_free) *>(
                SSL_CTX_new(TLS_method()), SSL_CTX_free);

            ssl::ctx_use_certificate_file("use_certificate", ssl_ctx_.get(),
                                          conf_.ssl_cert.c_str(),
                                          SSL_FILETYPE_PEM);

            ssl::ctx_use_PrivateKey_file("use_key", ssl_ctx_.get(),
                                         conf_.ssl_key.c_str(),
                                         SSL_FILETYPE_PEM);

            X509 *x509 = SSL_CTX_get0_certificate(ssl_ctx_.get());

            if (x509 == nullptr)
                throw InitializationError("Invalid certificate");

            ssl::x509_check_host("check_host", x509, conf_.server_name.c_str(),
                                 conf_.server_name.length(), 0, nullptr);

            ssl::ctx_check_private_key("private_key", ssl_ctx_.get());

            // Setup SNI here
            SSL_CTX_set_tlsext_servername_arg(ssl_ctx_.get(), this);
            SSL_CTX_set_tlsext_servername_callback(ssl_ctx_.get(),
                                                   sni_callback);
        }

        auto socket = prepare_socket(!config.ssl_cert.empty());

        std::string ip = config.ip;
        uint16_t port = config.port;

        event_register.register_event<ListenerEW, shared_socket, std::string,
                                      std::uint16_t>(
            std::move(socket), std::move(ip), std::move(port));
    }

    shared_socket VHost::create_and_bind(const misc::AddrInfo &addrinfos,
                                         bool ssl)
    {
        for (auto a : addrinfos)
        {
            try
            {
                shared_socket socket;
                if (ssl)
                {
                    socket = std::make_shared<SSLSocket>(
                        a.ai_family, a.ai_socktype, a.ai_protocol,
                        ssl_ctx_.get());
                }
                else
                {
                    socket = std::make_shared<DefaultSocket>(
                        a.ai_family, a.ai_socktype, a.ai_protocol);
                }

                fcntl(socket->fd_get()->fd_, F_SETFL, O_NONBLOCK);
                socket->setsockopt(SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 1);

                socket->bind(a.ai_addr, a.ai_addrlen);

                return socket;
            }
            catch (const std::system_error &e)
            {
                continue;
            }
        }

        throw std::runtime_error("Couldn't bind IP and port");
    }

    shared_socket VHost::prepare_socket(bool ssl)
    {
        auto hint = misc::AddrInfoHint()
                        .family(AF_UNSPEC)
                        .socktype(SOCK_STREAM)
                        .flags(AI_PASSIVE);

        auto addrinfos = misc::getaddrinfo(
            conf_.ip.c_str(), std::to_string(conf_.port).c_str(), hint);

        auto socket = create_and_bind(addrinfos, ssl);
        socket->listen(256);

        std::cout << "Listening on " << conf_.ip << ":" << conf_.port
                  << std::endl;

        return socket;
    }

    // int (*sni_calback)(SSL *s, int *al, void *arg)
    int VHost::sni_callback(SSL *ssl, int *, void *arg)
    {
        const char *server_name =
            SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);

        if (server_name == nullptr)
            return SSL_TLSEXT_ERR_NOACK;

        auto listener_vhost = static_cast<VHost *>(arg);

        auto vhost = dispatcher.find_vhost(
            server_name, listener_vhost->conf_.ip, listener_vhost->conf_.port);

        if (!vhost || vhost.value()->conf_.ssl_cert.empty())
            return SSL_TLSEXT_ERR_NOACK;

        SSL_set_SSL_CTX(ssl, vhost.value()->ssl_ctx_.get());

        return SSL_TLSEXT_ERR_OK;
    }

    std::string VHost::base64_decode(const std::string &in)
    {
        std::string out;

        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++)
            T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
                  [i]] = i;

        int val = 0, valb = -8;
        for (unsigned char c : in)
        {
            if (c == '=')
                break;
            if (T[c] == -1)
                return "";
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0)
            {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }

    bool VHost::check_auth(Request &req, std::shared_ptr<Connection> conn)
    {
        if (conf_.auth_basic.empty())
            return true;

        if (req.headers.count("Authorization") > 0)
        {
            std::smatch sm;
            const std::regex auth_regex("^([\\w]+) ([0-9a-zA-Z\\+/=]+)$");
            if (std::regex_search(req.headers["Authorization"], sm, auth_regex)
                && sm[1] == "Basic")
            {
                std::string credentials = base64_decode(sm[2]);

                if (!credentials.empty()
                    && conf_.auth_basic_users.count(credentials) != 0)
                    return true;
            }
        }

        auto res = std::make_shared<Response>(UNAUTHORIZED);

        res->headers["WWW-Authenticate"] =
            "Basic realm=\"" + conf_.auth_basic + "\"";

        event_register.register_event<SendResponseEW, shared_conn, shared_res>(
            std::move(conn), std::move(res));

        return false;
    }
} // namespace http
