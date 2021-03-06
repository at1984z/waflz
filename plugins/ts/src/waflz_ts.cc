//: ----------------------------------------------------------------------------
//: Copyright (C) 2020 Verizon.  All Rights Reserved.
//: All Rights Reserved
//:
//: \file:    waflz_ts.cc
//: \details: TODO
//: \author:  Andrey Ter-Zakhariants
//: \date:    05/15/2020
//:
//:   Licensed TODO
//:
//: ----------------------------------------------------------------------------
//: ----------------------------------------------------------------------------
//: includes
//: ----------------------------------------------------------------------------
#include "waflz/engine.h"
#include "waflz/profile.h"
#include "waflz/instances.h"
#include "waflz/rqst_ctx.h"
#include "waflz/geoip2_mmdb.h"
#include "waflz/def.h"
#include "waflz/trace.h"
#include "profile.pb.h"
#include "event.pb.h"
#include "support/ndebug.h"
#include "support/file_util.h"
#include <unistd.h>
#include <iostream>
#include <cstring>

#include "waflz_ts.h"

struct waflz_profile_t {
    ns_waflz::profile* profile;
    ns_waflz::engine*  engine;
};

struct waflz_transaction_t {
    waflz_profile_t* profile;
    int trace;  // 0 - no trace

    const char *client_ip;
    const char *host;
    int port;
    const char *method;
    const char *scheme;

    const char *url;  //(REQUEST_URI_RAW)  the same as uri
    const char *uri;  //(REQUEST_URI) : This variable holds the full request URL including the query string data (e.g., /index.php?p=X). However, it will never contain a domain name, even if it was provided on the request line.
    const char *path;  //1. (REQUEST_FILENAME): This variable holds the relative request URL without the query string part (e.g., /index.php).
                       //2. (REQUEST_BASENAME): This variable holds just the filename part of REQUEST_FILENAME (e.g., index.php).
    const char *query;  //(ARGS): ARGS is a collection and can be used on its own (means all arguments including the POST Payload),
    const char *protocol;
    const char *http_version;

    std::vector<std::pair<const char*, const char*> > headers;  //consider std::array<N, std::pair<> >, where N is 20?

    std::string rqst_line;  // GET /index.html HTTP/1.1
    std::string rqst_protocol;  // HTTP/1.1
    waflz_pb::event *event = NULL;

    waflz_transaction_t()
        : profile(nullptr),
          trace(0),
          client_ip(nullptr),
          host(nullptr),
          port(0),
          method(nullptr),
          scheme(nullptr),
          url(nullptr),
          uri(nullptr),
          path(nullptr),
          query(nullptr),
          protocol(nullptr),
          http_version(nullptr),
          headers(),
          rqst_line(),
          rqst_protocol(),
          event(nullptr)
    {}
    ~waflz_transaction_t()
    {
        delete event;
    }
    waflz_transaction_t(const waflz_transaction_t&);
    waflz_transaction_t& operator=(const waflz_transaction_t&);
};

//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
//static const char *s_ip = "127.0.0.1";
static int32_t get_rqst_src_addr_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_src_addr_cb: " << t->client_ip << "\n";
        }
        *a_data = t->client_ip;  //s_ip;
        *a_len = strlen(t->client_ip);  //strlen(s_ip);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_line_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        t->rqst_line.assign(t->method);
        t->rqst_line.append(" ");
        t->rqst_line.append(t->uri);
        t->rqst_line.append(" ");
        t->rqst_line.append(t->protocol);
        t->rqst_line.append("/");
        t->rqst_line.append(t->http_version);
        if (t->trace) {
            std::cout << "get_rqst_line_cb: " << t->rqst_line << "\n";
        }
        //static const char s_line[] = "GET /test.pl HTTP/1.1";
        *a_data = t->rqst_line.c_str();  //s_line;
        *a_len = t->rqst_line.size();  //strlen(s_line);
        return 0;
}
//! ----------------------------------------------------------------------------
//! get host callback
//! ----------------------------------------------------------------------------
static int32_t get_rqst_host_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_host_cb: " << t->host << "\n";
        }
        //static const char s_uri[] = "127.0.0.1";
        *a_data = t->host;  //s_uri;
        *a_len = strlen(t->host);  //strlen(s_uri);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_query_str_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_query_str_cb: " << t->query << "\n";
        }
        //static const char s_line[] = "param1=test&para2=test2";  //TODO no question mark?
        *a_data = t->query;  //s_line;
        *a_len = strlen(t->query);  //strlen(s_line);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
