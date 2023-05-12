//
// Created by andrei on 12.05.23.
//

#include "PropertyDialog.h"

#include "rcr-gnome.h"

PropertyDialog::PropertyDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder), client(nullptr)
{
    bindWidgets();
}

PropertyDialog::~PropertyDialog()
{

}

void PropertyDialog::bindWidgets() {
    mRefBuilder->get_widget("cbPropertyKey", cbPropertyKey);
    mRefBuilder->get_widget("entryPropertyValue", entryPropertyValue);

    mRefBuilder->get_widget("bPropertyDelete", refButtonPropertyDelete);
    mRefBuilder->get_widget("bPropertySave", refButtonPropertySave);
    mRefBuilder->get_widget("bPropertyCancel", refButtonPropertyCancel);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("propertySave", sigc::mem_fun(*this, &PropertyDialog::onSave));
    mRefActionGroup->add_action("propertyCancel", sigc::mem_fun(*this, &PropertyDialog::onCancel));
    mRefActionGroup->add_action("propertyDelete", sigc::mem_fun(*this, &PropertyDialog::onDelete));

    insert_action_group("rcr", mRefActionGroup);
}

bool PropertyDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            response(Gtk::RESPONSE_OK);
            onSave();
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

void PropertyDialog::onDelete()
{
    if (client) {
        if (confirmDelete()) {
            // TODO
        }
    }
    hide();
}

void PropertyDialog::onSave()
{
    if (client) {
        // TODO
    }
    hide();
}

void PropertyDialog::onCancel()
{
    hide();
}

void PropertyDialog::setClient(
    GRcrClient *aClient
)
{
    client = aClient;
}

void PropertyDialog::set(
    const std::string &key,
    const std::string &value
)
{
    entryPropertyValue->set_text(value);
    selectPropertyKey(key);
}

void PropertyDialog::selectPropertyKey(
    const std::string &key
) {
    auto children = cbPropertyKey->get_model()->children();
    auto c = 0;
    for (auto iter = children.begin(), end = children.end(); iter != end; ++iter) {
        std::string k;
        iter->get_value(1, k);
        if (k == key)
            break;
        c++;
    }
    cbPropertyKey->set_active(c);
}

bool PropertyDialog::confirmDelete() {
    Gtk::MessageDialog dlg(_("Delete property?"));
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}
