#include <gtkmm.h>
#include <gdk/gdkkeysyms.h>
#include <thread>

#include "top-window.h"
#include "RCQuery.h"
#include "string-helper.h"
#include "utilfile.h"

#define DEF_NAME_ALL	_("All")

#define MIT_LICENSE _("Copyright (c) 2023 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer \n\n\
Institute of Cosmophysical Research and Aeronomy of Siberian \n\n\
Branch of the Russian Academy of Sciences \n\n\
MIT license\n\n\
Permission is hereby granted, free of charge, to any \n\
person obtaining a copy of this software and associated \n\
documentation files (the \"Software\"), to deal in the \n\
Software without restriction, including without limitation \n\
the rights to use, copy, modify, merge, publish, distribute, \n\
sublicense, and/or sell copies of the Software, and to permit \n\
persons to whom the Software is furnished to do so, subject \n\
to the following conditions:\n\n\
The above copyright notice and this permission notice shall \n\
be included in all copies or substantial portions of the \n\
Software.\n\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY \n\
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE \n\
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR\" \n\
PURPOSE AND NONINFRINGEMENT IN NO EVENT SHALL THE AUTHORS \n\
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR \n\
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT\" \n\
OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION \n\
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN \n\
THE SOFTWARE.")

void TopWindow::bindWidgets() {
    mRefBuilder->get_widget("entryQuery", mEntryQuery);
    mRefBuilder->get_widget("cbSymbol", mComboBoxSymbol);

    mComboBoxSymbol->signal_changed().connect(sigc::mem_fun(*this, &TopWindow::onSymbolSelected));
    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("connect", sigc::mem_fun(*this, &TopWindow::onFileConnect));
    mRefActionGroup->add_action("quit", sigc::mem_fun(*this, &TopWindow::onFileQuit));
    mRefActionGroup->add_action("about", sigc::mem_fun(*this, &TopWindow::onHelpAbout));
    mRefActionGroup->add_action("statistics", sigc::mem_fun(*this, &TopWindow::onHelpStatistics));

    mRefActionGroup->add_action("importFile", sigc::mem_fun(*this, &TopWindow::onStartImportFile));
    mRefActionGroup->add_action("importDirectory", sigc::mem_fun(*this, &TopWindow::onStartImportDirectory));
    mRefActionGroup->add_action("login", sigc::mem_fun(*this, &TopWindow::onLogin));
    mRefActionGroup->add_action("register", sigc::mem_fun(*this, &TopWindow::onRegister));

    mRefActionGroup->add_action("userList", sigc::mem_fun(*this, &TopWindow::onUserList));
    mRefActionGroup->add_action("userAdd", sigc::mem_fun(*this, &TopWindow::onUserAdd));
    mRefActionGroup->add_action("userBox", sigc::mem_fun(*this, &TopWindow::onUserBox));
    mRefActionGroup->add_action("properties", sigc::mem_fun(*this, &TopWindow::onProperties));

    mRefActionGroup->add_action("boxEdit", sigc::mem_fun(*this, &TopWindow::onBoxEdit));
    mRefActionGroup->add_action("boxNew", sigc::mem_fun(*this, &TopWindow::onBoxNew));
    mRefActionGroup->add_action("boxDelete", sigc::mem_fun(*this, &TopWindow::onBoxDelete));

    // labelStatisticsComponents

    insert_action_group("rcr", mRefActionGroup);

    mRefBuilder->get_widget("tvBox", mTreeViewBox);
    mRefBuilder->get_widget("tvCard", mTreeViewCard);
    mRefBuilder->get_widget("labelMessage", mLabelMessage);
    mRefBuilder->get_widget("progressBar", mProgressBar);

    mRefListStoreSymbol = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreSymbol"));
    mRefTreeStoreBox = Glib::RefPtr<Gtk::TreeStore>::cast_static(mRefBuilder->get_object("treestoreBox"));
    mRefListStoreCard = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreCard"));

    mTreeViewSelectionBox = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsBox"));
    mTreeViewSelectionCard = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsCard"));
}

