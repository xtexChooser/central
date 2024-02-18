# ========== Atremis Executables ==========

$(call fs-file)
V_PATH		= /usr/local/bin/atre
V_SYMLINK	= $(ATRE_DIR)/services/atremis/bin/atre
$(call end)

$(call fs-file)
V_PATH		= /usr/local/bin/tiang
V_SYMLINK	= $(ATRE_DIR)/services/atremis/bin/tiang
$(call end)

# ========== Atremis Systemd Services ==========

$(call fs-file)
V_PATH		= $(SYSTEMD_UNITS_DIR)/atre-pull.service
V_COPY		= $(ATRE_DIR)/services/atremis/systemd/atre-pull.service
V_POST		= systemd-daemon-reload
$(call end)

$(call fs-file)
V_PATH		= $(SYSTEMD_UNITS_DIR)/atre-pull.timer
V_COPY		= $(ATRE_DIR)/services/atremis/systemd/atre-pull.timer
V_DEPS		+= $(SYSTEMD_UNITS_DIR)/atre-pull.service
V_POST		= systemd-daemon-reload
$(call end)

$(call systemd-unit)
V_UNIT		= atre-pull.timer
V_ENABLED	= y
V_RUNNING	= y
$(call end)

# ========== dinit ==========
$(call package)
V_PKG		= dinit
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/dinit
$(call end)

$(call package)
V_PKG		= dinit-systemd
V_INSTALLED	= y
V_INST_FILE	= /usr/lib/systemd/system/dinit.service
$(call end)

$(call systemd-unit)
V_UNIT		= dinit.service
V_ENABLED	= y
V_RUNNING	= y
V_DEPS		= pkg-dinit-systemd $(DINITD_DIR)/boot
$(call end)

$(call fs-directory)
V_PATH		= $(DINITD_DIR)
V_EXIST		= y
$(call end)

$(call fs-file)
V_PATH		= $(DINITD_DIR)/boot
V_TEMPLATE	= bash-tpl $(STATES_DIR)/services/atremis/dinit.d/boot
$(call end)

$(call fs-directory)
V_PATH		= $(DINITD_DIR)/boot.d
V_EXIST		= y
$(call end)

# ========== cronie ==========
$(call package)
V_PKG		= cronie
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/crond
$(call end)

$(call systemd-unit)
V_UNIT		= cronie.service
V_ENABLED	= y
V_RUNNING	= y
V_DEPS		= pkg-cronie
$(call end)

# ========== packages ==========
$(call package)
V_PKG		= podman
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/podman
$(call end)
$(call run-on-apply, pkg-podman)

# ========== maintainer packages ==========
$(call package)
V_PKG		= ripgrep
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/rg
$(call end)
$(call package)
V_PKG		= neovim
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/nvim
$(call end)
$(call package)
V_PKG		= fish
V_INSTALLED	= y
V_INST_FILE	= /usr/bin/fish
$(call end)
