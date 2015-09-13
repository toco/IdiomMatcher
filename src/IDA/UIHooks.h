//
// Created by Tobias Conradi on 01.07.15.
// Licensed under MIT License, see LICENSE for full text.

#ifndef IDIOMMATCHER_UIHOOKS_H
#define IDIOMMATCHER_UIHOOKS_H

#include <ida.hpp>
#include <kernwin.hpp>

namespace IDAAPI {
    class UIHooks {
        static int ui_notification_callback(void *, int, va_list);

    public:
        UIHooks();
        virtual ~UIHooks();

        virtual void populating_tform_popup(TForm *form, TPopupMenu *menu) { };
        virtual void finish_populating_tform_popup(TForm *form, TPopupMenu *menu) { };
    };
}

#endif //IDIOMMATCHER_UIHOOKS_H
