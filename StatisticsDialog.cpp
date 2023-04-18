//
// Created by andrei on 18.04.23.
//

#include "StatisticsDialog.h"

StatisticsDialog::StatisticsDialog(
    BaseObjectType* cobject,
    const Glib::RefPtr<Gtk::Builder>& refBuilder
)
    : Gtk::Dialog(cobject), mRefBuilder(refBuilder)
{
    bindWidgets();
}

StatisticsDialog::~StatisticsDialog()
{

}

void StatisticsDialog::bindWidgets() {
    mRefBuilder->get_widget("labelStatisticsComponents", refLabelComponents);
    mRefBuilder->get_widget("labelStatisticsTotal", refLabelTotal);
    mRefBuilder->get_widget("buttonStatisticsClose", refButtonClose);

    mRefActionGroup = Gio::SimpleActionGroup::create();
    mRefActionGroup->add_action("closeStatistics", sigc::mem_fun(*this, &StatisticsDialog::onCancel));
    insert_action_group("rcr", mRefActionGroup);
}

void StatisticsDialog::onCancel()
{
    hide();
}

void StatisticsDialog::setStat(
    uint64_t componentCount,
    uint64_t total
) {
    refLabelComponents->set_text(std::to_string(componentCount));
    refLabelTotal->set_text(std::to_string(total));
}
