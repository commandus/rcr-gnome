//
// Created by andrei on 13.04.23.
//

#include "RegisterDialog.h"

RegisterDialog::RegisterDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

RegisterDialog::~RegisterDialog()
{

}

void RegisterDialog::bindWidgets() {
    mRefBuilder->get_widget("entryRegisterLogin", refEntryLogin);
    mRefBuilder->get_widget("entryRegisterPassword", refEntryPassword);
    mRefBuilder->get_widget("entryRegisterRetypePassword", refEntryRetypePassword);
    mRefBuilder->get_widget("bLoginSwitchToRegister", refButtonLoginSwitchToRegister);
    mRefBuilder->get_widget("bLogin", refButtonOK);
    mRefBuilder->get_widget("bLoginCancel", refButtonCancel);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("registerUser", sigc::mem_fun(*this, &RegisterDialog::onRegister));
    mRefActionGroup->add_action("registerUserCancel", sigc::mem_fun(*this, &RegisterDialog::onCancel));
    mRefActionGroup->add_action("registerSwitchToLogin", sigc::mem_fun(*this, &RegisterDialog::onSwitchToRegister));

    insert_action_group("rcr", mRefActionGroup);
}

void RegisterDialog::onSwitchToRegister()
{
    response(Gtk::RESPONSE_REJECT);
    hide();
}

void RegisterDialog::onRegister()
{
    hide();
}

void RegisterDialog::onCancel() {
    hide();
}

bool RegisterDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            response(Gtk::RESPONSE_OK);
            onRegister();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}

void RegisterDialog::user(
    rcr::User *retVal
) {
    retVal->set_name(refEntryLogin->get_text());
    retVal->set_password(refEntryPassword->get_text());
}

void RegisterDialog::setUser(
    const rcr::User &user
)
{
    refEntryLogin->set_text(user.name());
    refEntryPassword->set_text(user.password());
    refEntryRetypePassword->set_text(user.password());
}
