#include <iostream>
#include <sstream>
#include <gdk/gdkkeysyms.h>
#include <gtkmm.h>

#include "top-window.h"

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

/**
 *
 * @param cobject
 * @param refBuilder
 */
TopWindow::TopWindow (BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder)
	: client(nullptr), mLastSymbol("D"),
    Gtk::Window(cobject), mRefBuilder (refBuilder)
{
    mRefBuilder->get_widget("bRefresh", mButtonRefresh);
	mRefBuilder->get_widget("entrySearch", mEntryQuery);
    mRefBuilder->get_widget("entryHost", mEntryHost);
    mRefBuilder->get_widget("cbSymbol", mComboBoxSymbol);
    mComboBoxSymbol->signal_changed().connect(sigc::mem_fun(*this, &TopWindow::onSymbolSelected));
    mButtonRefresh->signal_clicked().connect(sigc::mem_fun(*this, &TopWindow::onRefresh));

	mRefActionGroup = Gio::SimpleActionGroup::create();
	mRefActionGroup->add_action("quit",
		sigc::mem_fun(*this, &TopWindow::onFileQuit));

	mRefActionGroup->add_action("about",
		sigc::mem_fun(*this, &TopWindow::onHelpAbout));
	insert_action_group("rcr", mRefActionGroup);
 
	mRefBuilder->get_widget("tvBox", mTreeViewBox);
    mRefBuilder->get_widget("tvCard", mTreeViewCard);

    mRefListStoreSymbol = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreSymbol"));
    mRefListStoreBox = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreBox"));
    mRefListStoreCard = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreCard"));

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
			return box == boxId || box == 0;
		}
		return true;
	});
	mTreeViewCard->set_model(mRefTreeModelFilterCard);

	mTreeViewSelectionBox = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsBox"));
	mTreeViewSelectionBox->signal_changed().connect(
			sigc::bind <Glib::RefPtr<Gtk::TreeSelection>> (sigc::mem_fun(*this, &TopWindow::onBoxSelected), mTreeViewSelectionBox));

	add_events(Gdk::KEY_PRESS_MASK);

    mFileFilterXLSX = Gtk::FileFilter::create();
	mFileFilterXLSX->set_name("Excel xlsx");
	mFileFilterXLSX->add_mime_type("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet");
}

void TopWindow::onBoxSelected(
	Glib::RefPtr<Gtk::TreeSelection> selection
) {
	// reload message tree view
	mRefTreeModelFilterCard->refilter();
}

void TopWindow::onMessageSelected(
	Glib::RefPtr<Gtk::TreeSelection> selection
) {
	if (selection) {
		Gtk::TreeModel::iterator iter = selection->get_selected();
		if (iter) {
			Gtk::TreeModel::Row row = *iter;
			Glib::ustring v;
			row.get_value(0, v);
			Glib::ustring fromName;
			row.get_value(18, fromName);
			guint64 fromId;
			row.get_value(17, fromId);
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
	switch (event->keyval)
	{
		case GDK_KEY_Return:
			// TODO send
            searchCard(mEntryQuery->get_text(), mLastSymbol);
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

void TopWindow::onRefresh() {
    std::string host = mEntryHost->get_text();
    if (client)
        delete client;
    client = new GRcrClient(host);

    mComboBoxSymbol->unset_model();
    mTreeViewBox->unset_model();
    client->loadSymbols(mRefListStoreSymbol);
    mComboBoxSymbol->set_model(mRefListStoreSymbol);

    selectSymbol(mLastSymbol);
    client->loadBoxes(mRefListStoreBox);
    mTreeViewBox->set_model(mRefListStoreBox);
}

void TopWindow::selectSymbol(
    const std::string &symbol
) {
    auto children = mRefListStoreSymbol->children();
    auto c = 0;
    for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
        std::string sym;
        iter->get_value(2, sym);
        std::cerr << sym << std::endl;
        if (sym == symbol)
            break;
        c++;
    }
    mComboBoxSymbol->set_active(c);
}

void TopWindow::onSymbolSelected() {
    std::string sym;
    mComboBoxSymbol->get_active()->get_value(2, sym);
    mLastSymbol = sym;
}

void TopWindow::searchCard(
    const std::string &q,
    const std::string &symbol
)
{
    mTreeViewCard->unset_model();
    client->query(q, symbol, mRefListStoreCard);
    mTreeViewCard->set_model(mRefListStoreCard);
}
