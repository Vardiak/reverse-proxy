NULL =

EXTRA_DIST = autogen.sh
ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}
AM_MAKEFLAGS = --no-print-directory

AM_CPPFLAGS = \
              -include $(top_builddir)/config.h \
              -I${top_srcdir}/src \
              -I${top_srcdir}/lib

AM_CXXFLAGS = \
              $(PICKY_CXXFLAGS) \
              $(WARN_CXXFLAGS) \
              $(BOOST_CPPFLAGS) \
              -Werror=unused-parameter \
              -Werror=missing-field-initializers \
              -Werror=overloaded-virtual \
              -Wno-inline \
              $(NULL)

AM_LDFLAGS = \
             $(WARN_LDFLAGS) \
             $(BOOST_LDFLAGS) \
             -lev \
             -lssl \
             -lcrypto \
             $(NULL)

AM_LDFLAGS += \
              -Wno-unused-command-line-argument \
              $(NULL)

bin_PROGRAMS = spider
lib_LTLIBRARIES = libspider.la

SOURCES = \
          src/config/config.hh \
          src/config/config.cc \
          src/error/init-error.hh \
          src/error/not-implemented.hh \
          src/error/parsing-error.hh \
          src/events/event-loop.cc \
          src/events/event-loop.hh \
          src/events/events.cc \
          src/events/events.hh \
          src/events/listener.hh \
          src/events/listener.cc \
          src/events/recv-request.cc \
          src/events/recv-request.hh \
          src/events/recv-response.cc \
          src/events/recv-response.hh \
          src/events/register.cc \
          src/events/register.hh \
          src/events/register.hxx \
          src/events/send-response.cc \
          src/events/send-response.hh \
          src/events/send-request.cc \
          src/events/send-request.hh \
          src/misc/addrinfo/addrinfo-error.cc \
          src/misc/addrinfo/addrinfo-error.hh \
          src/misc/addrinfo/addrinfo-iterator.hh \
          src/misc/addrinfo/addrinfo.cc \
          src/misc/addrinfo/addrinfo.hh \
          src/misc/fd.cc \
          src/misc/fd.hh \
          src/misc/json.hh \
          src/misc/openssl/base64.cc \
          src/misc/openssl/base64.hh \
          src/misc/openssl/ssl-error.cc \
          src/misc/openssl/ssl-error.hh \
          src/misc/openssl/ssl-wrapper.hh \
          src/misc/openssl/ssl.hh \
          src/misc/socket.hh \
          src/misc/sys-wrapper.hh \
          src/misc/unistd.hh \
	  src/misc/readiness/readiness.cc \
	  src/misc/readiness/readiness.hh \
          src/request/error.cc \
          src/request/error.hh \
          src/request/message.cc \
          src/request/message.hh \
          src/request/request.cc \
          src/request/request.hh \
          src/request/response.cc \
          src/request/response.hh \
          src/request/types.hh \
          src/socket/default-socket.cc \
          src/socket/default-socket.hh \
          src/socket/socket.hh \
          src/socket/ssl-socket.cc \
          src/socket/ssl-socket.hh \
          src/vhost/connection.hh \
          src/vhost/dispatcher.cc \
          src/vhost/dispatcher.hh \
          src/vhost/upstream.cc \
          src/vhost/upstream.hh \
          src/vhost/vhost-factory.hh \
          src/vhost/vhost-factory.cc \
          src/vhost/vhost-reverse-proxy.hh \
          src/vhost/vhost-reverse-proxy.cc \
          src/vhost/vhost-static-file.hh \
          src/vhost/vhost-static-file.cc \
          src/vhost/vhost.hh \
          src/vhost/vhost.cc \
          $(NULL)


# Compile source files again to avoid silenting undefined references.
spider_SOURCES = \
                 $(SOURCES) \
                 src/main.cc \
                 $(NULL)

# https://www.gnu.org/software/automake/manual/html_node/Objects-created-both-with-libtool-and-without.html
spider_CXXFLAGS = $(AM_CXXFLAGS)

libspider_la_SOURCES = \
                       $(SOURCES) \
                       $(NULL)

if USING_CRITERION
 check_PROGRAMS = testsuite
 TESTS = $(check_PROGRAMS)

 testsuite_SOURCES = tests/unit/config.cc tests/unit/request.cc tests/unit/response.cc tests/unit/forwarded_transition.cc $(SOURCES)
 testsuite_CXXFLAGS = $(AM_CXXFLAGS) $(CRITERION_CFLAGS)
 testsuite_CPPFLAGS = $(AM_CPPFLAGS)
 testsuite_LDFLAGS = $(AM_LDFLAGS) $(CRITERION_LIBS)
endif
