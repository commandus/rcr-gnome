//
// Created by andrei on 06.04.23.
//

#include "CardWindow.h"

#include <iostream>

#include "MeasureUnit.h"
#include "StockOperation.h"
#include "string-helper.h"

CardWindow::CardWindow(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Window(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
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
    if (!client)
        return;
    std::string sBid = refEntryBox->get_text();
    StockOperation::parseBoxes(boxId, sBid, 0, sBid.size());
    Gtk::TreeModel::iterator it = refCBSymbol->get_active();
    if (!it)
        return;
    // get symbol
    uint64_t symbolId;
    std::string sym;
    it->get_value(1, symbolId);
    it->get_value(2, sym);

    std::string name = refEntryName->get_text();
    std::string sNominal = refEntryNominal->get_text();
    uint64_t nominal = std::strtoull(sNominal.c_str(), nullptr, 10);

    // properties
    std::string properties;
    //getProperties(properties);

    std::string sQty = refEntryQuantity->get_text();
    uint64_t qty = std::strtoull(sQty.c_str(), nullptr, 10);
    std::string boxName = refEntryBox->get_text();

    rcr::Card card;
    card.set_id(id);
    card.set_symbol_id(symbolId);
    card.set_name(name);
    card.set_uname(toUpperCase(name));
    card.set_nominal(nominal);

    client->updateCardPackage(isNew, card, packageId, properties, boxId, boxName, qty);
    hide();
}

void CardWindow::onCardRm() {
    rcr::Card card;
    card.set_id(id);
    client->rmCardPackage(card, packageId);
    hide();
}

void CardWindow::onCardCancel() {
    hide();
}

/**
 * Change component type
 */
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

void CardWindow::setBox(
    uint64_t aPackageId,
    uint64_t aBoxId,
    const std::string &aName,
    const std::string &propertiesString
) {
    packageId = aPackageId;
    boxId = aBoxId;
    name = aName;
    refEntryBox->set_text(StockOperation::boxes2string(boxId));
}
