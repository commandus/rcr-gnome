//
// Created by andrei on 12.04.23.
//

#include "BoxConfirmDialog.h"
#include "StockOperation.h"

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
    mRefBuilder->get_widget("checkNumberInFileName", refСheckNumberInFileName);
    refСheckNumberInFileName->set_active(true);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("confirmBoxContinue", sigc::mem_fun(*this, &BoxConfirmDialog::onBoxConfirm));
    mRefActionGroup->add_action("confirmBoxCancel", sigc::mem_fun(*this, &BoxConfirmDialog::onBoxCancel));
    mRefActionGroup->add_action("numberInFileName", sigc::mem_fun(*this, &BoxConfirmDialog::onBoxNumberInFileName));

    insert_action_group("rcr", mRefActionGroup);
}

void BoxConfirmDialog::onBoxConfirm() {
    hide();
}

void BoxConfirmDialog::onBoxCancel() {
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

void BoxConfirmDialog::setBox(
    const uint64_t &boxId
) {
    std::string sb = StockOperation::boxes2string(boxId);
    if (sb == "0")
        sb = "";
    refEntryBox->set_text(sb);
}

uint64_t BoxConfirmDialog::box() {
    std::string sb = refEntryBox->get_text();
    uint64_t r;
    StockOperation::parseBoxes(r, sb, 0, sb.size());
    return r;
}

void BoxConfirmDialog::onBoxNumberInFileName()
{

}

bool BoxConfirmDialog::numberInFileName()
{
    return refСheckNumberInFileName->get_active();
}

void BoxConfirmDialog::setNumberInFileName(bool value)
{
    refСheckNumberInFileName->set_active(value);
}
