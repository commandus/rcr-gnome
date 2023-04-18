//
// Created by andrei on 29.03.23.
//

#ifndef RCR_GNOME_GRCRCLIENT_H
#define RCR_GNOME_GRCRCLIENT_H

#include <string>
#include <grpc++/grpc++.h>

#include "gen/rcr.grpc.pb.h"
#include "MeasureUnit.h"

#include <gtkmm.h>
#include <gtkmm/treeview.h>

class GRcrClient {
public:
    std::unique_ptr<rcr::Rcr::Stub> stub;
    std::shared_ptr<grpc::Channel> channel;
    rcr::DictionariesResponse dictionaries;
    
    GRcrClient(
        std::shared_ptr<grpc::Channel> channel,
        const std::string &username,
        const std::string &password
    );
    GRcrClient(
        const std::string &host
    );
    virtual ~GRcrClient();
    void loadSymbols(
        Glib::RefPtr<Gtk::ListStore> target
    );
    bool loadBoxes(
        Glib::RefPtr<Gtk::TreeStore> treeStore
    );

    int findSymbol(const std::string &symbol);
    COMPONENT findSymbol(uint64_t symbolId);

    void query(
        const std::string &q,
        const std::string &symbol,
        Glib::RefPtr<Gtk::ListStore> listStore
    );

    std::string properties2string(
        const rcr::DictionariesResponse &dictionaries,
        const google::protobuf::RepeatedPtrField<::rcr::PropertyWithName> &properties
    );

    void reorderBoxesByBoxId(
        rcr::BoxResponse &response
    );

    void reorderDisctionaries(
        rcr::DictionariesResponse &value
    );

    void reorderCards(
        rcr::CardQueryResponse &value
    );

    bool importFile(
        const std::string &symbol,
        const std::string &fileName,
        uint64_t boxId,
        bool numberInFileName
    );

    bool importDirectory(
        const std::string &symbol,
        const std::string &fileName,
        uint64_t boxId,
        bool numberInFileName
    );

    void loadUsers(
        Glib::RefPtr<Gtk::ListStore> retVal,
        const rcr::User &user
    );

    void getStatistics(
        uint64_t &componentCount,
        uint64_t &total
    );
};

#endif //RCR_GNOME_GRCRCLIENT_H
