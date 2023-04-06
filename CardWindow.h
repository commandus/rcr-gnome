//
// Created by andrei on 06.04.23.
//

#ifndef RCR_GNOME_CARDWINDOW_H
#define RCR_GNOME_CARDWINDOW_H

#include <gtkmm.h>

class CardWindow: public Gtk::Window {
protected:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;

    Glib::RefPtr<Gtk::ListStore> mRefListStoreComponent;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreMeasure;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreProperty;

    Gtk::ComboBox* mRefCBComponent;
    Gtk::ComboBox* mRefCBMeasure;

    Gtk::Entry* mRefEntryName;
    Gtk::Entry* mRefEntryBox;
    Gtk::Entry* mRefEntryQuantity;

    Gtk::ToolButton* mRefButtonCardSave;
    Gtk::ToolButton* mRefButtonCardCancel;
    Gtk::ToolButton* mRefButtonCardRm;

    Gtk::TreeView* mRefTreeViewProperty;

    Gtk::Label* mRefLabelNominal;
    Gtk::Label* mRefLabelName;

    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionProperty;

    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;

public:
    CardWindow(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
    virtual ~CardWindow();
protected:
    void bindWidgets();
    void onCardSave();
    void onCardRm();
    void onCardCancel();
};

#endif //RCR_GNOME_CARDWINDOW_H
