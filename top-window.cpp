#include <iostream>
#include <sstream>
#include <gdk/gdkkeysyms.h>

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

TopWindow::TopWindow()
{
}

TopWindow::TopWindow (BaseObjectType *cobject, const Glib::RefPtr<Gtk::Builder> &refBuilder)
	: Gtk::Window(cobject), mRefBuilder (refBuilder)
{
	mRefBuilder->get_widget("searchEntry", mEntryMessage);

	mRefActionGroup = Gio::SimpleActionGroup::create();
	mRefActionGroup->add_action("quit",
		sigc::mem_fun(*this, &TopWindow::onFileQuit));
	mRefActionGroup->add_action("about",
		sigc::mem_fun(*this, &TopWindow::onHelpAbout));
	insert_action_group("wpn", mRefActionGroup);
 
	mRefBuilder->get_widget("treeviewBox", mTreeViewClient);

	mRefListStoreClient = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreClient"));
	mRefListStoreMessage = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreMessage"));

	mRefTreeModelFilterMessage = Gtk::TreeModelFilter::create(mRefListStoreMessage);
	mRefTreeModelFilterMessage->set_visible_func(
	[this] (const Gtk::TreeModel::const_iterator& it) -> bool
	{
		Gtk::TreeModel::Row row = *it;
		Glib::ustring from;
		row.get_value(3, from);
		Gtk::TreeModel::iterator iter = mTreeViewSelectionClient->get_selected();
		if (iter) {
			Gtk::TreeModel::Row row = *iter;
			Glib::ustring key;
			row.get_value(2, key);
			return key == from || key == "*";
		}
		return true;
	});
	
	// mTreeViewMessage->set_model(mRefTreeModelFilterMessage);

	mTreeViewSelectionClient = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("treeviewSelectionClient"));
	mTreeViewSelectionClient->signal_changed().connect(
			sigc::bind <Glib::RefPtr<Gtk::TreeSelection>> (sigc::mem_fun(*this, &TopWindow::onClientSelected), mTreeViewSelectionClient));

	mTreeViewSelectionMessage = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("treeviewSelectionMessage"));
	mTreeViewSelectionMessage->signal_changed().connect(
			sigc::bind <Glib::RefPtr<Gtk::TreeSelection>> (sigc::mem_fun(*this, &TopWindow::onMessageSelected), mTreeViewSelectionMessage));
	
	add_events(Gdk::KEY_PRESS_MASK);

	mFileFilterWPN = Gtk::FileFilter::create();
	mFileFilterWPN->set_name("wpn client files");
	mFileFilterWPN->add_mime_type("application/javascript");
}

/**
 * Columns:
 * 0  name
 * 1  id
 * 2  publicKey
 */
void TopWindow::onClientSelected(
	Glib::RefPtr<Gtk::TreeSelection> selection
) {
	// reload message tree view
	mRefTreeModelFilterMessage->refilter();
}

/**
 * Columns:
 * 0  message
 * 1  msg NULL
 * 2  persistent_id
 * 3  from
 * 4  appName
 * 5  appId
 * 6  sent
 * 7  title
 * 8  icon
 * 9  sound
 * 10 link
 * 11 linkType
 * 12 category
 * 13 extra
 * 14 data
 * 15 urgency
 * 16 timeout
 * 17 fromId
 * 18 fromName
 */
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

void TopWindow::onButtonClickSend() {
	// get message text to be send
	std::string v = "";
	if (mEntryMessage) {
		v = mEntryMessage->get_text();
	}
	// nothing to do
	if (v.empty())
		return;
	// clear edit control
	mEntryMessage->set_text("");
	// smth wrong
	if (!mRefListStoreMessage)
		return;
	Gtk::TreeModel::Row row = *mRefListStoreMessage->append();
	row.set_value <Glib::ustring>(0, v); 
	// row.set_value <Glib::rustring>(1, msg);
}

TopWindow::~TopWindow() {
}

/**
void TopWindow::loadClients(
	Glib::RefPtr<Gtk::ListStore> listStore,
	ConfigFile *config
) 
{
	if (!listStore)
		return;
	listStore->clear();
	if (!config)
		return;

	Gtk::TreeModel::Row row = *listStore->append();
	row.set_value <Glib::ustring>(0, DEF_NAME_ALL);
	row.set_value(1, 0);
	row.set_value <Glib::ustring>(2, "*");

	for (std::vector<Subscription>::const_iterator it(config->subscriptions->list.begin()); it!= config->subscriptions->list.end(); ++it) {
		Gtk::TreeModel::Row row = *listStore->append();
		std::string name;
		name = it->getName();
		if (name.empty())
			name = "noname";
		std::stringstream n;
		n << it->getWpnKeys().id << "(" << name << ")";
		row.set_value <Glib::ustring>(0, n.str());
		guint64 id = it->getWpnKeys().id;
		row.set_value(1, id);
		row.set_value <Glib::ustring>(2, it->getWpnKeys().getPublicKey());
	}
}
 */

bool TopWindow::on_key_press_event(GdkEventKey* event)
{
	switch (event->keyval)
	{
		case GDK_KEY_Return:
			this->onButtonClickSend();
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
	mAboutDialog->set_logo(Gdk::Pixbuf::create_from_resource("/../glade/ic_launcher.png", -1, 40, true));
	mAboutDialog->set_program_name("wpn for Linux desktop");
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

/**
 * 
 * https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c
 */
inline bool fileExists
(
	const std::string& fn
) {
	struct stat buffer;   
	return (stat (fn.c_str(), &buffer) == 0); 
}

// libwpnpp.a
std::string getDefaultConfigFileName(const std::string &filename);

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
