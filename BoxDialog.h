//
// Created by andrei on 03.05.23.
//

#ifndef RCR_GNOME_BOXDIALOG_H
#define RCR_GNOME_BOXDIALOG_H

#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"

class BoxDialog : public Gtk::Dialog {
    GRcrClient *client;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Entry *entryBoxId;
    Gtk::Entry *entryBoxName;
    Gtk::Button *refButtonBoxDelete;
    Gtk::Button *refButtonBoxConfirm;
    Gtk::Button *refButtonBoxCancel;
    void bindWidgets();
    void onDelete();
    void onConfirm();
    void onCancel();
    bool on_key_press_event(GdkEventKey *event) override;
    bool confirmDelete();
protected:
    uint64_t id;
    uint64_t srcBoxId; // previous box id
public:
    BoxDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~BoxDialog();
    void setClient(GRcrClient *client);
    void set(
        uint64_t id,
        uint64_t boxId,
        const std::string &name
    );

};


#endif //RCR_GNOME_BOXDIALOG_H
