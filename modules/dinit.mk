DINITCTL = dinitctl
DINITCTL_SYSTEM = dinitctl --system

DINIT_SERVICE_VARS = V_TARGET_NAME V_SERVICE V_RUNNING V_SYSTEM V_DINITCTL V_POST $(v-deps-var)
define dinit-service0
$(eval V_TARGET_NAME?=dinit-$(V_SERVICE))
$(eval V_DINITCTL ?= $(if $(V_SYSTEM),$(DINITCTL_SYSTEM),$(DINITCTL)))

$(call mktrace, Define dinit service target: $(V_SERVICE))
$(call mktrace-vars,$(DINIT_SERVICE_VARS))
$(call apply-target,$(V_TARGET_NAME))
$(call vt-target,$(V_TARGET_NAME))
$(V_TARGET_NAME): $(v-deps) $(call imp-dep,pkg,dinit) \
		$(if $(V_SYSTEM),$(call file-imp-dep,/etc/dinit.d/$(V_SERVICE)) \
		$(call file-imp-dep,/lib/dinit.d/$(V_SERVICE)) \
		$(call file-imp-dep,/run/dinit.d/$(V_SERVICE)) \
		$(call file-imp-dep,/usr/local/lib/dinit.d/$(V_SERVICE))), \
		$(call file-imp-dep,$(HOME)/.config/dinit.d/$(V_SERVICE))
	export E_MAJOR=dinit E_SERVICE=$(V_SERVICE)
$(if $(call is-true,$(V_RUNNING)),
	if ! $(V_DINITCTL) is-started $(V_SERVICE) $(DROP_STDOUT); then
		$(V_DINITCTL) start $(V_SERVICE)
		$(call succ, Started dinit service $(V_SERVICE))
		$(call vpost, E_MINOR=started)
	fi
)
$(if $(call is-false,$(V_RUNNING)),
	if $(V_DINITCTL) is-started $(V_SERVICE) $(DROP_STDOUT); then
		$(V_DINITCTL) stop $(V_SERVICE)
		$(call succ, Stopped dinit service $(V_SERVICE))
		$(call vpost, E_MINOR=stopped)
	fi
)

$(call unset-vars)
endef

$(call define-func, dinit-service)

$(call vt-target, dinit-start dinit-stop dinit-restart dinit-reload dinit-shutdown)
dinit-start:
	$(DINITCTL) start $(E_SERVICE)
	$(call succ, Started dinit service $(E_SERVICE))
dinit-stop:
	$(DINITCTL) stop $(E_SERVICE)
	$(call succ, Stopped dinit service $(E_SERVICE))
dinit-restart:
	$(DINITCTL) restart $(E_SERVICE)
	$(call succ, Restarted dinit service $(E_SERVICE))
dinit-reload:
	$(DINITCTL) reload $(E_SERVICE)
	$(call succ, Reloaded dinit service $(E_SERVICE))
dinit-shutdown:
	$(DINITCTL) shutdown
	$(call succ, Shutting down dinit daemon)
