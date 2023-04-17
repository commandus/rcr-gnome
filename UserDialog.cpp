//
// Created by andrei on 17.04.23.
//

#include "UserDialog.h"

UserDialog::UserDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

UserDialog::~UserDialog()
{

}

void UserDialog::bindWidgets() {
    mRefBuilder->get_widget("entryUserLogin", refEntryUserLogin);
    mRefBuilder->get_widget("entryUserPassword", refEntryUserPassword);
    mRefBuilder->get_widget("toogleUserAdmin", refToogleUserAdmin);
    mRefBuilder->get_widget("bUserSave", refButtonSave);
    mRefBuilder->get_widget("bUserCancel", refButtonCancel);


    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("userCancel", sigc::mem_fun(*this, &UserDialog::onCancel));
    mRefActionGroup->add_action("userSave", sigc::mem_fun(*this, &UserDialog::onSave));
    mRefActionGroup->add_action("userRightsAdmin", sigc::mem_fun(*this, &UserDialog::onRightsAdmin));

    insert_action_group("rcr", mRefActionGroup);
}

void UserDialog::onRightsAdmin()
{

}

void UserDialog::onSave()
{
    hide();
}

void UserDialog::onCancel()
{
    hide();
}

void UserDialog::get(
    rcr::User &retVal
)
{
    std::string u = refEntryUserLogin->get_text();
    retVal.set_name(u);
    std::string p = refEntryUserPassword->get_text();
    if (!p.empty())
        retVal.set_password(p);
    if (refToogleUserAdmin->get_active())
        retVal.set_rights(1);
    else
        retVal.set_rights(0);
}

void UserDialog::set(
    bool hasAdminRights,
    const rcr::User &val
)
{
    if (!hasAdminRights)
        refToogleUserAdmin->set_visible(false);
    refEntryUserLogin->set_text(val.name());
    refEntryUserPassword->set_text(val.password());
    refToogleUserAdmin->set_active(val.rights() == 1);
}
