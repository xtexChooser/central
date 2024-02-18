X_CONTAINER_SERVICE_VARS = V_TARGET_NAME V_SERVICE V_STOPPED V_ARGS V_POST V_SVCDEPS $(v-deps-var)

define x-container-service0
$(eval V_PIDFILE?=/var/run/containers/$(V_SERVICE).pid)
$(eval V_DEP_VARS+=$(addprefix x-container-$(V_SERVICE)-,args start-cmd stop-cmd))
$(eval V_PRE_START?=true)
$(eval V_POST_START?=true)
$(eval V_PRE_STOP?=true)
$(eval V_POST_STOP?=true)
$(eval x-container-$(V_SERVICE)-args:=$(V_ARGS))
$(eval x-container-$(V_SERVICE)-start-cmd:=$(V_PRE_START); $(PODMAN) container run \
	--name $(V_SERVICE) --rm -d --pidfile=$(V_PIDFILE) --replace \
	--hostname=$(V_SERVICE) \
	$(V_ARGS); $(V_POST_START))
$(eval x-container-$(V_SERVICE)-stop-cmd:=$(V_PRE_STOP); $(PODMAN) container rm -f -i $(V_SERVICE); \
	rm -rf $(V_PIDFILE); $(V_POST_STOP))

$(call mktrace, Define x-container-service target: $(V_SERVICE))
$(call mktrace-vars,$(X_CONTAINER_SERVICE_VARS))

$(DINITD_DIR)/$(V_SERVICE): $(v-deps) $(VENDOR_MODULES_DIR)/containers.mk
	@cat >$$@ <<EOF
	type = bgprocess
	command = bash -c "$(subst ",\",$(subst $$$$,\$$$$,$(x-container-$(V_SERVICE)-start-cmd)))"
	stop-command = bash -c "$(subst ",\",$(subst $$$$,\$$$$,$(x-container-$(V_SERVICE)-stop-cmd)))"
	pid-file = $(V_PIDFILE)
	restart = true
	EOF
	$(DINITCTL) stop --force --ignore-unstarted $(V_SERVICE)
	$(DINITCTL) reload $(V_SERVICE)

$$(call dinit-service)
V_SERVICE	= $(V_SERVICE)
V_RUNNING	= $(call not,$(V_STOPPED))
V_DEPS		= $(V_SVCDEPS) $(DINITD_DIR)/$(V_SERVICE)
$$(call end)

$$(call fs-file)
V_PATH		= $$(DINITD_DIR)/boot.d/$(V_SERVICE)
$(if $(call not,$(call is-true,$(V_STOPPED))),V_SYMLINK = ../$(V_SERVICE),V_EXIST = n)
$$(call end)

$(call unset-vars)
endef
$(call define-func, x-container-service)
