//
// Created by andrei on 29.03.23.
//

#include <sstream>
#include "GRcrClient.h"

#include "StockOperation.h"

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

void GRcrClient::loadBoxes(
    Glib::RefPtr<Gtk::TreeStore> treeStore
)
{
    if (!treeStore)
        return;
    treeStore->clear();

    // root element
    Gtk::TreeModel::Row rootRow = *treeStore->append();
    rootRow.set_value <Glib::ustring>(0, DEF_NAME_ALL);
    rootRow.set_value(1, 0);
    rootRow.set_value(2, 0);

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
    }

    auto root = rootRow->children();
    // response.box() must be ordered
    BoxArray lastBox = {0, 0, 0, 0 };

    for (auto it = response.box().begin(); it != response.box().end(); ++it) {
        uint64_t bid = it->box_id();
        BoxArray currentBox;
        int bc = StockOperation::box2Array(currentBox, bid);    // 0- no box, 1..3


        Gtk::TreeModel::Row row = *treeStore->append(root);
        row.set_value(0, StockOperation::boxes2string(bid));
        row.set_value(1, it->id());
        row.set_value(2, it->box_id());
    }
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

    rcr::CardQueryResponse response;

    grpc::Status status = stub->cardQuery(&context, request, &response);
    if (!status.ok()) {
        std::cerr << "Error: " << status.error_code() << " " << status.error_message() << std::endl;
    }

    listStore->clear();

    for (auto it = response.cards().cards().begin(); it != response.cards().cards().end(); ++it) {
        for (auto p = it->packages().begin(); p != it->packages().end(); ++p) {
            std::string nominal = MeasureUnit::value(ML_RU, findSymbol(it->card().symbol_id()), it->card().nominal());
            if (nominal.find('0') == 0)
                nominal = "";
            std::cerr << "box: " << p->box()
                << " " << StockOperation::boxes2string(p->box()) << std::endl;
            Gtk::TreeModel::Row row = *listStore->append();
            row.set_value(0, it->card().name());
            row.set_value(1, nominal);
            row.set_value(2, properties2string(dictionaries, it->properties()));
            row.set_value(3, StockOperation::boxes2string(p->box()));
            row.set_value(4, p->qty());
            row.set_value(5, p->id());
            row.set_value(6, p->box());
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