//static const char *s_uri = "/test.pl";
static int32_t get_rqst_uri_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_uri_cb: " << t->uri << "\n";
        }
        *a_data = t->uri;  //s_uri;
        *a_len = strlen(t->uri);  //strlen(s_uri);
        return 0;
}
//: ----------------------------------------------------------------------------
//: s_get_rqst_method_cb
//: ----------------------------------------------------------------------------
//static const char *s_method = "GET";
static int32_t get_rqst_method_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_method_cb: " << t->method << "\n";
        }
        *a_data = t->method;  //s_method;
        *a_len = strlen(t->method);  //strlen(s_method);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_protocol_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        t->rqst_protocol.assign(t->protocol);
        t->rqst_protocol.append("/");
        t->rqst_protocol.append(t->http_version);
        if (t->trace) {
            std::cout << "get_rqst_protocol_cb: " << t->rqst_protocol << "\n";
        }
        //static const char s_line[] = "HTTP/1.1";
        *a_data = t->rqst_protocol.c_str();  //s_line;
        *a_len = t->rqst_protocol.size();  //strlen(s_line);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_scheme_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_scheme_cb: " << t->scheme << "\n";
        }
        //static const char s_line[] = "http";
        *a_data = t->scheme;  //s_line;
        *a_len = strlen(t->scheme);  //strlen(s_line);
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_port_cb(uint32_t *a_val, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_port_cb: " << t->port << "\n";
        }
        *a_val = t->port;  //80;
        return 0;
}
//: ----------------------------------------------------------------------------
//: TODO
//: ----------------------------------------------------------------------------
static int32_t get_rqst_url_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_url_cb: " << t->url << "\n";
        }
        //static const char s_line[] = "127.0.0.1/test.pl?param1=test&para2=test2";
        *a_data = t->url;  //s_line;
        *a_len = strlen(t->url);  //strlen(s_line);
        return 0;
}
//: ----------------------------------------------------------------------------
//: s_get_rqst_path_cb
//: ----------------------------------------------------------------------------
//static const char *s_path = "/test.pl";
static int32_t get_rqst_path_cb(const char **a_data, uint32_t *a_len, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_path_cb: " << t->path << "\n";
        }
        *a_data = t->path;  //s_path;
        *a_len = strlen(t->path);  //strlen(s_path);
        return 0;
}
//: ----------------------------------------------------------------------------
//: get_rqst_header_size_cb
//: ----------------------------------------------------------------------------
static int32_t get_rqst_header_size_cb(uint32_t *a_val, void *a_ctx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        if (t->trace) {
            std::cout << "get_rqst_header_size_cb: " << t->headers.size() << "\n";
        }
        *a_val = t->headers.size();  //10;
        return 0;
}
//: ----------------------------------------------------------------------------
//: get_rqst_header_w_idx_cb
//: ----------------------------------------------------------------------------
static int32_t get_rqst_header_w_idx_cb(const char **ao_key,
                                        uint32_t *ao_key_len,
                                        const char **ao_val,
                                        uint32_t *ao_val_len,
                                        void *a_ctx,
                                        uint32_t a_idx)
{
        waflz_transaction_t* t = (waflz_transaction_t*)a_ctx;
        *ao_key = NULL;
        *ao_key_len = 0;
        *ao_val = NULL;
        *ao_val_len = 0;
        if (t->headers.size() > 0 && a_idx < t->headers.size())
        {
                if (t->trace) {
                        std::cout << "get_rqst_header_w_idx_cb: a_idx: " << a_idx << " k: [" << t->headers[a_idx].first << "] v: [" << t->headers[a_idx].second << "]\n";
                }
                *ao_key = t->headers[a_idx].first;
                *ao_key_len = strlen(t->headers[a_idx].first);
                *ao_val = t->headers[a_idx].second;
                *ao_val_len = strlen(t->headers[a_idx].second);
        }
        return 0;
}

waflz_profile_t* waflz_profile_new_load(const char* rule_dir,
                                        const char* profile_file_name)
{
    int32_t l_s;
    
    //must be a singleton
    static ns_waflz::engine* we = nullptr;
    if (!we) {
        we = new ns_waflz::engine();
        we->set_ruleset_dir(rule_dir); //done

        l_s = we->init(); //done
        if (l_s != WAFLZ_STATUS_OK) {
            //TODO cleanup
            return nullptr;
        }
    }

    waflz_profile_t* wp = new waflz_profile_t;
    wp->engine = we;

    char *l_buf;
    uint32_t l_buf_len;

    l_s = ns_waflz::read_file(profile_file_name, &l_buf, l_buf_len);
    if(l_s != WAFLZ_STATUS_OK) {
        NDBG_PRINT("error read_file: %s\n", profile_file_name);
        return nullptr;
    }

    wp->profile = new ns_waflz::profile(*wp->engine);

    l_s = wp->profile->load(l_buf, l_buf_len);
    if (l_s != WAFLZ_STATUS_OK) {
        NDBG_PRINT("error[%d]: %s\n", l_s, wp->profile->get_err_msg());
        //TODO cleanup
        return nullptr;
    }
    
    return wp;
}

