//
// Created by andrei on 29.03.23.
//
#include <sstream>
#include "GRcrClient.h"

#include "StockOperation.h"
#include "string-helper.h"
#include "utilstring.h"
#include "utilfile.h"
#include "rcr-gnome.h"
#include "QueryProperties.h"

#define DEF_NAME_ALL _("All")

/*
 * grpcClient.cpp
 */

void GRcrClient::start(
    int module,
    const std::string &message
)
{
    if (state)
        state->onCallStarted(module, message);
}

void GRcrClient::finish(
    int module,
    int code,
    const std::string &message
)
{
    if (state)
        state->onCallFinished(module, code, message);
}

GRcrClient::GRcrClient(
    std::shared_ptr<grpc::Channel> aChannel
)
    : state(nullptr)
{
    channel = aChannel;
    stub = rcr::Rcr::NewStub(aChannel);
}

GRcrClient::GRcrClient(
    const std::string &host
)
    : state(nullptr)
{
    std::stringstream ss;
    ss << host << ":" << 50051;
    std::string target(ss.str());
    channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub = rcr::Rcr::NewStub(channel);
}

void GRcrClient::setUser(
    const rcr::User &user
)
{
    this->user = user;
}

GRcrClient::~GRcrClient()
{
}

bool GRcrClient::loadBoxes(
    Glib::RefPtr<Gtk::TreeStore> treeStore
)
{
    const Gtk::TreeModel::iterator *r = nullptr;
    if (!treeStore)
        return false;
    start(OP_LOAD_BOXES, _("Loading boxes"));
    treeStore->clear();

    Gtk::TreeModel::Row topRows[5];
    // root element
    topRows[0] = *treeStore->append();
    topRows[0].set_value <Glib::ustring>(0, DEF_NAME_ALL);  // 0- box & name
    topRows[0].set_value(1, 0);                             // 1- id
    topRows[0].set_value(2, 0);                             // 2- box
    topRows[0].set_value <Glib::ustring>(3, "");            // 3- name

    grpc::ClientContext context;
    rcr::BoxRequest request;
    request.mutable_list()->set_offset(0);
    request.mutable_list()->set_size(1024);
    request.set_start(0);
    request.set_depth(4);

    rcr::BoxResponse response;

    grpc::Status status = stub->getBox(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_LOAD_BOXES, status.error_code(), _("Loading boxes error"));
        return false;
    }

    // just make sure is box ordered
    reorderBoxesByBoxId(response);

    // response.box() must be ordered
    BoxArray lastBox = {0xffff, 0xffff, 0xffff, 0xffff };
    for (auto it = response.box().begin(); it != response.box().end(); ++it) {
        uint64_t bid = it->box_id();
        BoxArray currentBox;
        int bc = StockOperation::box2Array(currentBox, bid);    // 0- no box, 1..3
        Gtk::TreeModel::Row row;

        bool forceFill = false;
        for (int i = 3; i >= 0; i--) {
            if (currentBox.a[i] == 0)
                break;
            if ((currentBox.a[i] != lastBox.a[i]) || forceFill) {
                // 0xffff000000000000
                // 0xffffffff00000000
                // 0xffffffffffff0000
                // 0xffffffffffffffff
                uint64_t b = bid & (0xffffffffffffffff << (16 * i));
                const Gtk::TreeModel::iterator &bb = treeStore->append(topRows[3 - i].children());
                std::string name = StockOperation::boxes2string(b);
                if (!it->name().empty())
                    name = name + " " + it->name();

                bb->set_value(0, name);
                bb->set_value(1, it->id());
                bb->set_value(2, b);
                bb->set_value(3, it->name());
                topRows[4 - i] = *bb;
                lastBox.a[i] = currentBox.a[i];
                forceFill = true;
            }
        }
    }
    finish(OP_LOAD_BOXES, 0, _("Loading boxes completed successfully"));
    return true;
}

