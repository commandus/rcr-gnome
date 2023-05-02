//
// Created by andrei on 17.04.23.
//

#ifndef RCR_GNOME_USERLISTDIALOG_H
#define RCR_GNOME_USERLISTDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"

class UserListDialog : public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Button *refButtonAdd;
    Gtk::Button *refButtonRemove;
    Gtk::Button *refButtonBoxes;
    Gtk::Button *refButtonCancel;
    Gtk::TreeView *refTreeViewUser;
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelectionUser;
    Glib::RefPtr<Gtk::ListStore> refListStoreUsers;
    void onUserActivated(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column
    );

    void bindWidgets();
    void onAdd();
    void onRemove();
    void onBoxes();
    void onCancel();
    bool on_key_press_event(GdkEventKey *event) override;
public:
    rcr::User selectedUser;
    UserListDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~UserListDialog();
    void load(
        GRcrClient *client,
        RcrSettings *settings
    );
    void editUser();
};

#endif //RCR_GNOME_USERLISTDIALOG_H
