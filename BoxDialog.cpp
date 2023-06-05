//
// Created by andrei on 03.05.23.
//

#include "BoxDialog.h"

//
// Created by andrei on 28.04.23.
//

#include "rcr-gnome.h"
#include "StockOperation.h"

BoxDialog::BoxDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder), client(nullptr)
{
    bindWidgets();
}

BoxDialog::~BoxDialog()
{

}

void BoxDialog::bindWidgets() {
    mRefBuilder->get_widget("entryBoxId", entryBoxId);
    mRefBuilder->get_widget("entryBoxName", entryBoxName);

    mRefBuilder->get_widget("bPropertyTypeEditDelete", refButtonBoxDelete);
    mRefBuilder->get_widget("bPropertyTypeEditCancel", refButtonBoxConfirm);
    mRefBuilder->get_widget("bPropertyTypeEditSave", refButtonBoxCancel);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("boxConfirm", sigc::mem_fun(*this, &BoxDialog::onConfirm));
    mRefActionGroup->add_action("boxCancel", sigc::mem_fun(*this, &BoxDialog::onCancel));
    mRefActionGroup->add_action("boxDelete", sigc::mem_fun(*this, &BoxDialog::onDelete));

    insert_action_group("rcr", mRefActionGroup);
}

bool BoxDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            response(Gtk::RESPONSE_OK);
            onConfirm();
            break;
        case GDK_KEY_minus:
            response(Gtk::RESPONSE_REJECT);
            onDelete();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}

void BoxDialog::onDelete()
{
    if (client) {
        if (confirmDelete()) {
            client->rmBox(id);
        }
    }
    hide();
}

void BoxDialog::onConfirm()
{
    if (client) {
        uint64_t boxId = StockOperation::parseBoxes(entryBoxId->get_text());
        client->saveBox(id, srcBoxId, boxId, entryBoxName->get_text());
    }
    hide();
}

void BoxDialog::onCancel()
{
    hide();
}

void BoxDialog::setClient(
    GRcrClient *aClient
)
{
    client = aClient;
}

void BoxDialog::set(
    uint64_t aId,
    uint64_t aBoxId,
    const std::string &aName
)
{
    id = aId;
    srcBoxId = aBoxId;
    std::string bn = StockOperation::boxes2string(aBoxId);
    if (bn == "0")
        bn = "";
    entryBoxId->set_text(bn);
    std::string name;
    if (aName == bn)
        name = "";
    else
        name = aName;
    entryBoxName->set_text(name);
}

bool BoxDialog::confirmDelete() {
    Gtk::MessageDialog dlg(_("Delete box?"));
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}
