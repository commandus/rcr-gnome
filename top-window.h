#ifndef TOPWINDOW_H
#define	TOPWINDOW_H

#include <functional>
#include <gtkmm.h>

#include "GRcrClient.h"
#include "RcrSettings.h"

#include "CardWindow.h"

class TopWindow: public Gtk::Window {
public:
    TopWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

    // window to edit component card
    CardWindow *cardWindow;

    virtual ~TopWindow();

    void loadSettings();

    void saveSettings();

protected:
    bool on_key_press_event(GdkEventKey *event) override;

    void onHelpAbout();

    void onFileQuit();

    void onFileConnect();

    void onBoxSelected(Glib::RefPtr<Gtk::TreeSelection> selection);

    void onSymbolSelected();

    void onCardSelected(Glib::RefPtr<Gtk::TreeSelection> selection);

    void onAboutDialogResponse(int responseId);

private:
    RcrSettings *settings;
    GRcrClient *client;

    Glib::RefPtr<Gtk::ListStore> mRefListStoreSymbol;
    Glib::RefPtr<Gtk::TreeStore> mRefTreeStoreBox;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreCard;

    Gtk::AboutDialog *mAboutDialog;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Gtk::Entry *mEntryQuery;

    Gtk::TreeView *mTreeViewBox;
    Gtk::TreeView *mTreeViewCard;
    Gtk::ComboBox *mComboBoxSymbol;

    Glib::RefPtr<Gtk::FileFilter> mFileFilterXLSX;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionBox;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionCard;
    Glib::RefPtr<Gtk::TreeModelFilter> mRefTreeModelFilterCard;

    void selectSymbol(const std::string &symbol);

    void selectBox(const uint64_t boxId);

    void searchCard(const std::string &query, const std::string &symbol);

    void bindWidgets();

    void doQuery();

    void createCardWindow();

    void onHideCardWindow(Gtk::Window *window);

    void editCard();

    void editCard(const std::string &name, const std::string &nominal, const std::string &properties, const std::string &boxName,
        uint64_t qty, uint64_t id, uint64_t boxId, bool isNew);
};

#endif