void TopWindow::loadSettings() {
    settings = new RcrSettings("", getCurrentDir().c_str(), "rcr.json");
    settings->selected = 0;
    if (settings->selected >= settings->settings.service_size()) {
        // default
        settings->selected = 0;
        mEntryQuery->set_text("");
        return;
    }
    mEntryQuery->set_text(settings->settings.service(settings->selected).last_query());
}

void TopWindow::saveSettings() {
    settings->save();
}

/**
 *
 * @param cobject
 * @param refBuilder
 */
TopWindow::TopWindow (BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder)
	: client(nullptr),
    Gtk::Window(cobject), mRefBuilder (refBuilder)
{
    createDialogs();
    bindWidgets();
    // 0 name 1 boxname 2 properties 3 id 4 qty 5 rem 6 boxid 7 symbol_id 8- packageId
    mRefTreeModelFilterCard = Gtk::TreeModelFilter::create(mRefListStoreCard);
	mRefTreeModelFilterCard->set_visible_func(
	[this] (const Gtk::TreeModel::const_iterator& it) -> bool {
		Gtk::TreeModel::Row row = *it;
		uint64_t boxId;
		row.get_value(6, boxId);
		Gtk::TreeModel::iterator iter = mTreeViewSelectionBox->get_selected();
		if (iter) {
            Gtk::TreeModel::Row row = *iter;
            uint64_t box;
            row.get_value(2, box); // 2- box
            return StockOperation::isBoxInBoxes(boxId, box);
        }
		return true;
	});

	mTreeViewCard->set_model(Glib::RefPtr<Gtk::TreeModelSort>(Gtk::TreeModelSort::create(mRefTreeModelFilterCard)));
    mTreeViewCard->get_column(0)->set_sort_column(0);
    mTreeViewCard->get_column(1)->set_sort_column(1);
    mTreeViewCard->get_column(2)->set_sort_column(2);
    mTreeViewCard->get_column(3)->set_sort_column(3);
    mTreeViewCard->get_column(4)->set_sort_column(4);
    mTreeViewCard->signal_row_activated().connect(sigc::mem_fun(*this, &TopWindow::onCardActivated), mTreeViewCard);
	mTreeViewSelectionBox->signal_changed().connect(
			sigc::bind <Glib::RefPtr<Gtk::TreeSelection>> (sigc::mem_fun(*this, &TopWindow::onBoxSelected), mTreeViewSelectionBox));

	add_events(Gdk::KEY_PRESS_MASK);

    mFileFilterXLSX = Gtk::FileFilter::create();
	mFileFilterXLSX->set_name(_("Excel spreadsheet files"));
	mFileFilterXLSX->add_mime_type("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");

    loadSettings();
    onFileConnect();
}

void TopWindow::onBoxSelected(
	Glib::RefPtr<Gtk::TreeSelection> selection
) {
    Gtk::TreeModel::iterator iter = selection->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        uint64_t b;
        row.get_value(2, b);
        // save
        settings->settings.mutable_service(settings->selected)->set_last_box(b);
        // saveSettings();
        doQuery();
        // doQuery or reload card tree view if not- uncomment below line
        mRefTreeModelFilterCard->refilter();
    }
}

void TopWindow::onCardSelected(
	Glib::RefPtr<Gtk::TreeSelection> selection
) {
	if (selection) {
		Gtk::TreeModel::iterator iter = selection->get_selected();
		if (iter) {
			Gtk::TreeModel::Row row = *iter;
			std::string name;
			row.get_value(0, name);
            std::string nominal;
			row.get_value(1, nominal);
            std::string properties;
            row.get_value(2, properties);
            std::string boxname;
            row.get_value(3, boxname);
            uint64_t qty;
            row.get_value(4, qty);
            uint64_t box;
            row.get_value(5, box);
		}
	}
}

TopWindow::~TopWindow() {
    if (client) {
        delete client;
        client = nullptr;
    }
}

