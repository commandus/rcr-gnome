//
// Created by andrei on 06.04.23.
//

#include "CardWindow.h"

#include <gdk/gdkkeysyms.h>
#include <gtkmm.h>
#include <iostream>

CardWindow::CardWindow(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Window(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
    std::cerr << "*** CardWindow ***" << std::endl;
}

CardWindow::~CardWindow()
{

}

void CardWindow::bindWidgets() {
    mRefListStoreComponent = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreSymbol"));
    mRefListStoreMeasure = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreMeasure"));
    mRefListStoreProperty = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreProperty"));

    mRefBuilder->get_widget("cbComponent", mRefCBComponent);
    mRefCBComponent->set_model(mRefListStoreComponent);
    mRefBuilder->get_widget("cbMeasure", mRefCBMeasure);
    mRefBuilder->get_widget("entryName", mRefEntryName);
    mRefBuilder->get_widget("entryBox", mRefEntryBox);
    mRefBuilder->get_widget("entryQuantity", mRefEntryQuantity);
    mRefBuilder->get_widget("bCardSave", mRefButtonCardSave);
    mRefBuilder->get_widget("bCardCancel", mRefButtonCardCancel);
    mRefBuilder->get_widget("bCardRm", mRefButtonCardRm);
    mRefBuilder->get_widget("tvProperty", mRefTreeViewProperty);
    mRefBuilder->get_widget("lNominal", mRefLabelNominal);
    mRefBuilder->get_widget("lName", mRefLabelName);
    mTreeViewSelectionProperty = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsProperty"));

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("cardsave", sigc::mem_fun(*this, &CardWindow::onCardSave));
    mRefActionGroup->add_action("cardrm", sigc::mem_fun(*this, &CardWindow::onCardRm));
    mRefActionGroup->add_action("cardcancel", sigc::mem_fun(*this, &CardWindow::onCardCancel));
}

void CardWindow::onCardSave() {

}

void CardWindow::onCardRm() {

}

void CardWindow::onCardCancel() {
    hide();
}
