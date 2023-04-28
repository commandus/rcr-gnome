//
// Created by andrei on 28.04.23.
//

#include "PropertyTypeDialog.h"
#include "rcr-gnome.h"

PropertyTypeDialog::PropertyTypeDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder), client(nullptr), propertyTypeEditDialog(nullptr)
{
    bindWidgets();
}

PropertyTypeDialog::~PropertyTypeDialog()
{

}

void PropertyTypeDialog::bindWidgets() {
    mRefListstorePropertyType = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststorePropertyType"));

    mRefBuilder->get_widget("tvPropertyType", tvPropertyType);
    tvPropertyType->signal_row_activated().connect(sigc::mem_fun(*this, &PropertyTypeDialog::onPropertyTypeActivated), tvPropertyType);

    mRefBuilder->get_widget("bPropertyTypeAdd", refButtonPropertyTypeAdd);
    mRefBuilder->get_widget("bPropertyTypeClose", refButtonPropertyTypeClose);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("propertyTypeAdd", sigc::mem_fun(*this, &PropertyTypeDialog::onAdd));
    mRefActionGroup->add_action("propertyTypeClose", sigc::mem_fun(*this, &PropertyTypeDialog::onClose));

    insert_action_group("rcr", mRefActionGroup);
}

void PropertyTypeDialog::onPropertyTypeActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    if (!propertyTypeEditDialog)
        return;
    propertyTypeEditDialog->run();
    hide();
}

void PropertyTypeDialog::onAdd()
{
    if (!propertyTypeEditDialog)
        return;
    propertyTypeEditDialog->run();
    hide();
}

void PropertyTypeDialog::onClose()
{
    hide();
}

void PropertyTypeDialog::setClient(
    GRcrClient *aClient,
    PropertyTypeEditDialog *aPropertyTypeEditDialog
)
{
    client = aClient;
    propertyTypeEditDialog = aPropertyTypeEditDialog;
    loadPropertyTypes();
}

void PropertyTypeDialog::loadPropertyTypes()
{
    if (!client)
        return;
    // before do it, unbind GUI elements first
    mRefListstorePropertyType->clear();
    for (auto it = client->dictionaries.property_type().begin(); it != client->dictionaries.property_type().end(); ++it) {
        Gtk::TreeModel::Row row = *mRefListstorePropertyType->append();
        row.set_value(0, it->id());
        row.set_value(1, it->key());
        row.set_value(2, it->description());
    }
}
