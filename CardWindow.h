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
    void onCardActivated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);

    void bindWidgets();
    void onCardSave();
    void onCardRm();
    void onCardCancel();
    void onAddProperty();
    void editProperty();
    void onSymbolSelected();
    bool confirmDeleteCard();
public:
    GRcrClient *client;
    PropertyDialog *propertyDialog;
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

    COMPONENT getSelectedComponent();
    void selectSymbolId(uint64_t symbolId);

    void setCard(
        uint64_t id,
        bool isNew,
        uint64_t symbolId,
        const std::string &name,
        const std::string &nominal,
        uint64_t qty,
        uint64_t packageId,
        uint64_t boxId,
        const std::string &boxName,
        const std::string &properties
    );

    void listProperties();

    void setProperty(
        const std::string &key,
        const std::string &value
    );
};

#endif //RCR_GNOME_CARDWINDOW_H