bool TopWindow::on_key_press_event(GdkEventKey* event)
{
	switch (event->keyval) {
        case GDK_KEY_F4:
            if (mTreeViewBox->has_focus()) {
                onBoxEdit();
            }
            break;
        case GDK_KEY_Delete:
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            if (mTreeViewBox->has_focus()) {
                onBoxDelete();
            }
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            if (mTreeViewBox->has_focus()) {
                onBoxNew();
            }
            break;
		case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            if (mTreeViewCard->is_focus()) {
                editCard();
            } else {
                doQuery();
            }
			break;
		default:
			return Gtk::Window::on_key_press_event(event);
	}
	return FALSE; 
}

void TopWindow::onHelpAbout()
{
	mAboutDialog = new Gtk::AboutDialog();
    mAboutDialog->set_size_request(640, 480);
	mAboutDialog->set_transient_for(*this);
	mAboutDialog->set_logo(Gdk::Pixbuf::create_from_resource("/icons/data/icons/48x48/apps/rcr.png", -1, 40, true));
	mAboutDialog->set_program_name(_("rcr for Gnome"));
	std::string v;
#ifdef HAVE_CONFIG_H
	v = PACKAGE_STRING;
#else
    #ifdef RCR_VERSION
#define STRINGIFY2( x) #x
#define STRINGIFY(x) STRINGIFY2(x)
        v = STRINGIFY(RCR_VERSION);
    #else
        v = _("Custom build");
    #endif

#endif
	mAboutDialog->set_version(v);
	mAboutDialog->set_copyright(_("2023 Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences"));
	mAboutDialog->set_comments(_("In memory of Konstantin Zolotovsky"));
	mAboutDialog->set_license(MIT_LICENSE);

	mAboutDialog->set_website("https://ikfia.ysn.ru/");
	mAboutDialog->set_website_label("ikfia.ysn.ru");

	std::vector<Glib::ustring> list_authors;
	list_authors.push_back(_("Andrei Ivanov andrey.ivanov@ikfia.ysn.ru"));
    list_authors.push_back(_("Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy http://ikfia.ysn.ru/"));
    list_authors.push_back(_("ICU https://icu.unicode.org/"));
    list_authors.push_back(_("xlnt https://github.com/tfussell/xlnt"));
    list_authors.push_back(_("gRPC https://grpc.io/"));

    mAboutDialog->set_authors(list_authors);
	mAboutDialog->signal_response().connect(
		sigc::mem_fun(*this, &TopWindow::onAboutDialogResponse));

	mAboutDialog->show();
	mAboutDialog->present();
}

void TopWindow::onHelpStatistics()
{
    uint64_t componentCount, total;
    client->getStatistics(componentCount, total);
    statisticsDialog->setStat(componentCount, total);
    statisticsDialog->run();
}

void TopWindow::onFileQuit()
{
	hide();
}

void TopWindow::onAboutDialogResponse(int responseId)
{
	switch (responseId) {
		case Gtk::RESPONSE_CLOSE:
		case Gtk::RESPONSE_CANCEL:
		case Gtk::RESPONSE_DELETE_EVENT:
			mAboutDialog->hide();
			break;
		default:
			break;
	}
}

void TopWindow::onFileConnect() {
    if (client)
        delete client;
    client = new GRcrClient(settings->settings.service(settings->selected).addr());
    client->setServiceState(this);
    client->loadDictionaries();
    mComboBoxSymbol->unset_model();
    client->bindSymbols(mRefListStoreSymbol);
    mComboBoxSymbol->set_model(mRefListStoreSymbol);

    selectSymbol(mComboBoxSymbol, settings->settings.service(settings->selected).last_component_symbol());
    reloadBoxTree();
    // bind client with dictionaries to dialog
    propertyTypeDialog->setClient(reinterpret_cast<GtkWindow *>(this), client, propertyTypeEditDialog);
}

void TopWindow::reloadBoxTree() {
    mTreeViewBox->unset_model();
    client->loadBoxes(mRefTreeStoreBox);
    mTreeViewBox->set_model(mRefTreeStoreBox);
    mTreeViewBox->expand_all();
    selectBox(settings->settings.service(settings->selected).last_box());
}

