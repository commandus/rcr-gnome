#ifndef TOPWINDOW_H
#define	TOPWINDOW_H

#include <functional>
#include <gtkmm.h>

#include "rcr-gnome.h"

#include "GRcrClient.h"
#include "RcrSettings.h"

#include "CardWindow.h"
#include "BoxDialog.h"
#include "BoxConfirmDialog.h"
#include "LoginDialog.h"
#include "RegisterDialog.h"
#include "UserListDialog.h"
#include "UserDialog.h"
#include "StatisticsDialog.h"
#include "PropertyTypeDialog.h"
#include "PropertyTypeEditDialog.h"
#include "PropertyDialog.h"

class TopWindow: public Gtk::Window, public ServiceStateIntf {
public:
    TopWindow(BaseObjectType *, const Glib::RefPtr<Gtk::Builder> &);

    // window to edit component card
    CardWindow *cardWindow;
    // box dialog
    BoxDialog *boxDialog;
    // confirmation dialog
    BoxConfirmDialog *boxConfirmDialog;
    // Login dialog
    LoginDialog *loginDialog;
    // Register a new user dialog
    RegisterDialog *registerDialog;
    // User list
    UserListDialog *userListDialog;
    // User
    UserDialog *userDialog;
    // Statistics
    StatisticsDialog *statisticsDialog;
    // Property type
    PropertyTypeDialog *propertyTypeDialog;
    PropertyTypeEditDialog *propertyTypeEditDialog;
    // Property
    PropertyDialog *propertyDialog;

    virtual ~TopWindow();

    void loadSettings();
    void saveSettings();

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
    Gtk::Label *mLabelMessage;
    Gtk::ProgressBar *mProgressBar;

    Glib::RefPtr<Gtk::FileFilter> mFileFilterXLSX;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionBox;
    Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionCard;
    Glib::RefPtr<Gtk::TreeModelFilter> mRefTreeModelFilterCard;

    bool on_key_press_event(GdkEventKey *event) override;

    void onHelpAbout();

    void onHelpStatistics();
    void onFileQuit();
    void onFileConnect();
    void onBoxSelected(Glib::RefPtr<Gtk::TreeSelection> selection);
    void onSymbolSelected();
    void onCardSelected(Glib::RefPtr<Gtk::TreeSelection> selection);
    void onAboutDialogResponse(int responseId);
    void onStartImportFile();
    void onStartImportDirectory();
    void onLogin();
    void onRegister();
    void onUserList();
    void onUserAdd();
    void onUserBox();
    void onBoxEdit();
    void onBoxNew();
    void onBoxDelete();
    void onProperties();
    void selectSymbol(
        Gtk::ComboBox *cb,
        const std::string &symbol
    );
    void selectSymbolId(
        Gtk::ComboBox *cb,
        uint64_t symbold
    );

    void selectBox(
        const uint64_t boxId
    );
    void searchCard(
        const std::string &query,
        const std::string &symbol
    );

    void bindWidgets();
    void doQuery();

    void showMessageTimeout(
        int module,
        int code,
        const std::string &message,
        const int timeout = 0
    );

    void onCallStarted(
        int module,
        const std::string &message = ""
    ) override;

    void onCallFinished(
        int module,
        int code,
        const std::string &message = ""
    ) override;

    void reflectChangesCard();

    // Not used yet
    void onProgress(
        int pos,
        int total
    ) override;

    // dialogs
    void createDialogs();

    // dialog close event handlers
    void onHideCardWindow(Gtk::Window *window);

    void onHideboxConfirmWindow(Gtk::Window *window);

    // call dialog
    void editCard();

    void editCard(
        GRcrClient *client,
        uint64_t symbolId,
        const std::string &name,
        const std::string &nominal,
        const std::string &properties,
        const std::string &boxName,
        uint64_t qty,
        uint64_t id,
        uint64_t packageId,
        uint64_t boxId,
        bool isNew
    );

    bool confirmBox(
        uint64_t &box_id,
        bool &numberInFileName
    );

    void onCardActivated(
        const Gtk::TreeModel::Path &path,
        Gtk::TreeViewColumn *column
    );

    void runImportExcel(
        const std::string &symbol,
        const std::string &path,
        uint64_t box,
        bool isDirectory = false,
        bool numberInFileName = true
    );

    void reloadBoxTree();

    void editUser(rcr::User *user);

    bool confirmDeleteBox(
        Gtk::TreeModel::Row &row
    );

    void reflectRemoveCard();
};

#endif
