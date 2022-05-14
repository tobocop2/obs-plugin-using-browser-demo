#pragma once

#include <string>

#include <QAction>
#include <QMenu>
#include <QSharedPointer>
#include <memory>

#include <obs-frontend-api.h>


namespace demo {
    class BrowserDialog;

    class DemoObject: public QObject {
        Q_OBJECT;

        private:

            demo::BrowserDialog* _browser;

        public:
            ~DemoObject();

            DemoObject(const std::string &startupJs);

        private:
            static void obs_event_handler(obs_frontend_event event, void* private_data);

            void load();

            void unload();

        private:

            static std::shared_ptr<demo::DemoObject> _instance;

            std::string _startupJs;

        public:
            static void initialize(const std::string &startupJs);

            static void finalize();

            static std::shared_ptr<demo::DemoObject> instance();
    };
} 
