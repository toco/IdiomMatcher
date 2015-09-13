//
// Created by Tobias Conradi on 01.07.15.
// Licensed under MIT License, see LICENSE for full text.

#include "UIHooks.h"
#include <loader.hpp>


namespace IDAAPI {
    UIHooks::UIHooks() {
        if (!hook_to_notification_point(HT_UI,UIHooks::ui_notification_callback,this)) {
            msg("IDAAPI::UIHooks:UIHooks() failed to hook UI notification.\n");
        }
    }

    UIHooks::~UIHooks() {
        if (!unhook_from_notification_point(HT_UI,UIHooks::ui_notification_callback,this)) {
            msg("IDAAPI::UIHooks::~UIHooks() failed to unhook UI notification.\n");
        }
    }

    int UIHooks::ui_notification_callback(void *user_data, int notif_code, va_list va) {
        UIHooks *self = (UIHooks*)user_data;

        switch (notif_code) {
            case ui_populating_tform_popup: {
                TForm *form = va_arg(va, TForm*);
                TPopupMenu *menu = va_arg(va, TPopupMenu*);
                self->populating_tform_popup(form,menu);
                break;
            }
            case ui_finish_populating_tform_popup: {
                TForm *form = va_arg(va,TForm*);
                TPopupMenu *menu = va_arg(va,TPopupMenu*);
                self->finish_populating_tform_popup(form,menu);
                break;
            }
            default:
                break;
        }
        return 0;
    }
}