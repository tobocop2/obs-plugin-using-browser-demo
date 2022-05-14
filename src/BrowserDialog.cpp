#include <stdexcept>
#include <string>

#include <obs-frontend-api.h>

#include "plugin-macros.generated.h"
#include "BrowserDialog.hpp"

const char* url = "https://tobias.perelste.in";

demo::BrowserDialog::~BrowserDialog() {}

demo::BrowserDialog::BrowserDialog(const std::string &startupJs) : QDialog(reinterpret_cast<QWidget*>(obs_frontend_get_main_window()))
{
    setMinimumSize(640, 480);
    {
        _layout = new QVBoxLayout();
        _layout->setContentsMargins(0, 0, 0, 0);
        _layout->setSpacing(0);
        setLayout(_layout);
    }

    _cef = obs::browser::instance();
    if (!_cef) {
        throw std::runtime_error(std::string(PLUGIN_NAME) + ": Failed to load obs-browser module.");
    }

    _cef_widget = _cef->create_widget(this, url);
    _cef_widget->setStartupScript(startupJs);
    _layout->addWidget(_cef_widget);
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
}

void demo::BrowserDialog::show()
{
    _cef_widget->setURL(url);
    static_cast<QWidget*>(this)->show();
}

void demo::BrowserDialog::closeEvent(QCloseEvent*)
{
    emit cancelled();
}

