#include <memory>

#include <QDesktopServices>
#include <QMainWindow>
#include <QMenuBar>
#include <QTranslator>

#include <obs-frontend-api.h>

#include "DemoObject.hpp"
#include "BrowserDialog.hpp"

demo::DemoObject::~DemoObject()
{
    obs_frontend_remove_event_callback(obs_event_handler, this);
}

demo::DemoObject::DemoObject() : _browser()
{
    obs_frontend_add_event_callback(obs_event_handler, this);
}

void demo::DemoObject::obs_event_handler(obs_frontend_event event, void* private_data)
{
    demo::DemoObject* ui = reinterpret_cast<demo::DemoObject*>(private_data);
    if (event == OBS_FRONTEND_EVENT_FINISHED_LOADING) ui->load();
    else if (event == OBS_FRONTEND_EVENT_EXIT) ui->unload();
}

void demo::DemoObject::load()
{
    _browser = new demo::BrowserDialog();
	_browser->show();
}

void demo::DemoObject::unload()
{
    _browser->deleteLater();
    _browser = nullptr;
}

std::shared_ptr<demo::DemoObject> demo::DemoObject::_instance = nullptr;

void demo::DemoObject::initialize()
{
    if (!demo::DemoObject::_instance)
        demo::DemoObject::_instance = std::make_shared<demo::DemoObject>();
}

void demo::DemoObject::finalize()
{
    demo::DemoObject::_instance.reset();
}

std::shared_ptr<demo::DemoObject> demo::DemoObject::instance()
{
    return demo::DemoObject::_instance;
}
