#include "rcr-app.h"
#include "top-window.h"
#include <iostream>

RcrApplication::RcrApplication()
	: Gtk::Application("com.commandus.rcr")
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
		std::cerr << "Building menus failed: " << ex.what();
	}
}

void RcrApplication::on_activate()
{
	createTopWindow();
}

void RcrApplication::createTopWindow()
{
	// Load the Glade file and instiate its widgets:
	TopWindow* topWindow = 0;
	mRefBuilder->get_widget_derived("topWindow", topWindow);

	// Make sure that the application runs for as long this window is still open:
	add_window(*topWindow);
	// Delete the window when it is hidden.
	topWindow->signal_hide().connect(sigc::bind<Gtk::Window*>(
	sigc::mem_fun(*this, &RcrApplication::onWindowHide), topWindow));
	topWindow->show_all();
}

void RcrApplication::onWindowHide(Gtk::Window* window)
{
	delete window;
}
