/**
 * \file vhost/vhost-static-file.hh
 * \brief VHostStaticFile declaration.
 */

#pragma once

#include <filesystem>

#include "config/config.hh"
#include "request/request.hh"
#include "request/response.hh"
#include "vhost/connection.hh"
#include "vhost/vhost.hh"

namespace http
{
    /**
     * \class VHostStaticFile
     * \brief VHost serving static files.
     */
    class VHostStaticFile : public VHost
    {
    public:
        friend class VHostFactory;
        virtual ~VHostStaticFile() = default;

    private:
        /**
         * \brief Constructor called by the factory.
         *
         * \param config VHostConfig virtual host configuration.
         */
        explicit VHostStaticFile(const VHostConfig &config);

    public:
        /**
         * \brief Send response.
         *
         * \param req Request.
         * \param conn Connection.
         *
         * Note that these iterators will only be useful starting from SRPS.
         */
        void respond(shared_req, std::shared_ptr<Connection>) final;

        std::filesystem::path normalize_URI(std::string uri);
        std::string uri_decode(std::string &url);
        void send_auto_index(shared_req req, shared_conn conn,
                             std::filesystem::path path);
    };
} // namespace http
