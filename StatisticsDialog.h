//
// Created by andrei on 18.04.23.
//

#ifndef RCR_GNOME_STATISTICSDIALOG_H
#define RCR_GNOME_STATISTICSDIALOG_H

#include <gtkmm.h>
#include "gen/rcr.pb.h"
#include "GRcrClient.h"
#include "RcrSettings.h"

class StatisticsDialog : public Gtk::Dialog {
private:
    Glib::RefPtr<Gtk::Builder> mRefBuilder;
    Glib::RefPtr<Gio::SimpleActionGroup> mRefActionGroup;
    Gtk::Label *refLabelComponents;
    Gtk::Label *refLabelTotal;
    Gtk::Button *refButtonClose;
    void bindWidgets();
    void onCancel();
public:
    StatisticsDialog(
        BaseObjectType* cobject,
        const Glib::RefPtr<Gtk::Builder>&
    );
    virtual ~StatisticsDialog();

    void setStat(uint64_t componentCount, uint64_t total);
};


#endif //RCR_GNOME_STATISTICSDIALOG_H