void GRcrClient::loadDictionaries()
{
    start(OP_LOAD_DICTIONARIES, _("Loading dictionaries"));
    grpc::ClientContext context;
    rcr::DictionariesRequest request;
    request.set_flags(0);

    grpc::Status status = stub->getDictionaries(&context, request, &dictionaries);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_LOAD_DICTIONARIES, status.error_code(), _("Loading dictionaries completed successfully"));
        return;
    }

    // before do it, unbind GUI elements first
    // just make sure does symbols ordered
    reorderDisctionaries(dictionaries);
    finish(OP_LOAD_DICTIONARIES, 0, _("Loading dictionaries completed successfully"));
}

void GRcrClient::bindSymbols(
    Glib::RefPtr<Gtk::ListStore> target
)
{
    if (!target)
        return;
    start(OP_LOAD_SYMBOLS, _("Loading symbols"));
    // before do it, unbind GUI elements first
    target->clear();
    // root element
    Gtk::TreeModel::Row row = *target->append();
    row.set_value <Glib::ustring>(0, DEF_NAME_ALL);
    row.set_value(1, 0);
    row.set_value <Glib::ustring>(2, "");
    row.set_value <Glib::ustring>(3, "");
    row.set_value <ulong>(4, 0);

    // just make sure does symbols ordered
    reorderDisctionaries(dictionaries);

    for (auto it = dictionaries.symbol().begin(); it != dictionaries.symbol().end(); ++it) {
        Gtk::TreeModel::Row row = *target->append();
        row.set_value(0, it->description());
        row.set_value(1, it->id());
        row.set_value(2, it->sym());
        row.set_value(3, it->unit());
        row.set_value(4, it->pow10());
    }
    finish(OP_LOAD_SYMBOLS, 0, _("Loading symbols completed successfully"));
}

int GRcrClient::findSymbol(
    const std::string &symbol
)
{
    // find out in dictionaries
    auto it = std::find_if(dictionaries.symbol().begin(), dictionaries.symbol().end(),
        [symbol](const rcr::Symbol &s) {
            return s.sym() == symbol;
        }
    );
    if (it == dictionaries.symbol().end())
        return 0;   // nothing means All
    return it->id();
}

COMPONENT GRcrClient::findSymbol(
    uint64_t symbolId
)
{
    // TODO fix somehow
    if (symbolId)
        return (COMPONENT) (symbolId - 1);
    else
        return (COMPONENT) symbolId;
}

void GRcrClient::query(
    const std::string &q,
    const std::string &symbol,
    Glib::RefPtr<Gtk::ListStore> listStore
) {
    start(OP_QUERY, _("Executing query"));
    grpc::ClientContext context;
    rcr::CardQueryRequest request;
    request.set_query(q);
    request.set_measure_symbol(symbol);
    request.mutable_list()->set_offset(0);
    request.mutable_list()->set_size(100);

#if CMAKE_BUILD_TYPE == Debug
    std::cerr << pb2JsonString(request) << std::endl;
#endif

    rcr::CardQueryResponse response;

    grpc::Status status = stub->cardQuery(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_QUERY, status.error_code(), _("Executing query completed with errors"));
        return;
    }

#if CMAKE_BUILD_TYPE == Debug
    std::cerr << pb2JsonString(response) << std::endl;
#endif
    listStore->clear();
    reorderCards(response);
    for (auto it = response.cards().cards().begin(); it != response.cards().cards().end(); it++) {
        for (auto p = it->packages().begin(); p != it->packages().end(); p++) {
            std::string nominal = MeasureUnit::value(ML_RU, findSymbol(it->card().symbol_id()), it->card().nominal());
            if (nominal.find('0') == 0)
                nominal = "";
            Gtk::TreeModel::Row row = *listStore->append();
            row.set_value(0, it->card().name());
            row.set_value(1, nominal);
            row.set_value(2, properties2string(dictionaries, it->properties()));
            row.set_value(3, StockOperation::boxes2string(p->box()));
            row.set_value(4, p->qty());
            row.set_value(5, it->card().id());
            row.set_value(6, p->box());
            row.set_value(7, it->card().symbol_id());
            row.set_value(8, p->id());
        }
    }
    finish(OP_QUERY, 0, _("Executing query completed successfully"));
}

std::string GRcrClient::properties2string(
    const rcr::DictionariesResponse &dictionaries,
    const google::protobuf::RepeatedPtrField<::rcr::PropertyWithName> &properties
)
{
    std::stringstream ss;
    for (auto p = properties.begin(); p != properties.end(); ++p) {
        ss << p->property_type() << ":" << p->value() <<  " ";
    }
    return ss.str();
}

