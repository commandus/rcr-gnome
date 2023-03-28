#ifndef TOPWINDOW_H
#define	TOPWINDOW_H

#include <functional>
#include <gtkmm.h>


using namespace std::placeholders; // for `_1`

class TopWindow: public Gtk::Window {
public:
	TopWindow();
	TopWindow(BaseObjectType*, const Glib::RefPtr<Gtk::Builder>&);
	virtual ~TopWindow();
protected:
	void onButtonClickSend();
	bool on_key_press_event(GdkEventKey *event) override;
	void onHelpAbout();
	void onFileQuit();
	void onClientSelected(Glib::RefPtr<Gtk::TreeSelection> selection);
	void onMessageSelected(Glib::RefPtr<Gtk::TreeSelection> selection);
	Glib::RefPtr<Gtk::Builder> mRefBuilder;
	Gtk::Entry *mEntryMessage;
	Gtk::TreeView *mTreeViewClient;
	Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionClient;
	Glib::RefPtr<Gtk::TreeSelection> mTreeViewSelectionMessage;
	Glib::RefPtr<Gtk::ListStore> mRefListStoreClient;
	Glib::RefPtr<Gtk::ListStore> mRefListStoreMessage;
	Glib::RefPtr<Gtk::TreeModelFilter> mRefTreeModelFilterMessage;
	Gtk::AboutDialog *mAboutDialog;
	void onAboutDialogResponse(int responseId);
private:
	Glib::RefPtr<Gtk::FileFilter> mFileFilterWPN;
	Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
};

#endif
