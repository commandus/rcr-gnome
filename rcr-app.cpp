#include "rcr-app.h"
#include "top-window.h"
#include <iostream>

RcrApplication::RcrApplication()
	: Gtk::Application("rcr.gnome")
{
	Glib::set_application_name("rcr");
}

Glib::RefPtr<RcrApplication> RcrApplication::create()
{
	return Glib::RefPtr<RcrApplication>(new RcrApplication());
}
	
void RcrApplication::on_startup()
{
	// Call the base class's implementation:
	Gtk::Application::on_startup();
	mRefBuilder = Gtk::Builder::create();
	try {
		// remove resource.c, make or rebuild resource.c manually:
		// glib-compile-resources rcr-gnome.resource.xml --target=resources.c --generate-source
		mRefBuilder->add_from_resource(RES_GLADE);
	} catch (const Glib::Error& ex) {
		std::cerr << _("Building menus failed: ") << ex.what();
	}
}

void RcrApplication::on_activate()
{
	createTopWindow();
}

void RcrApplication::createTopWindow()
{
	// Load the Glade file and instate its widgets:
	TopWindow* topWindow = nullptr;
	mRefBuilder->get_widget_derived("topWindow", topWindow);

    GtkIconTheme *icon_theme = gtk_icon_theme_get_default();
    gtk_icon_theme_add_resource_path(icon_theme, "/icons");
#if CMAKE_BUILD_TYPE == Debug
    /*
    // Print list of available icons
    GList *iconList = gtk_icon_theme_list_icons(icon_theme, nullptr);
    for (GList *l = iconList; l != nullptr; l = l->next) {
        std::cout << (char *) l->data << std::endl;
    }
    g_list_free(iconList);
    if (!gtk_icon_theme_has_icon(icon_theme,"rcr.gnome")) {
        std::cerr << "Icon nof found" << std::endl;
    }
     */
#endif
    gtk_window_set_default_icon_name("rcr.gnome");
    topWindow->set_icon_name("rcr.gnome");

	// Make sure that the application runs for as long this window is still open:
	add_window(*topWindow);
	// Delete the window when it is hidden.
	topWindow->signal_hide().connect(sigc::bind<Gtk::Window*>(
	    sigc::mem_fun(*this, &RcrApplication::onWindowHide), topWindow));
	topWindow->show_all();
}

void RcrApplication::onWindowHide(Gtk::Window* window)
{
    ((TopWindow*) window)->saveSettings();
	delete window;
}
