#include <gdk/gdkkeysyms.h>
#include <gtkmm.h>

#include "top-window.h"
#include "RCQuery.h"
#include "string-helper.h"
#include "utilfile.h"

#define DEF_NAME_ALL	"All"

#define MIT_LICENSE "Copyright (c) 2023 andrey.ivanov@ikfia.ysn.ru Yu.G. Shafer \n\n\
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
THE SOFTWARE."

void TopWindow::bindWidgets() {
    mRefBuilder->get_widget("entryQuery", mEntryQuery);
    mRefBuilder->get_widget("cbSymbol", mComboBoxSymbol);

    mComboBoxSymbol->signal_changed().connect(sigc::mem_fun(*this, &TopWindow::onSymbolSelected));
    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("connect",
                                sigc::mem_fun(*this, &TopWindow::onFileConnect));
    mRefActionGroup->add_action("quit",
                                sigc::mem_fun(*this, &TopWindow::onFileQuit));
    mRefActionGroup->add_action("about",
                                sigc::mem_fun(*this, &TopWindow::onHelpAbout));
    insert_action_group("rcr", mRefActionGroup);

    mRefBuilder->get_widget("tvBox", mTreeViewBox);

    mRefBuilder->get_widget("tvCard", mTreeViewCard);

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
    createCardWindow();
    bindWidgets();
    // 0 name 1 boxname 2 properties 3 id 4 qty 5 rem 6 boxid
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
	mFileFilterXLSX->set_name("Excel xlsx");
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
        // doQery or reload card tree view if not- uncomment below line
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
	mAboutDialog->set_transient_for(*this);
	mAboutDialog->set_logo(Gdk::Pixbuf::create_from_resource("/ic_launcher.png", -1, 40, true));
	mAboutDialog->set_program_name("rcr for Gnome");
	std::string v;
#ifdef HAVE_CONFIG_H
	v = PACKAGE_STRING;
#else	
	v = "Custom build";
#endif
	mAboutDialog->set_version(v);
	mAboutDialog->set_copyright("2023 Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences");
	mAboutDialog->set_comments("rcr Gnome client");
	mAboutDialog->set_license(MIT_LICENSE);

	mAboutDialog->set_website("https://ikfia.ysn.ru/");
	mAboutDialog->set_website_label("ikfia.ysn.ru");

	std::vector<Glib::ustring> list_authors;
	list_authors.push_back("Andrei Ivanov andrey.ivanov@ikfia.ysn.ru");
	mAboutDialog->set_authors(list_authors);

	mAboutDialog->signal_response().connect(
		sigc::mem_fun(*this, &TopWindow::onAboutDialogResponse));

	mAboutDialog->show();
	mAboutDialog->present();
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

    mComboBoxSymbol->unset_model();
    mTreeViewBox->unset_model();
    client->loadSymbols(mRefListStoreSymbol);
    mComboBoxSymbol->set_model(mRefListStoreSymbol);

    selectSymbol(settings->settings.service(settings->selected).last_component_symbol());
    client->loadBoxes(mRefTreeStoreBox);
    mTreeViewBox->set_model(mRefTreeStoreBox);
    mTreeViewBox->expand_all();

    selectBox(settings->settings.service(settings->selected).last_box());
}

void TopWindow::selectSymbol(
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
    mComboBoxSymbol->set_active(c);
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
            std::cerr << "Selected box " << StockOperation::boxes2string(boxId) << std::endl;
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

void TopWindow::doQuery() {
    searchCard(mEntryQuery->get_text(), settings->settings.service(settings->selected).last_component_symbol());
}

void TopWindow::createCardWindow() {
    mRefBuilder->get_widget_derived("cardWindow", cardWindow);
    cardWindow->signal_hide().connect(sigc::bind<Gtk::Window *>(
            sigc::mem_fun(*this, &TopWindow::onHideCardWindow), cardWindow));
    // cardWindow->set_modal();
}

void TopWindow::onHideCardWindow(Gtk::Window *window) {
    Gtk::TreeModel::iterator iter = mTreeViewSelectionCard->get_selected();
    if (!iter)
        return;
    // update
    // show main window
}

void TopWindow::editCard() {
    Gtk::TreeModel::iterator iter = mTreeViewSelectionCard->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    std::string name, nominal, properties, boxname;
    uint64_t qty, id, box;
    row.get_value(0, name);
    row.get_value(1, nominal);
    row.get_value(2, properties);
    row.get_value(3, boxname);
    row.get_value(4, qty);
    row.get_value(5, id);
    row.get_value(6, box);
    editCard(name, nominal, properties, boxname, qty, id, box, false);
}

void TopWindow::editCard(
    const std::string &name,
    const std::string &nominal,
    const std::string &properties,
    const std::string &boxName,
    uint64_t qty,
    uint64_t id,
    uint64_t boxId,
    bool isNew
) {
    cardWindow->show_all();
}

void TopWindow::onCardActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    editCard();
}
