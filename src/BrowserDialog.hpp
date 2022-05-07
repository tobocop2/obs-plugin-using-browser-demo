#pragma once

#include <QBoxLayout>
#include <QDialog>

#include "obs-browser.hpp"

namespace demo {
    class BrowserDialog: public QDialog {
        Q_OBJECT;

        private:
            QVBoxLayout* _layout;
            QCef*        _cef;
            QCefWidget*  _cef_widget;

        public:
            ~BrowserDialog();

            BrowserDialog();

            void show();

        protected:
            void closeEvent(QCloseEvent*) override;

        signals:
        ; 
            void cancelled();
    };
}
