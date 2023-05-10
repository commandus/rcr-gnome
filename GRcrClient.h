//
// Created by andrei on 29.03.23.
//

#ifndef RCR_GNOME_GRCRCLIENT_H
#define RCR_GNOME_GRCRCLIENT_H

#include <string>
#include <grpc++/grpc++.h>

#include "gen/rcr.grpc.pb.h"
#include "MeasureUnit.h"
#include "ServiceStateIntf.h"

#include <gtkmm.h>
#include <gtkmm/treeview.h>

class GRcrClient {
private:
    ServiceStateIntf *state;
    std::string username;
    std::string password;

    void start(
        int module,
        const std::string &message = ""
    );
    void finish(
        int module,
        int code,
        const std::string &message = ""
    );
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
    void loadDictionaries();
    void bindSymbols(
        Glib::RefPtr<Gtk::ListStore> target
    );
    bool loadBoxes(
        Glib::RefPtr<Gtk::TreeStore> treeStore
    );

    int findSymbol(
        const std::string &symbol
    );
    COMPONENT findSymbol(
        uint64_t symbolId
    );

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

    void setServiceState(
        ServiceStateIntf *value
    );

    void savePropertyType(
        uint64_t id,
        const std::string &key,
        const std::string &desctiption
    );
    void rmPropertyType(
        uint64_t id
    );

    bool rmBox(
        uint64_t boxId
    );

    bool saveBox(
        uint64_t id,
        uint64_t boxId,
        const std::string &name
    );
};

#endif //RCR_GNOME_GRCRCLIENT_H
