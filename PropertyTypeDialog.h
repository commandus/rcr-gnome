//
// Created by andrei on 28.04.23.
//

#ifndef RCR_GNOME_PROPERTYTYPEDIALOG_H
#define RCR_GNOME_PROPERTYTYPEDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"
#include "PropertyTypeEditDialog.h"

class PropertyTypeDialog : public Gtk::Dialog {
private:
    GRcrClient *client;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gtk::ListStore> mRefListstorePropertyType;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::TreeView *tvPropertyType;
    Gtk::Button *refButtonPropertyTypeAdd;
    Gtk::Button *refButtonPropertyTypeClose;
    PropertyTypeEditDialog *propertyTypeEditDialog;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionPropertyType;

    void loadPropertyTypes();

    void onPropertyTypeActivated(
        const Gtk::TreeModel::Path& path,
        Gtk::TreeViewColumn* column
    );

    void bindWidgets();
    void onAdd();
    void onClose();
    bool on_key_press_event(GdkEventKey *event) override;
public:
    PropertyTypeDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~PropertyTypeDialog();
    void setClient(
        GRcrClient *client,
        PropertyTypeEditDialog *propertyTypeEditDialog
    );
};

#endif //RCR_GNOME_PROPERTYTYPEDIALOG_H
