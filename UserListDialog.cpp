//
// Created by andrei on 17.04.23.
//

#include "UserListDialog.h"

UserListDialog::UserListDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

UserListDialog::~UserListDialog()
{

}

void UserListDialog::bindWidgets() {
    mRefBuilder->get_widget("buttonUserListAdd", refButtonAdd);
    mRefBuilder->get_widget("buttonUserListRemove", refButtonRemove);
    mRefBuilder->get_widget("buttonUserListBoxes", refButtonBoxes);
    mRefBuilder->get_widget("buttonUserListClose", refButtonCancel);

    mRefBuilder->get_widget("treeviewUsers", refTreeViewUser);
    refTreeSelectionUser = Glib::RefPtr<Gtk::TreeSelection>::cast_static(mRefBuilder->get_object("treeviewselectionUsers"));
    refListStoreUsers = Glib::RefPtr<Gtk::ListStore>::cast_static(mRefBuilder->get_object("liststoreUsers"));

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("userListAdd", sigc::mem_fun(*this, &UserListDialog::onAdd));
    mRefActionGroup->add_action("userListRemove", sigc::mem_fun(*this, &UserListDialog::onRemove));
    mRefActionGroup->add_action("userListBoxes", sigc::mem_fun(*this, &UserListDialog::onBoxes));
    mRefActionGroup->add_action("userListClose", sigc::mem_fun(*this, &UserListDialog::onCancel));

    insert_action_group("rcr", mRefActionGroup);

    refTreeViewUser->signal_row_activated().connect(sigc::mem_fun(*this, &UserListDialog::onUserActivated), refTreeViewUser);
}

bool UserListDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_plus:
            response(Gtk::RESPONSE_YES);
            onAdd();
            break;
        case GDK_KEY_minus:
            response(Gtk::RESPONSE_NO);
            onRemove();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;

}

void UserListDialog::onUserActivated(
    const Gtk::TreeModel::Path& path,
    Gtk::TreeViewColumn* column
)
{
    editUser();
}

void UserListDialog::onAdd()
{
    hide();
}

void UserListDialog::onRemove()
{
    hide();
}

void UserListDialog::onBoxes() {
    hide();
}

void UserListDialog::onCancel() {
    hide();
}

void UserListDialog::load(
    GRcrClient *client,
    RcrSettings *settings
)
{
    refTreeViewUser->unset_model();
    client->loadUsers(refListStoreUsers, settings->settings.user());
    refTreeViewUser->set_model(refListStoreUsers);
}

void UserListDialog::editUser()
{
    Gtk::TreeModel::iterator iter = refTreeSelectionUser->get_selected();
    if (!iter)
        return;
    Gtk::TreeModel::Row row = *iter;
    std::string name, password;
    int32_t rights;
    uint64_t token, id;;

    row.get_value(0, name);
    row.get_value(1, rights);
    row.get_value(2, password);
    row.get_value(3, token);
    row.get_value(4, id);

    selectedUser.set_id(id);
    selectedUser.set_name(name);
    selectedUser.set_password(password);
    selectedUser.set_token(token);
    selectedUser.set_rights(rights);
    // close dialog
    response(GTK_RESPONSE_ACCEPT);
    hide();
}
