//
// Created by andrei on 12.04.23.
//

#include <iostream>
#include "BoxConfirmDialog.h"

BoxConfirmDialog::BoxConfirmDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
        : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

BoxConfirmDialog::~BoxConfirmDialog()
{

}

void BoxConfirmDialog::bindWidgets() {
    mRefBuilder->get_widget("entryConfirmBox", refEntryBox);
    mRefBuilder->get_widget("bConfirmBoxContinue", refButtonContinue);
    mRefBuilder->get_widget("bConfirmBoxCancel", refButtonCancel);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("confirmBoxContinue", sigc::mem_fun(*this, &BoxConfirmDialog::onBoxConfirm));
    mRefActionGroup->add_action("confirmBoxCancel", sigc::mem_fun(*this, &BoxConfirmDialog::onBoxCancel));

    insert_action_group("rcr", mRefActionGroup);
}

void BoxConfirmDialog::onBoxConfirm() {
    std::cerr << "==Confirm==" << std::endl;
    hide();
}

void BoxConfirmDialog::onBoxCancel() {
    std::cerr << "==Cancel==" << std::endl;
    hide();
}

bool BoxConfirmDialog::on_key_press_event(GdkEventKey* event)
{
    switch (event->keyval) {
        case GDK_KEY_Return:
        case GDK_KEY_KP_Enter:
            response(Gtk::RESPONSE_OK);
            onBoxConfirm();
            break;
        default:
            return Gtk::Window::on_key_press_event(event);
    }
    return FALSE;
}
