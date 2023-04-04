//
// Created by andrei on 03.04.23.
//

#include "RcrSettings.h"
#include "config-filename.h"
#include "utilfile.h"
#include "utilstring.h"
#include <google/protobuf/util/json_util.h>

RcrSettings::RcrSettings(
    const std::string &profileName,
    const char *programPath,
    const std::string &configFileName
)
{
    fileName = getDefaultConfigFileName(programPath, configFileName);
    std::string json = file2string(fileName.c_str());
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;
    google::protobuf::util::JsonStringToMessage(json, &settings, options);
    if (settings.service_size() == 0) {
        auto s = settings.add_service();
        s->set_last_component_symbol("D");
        s->set_last_box(0);
        s->set_last_query("");
        s->set_name("localhost");
        s->set_addr("localhost");
        s->set_port(50051);
        s->set_id(0);
    }
    selected = 0;
}

void RcrSettings::save() {
    google::protobuf::util::JsonPrintOptions formattingOptions;
    formattingOptions.add_whitespace = true;
    formattingOptions.always_print_primitive_fields = true;
    formattingOptions.preserve_proto_field_names = true;
    std::string r;
    google::protobuf::util::MessageToJsonString(settings, &r, formattingOptions);
    string2file(fileName.c_str(), r);
}
