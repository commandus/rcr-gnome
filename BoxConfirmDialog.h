//
// Created by andrei on 12.04.23.
//

#ifndef RCR_GNOME_BOXCONFIRMDIALOG_H
#define RCR_GNOME_BOXCONFIRMDIALOG_H

#include <gtkmm.h>

class BoxConfirmDialog: public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Entry *refEntryBox;
    Gtk::CheckButton *refСheckNumberInFileName;
    Gtk::Button *refButtonContinue;
    Gtk::Button *refButtonCancel;

    void bindWidgets();
    void onBoxConfirm();
    void onBoxCancel();
    bool on_key_press_event(GdkEventKey *event) override;
public:
    BoxConfirmDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~BoxConfirmDialog();

    uint64_t box();

    void setBox(
        const uint64_t &boxId
    );

    bool numberInFileName();

    void setNumberInFileName(bool value);


    void onBoxNumberInFileName();
};

#endif //RCR_GNOME_BOXCONFIRMDIALOG_H
