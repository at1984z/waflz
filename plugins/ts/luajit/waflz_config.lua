-- module containing values to be persisted across "traffic_ctl config reload"

local waflz_config = {}

waflz_config.rulesfile = "/usr/local/var/waflz/example.conf"
waflz_config.rules = nil

return waflz_config
