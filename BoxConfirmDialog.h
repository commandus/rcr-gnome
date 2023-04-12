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
public:
    uint64_t boxId;

    Gtk::Entry* refEntryBox;
    Gtk::Button* refButtonContinue;
    Gtk::Button* refButtonCancel;

    BoxConfirmDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~BoxConfirmDialog();
protected:
    bool on_key_press_event(GdkEventKey *event) override;
    void bindWidgets();
    void onBoxConfirm();
    void onBoxCancel();
};

#endif //RCR_GNOME_BOXCONFIRMDIALOG_H
