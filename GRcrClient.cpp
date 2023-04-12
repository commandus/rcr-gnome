//
// Created by andrei on 29.03.23.
//

#define DEBUG

#include <sstream>
#include "GRcrClient.h"

#include "StockOperation.h"
#include "string-helper.h"
#include "utilstring.h"
#include "utilfile.h"

#define DEF_NAME_ALL "All"

/*
 * grpcClient.cpp
 */

GRcrClient::GRcrClient(
    std::shared_ptr<grpc::Channel> aChannel,
    const std::string &username,
    const std::string &password
)
{
    channel = aChannel;
    stub = rcr::Rcr::NewStub(aChannel);
}

GRcrClient::GRcrClient(
    const std::string &host
)
{
    std::stringstream ss;
    ss << host << ":" << 50051;
    std::string target(ss.str());
    channel = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub = rcr::Rcr::NewStub(channel);

    grpc::ClientContext context;
    rcr::DictionariesRequest request;
    request.set_flags(0);

    grpc::Status status = stub->getDictionaries(&context, request, &dictionaries);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
    }
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
    treeStore->clear();

    Gtk::TreeModel::Row topRows[5];
    // root element
    topRows[0] = *treeStore->append();
    topRows[0].set_value <Glib::ustring>(0, DEF_NAME_ALL);
    topRows[0].set_value(1, 0);
    topRows[0].set_value(2, 0);

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
                bb->set_value(0, StockOperation::boxes2string(b));
                bb->set_value(1, 0);
                bb->set_value(2, b);
                topRows[4 - i] = *bb;
                lastBox.a[i] = currentBox.a[i];
                forceFill = true;
            }
        }
    }
    return true;
}

void GRcrClient::loadSymbols(
    Glib::RefPtr<Gtk::ListStore> listStore
)
{
    if (!listStore)
        return;
    // before do it, unbind GUI elements first
    listStore->clear();
    // root element
    Gtk::TreeModel::Row row = *listStore->append();
    row.set_value <Glib::ustring>(0, DEF_NAME_ALL);
    row.set_value(1, 0);
    row.set_value <Glib::ustring>(2, "");
    row.set_value <Glib::ustring>(3, "");
    row.set_value <ulong>(4, 0);

    // just make sure is symbols ordered
    reorderDisctionaries(dictionaries);

    for (auto it = dictionaries.symbol().begin(); it != dictionaries.symbol().end(); ++it) {
        Gtk::TreeModel::Row row = *listStore->append();
        row.set_value(0, it->description());
        row.set_value(1, it->id());
        row.set_value(2, it->sym());
        row.set_value(3, it->unit());
        row.set_value(4, it->pow10());
    }
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
    return (COMPONENT) (symbolId - 1);
}

void GRcrClient::query(
    const std::string &q,
    const std::string &symbol,
    Glib::RefPtr<Gtk::ListStore> listStore
) {
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
    }

    listStore->clear();

    reorderCards(response);

    for (auto it = response.cards().cards().begin(); it != response.cards().cards().end(); ++it) {
        for (auto p = it->packages().begin(); p != it->packages().end(); ++p) {
            std::string nominal = MeasureUnit::value(ML_RU, findSymbol(it->card().symbol_id()), it->card().nominal());
            if (nominal.find('0') == 0)
                nominal = "";
            Gtk::TreeModel::Row row = *listStore->append();
            row.set_value(0, it->card().name());
            row.set_value(1, nominal);
            row.set_value(2, properties2string(dictionaries, it->properties()));
            row.set_value(3, StockOperation::boxes2string(p->box()));
            row.set_value(4, p->qty());
            row.set_value(5, p->id());
            row.set_value(6, p->box());
            row.set_value(7, it->card().symbol_id());
        }
    }
}

std::string GRcrClient::properties2string(
    const rcr::DictionariesResponse &dictionaries,
    const google::protobuf::RepeatedPtrField<::rcr::PropertyWithName> &properties
)
{
    std::stringstream ss;
    for (auto p = properties.begin(); p != properties.end(); ++p) {
        ss << p->value() <<  " ";
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
    uint64_t boxId
) {
    std::string content = file2string(fileName.c_str());
    if (content.empty())
        return false;

    grpc::ClientContext context;
    rcr::ImportExcelRequest request;
    request.set_symbol(symbol);
    request.set_prefix_box(boxId);
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
        return false;
    }
    return true;
}

bool GRcrClient::importDirectory(
    const std::string &symbol,
    const std::string &path,
    uint64_t boxId
) {
    grpc::ClientContext context;
    rcr::ImportExcelRequest request;
    request.set_symbol(symbol);
    request.set_prefix_box(boxId);

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
        return false;
    }
    return true;
}
