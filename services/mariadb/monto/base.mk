$(call x-container-service)
V_SERVICE	= monto
V_DEPS		+= /var/lib/mariadb/monto/my.cnf
V_DEPS_ORD	+= /var/lib/mariadb/monto
V_ARGS		+= --mount=type=bind,src=/var/lib/mariadb/monto,dst=/var/lib/mariadb
V_ARGS		+= --mount=type=bind,src=/var/run/mariadb/monto,dst=/var/run/mariadb
V_ARGS		+= --memory=256M
V_ARGS		+= --publish=3307:3306/tcp
V_ARGS 		+= codeberg.org/xens/mariadb:latest
V_ARGS 		+= --user=root
$(call end)

$(call fs-file)
V_PATH		= /var/lib/mariadb/monto/my.cnf
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/mariadb/monto/my.cnf
$(call end)

$(call add-fs-directory,/var/lib/mariadb/monto)
$(call add-fs-directory,/var/run/mariadb/monto)
