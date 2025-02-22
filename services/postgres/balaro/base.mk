BALARO_VERSION ?= 17.3

$(call x-container-service)
V_SERVICE	= balaro
V_DEPS		+= /var/lib/postgresql/balaro/postgres.conf
V_DEPS		+= /var/lib/postgresql/balaro/postgresql.conf
V_DEPS		+= /var/lib/postgresql/balaro/pg_ident.conf
V_DEPS		+= /var/lib/postgresql/balaro/pg_hba.conf
V_DEPS_ORD	+= /var/lib/postgresql/balaro /var/lib/postgresql/balaro/data /var/run/postgresql
V_ARGS		+= --mount=type=bind,src=/var/lib/postgresql/balaro,dst=/var/lib/postgresql
V_ARGS		+= --mount=type=bind,src=/var/lib/postgresql/balaro/data,dst=/var/lib/postgresql/data
V_ARGS		+= --mount=type=bind,src=/var/run/postgresql,dst=/var/run/postgresql
V_ARGS		+= --memory=128M
V_ARGS		+= --user=root:root
V_ARGS		+= --publish=5433:5432/tcp
V_ARGS 		+= codeberg.org/xens/postgres:$(strip $(BALARO_VERSION))
$(call end)

$(call fs-file)
V_PATH		= /var/lib/postgresql/balaro/postgres.conf
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/postgres/balaro/postgres.conf
$(call end)

$(call fs-file)
V_PATH		= /var/lib/postgresql/balaro/postgresql.conf
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/postgres/balaro/postgresql.conf
$(call end)

$(call fs-file)
V_PATH		= /var/lib/postgresql/balaro/pg_ident.conf
V_CREATE	= empty
$(call end)

$(call fs-file)
V_PATH		= /var/lib/postgresql/balaro/pg_hba.conf
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/postgres/balaro/pg_hba.conf
$(call end)

$(call add-fs-directory,/var/lib/postgresql/balaro)
$(call add-fs-directory,/var/lib/postgresql/balaro/data)
$(call add-fs-directory,/var/run/postgresql)
