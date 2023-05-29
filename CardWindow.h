//
// Created by andrei on 06.04.23.
//

#ifndef RCR_GNOME_CARDWINDOW_H
#define RCR_GNOME_CARDWINDOW_H

#include <gtkmm.h>
#include "GRcrClient.h"
#include "PropertyDialog.h"

class CardWindow: public Gtk::Window {
private:
    PropertyDialog *propertyDialog;
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

    bool on_key_press_event(GdkEventKey* event);

    void bindWidgets();
    void onCardSave();
    void onCardRm();
    void onCardCancel();
    void onAddProperty();
    void onSymbolSelected();
    bool confirmDeleteCard();
public:
    GRcrClient *client;
    // save prev values
    uint64_t symbolId;
    uint64_t id;
    uint64_t boxId;
    uint64_t packageId;
    std::string boxName;
    std::string properties;
    bool isNew;

    Gtk::ComboBox* refCBSymbol;
    Gtk::ComboBox* refCBMeasure;

    Gtk::Entry* refEntryName;
    Gtk::Entry* refEntryBox;
    Gtk::Entry* refEntryQuantity;
    Gtk::Entry* refEntryNominal;

    Gtk::TreeView* refTreeViewProperty;
    Gtk::ToolButton *refButtonPropertyAdd;

    CardWindow(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
    virtual ~CardWindow();

    void setBox(
        uint64_t packageId,
        uint64_t boxId,
        const std::string &BoxName,
        const std::string &propertiesString,
        PropertyDialog *propertyDialog
    );

    COMPONENT getSelectedComponent();
};

#endif //RCR_GNOME_CARDWINDOW_H
