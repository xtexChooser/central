$(call fs-file)
V_PATH		= /usr/local/bin/atre
V_SYMLINK	= $(ATRE_DIR)/services/atremis/bin/atre
$(call end)

$(call fs-file)
V_PATH		= /usr/local/bin/tiang
V_SYMLINK	= $(ATRE_DIR)/services/atremis/bin/tiang
$(call end)

ATRE_SYSTEMD_USER_DIR=/home/service/.config/systemd/user

$(call fs-file)
V_PATH		= $(ATRE_SYSTEMD_USER_DIR)/atre-pull.service
V_COPY		= $(ATRE_DIR)/services/atremis/systemd/atre-pull.service
V_POST		= systemd-user-daemon-reload
$(call end)

$(call fs-file)
V_PATH		= $(ATRE_SYSTEMD_USER_DIR)/atre-pull.timer
V_COPY		= $(ATRE_DIR)/services/atremis/systemd/atre-pull.timer
V_DEPS		+= $(ATRE_SYSTEMD_USER_DIR)/atre-pull.service
V_POST		= systemd-user-daemon-reload
$(call end)

$(call systemd-unit)
V_UNIT		= atre-pull.timer
V_USER		= y
V_ENABLED	= y
V_DEPS		= $(ATRE_SYSTEMD_USER_DIR)/atre-pull.timer
$(call end)

$(call loginctl)
V_USER		= service
V_LINGER	= y
$(call end)
