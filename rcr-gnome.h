//
// Created by andrei on 20.04.23.
//

#ifndef RCR_GNOME_RCR_GNOME_H
#define RCR_GNOME_RCR_GNOME_H

// i18n
#include <libintl.h>
#define _(String) gettext (String)

#define progname    "rcr-gnome"

enum {
    OP_LOAD_SYMBOLS,
    OP_LOAD_DICTIONARIES,
    OP_LOAD_BOXES,
    OP_QUERY,
    OP_IMPORT_FILE,
    OP_IMPORT_DIR,
    OP_LOAD_USERS,
    OP_STATISTICS,
    OP_SAVE_PROPERTY_TYPE,
    OP_RM_PROPERTY_TYPE,
    OP_SAVE_BOX,
    OP_RM_BOX,
    OP_SAVE_CARD,
    OP_RM_CARD
} RCR_OPERATION;

#endif //RCR_GNOME_RCR_GNOME_H
