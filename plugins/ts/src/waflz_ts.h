//: ----------------------------------------------------------------------------
//: Copyright (C) 2020 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    waflz_ts.h
//: \details: TODO
//: \author:  Andrey Ter-Zakhariants
//: \date:    05/15/2020
//:
//:   Licensed TODO
//:
//: ----------------------------------------------------------------------------


#ifndef _WAFLZ_TS_H_
#define _WAFLZ_TS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct waflz_profile_t waflz_profile_t;
typedef struct waflz_transaction_t waflz_transaction_t;
typedef struct waflz_event_t
{
    long rule_id;
    char *log;
} waflz_event_t;

waflz_profile_t* waflz_profile_new_load(const char* rule_dir, const char* profile_file_name);
int waflz_profile_process(waflz_transaction_t* tx);
void waflz_profile_cleanup(waflz_profile_t* wp);

waflz_transaction_t* waflz_new_transaction(waflz_profile_t *profile, int trace);
int waflz_transaction_add_request_connection_uri(waflz_transaction_t *transaction,
                                                 const char *client_ip, const char *host, int port, const char* method, const char *scheme,
                                                 const char *url, const char *uri, const char *path, const char *query, const char *protocol, const char *http_version);
int waflz_transaction_add_request_header(waflz_transaction_t *transaction, const char *key, const char *value);
void waflz_transaction_cleanup(waflz_transaction_t *transaction);
int waflz_transaction_get_event(waflz_transaction_t *transaction, waflz_event_t *event);

#ifdef __cplusplus
}
#endif

#endif