void TopWindow::selectSymbol(
    Gtk::ComboBox *cb,
    const std::string &symbol
) {
    auto children = mRefListStoreSymbol->children();
    auto c = 0;
    for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
        std::string sym;
        iter->get_value(2, sym);
        if (sym == symbol)
            break;
        c++;
    }
    cb->set_active(c);
}

void TopWindow::selectSymbolId(
    Gtk::ComboBox *cb,
    uint64_t symbolId
) {
    auto children = mRefListStoreSymbol->children();
    auto c = 0;
    for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
        uint64_t id;
        iter->get_value(1, id);
        if (id == symbolId)
            break;
        c++;
    }
    cb->set_active(c);
}

static bool findBox(
    GtkTreeModel *m,
    GtkTreeIter *it,
    uint64_t boxId,
    std::function<void (GtkTreeIter *it)> cb
)
{
    bool valid = true;
    int c = 0;
    while (valid) {
        uint64_t b;
        gtk_tree_model_get(m, it, 2, &b, -1);

        if (b == boxId) {
            cb(it);
            return true;
        }
        GtkTreeIter itc;
        bool validChild = gtk_tree_model_iter_children(m, &itc, it);
        while (validChild) {
            if (findBox(m, &itc, boxId, cb))
                return false;
            validChild = gtk_tree_model_iter_next(m, &itc);
        }
        c++;
        valid = gtk_tree_model_iter_next(m, it);
    }
    return false;
}

void TopWindow::selectBox(
    const uint64_t boxId
)
{
    GtkTreeModel *m = gtk_tree_view_get_model(mTreeViewBox->gobj());
    GtkTreeIter it;
    bool valid = gtk_tree_model_get_iter_first(m, &it);
    if (valid) {
        auto s = mTreeViewSelectionBox;
        auto b = mTreeViewBox;
        if (findBox(m, &it, boxId, [b, s](GtkTreeIter *it){
            s->select((const Gtk::TreeIter&) it);
            Gtk::TreeViewColumn fc;
        })) {
#if CMAKE_BUILD_TYPE == Debug
            std::cerr << _("Selected box ") << StockOperation::boxes2string(boxId) << std::endl;
#endif
        }
    }
}

void TopWindow::onSymbolSelected() {
    std::string sym;
    mComboBoxSymbol->get_active()->get_value(2, sym);
    settings->settings.mutable_service(settings->selected)->set_last_component_symbol(sym);
}

void TopWindow::searchCard(
    const std::string &q,
    const std::string &symbol
)
{
    COMPONENT component;
    RCQuery query(ML_RU, q, component);

    query.componentName = toUpperCase(query.componentName);
    if (query.componentName.find('*') == std::string::npos)
        query.componentName += "*";
    mTreeViewCard->unset_model();

    std::string qs = query.toString();
    uint64_t b = settings->settings.service(settings->selected).last_box();
    if (b)
        qs += " " + StockOperation::boxes2string(b);
        client->query(qs, symbol, mRefListStoreCard);

    mTreeViewCard->set_model(Glib::RefPtr<Gtk::TreeModelSort>(Gtk::TreeModelSort::create(mRefTreeModelFilterCard)));
    settings->settings.mutable_service(settings->selected)->set_last_query(q);
}

void TopWindow::onCallStarted(
    int module,
    const std::string &message
)
{
    mLabelMessage->set_text(message);
    mProgressBar->pulse();
}

