//
// Created by andrei on 13.04.23.
//

#ifndef RCR_GNOME_LOGINDIALOG_H
#define RCR_GNOME_LOGINDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"

class LoginDialog : public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::ToolButton* refButtonRegisterSwitchToLogin;
    Gtk::Button* refButtonOK;
    Gtk::Button* refButtonCancel;
    void bindWidgets();
    void onLogin();
    void onCancel();
    void onSwitchToLogin();
    bool on_key_press_event(GdkEventKey *event) override;
public:
    Gtk::Entry* refEntryLogin;
    Gtk::Entry* refEntryPassword;

    LoginDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~LoginDialog();

    void user(
        rcr::User *retVal
    );

    void setUser(
        const rcr::User &user
    );
};

#endif //RCR_GNOME_LOGINDIALOG_H