void GRcrClient::reorderBoxesByBoxId(
    rcr::BoxResponse &boxes
) {
    std::sort(boxes.mutable_box()->begin(), boxes.mutable_box()->end(),
              [] (const rcr::Box& a, const rcr::Box& b){
            return a.box_id() < b.box_id();
    });
}

void GRcrClient::reorderDisctionaries(
    rcr::DictionariesResponse &value
) {
    std::sort(value.mutable_symbol()->begin(), value.mutable_symbol()->end(),
      [] (const rcr::Symbol& a, const rcr::Symbol& b){
          return a.sym() < b.sym();
    });
    std::sort(value.mutable_operation()->begin(), value.mutable_operation()->end(),
    [] (const rcr::Operation& a, const rcr::Operation& b){
      return a.symbol() < b.symbol();
    });
    std::sort(value.mutable_property_type()->begin(), value.mutable_property_type()->end(),
    [] (const rcr::PropertyType& a, const rcr::PropertyType& b){
      return a.description() < b.description();
    });
}

void GRcrClient::reorderCards(rcr::CardQueryResponse &value) {
    std::sort(value.mutable_cards()->mutable_cards()->begin(), value.mutable_cards()->mutable_cards()->end(),
        [] (const rcr::CardNPropetiesPackages& a, const rcr::CardNPropetiesPackages& b){
          return a.card().name() < b.card().name();
        });
}

bool GRcrClient::importFile(
    const std::string &symbol,
    const std::string &fileName,
    uint64_t boxId,
    bool numberInFileName
) {
    std::string content = file2string(fileName.c_str());
    if (content.empty())
        return false;
    start(OP_IMPORT_FILE, _("Import boxes"));
    grpc::ClientContext context;
    rcr::ImportExcelRequest request;
    request.set_symbol(symbol);
    request.set_prefix_box(boxId);
    request.set_number_in_filename(numberInFileName);
    auto f = request.add_file();
    f->set_name(fileName);
    f->set_content(content);

#if CMAKE_BUILD_TYPE == Debug
    std::cerr << pb2JsonString(request) << std::endl;
#endif

    rcr::OperationResponse response;

    grpc::Status status = stub->importExcel(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_IMPORT_FILE, status.error_code(), _("Import boxes failed"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_IMPORT_FILE, response.code(), _("Import boxes failed"));
        return false;
    }

    finish(OP_IMPORT_FILE, 0, _("Import boxes completed successfully"));
    std::cerr << _("Import boxes completed successfully") << std::endl;
    return true;
}

bool GRcrClient::importDirectory(
    const std::string &symbol,
    const std::string &path,
    uint64_t boxId,
    bool numberInFileName
) {
    start(OP_IMPORT_DIR, _("Import boxes"));
    grpc::ClientContext context;
    rcr::ImportExcelRequest request;
    request.set_symbol(symbol);
    request.set_prefix_box(boxId);
    request.set_number_in_filename(numberInFileName);

    std::vector<std::string> files;
    util::filesInPath(path, ".xlsx", 0, &files);
    for (auto it = files.begin(); it != files.end(); it++) {
        std::string content = file2string(it->c_str());
        if (content.empty())
            continue;
        auto f = request.add_file();
        f->set_name(*it);
        f->set_content(content);
    }
#if CMAKE_BUILD_TYPE == Debug
    std::cerr << pb2JsonString(request) << std::endl;
#endif

    rcr::OperationResponse response;

    grpc::Status status = stub->importExcel(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_IMPORT_DIR, status.error_code(), _("Import boxes failed"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_RM_BOX, response.code(), _("Import boxes failed"));
        return false;
    }
    finish(OP_IMPORT_DIR, 0, _("Import boxes completed successfully"));
    return true;
}