void TopWindow::onCallFinished(
    int module,
    int code,
    const std::string &message
)
{
    mProgressBar->set_fraction(0.0f);
    mLabelMessage->set_text(message);
    g_timeout_add_seconds(3, [](void *label){
        ((Gtk::Label *) label)->set_text("");
        return (gboolean) false;
    }, mLabelMessage);
    switch (module) {
        case OP_SAVE_CARD:
            reflectChangesCard();
            break;
        case OP_RM_CARD:
            reflectRemoveCard();
            break;
        case OP_IMPORT_FILE:
            reloadBoxTree();
            {
                Gtk::MessageDialog dialog(*this, code == 0 ?
                    _("Excel file imported successfully") :
                    _("Error import Excel file"));
                dialog.set_secondary_text(message);
                dialog.run();
            }
            break;
        case OP_IMPORT_DIR:
            reloadBoxTree();
            {
                Gtk::MessageDialog dialog(*this, code == 0 ?
                    _("Excel files imported successfully") :
                    _("Error import Excel files"));
                dialog.set_secondary_text(message);
                dialog.run();
            }
            break;
        default:
            break;
    }
}
// Not used yet
void TopWindow::onProgress(
    int pos,
    int total
)
{
    mProgressBar->pulse();
}

void TopWindow::doQuery() {
    searchCard(mEntryQuery->get_text(), settings->settings.service(settings->selected).last_component_symbol());
}

void TopWindow::createDialogs() {
    // card window is not a dialog
    mRefBuilder->get_widget_derived("cardWindow", cardWindow);
    cardWindow->signal_hide().connect(sigc::bind<Gtk::Window *>(
            sigc::mem_fun(*this, &TopWindow::onHideCardWindow), cardWindow));
    // cardWindow->set_modal();
    // dialogs
    mRefBuilder->get_widget_derived("boxDialog", boxDialog);
    mRefBuilder->get_widget_derived("boxConfirmDialog", boxConfirmDialog);
    mRefBuilder->get_widget_derived("loginUserDialog", loginDialog);
    mRefBuilder->get_widget_derived("registerUserDialog", registerDialog);
    mRefBuilder->get_widget_derived("userListDialog", userListDialog);
    mRefBuilder->get_widget_derived("userDialog", userDialog);
    mRefBuilder->get_widget_derived("statisticsDialog", statisticsDialog);
    mRefBuilder->get_widget_derived("propertyTypeDialog", propertyTypeDialog);
    mRefBuilder->get_widget_derived("propertyTypeEditDialog", propertyTypeEditDialog);
}

void TopWindow::onHideCardWindow(Gtk::Window *window) {
    Gtk::TreeModel::iterator iter = mTreeViewSelectionCard->get_selected();
    if (!iter)
        return;
    // update
    // show main window
}

void TopWindow::onHideboxConfirmWindow(Gtk::Window *window) {
    // update
    // show main window
}

void TopWindow::editCard() {
    Gtk::TreeModel::iterator iter = mTreeViewSelectionCard->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    std::string name, nominal, properties, boxName;
    uint64_t qty, id, box, symbol_id, packageId;
    row.get_value(0, name);
    row.get_value(1, nominal);
    row.get_value(2, properties);
    row.get_value(3, boxName);
    row.get_value(4, qty);
    row.get_value(5, id);
    row.get_value(6, box);
    row.get_value(7, symbol_id);
    row.get_value(8, packageId);

    editCard(client, symbol_id, name, nominal, properties, boxName, qty, id, packageId, box, false);
}

void TopWindow::editCard(
    GRcrClient *aClient,
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
) {
    cardWindow->client = aClient;
    cardWindow->id = id;
    cardWindow->isNew = isNew;
    cardWindow->show_all();
    selectSymbolId(cardWindow->refCBSymbol, symbolId);
    cardWindow->refEntryName->set_text(name);
    cardWindow->refEntryNominal->set_text(nominal);
    cardWindow->refEntryQuantity->set_text(std::to_string(qty));
    cardWindow->setBox(packageId, boxId, boxName, properties);
}

bool TopWindow::confirmBox(
    uint64_t &box_id,
    bool numberInFileName
)
{
    boxConfirmDialog->setBox(box_id);
    boxConfirmDialog->setNumberInFileName(numberInFileName);
    int r = boxConfirmDialog->run();
    if (r != Gtk::RESPONSE_OK)
        return false;
    box_id = boxConfirmDialog->box();
    // uncomment if create box in the root denied
    // return (StockOperation::parseBoxes(box_id, sb, 0, sb.size()) > 0 && box_id);
    return true;
}

