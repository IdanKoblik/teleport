#ifndef ROLE_H_
#define ROLE_H_

#include <ctype.h>
#include <string.h>

#define ROLES \
    ROLE(SERVER) \
    ROLE(CLIENT) \
    ROLE(UNKNOWN)

typedef enum {
#define ROLE(name) name,
  ROLES
#undef ROLE
} ROLE;

static inline const char *role_to_string(ROLE role) {
    static const char *roles[] = {
#define ROLE(name) #name,
        ROLES
#undef ROLE
    };

    if (role < 0 || role >= (int)(sizeof(roles) / sizeof(roles[0])))
        return "UNKNOWN";

    return roles[role];
}

static inline ROLE string_to_role(const char *str) {
    char buf[32];
    size_t i;
    for (i = 0; i < sizeof(buf) - 1 && str[i] != '\0'; i++)
        buf[i] = (char)toupper((unsigned char)str[i]);
    buf[i] = '\0';

#define ROLE(name) if (strcmp(buf, #name) == 0) return name;
    ROLES
#undef ROLE

    return UNKNOWN;
}

#endif // ROLE_H_