void GRcrClient::loadUsers(
    Glib::RefPtr<Gtk::ListStore> listStore,
    const rcr::User &user
) {
    if (!listStore)
        return;
    start(OP_LOAD_USERS, _("Loading users"));
    // before do it, unbind GUI elements first
    listStore->clear();
    rcr::UserRequest request;
    *request.mutable_user() = user;
    grpc::ClientContext context;
    rcr::UserResponse response;
    stub->lsUser(&context, request, &response);
    rcr::User u;
    for (int i = 0; i < response.user_size(); i++) {
        Gtk::TreeModel::Row row = *listStore->append();
        row.set_value(0, response.user(i).name());
        row.set_value(1, response.user(i).rights());
        row.set_value(2, response.user(i).password());
        row.set_value(3, response.user(i).token());
        row.set_value(4, response.user(i).id());
    }
    finish(OP_LOAD_USERS, 0, _("Loading users completed successfully"));
}

void GRcrClient::getStatistics(
    uint64_t &componentCount,
    uint64_t &total
) {
    grpc::ClientContext context;
    rcr::CardQueryRequest request;
    request.set_query("* sum");
    rcr::CardQueryResponse response;
    start(OP_STATISTICS, _("Getting statistics"));
    grpc::Status status = stub->cardQuery(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
    }
    componentCount = response.rslt().count();
    total = response.rslt().sum();
    finish(OP_STATISTICS, 0, _("Getting statistics completed successfully"));
}

void GRcrClient::setServiceState(ServiceStateIntf *value) {
    state = value;
}

