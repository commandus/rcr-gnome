//
// Created by andrei on 13.04.23.
//

#ifndef RCR_GNOME_REGISTERDIALOG_H
#define RCR_GNOME_REGISTERDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"

class RegisterDialog  : public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Entry* refEntryLogin;
    Gtk::Entry* refEntryPassword;
    Gtk::Entry* refEntryRetypePassword;
    Gtk::ToolButton* refButtonLoginSwitchToRegister;
    Gtk::Button* refButtonOK;
    Gtk::Button* refButtonCancel;
    bool on_key_press_event(GdkEventKey *event) override;
    void bindWidgets();
    void onRegister();
    void onCancel();
    void onSwitchToRegister();
public:
    RegisterDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~RegisterDialog();

    void user(
        rcr::User *retVal
    );

    void setUser(
        const rcr::User &user
    );

};

#endif //RCR_GNOME_REGISTERDIALOG_H
