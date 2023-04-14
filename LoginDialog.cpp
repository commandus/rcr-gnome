//
// Created by andrei on 13.04.23.
//

#include "LoginDialog.h"

LoginDialog::LoginDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

LoginDialog::~LoginDialog()
{

}

void LoginDialog::bindWidgets() {
    mRefBuilder->get_widget("entryLogin", refEntryLogin);
    mRefBuilder->get_widget("entryPassword", refEntryPassword);
    mRefBuilder->get_widget("bRegisterSwitchToLogin", refButtonRegisterSwitchToLogin);
    mRefBuilder->get_widget("bLogin", refButtonOK);
    mRefBuilder->get_widget("bLoginCancel", refButtonCancel);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("login", sigc::mem_fun(*this, &LoginDialog::onLogin));
    mRefActionGroup->add_action("logincancel", sigc::mem_fun(*this, &LoginDialog::onCancel));
    mRefActionGroup->add_action("loginSwitchToRegister", sigc::mem_fun(*this, &LoginDialog::onSwitchToLogin));

    insert_action_group("rcr", mRefActionGroup);
}

void LoginDialog::onSwitchToLogin() {
    response(Gtk::RESPONSE_REJECT);
    hide();
}

void LoginDialog::onLogin()
{
    hide();
}

void LoginDialog::onCancel()
{
    hide();
}

bool LoginDialog::on_key_press_event(
    GdkEventKey* event
)
{
    switch (event->keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            response(Gtk::RESPONSE_OK);
            onLogin();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}

void LoginDialog::user(
    rcr::User *retVal
) {
    retVal->set_name(refEntryLogin->get_text());
    retVal->set_password(refEntryPassword->get_text());
}

void LoginDialog::setUser(
    const rcr::User &user
)
{
    refEntryLogin->set_text(user.name());
    refEntryPassword->set_text(user.password());
}

