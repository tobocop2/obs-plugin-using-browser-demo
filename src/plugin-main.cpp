/*
Plugin Name
Copyright (C) <Year> <Developer> <Email Address>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program. If not, see <https://www.gnu.org/licenses/>
*/

#include <filesystem>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <stdexcept>
#include <thread>
#include <time.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
    #include <winsock.h>
    #include <windows.h>
#elif __APPLE__ || __linux__ || __unix__
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#else
#   error "Unknown compiler"
#endif

#include <obs-module.h>

// uwebsockets
#include <App.h>
#include <nlohmann/json.hpp>

#include "plugin-macros.generated.h"

#include "obs-browser.hpp"
#include "DemoObject.hpp"
#include "uuid.hpp"
#include "date.h"

uWS::App* globalApp;

namespace protocol {
    struct Request {
        std::string requestId;
        std::string eventId;
        nlohmann::json eventArgs;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Request, requestId, eventId, eventArgs)
    };

    struct Response {
        std::string responseRequestId;
        std::string eventId;
        nlohmann::json eventArgs;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Response, responseRequestId, eventId, eventArgs)
    };
}


OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(PLUGIN_NAME, "en-US")


// adapted from https://stackoverflow.com/a/20850182
uint16_t getFreePort() {
    struct sockaddr_in serv_addr;
    bzero(reinterpret_cast<char *>(&serv_addr), sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = 0;

    uint16_t port = 0;
    for (int i=0; i < 1000; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            std::cout << "socket error" << std::endl;
            continue;
        }
        std::cout << "opened socket fd: " << sock << std::endl;
        if (bind(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
            if (errno == EADDRINUSE) {
                std::cout
                    << "the port is not available. already to other process"
                    << std::endl;
            } else {
                std::cout
                    << "could not bind to process "
                    << errno
                    << strerror(errno)
                    << std::endl;
            }
        }
        socklen_t len = sizeof(serv_addr);
        if (getsockname(sock, (struct sockaddr *)&serv_addr, &len) == -1) {
            std::cerr << "getsockname" << std::endl;
            continue;
        }

        port = ntohs(serv_addr.sin_port);
        std::cout << "port number " << port << std::endl;

        if (close(sock) < 0) {
            std::cout << "did not close: " <<  strerror(errno) << std::endl;
            continue;
        }
        break;
    }
    return port;
}

void createServer(const uint16_t port) {
    /* ws->getUserData returns one of these */
    struct PerSocketData {
      /* Fill with user data */
    };

    /* Keep in mind that uWS::SSLApp({options}) is the same as uWS::App() when compiled without SSL
     * support. You may swap to using uWS:App() if you don't need SSL */
    uWS::App app = uWS::App()
        .ws<PerSocketData>(
            "/*",
            {
                .compression = uWS::SHARED_COMPRESSOR,
                .maxPayloadLength = 16 * 1024 * 1024,
                .idleTimeout = 16,
                .maxBackpressure = 1 * 1024 * 1024,
                .closeOnBackpressureLimit = false,
                .resetIdleTimeoutOnSend = false,
                .sendPingsAutomatically = true,
                /* Handlers */
                .upgrade = nullptr,
                .open = [](auto* ws) {
                    /* Open event here, you may access ws->getUserData() which
                     * points to a PerSocketData struct */
                    ws->subscribe("api");
                    },
                .message =
                    [](auto* ws, std::string_view message, uWS::OpCode opCode) {
                        nlohmann::json jsonRes;
                        try {
                            jsonRes = nlohmann::json::parse(message);
                            protocol::Response res = jsonRes;
                            if (res.eventId == "apiCallResponse") {
                                if (!res.eventArgs.empty()) {
                                    std::cout << "result: " << res.eventArgs.dump() << std::endl;
                                }
                            }
                        } catch(...) {
                            std::cerr << "Error parsing json: " << message << std::endl;
                        }
                    },
                .drain =
                    [](auto* ws) {
                        std::cout
                            << "buffered amount:"
                            << ws->getBufferedAmount()
                            << "\n";
                    },
                .ping =
                    [](auto* /*ws*/, std::string_view) {
                        /* Not implemented yet */
                    },
                .pong =
                    [](auto* /*ws*/, std::string_view) {
                        /* Not implemented yet */
                    },
                .close =
                    [](auto*, int, std::string_view) {
                        /* You may access ws->getUserData() here */
                    }
            }
        )
        .listen(port,
                [&](auto* listen_socket) {
                  if (listen_socket) {
                    std::cout << "Listening on port " << port << std::endl;
                  }
                }
        );

      struct us_loop_t* loop = (struct us_loop_t*)uWS::Loop::get();
      struct us_timer_t* delayTimer = us_create_timer(loop, 0, 0);

      // broadcast the unix time as millis every 8 millis
      us_timer_set(
          delayTimer,
          [](struct us_timer_t* /*t*/) {
              std::ostringstream oss;
              using namespace date;
              oss << std::chrono::system_clock::now();

              nlohmann::json req = protocol::Request {
                  .requestId = uuid::generate_uuid_v4(),
                  .eventId = "callApiMethod",
                  .eventArgs = nlohmann::json::array({"convertToEpoch", { oss.str() } })
              };

              globalApp->publish("api", req.dump(), uWS::OpCode::TEXT, false);


          },
          8, 8);

    globalApp = &app;
    app.run();
}

// https://ideone.com/jr7M5a
template<typename ... Args>
std::string string_format(const std::string& format, Args ... args) {
    // Extra space for '\0'
    size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1;
    if (size <= 0) {
        throw std::runtime_error("Error during formatting.");
    }

    std::unique_ptr<char[]> buf(new char[ size ]);
    snprintf(buf.get(), size, format.c_str(), args...);

    // We don't want the '\0' inside
    return std::string( buf.get(), buf.get() + size - 1 );
}


bool obs_module_load(void) {
    blog(LOG_INFO, "plugin loaded successfully (version %s)",
         PLUGIN_VERSION);
    blog(LOG_INFO, "obs browser version: %d", obs::browser::version());

    const auto port = getFreePort();

    if (port <= 0) {
        throw std::runtime_error("Unable to assign port");
    }

    const std::filesystem::path currentFile {__FILE__};
    const std::string dirname = currentFile.parent_path();
    std::ifstream t(dirname + "/client.js");
    std::stringstream buffer;
    buffer << t.rdbuf();
    const std::string startupJs = string_format(buffer.str(), port);

    std::thread server(createServer, port);
    demo::DemoObject::initialize(startupJs);

    server.detach();

    return true;
}

void obs_module_unload()
{
    blog(LOG_INFO, "plugin unloaded");
    demo::DemoObject::finalize();
}
