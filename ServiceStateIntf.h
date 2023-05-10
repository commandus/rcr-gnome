//
// Created by andrei on 21.04.23.
//

#ifndef RCR_GNOME_SERVICESTATEINTF_H
#define RCR_GNOME_SERVICESTATEINTF_H


#include <string>

class ServiceStateIntf {
public:
    virtual void onCallStarted(int module, const std::string &message = "") = 0;
    virtual void onCallFinished(int module, int code, const std::string &message = "") = 0;
    // Not used yet
    virtual void onProgress(int pos, int total) = 0;
};

#endif //RCR_GNOME_SERVICESTATEINTF_H
