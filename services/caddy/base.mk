$(call x-container-service)
V_SERVICE	= caddy
V_DEPS		+= /etc/caddy/Caddyfile
V_ARGS		+= --cap-add=CAP_NET_BIND_SERVICE
V_ARGS		+= --env HOME=/root
V_ARGS		+= --mount=type=bind,src=/etc/caddy,dst=/etc/caddy,ro=true
V_ARGS		+= --mount=type=bind,src=/var/run,dst=/var/run
V_ARGS		+= --mount=type=bind,src=/var/lib/caddy,dst=/data/caddy
V_ARGS		+= --publish=80:80/tcp --publish=80:80/udp
V_ARGS		+= --publish=443:443/tcp --publish=443:443/udp
V_ARGS		+= --memory=64M
$(call invoke-hooks,caddy-container-opts)
V_ARGS 		+= codeberg.org/xvnet/x-caddy
$(call end)

CADDY_INCLUDES :=
$(call fs-file)
V_PATH		= /etc/caddy/Caddyfile
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/caddy/Caddyfile
V_DEP_VARS	+= CADDY_INCLUDES
$(call end)
$(call defer-deps,/etc/caddy/Caddyfile,CADDY_INCLUDES)