bool GRcrClient::savePropertyType(
    uint64_t id,
    const std::string &key,
    const std::string &description
) {
    grpc::ClientContext context;
    rcr::ChPropertyTypeRequest chPropertyTypeRequest;
    rcr::OperationResponse response;
    *chPropertyTypeRequest.mutable_user() = user;
    if (id == 0) {
        chPropertyTypeRequest.set_operationsymbol("+");
    } else {
        chPropertyTypeRequest.set_operationsymbol("=");
    }
    chPropertyTypeRequest.mutable_value()->set_key(key);
    chPropertyTypeRequest.mutable_value()->set_description(description);

    start(OP_SAVE_PROPERTY_TYPE, _("Saving property type"));
    grpc::Status status = stub->chPropertyType(&context, chPropertyTypeRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_SAVE_PROPERTY_TYPE, status.error_code(), _("Save property type with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_SAVE_PROPERTY_TYPE, response.code(), _("Save property type with errors"));
        return false;
    }

    finish(OP_SAVE_PROPERTY_TYPE, status.error_code(), _("Save property type successfully"));
    // reload dictionaries
    loadDictionaries();
    return true;
}

bool GRcrClient::rmPropertyType(
    uint64_t id
)
{
    grpc::ClientContext context;
    rcr::ChPropertyTypeRequest chPropertyTypeRequest;
    rcr::OperationResponse response;
    *chPropertyTypeRequest.mutable_user() = user;
    chPropertyTypeRequest.set_operationsymbol("-");
    chPropertyTypeRequest.mutable_value()->set_id(id);
    start(OP_RM_PROPERTY_TYPE, _("Removing property type"));
    grpc::Status status = stub->chPropertyType(&context, chPropertyTypeRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_RM_PROPERTY_TYPE, status.error_code(), _("Removed property type with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_RM_BOX, response.code(), _("Removed property type with errors"));
        return false;
    }
    finish(OP_RM_PROPERTY_TYPE, 0, _("Removed property type successfully"));
    // reload dictionaries
    loadDictionaries();
    return true;
}

bool GRcrClient::rmBox(
    uint64_t boxId
) {
    grpc::ClientContext context;
    rcr::ChBoxRequest chBoxRequest;
    rcr::OperationResponse response;
    *chBoxRequest.mutable_user() = user;
    chBoxRequest.set_operationsymbol("-");
    chBoxRequest.mutable_value()->set_box_id(boxId);
    start(OP_RM_BOX, _("Removing box"));
    grpc::Status status = stub->chBox(&context, chBoxRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_RM_BOX, status.error_code(), _("Removed box with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_RM_BOX, response.code(), _("Removed box with errors"));
        return false;
    }
    finish(OP_RM_BOX, 0, _("Removed box successfully"));
    return true;
}

bool GRcrClient::saveBox(
    uint64_t id,
    uint64_t srcBoxId,
    uint64_t targetBoxId,
    const std::string &name
) {
    grpc::ClientContext context;
    rcr::ChBoxRequest chBoxRequest;
    rcr::OperationResponse response;
    *chBoxRequest.mutable_user() = user;
    chBoxRequest.mutable_value()->set_box_id(targetBoxId);
    if (id) {
        // exists already
        if (srcBoxId == targetBoxId) {
            chBoxRequest.mutable_value()->set_id(id);
            chBoxRequest.set_operationsymbol( "=");

        } else {
            chBoxRequest.mutable_value()->set_id(targetBoxId);  // sorry
            chBoxRequest.mutable_value()->set_box_id(srcBoxId);
            chBoxRequest.set_operationsymbol("/");
        }
    } else
        chBoxRequest.set_operationsymbol("+");
    chBoxRequest.mutable_value()->set_name(name);
    start(OP_SAVE_BOX, _("Saving box"));
    grpc::Status status = stub->chBox(&context, chBoxRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_SAVE_BOX, status.error_code(), _("Save box with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_SAVE_BOX, response.code(), _("Save box with errors"));
        return false;
    }

    finish(OP_SAVE_BOX, 0, _("Save box successfully"));
    return true;
}

bool GRcrClient::updateCardPackage(
    bool isNew,
    const rcr::Card &card,
    uint64_t packageId,
    std::string &properties,
    uint64_t boxId,
    const std::string &boxName,
    uint64_t qty
) {
    grpc::ClientContext context;

    rcr::ChCardRequest chCardRequest;
    rcr::OperationResponse response;
    *chCardRequest.mutable_user() = user;
    chCardRequest.set_package_id(packageId);
    if (!isNew) {
        chCardRequest.set_operationsymbol("=");
        chCardRequest.mutable_value()->set_id(card.id());
    } else
        chCardRequest.set_operationsymbol("+");

    *chCardRequest.mutable_value() = card;
    chCardRequest.mutable_value()->set_uname(toUpperCase(card.name()));

    rcr::Package *p = chCardRequest.add_packages();
    p->set_card_id(card.id());
    p->set_id(packageId);
    p->set_box(boxId);
    p->set_box_name(boxName);
    p->set_qty(qty);


    // properties
    std::map <std::string, std::string> mP;
    size_t pp = 0;
    QueryProperties::parse(properties, pp, mP);
    for (auto mit = mP.begin(); mit != mP.end(); mit++) {
        // find out property type identifier
        auto f = std::find_if(dictionaries.property_type().begin(), dictionaries.property_type().end(), [mit](rcr::PropertyType pt) {
            return pt.key() == mit->first;
        });
        if (f == dictionaries.property_type().end())
            continue;
        rcr::Property *prop = chCardRequest.add_properties();
        prop->set_card_id(card.id());
        prop->set_property_type_id(f->id());
        prop->set_value(mit->second);
    }

    start(OP_SAVE_CARD, _("Saving card"));
    grpc::Status status = stub->chCard(&context, chCardRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_SAVE_CARD, status.error_code(), _("Save card with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_SAVE_CARD, response.code(), _("Save card with errors"));
        return false;
    }

    finish(OP_SAVE_CARD, 0, _("Save card successfully"));
    return true;
}

bool GRcrClient::rmCardPackage(
    rcr::Card &card,
    uint64_t packageId
) {
    grpc::ClientContext context;

    rcr::ChCardRequest chCardRequest;
    rcr::OperationResponse response;
    *chCardRequest.mutable_user() = user;
    chCardRequest.set_package_id(packageId);
    chCardRequest.set_operationsymbol("-");
    chCardRequest.mutable_value()->set_id(card.id());
    chCardRequest.set_package_id(packageId);
    start(OP_RM_CARD, _("Removing card"));
    grpc::Status status = stub->chCard(&context, chCardRequest, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
        finish(OP_RM_CARD, status.error_code(), _("Remove card with errors"));
        return false;
    }
    if (response.code()) {
        std::cerr << "Error: " << response.code() << " " << response.description() << std::endl;
        finish(OP_RM_CARD, response.code(), _("Remove card with errors"));
        return false;
    }

    finish(OP_RM_CARD, 0, _("Remove card successfully"));
    return true;
}
