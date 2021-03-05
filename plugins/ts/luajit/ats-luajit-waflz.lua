ts.add_package_path('/usr/local/var/lua/?.lua')

local waflz = require("waflz")
local waflz_config = require("waflz_config")
local ffi = require("ffi")

local waflz_profile

-- Initialization. Load the waflz configuration passed to this lua module
function __init__(argtb)
  if (#argtb) < 2 then
    ts.error("No Waflz Conf is given")
    return -1 
  end  

  waflz_config.rules_dir = argtb[1]	
  ts.debug("Waflz Rules dir is " .. waflz_config.rules_dir)

  waflz_config.profile_file_name = argtb[2]	
  ts.debug("Waflz Profile file is " .. waflz_config.profile_file_name)

  local result = waflz.waflz_profile_new_load(waflz_config.rules_dir, waflz_config.profile_file_name)
  waflz_profile = result;

end

-- Reload waflz configuration. Trigger by "traffic_ctl config reload"
function __reload__()
  ts.debug("Reloading Waflz Conf: ")
  -- TODO  implement
  
end

-- Entry point function run for each incoming request
function do_global_read_request()
  if(waflz_config.rules_dir == nil) then
    ts.debug("No rules loaded. Thus there is no processing done")
    return 0
  end
  local trace = 1
  local txn = waflz.waflz_new_transaction(waflz_profile, trace)

  -- processing for the connection information
  local client_ip, client_port, client_ip_family = ts.client_request.client_addr.get_addr()
  local incoming_port = ts.client_request.client_addr.get_incoming_port()
  local host = ts.client_request.get_url_host()
  local method = ts.client_request.get_method()
  local scheme = ts.client_request.get_url_scheme()

  -- processing for the uri information
  -- local url = ts.client_request.get_url()  -- http://host/path?query
  local path = ts.client_request.get_uri()  -- /path
  local uri = path  -- /path?query
  local query = ts.client_request.get_uri_args() or ''
  if (query ~= '') then
    uri = uri .. '?' .. query
  end
  local url = uri  -- in order to get consistent request line for waflz
  local protocol = 'HTTP'
  local http_version = ts.client_request.get_version()
  waflz.waflz_transaction_add_request_connection_uri(txn, client_ip, host, incoming_port, method, scheme, url, uri, path, query, protocol, http_version)

  -- processing for the request headers
  local hdrs = ts.client_request.get_headers()
  for k, v in pairs(hdrs) do
    waflz.waflz_transaction_add_request_header(txn, k, v)
  end
  local status = waflz.waflz_profile_process(txn)
  ts.debug("done with processing request: " .. status)

  -- detect if intervention is needed
  ts.ctx['status'] = nil
  if (status == 1) then
    ts.hook(TS_LUA_HOOK_SEND_RESPONSE_HDR, send_response)
    ts.ctx['status'] = 503
    ts.http.set_resp(503)
    waflz.waflz_transaction_cleanup(txn)
    ts.debug("done with setting custom response")
    return 0
  end  

  waflz.waflz_transaction_cleanup(txn)
  return 0
end

-- function run when sending response to client 
function send_response()
  -- retrieve status and reset the response with it
  local status = ts.ctx['status']
  if (status ~= nil) then
    ts.client_response.set_error_resp(status, 'Contents Reset by Waflz\n')
  end 
end