static ns_waflz::rqst_ctx_callbacks s_callbacks {
    get_rqst_src_addr_cb,
    get_rqst_host_cb,
    get_rqst_port_cb,
    get_rqst_scheme_cb,
    get_rqst_protocol_cb,
    get_rqst_line_cb,
    get_rqst_method_cb,
    get_rqst_url_cb,
    get_rqst_uri_cb,
    get_rqst_path_cb,
    get_rqst_query_str_cb,
    get_rqst_header_size_cb,
    NULL,    // get_rqst_data_w_key_cb_t m_get_rqst_header_w_key_cb;
    get_rqst_header_w_idx_cb,
    NULL,    // get_rqst_body_data_cb_t m_get_rqst_body_str_cb;
    NULL,    // get_rqst_data_cb_t m_get_rqst_local_addr_cb;
    NULL,    // get_rqst_data_size_cb_t m_get_rqst_canonical_port_cb;
    NULL,    // get_rqst_data_size_cb_t m_get_rqst_apparent_cache_status_cb;
    NULL,    // get_rqst_data_size_cb_t m_get_rqst_bytes_out_cb;
    NULL,    // get_rqst_data_size_cb_t m_get_rqst_bytes_in_cb;
    NULL,    // get_rqst_data_cb_t m_get_rqst_uuid_cb;
    NULL     // get_rqst_data_size_cb_t m_get_cust_id_cb;
};

int waflz_profile_process(waflz_transaction_t* tx)
{
    void *l_ctx = tx;
    waflz_pb::event *l_event = NULL;
    ns_waflz::rqst_ctx *l_rqst_ctx = NULL;

    int rc = 0;  //no internal errors; action - Pass;
    l_rqst_ctx = new ns_waflz::rqst_ctx(l_ctx, DEFAULT_BODY_SIZE_MAX, &s_callbacks);
    int32_t l_s = tx->profile->profile->process(&l_event, l_ctx, ns_waflz::PART_MK_ALL, &l_rqst_ctx);
    if (l_s == WAFLZ_STATUS_OK) {
        if (l_event) {
            if (l_event->sub_event_size() >= 1) {
                if (l_event->sub_event(0).has_rule_msg()) {
                    //REQUIRE((l_event->sub_event(0).rule_msg() == "Blacklist IP match"));
                    if (tx->trace) {
                        NDBG_PRINT("event: %s\n", l_event->ShortDebugString().c_str());
                    }
                    tx->event = l_event;  // make event available for the client query
                    l_event = NULL;
                    rc = 1;  //no internal errors; action - not Pass (Deny, etc)
                } else {
                    //TODO error processing
                    rc = -1;  //internal errors
                }
            } else {
                //TODO error processing
                rc = -1;
            }
        }
    } else {
        //TODO error processing
        rc = -1;
    }
    
    if (l_event) {
        delete l_event;
    }
    if (l_rqst_ctx) {
        delete l_rqst_ctx;
    }
    
    return rc;
}

void waflz_profile_cleanup(waflz_profile_t* wp)
{
    delete wp->profile;
    //delete wp->engine;
    delete wp;
}

waflz_transaction_t *waflz_new_transaction(waflz_profile_t *profile, int trace)
{
    waflz_transaction_t *t = new waflz_transaction_t;
    t->profile = profile;  //TODO move to constructor
    t->trace = trace;
    return t;
}

int waflz_transaction_add_request_connection_uri(waflz_transaction_t *t,
                                                 const char *client_ip, const char *host, int port, const char* method, const char *scheme,
                                                 const char *url, const char *uri, const char *path, const char *query, const char *protocol, const char *http_version)
{
    t->client_ip = client_ip;
    t->host = host;
    t->port = port;
    t->method = method;
    t->scheme = scheme;

    t->url = url;
    t->uri = uri;
    t->path = path;
    t->query = query;
    t->protocol = protocol;
    t->http_version = http_version;
    
    return 0;
}

int waflz_transaction_add_request_header(waflz_transaction_t *t, const char *key, const char *value)
{
    t->headers.emplace_back(std::pair<const char*, const char*>(key, value));
    return 0;
}

void waflz_transaction_cleanup(waflz_transaction_t *transaction)
{
    delete transaction;
}

int waflz_transaction_get_event(waflz_transaction_t *transaction, waflz_event_t *event)
{
    if (transaction->event) {
        auto& e = transaction->event->sub_event(0);
        auto s = e.ShortDebugString();
        auto sz = s.size() + 1;
        char* log = (char*)std::malloc(sz);  // a caller will do free()
        if (log) {
            std::strcpy(log, s.c_str());
            event->log = log;
        }
        if (e.has_rule_id()) {
            event->rule_id = e.rule_id();
        }
    }
    return 0;
}
