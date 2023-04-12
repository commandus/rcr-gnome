//
// Created by andrei on 06.04.23.
//

#include "CardWindow.h"

#include <iostream>

#include "MeasureUnit.h"

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

    mRefBuilder->get_widget("cbComponent", refCBSymbol);
    refCBSymbol->signal_changed().connect(sigc::mem_fun(*this, &CardWindow::onSymbolSelected));

    refCBSymbol->set_model(mRefListStoreComponent);
    mRefBuilder->get_widget("cbMeasure", refCBMeasure);
    mRefBuilder->get_widget("entryName", refEntryName);
    mRefBuilder->get_widget("entryBox", refEntryBox);
    mRefBuilder->get_widget("entryNominal", refEntryNominal);
    mRefBuilder->get_widget("entryQuantity", refEntryQuantity);
    mRefBuilder->get_widget("bCardSave", mRefButtonCardSave);
    mRefBuilder->get_widget("bCardCancel", mRefButtonCardCancel);
    mRefBuilder->get_widget("bCardRm", mRefButtonCardRm);
    mRefBuilder->get_widget("tvProperty", refTreeViewProperty);
    mRefBuilder->get_widget("lNominal", mRefLabelNominal);
    mRefBuilder->get_widget("lName", mRefLabelName);
    mTreeViewSelectionProperty = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsProperty"));

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("cardsave", sigc::mem_fun(*this, &CardWindow::onCardSave));
    mRefActionGroup->add_action("cardrm", sigc::mem_fun(*this, &CardWindow::onCardRm));
    mRefActionGroup->add_action("cardcancel", sigc::mem_fun(*this, &CardWindow::onCardCancel));

    insert_action_group("rcr", mRefActionGroup);
}

void CardWindow::onCardSave() {

}

void CardWindow::onCardRm() {

}

void CardWindow::onCardCancel() {
    hide();
}

void CardWindow::onSymbolSelected() {
    std::string sym;
    refCBSymbol->get_active()->get_value(2, sym);
    COMPONENT c = getComponentBySymbol(sym);
    std::vector<std::string> measureNParticles;
    measureNParticles.reserve(MAX_POW10);
    listUnitNParticle(measureNParticles, ML_RU, c);

    mRefListStoreMeasure->clear();
    if (measureNParticles.empty()) {
        refCBMeasure->set_visible(false);
        refEntryNominal->set_visible(false);
        return;
    }
    refCBMeasure->set_visible(true);
    refEntryNominal->set_visible(true);
    int id = 0;
    for (auto m = measureNParticles.begin(); m != measureNParticles.end(); m++) {
        Gtk::TreeModel::Row row = *mRefListStoreMeasure->append();
        row.set_value(0, *m);
        row.set_value(1, MeasureUnit::sym(c));
        id++;
        row.set_value(2, id);
    }
    refCBMeasure->set_active(0);
}
