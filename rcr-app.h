#include <gtk/gtk.h>
#include <gtkmm.h>

#define RES_GLADE				"/rcr-gnome.glade"

class RcrApplication : public Gtk::Application
{
protected:
	RcrApplication();
public:
	static Glib::RefPtr<RcrApplication> create();
protected:
	void on_startup() override;
	void on_activate() override;
private:
	Glib::RefPtr<Gtk::Builder> mRefBuilder;
	void createTopWindow();
	void onWindowHide(Gtk::Window* window);
};
