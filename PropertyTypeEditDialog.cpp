//
// Created by andrei on 28.04.23.
//

#include "PropertyTypeEditDialog.h"
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

    mRefBuilder->get_widget("bPropertyTypeEditCancel", refButtonPropertyTypeEditCancel);
    mRefBuilder->get_widget("bPropertyTypeEditSave", refButtonPropertyTypeEditSave);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("propertyTypeEditSave", sigc::mem_fun(*this, &PropertyTypeEditDialog::onSave));
    mRefActionGroup->add_action("propertyTypeEditCancel", sigc::mem_fun(*this, &PropertyTypeEditDialog::onCancel));

    insert_action_group("rcr", mRefActionGroup);
}

void PropertyTypeEditDialog::onSave()
{
    hide();
}

void PropertyTypeEditDialog::onCancel()
{
    hide();
}

void PropertyTypeEditDialog::setClient(
    GRcrClient *aClient
)
{
    client = aClient;
}
