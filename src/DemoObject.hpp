#pragma once

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

            DemoObject();

        private:
            static void obs_event_handler(obs_frontend_event event, void* private_data);

            void load();

            void unload();

        private:

            static std::shared_ptr<demo::DemoObject> _instance;

        public:
            static void initialize();

            static void finalize();

            static std::shared_ptr<demo::DemoObject> instance();
    };
} 
