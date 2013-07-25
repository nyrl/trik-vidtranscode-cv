

all-local: lib/osal.a

lib/osal.a: $(TIDSP_PATH_OSAL)/Makefile
	@$(MAKE) $(AM_MAKEFLAGS) \
	    -f $(TIDSP_PATH_OSAL)/Makefile \
	    "OSAL_INSTALL_DIR=$(TIDSP_PATH_OSAL_ROOT)" \
	    "CC=$(CC)" \
	    "CXX=$(CXX)" \
	    "LD=$(LD)" \
	    "AR=$(AR)" \
	    "CFLAGS=$(CFLAGS)" \
	    "CXXFLAGS=$(CXXFLAGS)" \
	    "LDFLAGS=$(LDFLAGS)" \
	    "SUFFIX=$(subst GCArm,,$(TIDSP_GPP_ARCH))" \
	    "XDC_TARGET=$(TIDSP_GPP_ARCH)" \
	    all


check-local:
	@true

clean-local:
	@$(MAKE) $(AM_MAKEFLAGS) \
	    -f $(TIDSP_PATH_OSAL)/Makefile \
	    "OSAL_INSTALL_DIR=$(TIDSP_PATH_OSAL_ROOT)" \
	    "XDC_TARGET=$(TIDSP_GPP_ARCH)" \
	    clean
	@rm -rf lib


