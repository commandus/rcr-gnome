//
// Created by andrei on 17.04.23.
//

#ifndef RCR_GNOME_USERDIALOG_H
#define RCR_GNOME_USERDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"

class UserDialog : public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Entry* refEntryUserLogin;
    Gtk::Entry* refEntryUserPassword;
    Gtk::ToggleToolButton* refToogleUserAdmin;

    Gtk::Button *refButtonSave;
    Gtk::Button *refButtonCancel;

    void bindWidgets();
    void onRightsAdmin();
    void onSave();
    void onCancel();
    bool on_key_press_event(GdkEventKey *event) override;
public:
    UserDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~UserDialog();

    void get(
        rcr::User &retVal
    );
    void set(
        bool hasAdminRights,
        const rcr::User &val
    );
};

#endif //RCR_GNOME_USERDIALOG_H