void TopWindow::onCardActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    editCard();
}

void TopWindow::onStartImportFile()
{
    uint64_t b = settings->settings.mutable_service(settings->selected)->last_box();
    bool numberInFileName = settings->settings.mutable_service(settings->selected)->number_in_filename();
    if (!confirmBox(b, numberInFileName))
        return;
    settings->settings.mutable_service(settings->selected)->set_number_in_filename(numberInFileName);
    settings->save();

    std::string symbol = settings->settings.service(settings->selected).last_component_symbol();
    COMPONENT c = getComponentBySymbol(symbol);
    std::string cn = MeasureUnit::description(ML_RU, c);

    std::stringstream ss;
    ss << _("Import ") << cn << _(" to box ") << StockOperation::boxes2string(b);
    std::string t = ss.str();

    Gtk::FileChooserDialog dialog(t, Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);
    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Import"), Gtk::RESPONSE_OK);
    dialog.add_filter(mFileFilterXLSX);
    int result = dialog.run();
    // Handle the response:
    if (result == Gtk::RESPONSE_OK) {
        std::string fn = dialog.get_filename();
        runImportExcel(symbol, fn, b, false, numberInFileName);
    }
}
void TopWindow::onLogin()
{
    loginDialog->setUser(settings->settings.user());
    int r = loginDialog->run();
    if (r == Gtk::RESPONSE_REJECT) {
        onRegister();
        return;
    }
    if (r != Gtk::RESPONSE_OK)
        return;
    loginDialog->user(settings->settings.mutable_user());
}

void TopWindow::onRegister()
{
    registerDialog->setUser(settings->settings.user());
    int r = registerDialog->run();
    if (r == Gtk::RESPONSE_REJECT) {
        onLogin();
        return;
    }
    if (r != Gtk::RESPONSE_OK)
        return;
    registerDialog->user(settings->settings.mutable_user());
}

void TopWindow::onUserList()
{
    if (!userListDialog)
        return;
    userListDialog->load(client, settings);
    int r = userListDialog->run();
    switch (r) {
        case Gtk::RESPONSE_YES:
            // add
            onUserAdd();
            break;
        case Gtk::RESPONSE_NO:
            // remove selected
            break;
        case Gtk::RESPONSE_APPLY:
            // boxes owned by selected user
            break;
        case Gtk::RESPONSE_CANCEL:
            // nothing
            break;
        case Gtk::RESPONSE_ACCEPT:
            // show user
            editUser(&userListDialog->selectedUser);
            break;
        default:
            break;
    }
}

void TopWindow::onUserAdd()
{
    editUser(nullptr);
}

void TopWindow::onProperties()
{
    propertyTypeDialog->run();
}

void TopWindow::onUserBox()
{
    // edit current user
    editUser(settings->settings.mutable_user());
}

void TopWindow::onBoxEdit()
{
    // edit selected box
    Gtk::TreeModel::iterator iter = mTreeViewSelectionBox->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        if (client) {
            uint64_t id, boxId;
            std::string name;
            row.get_value(0, name);
            row.get_value(1, id);
            row.get_value(2, boxId);
            boxDialog->setClient(client);
            boxDialog->set(id, boxId, name);
            int r = boxDialog->run();
            if (r == Gtk::RESPONSE_OK || r == Gtk::RESPONSE_REJECT) {
                // reload box tree on edit or remove
                reloadBoxTree();
            }
        }
    }
}

void TopWindow::onBoxNew()
{
    // add a new empty box
    if (client) {
        boxDialog->setClient(client);
        boxDialog->set(0, 0, "");
        int r = boxDialog->run();
        if (r == Gtk::RESPONSE_OK || r == Gtk::RESPONSE_REJECT) {
            // reload box tree on edit or remove
            reloadBoxTree();
        }
    }
}

