//
// Created by andrei on 28.04.23.
//

#ifndef RCR_GNOME_PROPERTYTYPEEDITDIALOG_H
#define RCR_GNOME_PROPERTYTYPEEDITDIALOG_H

#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"

class PropertyTypeEditDialog : public Gtk::Dialog {
private:
    GRcrClient *client;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Entry *entryPropertyTypeEditKey;
    Gtk::Entry *entryPropertyTypeEditDescription;
    Gtk::Button *refButtonPropertyTypeEditDelete;
    Gtk::Button *refButtonPropertyTypeEditSave;
    Gtk::Button *refButtonPropertyTypeEditCancel;
    void bindWidgets();
    void onDelete();
    void onSave();
    void onCancel();
    bool on_key_press_event(GdkEventKey *event) override;
protected:
    uint64_t id;
public:
    PropertyTypeEditDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~PropertyTypeEditDialog();
    void setClient(GRcrClient *client);
    void set(
        uint64_t id,
        const std::string &key,
        const std::string &description
    );
};

#endif //RCR_GNOME_PROPERTYTYPEEDITDIALOG_H
