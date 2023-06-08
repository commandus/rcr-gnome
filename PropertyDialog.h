//
// Created by andrei on 12.05.23.
//

#ifndef RCR_GNOME_PROPERTYDIALOG_H
#define RCR_GNOME_PROPERTYDIALOG_H

#include "gen/rcr.pb.h"
#include "GRcrClient.h"

class PropertyDialog : public Gtk::Dialog {
    GRcrClient *client;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::ComboBox *cbPropertyKey;
    Gtk::Entry *entryPropertyValue;
    Gtk::Button *refButtonPropertyDelete;
    Gtk::Button *refButtonPropertySave;
    Gtk::Button *refButtonPropertyCancel;
    void bindWidgets();
    void onDelete();
    void onSave();
    void onCancel();
    bool on_key_press_event(GdkEventKey *event) override;
    bool confirmDelete();
public:
    PropertyDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~PropertyDialog();
    void setClient(GRcrClient *client);
    std::string key();
    std::string value();
    void set(
        const std::string &key,
        const std::string &value
    );

    void selectPropertyKey(
        const std::string &key
    );
};

#endif //RCR_GNOME_PROPERTYDIALOG_H
