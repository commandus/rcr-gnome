//
// Created by andrei on 06.04.23.
//

#include "CardWindow.h"

#include <iostream>

#include "MeasureUnit.h"
#include "StockOperation.h"
#include "string-helper.h"
#include "rcr-gnome.h"
#include "QueryProperties.h"
#include "utilstring.h"

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
    mRefBuilder->get_widget("bPropertyAdd", refButtonPropertyAdd);
    mRefBuilder->get_widget("tvProperty", refTreeViewProperty);

    mRefBuilder->get_widget("lNominal", mRefLabelNominal);
    mRefBuilder->get_widget("lName", mRefLabelName);
    mTreeViewSelectionProperty = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsProperty"));

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("cardsave", sigc::mem_fun(*this, &CardWindow::onCardSave));
    mRefActionGroup->add_action("cardrm", sigc::mem_fun(*this, &CardWindow::onCardRm));
    mRefActionGroup->add_action("cardcancel", sigc::mem_fun(*this, &CardWindow::onCardCancel));
    mRefActionGroup->add_action("addProperty", sigc::mem_fun(*this, &CardWindow::onAddProperty));

    refTreeViewProperty->signal_row_activated().connect(sigc::mem_fun(*this, &CardWindow::onCardActivated), refTreeViewProperty);

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
    it->get_value(1, symbolId);

    std::string name = refEntryName->get_text();
    std::string sNominal = refEntryNominal->get_text();
    uint64_t nominal = std::strtoull(sNominal.c_str(), nullptr, 10);

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

bool CardWindow::confirmDeleteCard() {
    std::stringstream ss;
    ss << _("Delete card ") << boxName << "?";
    Gtk::MessageDialog dlg(ss.str());
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}

void CardWindow::onCardRm()
{
    if (confirmDeleteCard())
    {
        rcr::Card card;
        card.set_id(id);
        client->rmCardPackage(card, packageId);
    }
    hide();
}

void CardWindow::onCardCancel()
{
    hide();
}

void CardWindow::onAddProperty()
{
    // show dialog
    if (!propertyDialog)
        return;
    propertyDialog->setClient(client);
    propertyDialog->set("", "");
    int r = propertyDialog->run();
    if (r != Gtk::RESPONSE_OK)
        return;
    // add property
    setProperty(propertyDialog->key(), propertyDialog->value());
}

void CardWindow::onCardActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    editProperty();
}

void CardWindow::editProperty()
{
    Gtk::TreeModel::iterator iter = mTreeViewSelectionProperty->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    uint64_t id;
    std::string key;
    std::string value;
    row.get_value(0, id);
    row.get_value(1, key);
    row.get_value(2, value);
    // show dialog
    if (!propertyDialog)
        return;
    propertyDialog->setClient(client);
    propertyDialog->set(key, value);
    int r = propertyDialog->run();
    if (r != Gtk::RESPONSE_OK)
        return;
    // add property
    setProperty(propertyDialog->key(), propertyDialog->value());
}

/**
 * Change component type
 * Reflect in UI nominal and measurement unit widgets
 */
void CardWindow::onSymbolSelected() {
    std::string sym;
    COMPONENT c = getSelectedComponent();
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

bool CardWindow::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Escape:
            hide();
            break;
        case GDK_KEY_Delete:
        case GDK_KEY_minus:
        case GDK_KEY_KP_Subtract:
            onCardRm();
            break;
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            onCardSave();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}

COMPONENT CardWindow::getSelectedComponent() {
    std::string sym;
    refCBSymbol->get_active()->get_value(2, sym);
    return getComponentBySymbol(sym);
}

void CardWindow::selectSymbolId(
    uint64_t symbolId
) {
    auto children = refCBSymbol->get_model()->children();
    auto c = 0;
    for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
        uint64_t id;
        iter->get_value(1, id);
        if (id == symbolId)
            break;
        c++;
    }
    refCBSymbol->set_active(c);
}

void CardWindow::setCard(
    uint64_t aId,
    bool aIsNew,
    uint64_t aSymbolId,
    const std::string &aName,
    const std::string &aNominal,
    uint64_t aQty,
    uint64_t aPackageId,
    uint64_t aBoxId,
    const std::string &aBoxName,
    const std::string &aProperties
)
{
    id = aId;
    isNew = aIsNew;
    selectSymbolId(aSymbolId);
    refEntryName->set_text(aName);
    refEntryNominal->set_text(aNominal);
    refEntryQuantity->set_text(std::to_string(aQty));

    packageId = aPackageId;
    boxId = aBoxId;
    boxName = aBoxName;
    refEntryBox->set_text(StockOperation::boxes2string(aBoxId));
    properties = aProperties;
    listProperties();
}

void CardWindow::listProperties()
{
    mRefListStoreProperty->clear();
    std::map <std::string, std::string> mP;
    size_t pp = 0;
    QueryProperties::parse(properties, pp, mP);
    for (auto mit = mP.begin(); mit != mP.end(); mit++) {
        const Gtk::TreeModel::iterator &p = mRefListStoreProperty->append();
        p->set_value(1, mit->first);
        p->set_value(2, mit->second);
    }
}

void CardWindow::setProperty(
    const std::string &key,
    const std::string &value
) {
    std::map <std::string, std::string> mP;
    size_t pp = 0;
    QueryProperties::parse(properties, pp, mP);
    std::string v(value);
    v = trim(v);
    std::replace_if(v.begin(), v.end(), ::isspace, '_');
    if (!v.empty())
        mP[key] = v;
    properties = QueryProperties::toString(mP);
    listProperties();
}