void TopWindow::onBoxDelete()
{
    // delete selected box
    // edit selected box
    Gtk::TreeModel::iterator iter = mTreeViewSelectionBox->get_selected();
    if (iter) {
        Gtk::TreeModel::Row row = *iter;
        if (client) {
            if (confirmDeleteBox(row)) {
                uint64_t boxid;
                row.get_value(2, boxid);
                if (client->rmBox(boxid)) {
                    // reload box tree
                    reloadBoxTree();
                }
            }
        }
    }
}

void TopWindow::onStartImportDirectory()
{
    uint64_t b = settings->settings.mutable_service(settings->selected)->last_box();
    bool numberInFileName = settings->settings.mutable_service(settings->selected)->number_in_filename();
    if (!confirmBox(b, numberInFileName))
        return;
    settings->settings.mutable_service(settings->selected)->set_number_in_filename(numberInFileName);
    settings->save();
    std::string symbol = settings->settings.service(settings->selected).last_component_symbol();
    COMPONENT c = getComponentBySymbol(symbol);
    std::string cn = MeasureUnit::description(ML_RU, c);

    std::stringstream ss;
    ss << _("Import ") << cn << _(" to box ") << StockOperation::boxes2string(b);
    std::string t = ss.str();

    Gtk::FileChooserDialog dialog(t, Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_transient_for(*this);
    // Add response buttons the the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(_("Import"), Gtk::RESPONSE_OK);
    int result = dialog.run();
    // Handle the response:
    if (result == Gtk::RESPONSE_OK) {
        std::string fn = dialog.get_filename();
        runImportExcel(symbol, fn, b, true, numberInFileName);
    }
}

void TopWindow::runImportExcel(
    const std::string &symbol,
    const std::string &path,
    uint64_t box,
    bool isDirectory,
    bool numberInFileName
) {
    if (box == 0) {
        Gtk::MessageDialog dialog(*this, _("Wrong box specified"));
        dialog.set_secondary_text(path);
        dialog.run();
        return;
    }
    std::thread t([=]() {
        if (isDirectory) {
            client->importDirectory(symbol, path, box, numberInFileName);
        } else {
            client->importFile(symbol, path, box, numberInFileName);
        }
    });
    t.detach();
}

void TopWindow::editUser(rcr::User *user) {
    if (user)
        userDialog->set(settings->settings.user().rights() == 1, *user);
    int r = userDialog->run();
    if (r != Gtk::RESPONSE_OK)
        return;
}

bool TopWindow::confirmDeleteBox(
    Gtk::TreeModel::Row &row
) {
    std::string name;
    row.get_value(0, name);
    std::stringstream ss;
    ss << _("Delete box ") << name << "?";
    Gtk::MessageDialog dlg(ss.str());
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}

void TopWindow::reflectChangesCard() {
    // after card edit reflect changes
    Gtk::TreeModel::iterator iter = mTreeViewSelectionCard->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    uint64_t id = cardWindow->id;
    uint64_t symbolId = cardWindow->symbolId;
    std::string name = cardWindow->refEntryName->get_text();
    std::string sNominal = cardWindow->refEntryNominal->get_text();
    uint64_t nominal = std::strtoull(sNominal.c_str(), nullptr, 10);

    std::string sQty = cardWindow->refEntryQuantity->get_text();
    uint64_t qty = std::strtoull(sQty.c_str(), nullptr, 10);

    uint64_t box = cardWindow->boxId;
    uint64_t packageId = cardWindow->packageId;
    std::string boxName = cardWindow->boxName;
    std::string properties = cardWindow->properties;

    row.set_value(0, name);

    sNominal = MeasureUnit::value(ML_RU, cardWindow->getSelectedComponent(), nominal);
    if (sNominal.find('0') == 0)
        sNominal = "";
    row.set_value(1, sNominal);

    row.set_value(2, properties);
    row.set_value(3, boxName);
    row.set_value(4, qty);
    row.set_value(5, id);
    row.set_value(6, box);
    row.set_value(7, symbolId);
    row.set_value(8, packageId);
}

void TopWindow::reflectRemoveCard()
{
    // after card edit reflect changes
    doQuery();
}
