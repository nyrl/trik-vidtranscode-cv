

all-local: lib/cstubs.a

lib/cstubs.a: $(TIDSP_PATH_CSTUBS)/Makefile
	@$(MAKE) $(AM_MAKEFLAGS) \
	    -f $(TIDSP_PATH_CSTUBS)/Makefile \
	    "OSAL_INSTALL_DIR=$(TIDSP_PATH_OSAL_ROOT)" \
	    "CC=$(CC)" \
	    "LD=$(LD)" \
	    "AR=$(AR)" \
	    "CFLAGS=$(CFLAGS)" \
	    "LDFLAGS=$(LDFLAGS)" \
	    "XDC_TARGET=$(TIDSP_GPP_ARCH)" \
	    all


check-local:
	@true

clean-local:
	@$(MAKE) $(AM_MAKEFLAGS) \
	    -f $(TIDSP_PATH_CSTUBS)/Makefile \
	    "OSAL_INSTALL_DIR=$(TIDSP_PATH_OSAL_ROOT)" \
	    "XDC_TARGET=$(TIDSP_GPP_ARCH)" \
	    clean
	@rm -rf lib


