//
// Created by andrei on 03.04.23.
//

#ifndef RCR_GNOME_RCRSETTINGS_H
#define RCR_GNOME_RCRSETTINGS_H

#include <string>
#include "gen/rcr.pb.h"

class RcrSettings {
private:

public:
    explicit RcrSettings(
        const std::string &profileName,
        const char *programPath,
        const std::string &fileName
    );
    std::string fileName;
    int selected;
    rcr::Settings settings;

    void save();
};


#endif //RCR_GNOME_RCRSETTINGS_H
