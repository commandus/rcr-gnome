//
// Created by andrei on 28.04.23.
//

#include "PropertyTypeEditDialog.h"
#include "PropertyTypeDialog.h"
#include "rcr-gnome.h"

PropertyTypeEditDialog::PropertyTypeEditDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder), client(nullptr)
{
    bindWidgets();
}

PropertyTypeEditDialog::~PropertyTypeEditDialog()
{

}

void PropertyTypeEditDialog::bindWidgets() {
    mRefBuilder->get_widget("entryPropertyTypeEditKey", entryPropertyTypeEditKey);
    mRefBuilder->get_widget("entryPropertyTypeEditDescription", entryPropertyTypeEditDescription);

    mRefBuilder->get_widget("bPropertyTypeEditDelete", refButtonPropertyTypeEditDelete);
    mRefBuilder->get_widget("bPropertyTypeEditCancel", refButtonPropertyTypeEditCancel);
    mRefBuilder->get_widget("bPropertyTypeEditSave", refButtonPropertyTypeEditSave);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("propertyTypeEditSave", sigc::mem_fun(*this, &PropertyTypeEditDialog::onSave));
    mRefActionGroup->add_action("propertyTypeEditCancel", sigc::mem_fun(*this, &PropertyTypeEditDialog::onCancel));
    mRefActionGroup->add_action("propertyTypeEditDelete", sigc::mem_fun(*this, &PropertyTypeEditDialog::onDelete));

    insert_action_group("rcr", mRefActionGroup);
}

bool PropertyTypeEditDialog::on_key_press_event(GdkEventKey* event)
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

void PropertyTypeEditDialog::onDelete()
{
    if (client) {
        if (confirmDelete()) {
            client->rmPropertyType(id);
            if (propertyTypeDialog)
                propertyTypeDialog->loadPropertyTypes();
        }
    }
    hide();
}

void PropertyTypeEditDialog::onSave()
{
    if (client) {
        client->savePropertyType(id, entryPropertyTypeEditKey->get_text(), entryPropertyTypeEditDescription->get_text());
        if (propertyTypeDialog)
            propertyTypeDialog->loadPropertyTypes();
    }
    hide();
}

void PropertyTypeEditDialog::onCancel()
{
    hide();
}

void PropertyTypeEditDialog::setClient(
    GRcrClient *aClient,
    PropertyTypeDialog *aPropertyTypeDialog
)
{
    client = aClient;
    propertyTypeDialog = aPropertyTypeDialog;
}

void PropertyTypeEditDialog::set(
    uint64_t aId,
    const std::string &key,
    const std::string &description
)
{
    id = aId;
    entryPropertyTypeEditKey->set_text(key);
    entryPropertyTypeEditDescription->set_text(description);
}

bool PropertyTypeEditDialog::confirmDelete() {
    Gtk::MessageDialog dlg(_("Delete property?"));
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}
