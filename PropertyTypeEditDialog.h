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
    Gtk::Button *refButtonPropertyTypeEditSave;
    Gtk::Button *refButtonPropertyTypeEditCancel;
    void bindWidgets();
    void onSave();
    void onCancel();
public:
    PropertyTypeEditDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~PropertyTypeEditDialog();
    void setClient(GRcrClient *client);
};

#endif //RCR_GNOME_PROPERTYTYPEEDITDIALOG_H
