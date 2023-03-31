#ifndef TOPWINDOW_H
#define	TOPWINDOW_H

#include <functional>
#include <gtkmm.h>

#include "GRcrClient.h"

using namespace std::placeholders; // for `_1`

class TopWindow: public Gtk::Window {
public:
	TopWindow(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~TopWindow();
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
    std::string mLastSymbol;    ///< default "D"

    GRcrClient *client;

    Glib::RefPtr<Gtk::ListStore> mRefListStoreSymbol;
    Glib::RefPtr<Gtk::TreeStore> mRefTreeStoreBox;
    Glib::RefPtr<Gtk::ListStore> mRefListStoreCard;

    Gtk::AboutDialog *mAboutDialog;
    Gtk::Button *mButtonRefresh;
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Gtk::Entry *mEntryQuery;
    Gtk::Entry *mEntryHost;

    Gtk::TreeView *mTreeViewBox;
    Gtk::TreeView *mTreeViewCard;

	Gtk::ComboBox *mComboBoxSymbol;

	Glib::RefPtr<Gtk::FileFilter> mFileFilterXLSX;
	Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionBox;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionQuantity;
    Glib::RefPtr<Gtk::TreeModelFilter> mRefTreeModelFilterCard;

    void selectSymbol(const std::string &symbol);
    void searchCard(const std::string &query, const std::string &symbol);
};

#endif
