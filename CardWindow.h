//
// Created by andrei on 06.04.23.
//

#ifndef RCR_GNOME_CARDWINDOW_H
#define RCR_GNOME_CARDWINDOW_H

#include <gtkmm.h>

class CardWindow: public Gtk::Window {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;

    Glib::RefPtr<Gtk::ListStore> mRefListStoreComponent;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreMeasure;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreProperty;

    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;

protected:
    Gtk::ToolButton* mRefButtonCardSave;
    Gtk::ToolButton* mRefButtonCardCancel;
    Gtk::ToolButton* mRefButtonCardRm;

    Gtk::Label* mRefLabelNominal;
    Gtk::Label* mRefLabelName;

    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionProperty;

public:
    uint64_t id;
    uint64_t boxId;

    Gtk::ComboBox* refCBSymbol;
    Gtk::ComboBox* refCBMeasure;

    Gtk::Entry* refEntryName;
    Gtk::Entry* refEntryBox;
    Gtk::Entry* refEntryQuantity;
    Gtk::Entry* refEntryNominal;

    Gtk::TreeView* refTreeViewProperty;

    CardWindow(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
    virtual ~CardWindow();
protected:
    void bindWidgets();
    void onCardSave();
    void onCardRm();
    void onCardCancel();
    void onSymbolSelected();

};

#endif //RCR_GNOME_CARDWINDOW_H
