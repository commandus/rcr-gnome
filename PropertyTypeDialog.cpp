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

    mTreeViewSelectionPropertyType = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("tvsPropertyType"));
}

bool PropertyTypeDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Delete:
            if (tvPropertyType->has_focus()) {
                Gtk::TreeModel::iterator iter = mTreeViewSelectionPropertyType->get_selected();
                if (iter) {
                    Gtk::TreeModel::Row row = *iter;
                    if (client) {
                        if (confirmDelete(row)) {
                            uint64_t id;
                            row.get_value(0, id);
                            client->rmPropertyType(id);
                            loadPropertyTypes();
                        }
                    }
                }
            }
            break;
        case GDK_KEY_plus:
        case GDK_KEY_KP_Add:
            response(Gtk::RESPONSE_YES);
            onAdd();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}

void PropertyTypeDialog::onPropertyTypeActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    if (!propertyTypeEditDialog)
        return;

    Gtk::TreeModel::iterator iter = mTreeViewSelectionPropertyType->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    uint64_t id;
    std::string key, description;
    row.get_value(0, id);
    std::cerr << "==" << id << std::endl;

    row.get_value(1, key);
    row.get_value(2, description);
    propertyTypeEditDialog->setClient(client, this);
    propertyTypeEditDialog->set(id, key, description);

    hide();
    propertyTypeEditDialog->run();
}

void PropertyTypeDialog::onAdd()
{
    if (!propertyTypeEditDialog)
        return;
    hide();
    propertyTypeEditDialog->setClient(client, this);
    propertyTypeEditDialog->set(0, "", "");
    propertyTypeEditDialog->run();
}

void PropertyTypeDialog::onClose()
{
    hide();
}

void PropertyTypeDialog::setClient(
    GtkWindow *aParent,
    GRcrClient *aClient,
    PropertyTypeEditDialog *aPropertyTypeEditDialog
)
{
    parent = aParent;
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

bool PropertyTypeDialog::confirmDelete(
    Gtk::TreeModel::Row &row
) {
    std::string key, description;
    row.get_value(1, key);
    row.get_value(2, description);
    std::stringstream ss;
    ss << _("Delete ") << key << ":" << description << "?";
    Gtk::MessageDialog dlg(ss.str());
    dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dlg.set_secondary_text(_("Press Ok to delete. This operation Can not be undone"));
    return dlg.run() == GTK_RESPONSE_OK;
}
