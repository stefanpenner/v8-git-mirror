# Copyright 2012 the V8 project authors. All rights reserved.
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
#     * Neither the name of Google Inc. nor the names of its
#       contributors may be used to endorse or promote products derived
#       from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


# Variable default definitions. Override them by exporting them in your shell.
OUTDIR ?= out
TESTJOBS ?=
GYPFLAGS ?=
TESTFLAGS ?=
ANDROID_NDK_HOST_ARCH ?=
ANDROID_V8 ?= /data/local/tmp/v8
NACL_SDK_ROOT ?=

# Special build flags. Use them like this: "make library=shared"

# library=shared || component=shared_library
ifeq ($(library), shared)
  GYPFLAGS += -Dcomponent=shared_library
endif
ifdef component
  GYPFLAGS += -Dcomponent=$(component)
endif
# disassembler=on
ifeq ($(disassembler), on)
  GYPFLAGS += -Dv8_enable_disassembler=1
endif
# objectprint=on
ifeq ($(objectprint), on)
  GYPFLAGS += -Dv8_object_print=1
endif
# verifyheap=on
ifeq ($(verifyheap), on)
  GYPFLAGS += -Dv8_enable_verify_heap=1
endif
# tracemaps=on
ifeq ($(tracemaps), on)
  GYPFLAGS += -Dv8_trace_maps=1
endif
# backtrace=off
ifeq ($(backtrace), off)
  GYPFLAGS += -Dv8_enable_backtrace=0
else
  GYPFLAGS += -Dv8_enable_backtrace=1
endif
# verifypredictable=on
ifeq ($(verifypredictable), on)
  GYPFLAGS += -Dv8_enable_verify_predictable=1
endif
# snapshot=off
ifeq ($(snapshot), off)
  GYPFLAGS += -Dv8_use_snapshot='false'
endif
ifeq ($(snapshot), external)
  GYPFLAGS += -Dv8_use_external_startup_data=1
endif
# extrachecks=on/off
ifeq ($(extrachecks), on)
  GYPFLAGS += -Ddcheck_always_on=1 -Dv8_enable_handle_zapping=1
endif
ifeq ($(extrachecks), off)
  GYPFLAGS += -Ddcheck_always_on=0 -Dv8_enable_handle_zapping=0
endif
# slowdchecks=on/off
ifeq ($(slowdchecks), on)
  GYPFLAGS += -Dv8_enable_slow_dchecks=1
endif
ifeq ($(slowdchecks), off)
  GYPFLAGS += -Dv8_enable_slow_dchecks=0
endif
# debugsymbols=on
ifeq ($(debugsymbols), on)
  GYPFLAGS += -Drelease_extra_cflags=-ggdb3
endif
# gdbjit=on/off
ifeq ($(gdbjit), on)
  GYPFLAGS += -Dv8_enable_gdbjit=1
endif
ifeq ($(gdbjit), off)
  GYPFLAGS += -Dv8_enable_gdbjit=0
endif
# vtunejit=on
ifeq ($(vtunejit), on)
  GYPFLAGS += -Dv8_enable_vtunejit=1
endif
# unalignedaccess=on
ifeq ($(unalignedaccess), on)
  GYPFLAGS += -Dv8_can_use_unaligned_accesses=true
endif
# randomseed=12345, disable random seed via randomseed=0
ifdef randomseed
  GYPFLAGS += -Dv8_random_seed=$(randomseed)
endif
# soname_version=1.2.3
ifdef soname_version
  GYPFLAGS += -Dsoname_version=$(soname_version)
endif
# werror=no
ifeq ($(werror), no)
  GYPFLAGS += -Dwerror=''
endif
# presubmit=no
ifeq ($(presubmit), no)
  TESTFLAGS += --no-presubmit
endif
# strictaliasing=off (workaround for GCC-4.5)
ifeq ($(strictaliasing), off)
  GYPFLAGS += -Dv8_no_strict_aliasing=1
endif
# regexp=interpreted
ifeq ($(regexp), interpreted)
  GYPFLAGS += -Dv8_interpreted_regexp=1
endif
# i18nsupport=off
ifeq ($(i18nsupport), off)
  GYPFLAGS += -Dv8_enable_i18n_support=0
  TESTFLAGS += --noi18n
endif
# deprecationwarnings=on
ifeq ($(deprecationwarnings), on)
  GYPFLAGS += -Dv8_deprecation_warnings=1
endif
# vectorstores=on
ifeq ($(vectorstores), on)
  GYPFLAGS += -Dv8_vector_stores=1
endif
# imminentdeprecationwarnings=on
ifeq ($(imminentdeprecationwarnings), on)
  GYPFLAGS += -Dv8_imminent_deprecation_warnings=1
endif
# asan=on
ifeq ($(asan), on)
  GYPFLAGS += -Dasan=1 -Dclang=1
  TESTFLAGS += --asan
  ifeq ($(lsan), on)
    GYPFLAGS += -Dlsan=1
  endif
endif
ifdef embedscript
  GYPFLAGS += -Dembed_script=$(embedscript)
endif
ifeq ($(goma), on)
  GYPFLAGS += -Duse_goma=1
endif
# arm specific flags.
# arm_version=<number | "default">
ifneq ($(strip $(arm_version)),)
  GYPFLAGS += -Darm_version=$(arm_version)
else
# Deprecated (use arm_version instead): armv7=false/true
ifeq ($(armv7), false)
  GYPFLAGS += -Darm_version=6
else
ifeq ($(armv7), true)
  GYPFLAGS += -Darm_version=7
endif
endif
endif
# hardfp=on/off. Deprecated, use armfloatabi
ifeq ($(hardfp),on)
  GYPFLAGS += -Darm_float_abi=hard
else
ifeq ($(hardfp),off)
  GYPFLAGS += -Darm_float_abi=softfp
endif
endif
# fpu: armfpu=xxx
# xxx: vfp, vfpv3-d16, vfpv3, neon.
ifeq ($(armfpu),)
  GYPFLAGS += -Darm_fpu=default
else
  GYPFLAGS += -Darm_fpu=$(armfpu)
endif
# float abi: armfloatabi=softfp/hard
ifeq ($(armfloatabi),)
ifeq ($(hardfp),)
  GYPFLAGS += -Darm_float_abi=default
endif
else
  GYPFLAGS += -Darm_float_abi=$(armfloatabi)
endif
# armthumb=on/off
ifeq ($(armthumb), off)
  GYPFLAGS += -Darm_thumb=0
else
ifeq ($(armthumb), on)
  GYPFLAGS += -Darm_thumb=1
endif
endif
# arm_test_noprobe=on
# With this flag set, by default v8 will only use features implied
# by the compiler (no probe). This is done by modifying the default
# values of enable_armv7, enable_vfp3, enable_32dregs and enable_neon.
# Modifying these flags when launching v8 will enable the probing for
# the specified values.
ifeq ($(arm_test_noprobe), on)
  GYPFLAGS += -Darm_test_noprobe=on
endif

# Optionally enable wasm prototype.
# Assume you've placed a link to v8-native-prototype in third_party/wasm.
ifeq ($(wasm), on)
  GYPFLAGS += -Dv8_wasm=1
endif

# ----------------- available targets: --------------------
# - "grokdump": rebuilds heap constants lists used by grokdump
# - any arch listed in ARCHES (see below)
# - any mode listed in MODES
# - every combination <arch>.<mode>, e.g. "ia32.release"
# - "native": current host's architecture, release mode
# - any of the above with .check appended, e.g. "ia32.release.check"
# - "android": cross-compile for Android/ARM
# - "nacl" : cross-compile for Native Client (ia32 and x64)
# - default (no target specified): build all DEFAULT_ARCHES and MODES
# - "check": build all targets and run all tests
# - "<arch>.clean" for any <arch> in ARCHES
# - "clean": clean all ARCHES

# ----------------- internal stuff ------------------------

# Architectures and modes to be compiled. Consider these to be internal
# variables, don't override them (use the targets instead).
ARCHES = ia32 x64 x32 arm arm64 mips mipsel mips64el x87 ppc ppc64
DEFAULT_ARCHES = ia32 x64 arm
MODES = release debug optdebug
DEFAULT_MODES = release debug
ANDROID_ARCHES = android_ia32 android_x64 android_arm android_arm64 \
		 android_mipsel android_x87
NACL_ARCHES = nacl_ia32 nacl_x64

# List of files that trigger Makefile regeneration:
GYPFILES = third_party/icu/icu.gypi third_party/icu/icu.gyp \
	   build/shim_headers.gypi build/features.gypi build/standalone.gypi \
	   build/toolchain.gypi build/all.gyp build/mac/asan.gyp \
	   test/cctest/cctest.gyp \
	   test/unittests/unittests.gyp tools/gyp/v8.gyp \
	   tools/parser-shell.gyp testing/gmock.gyp testing/gtest.gyp \
	   buildtools/third_party/libc++abi/libc++abi.gyp \
	   buildtools/third_party/libc++/libc++.gyp samples/samples.gyp \
	   src/third_party/vtune/v8vtune.gyp src/d8.gyp

# If vtunejit=on, the v8vtune.gyp will be appended.
ifeq ($(vtunejit), on)
  GYPFILES += src/third_party/vtune/v8vtune.gyp
endif
# Generates all combinations of ARCHES and MODES, e.g. "ia32.release".
BUILDS = $(foreach mode,$(MODES),$(addsuffix .$(mode),$(ARCHES)))
ANDROID_BUILDS = $(foreach mode,$(MODES), \
                   $(addsuffix .$(mode),$(ANDROID_ARCHES)))
NACL_BUILDS = $(foreach mode,$(MODES), \
                   $(addsuffix .$(mode),$(NACL_ARCHES)))
# Generates corresponding test targets, e.g. "ia32.release.check".
CHECKS = $(addsuffix .check,$(BUILDS))
QUICKCHECKS = $(addsuffix .quickcheck,$(BUILDS))
ANDROID_CHECKS = $(addsuffix .check,$(ANDROID_BUILDS))
NACL_CHECKS = $(addsuffix .check,$(NACL_BUILDS))
# File where previously used GYPFLAGS are stored.
ENVFILE = $(OUTDIR)/environment

.PHONY: all check clean builddeps dependencies $(ENVFILE).new native \
        qc quickcheck $(QUICKCHECKS) turbocheck \
        $(addsuffix .quickcheck,$(MODES)) $(addsuffix .quickcheck,$(ARCHES)) \
        $(ARCHES) $(MODES) $(BUILDS) $(CHECKS) $(addsuffix .clean,$(ARCHES)) \
        $(addsuffix .check,$(MODES)) $(addsuffix .check,$(ARCHES)) \
        $(ANDROID_ARCHES) $(ANDROID_BUILDS) $(ANDROID_CHECKS) \
        $(NACL_ARCHES) $(NACL_BUILDS) $(NACL_CHECKS) \
        must-set-NACL_SDK_ROOT

# Target definitions. "all" is the default.
all: $(DEFAULT_MODES)

# Special target for the buildbots to use. Depends on $(OUTDIR)/Makefile
# having been created before.
buildbot:
	$(MAKE) -C "$(OUTDIR)" BUILDTYPE=$(BUILDTYPE) \
	        builddir="$(abspath $(OUTDIR))/$(BUILDTYPE)"

# Compile targets. MODES and ARCHES are convenience targets.
.SECONDEXPANSION:
$(MODES): $(addsuffix .$$@,$(DEFAULT_ARCHES))

$(ARCHES): $(addprefix $$@.,$(DEFAULT_MODES))

# Defines how to build a particular target (e.g. ia32.release).
$(BUILDS): $(OUTDIR)/Makefile.$$@
	@$(MAKE) -C "$(OUTDIR)" -f Makefile.$@ \
	         BUILDTYPE=$(shell echo $(subst .,,$(suffix $@)) | \
	                     python -c "print \
	                     raw_input().replace('opt', '').capitalize()") \
	         builddir="$(shell pwd)/$(OUTDIR)/$@"

native: $(OUTDIR)/Makefile.native
	@$(MAKE) -C "$(OUTDIR)" -f Makefile.native \
	         BUILDTYPE=Release \
	         builddir="$(shell pwd)/$(OUTDIR)/$@"

$(ANDROID_ARCHES): $(addprefix $$@.,$(MODES))

$(ANDROID_BUILDS): $(GYPFILES) $(ENVFILE) Makefile.android
	@$(MAKE) -f Makefile.android $@ \
	        ARCH="$(basename $@)" \
	        MODE="$(subst .,,$(suffix $@))" \
	        OUTDIR="$(OUTDIR)" \
	        GYPFLAGS="$(GYPFLAGS)"

$(NACL_ARCHES): $(addprefix $$@.,$(MODES))

$(NACL_BUILDS): $(GYPFILES) $(ENVFILE) \
		   Makefile.nacl must-set-NACL_SDK_ROOT
	@$(MAKE) -f Makefile.nacl $@ \
	        ARCH="$(basename $@)" \
	        MODE="$(subst .,,$(suffix $@))" \
	        OUTDIR="$(OUTDIR)" \
	        GYPFLAGS="$(GYPFLAGS)"

# Test targets.
check: all
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch=$(shell echo $(DEFAULT_ARCHES) | sed -e 's/ /,/g') \
	    $(TESTFLAGS)

$(addsuffix .check,$(MODES)): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --mode=$(basename $@) $(TESTFLAGS)

$(addsuffix .check,$(ARCHES)): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch=$(basename $@) $(TESTFLAGS)

$(CHECKS): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(basename $@) $(TESTFLAGS)

$(addsuffix .quickcheck,$(MODES)): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --mode=$(basename $@) $(TESTFLAGS) --quickcheck

$(addsuffix .quickcheck,$(ARCHES)): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch=$(basename $@) $(TESTFLAGS) --quickcheck

$(QUICKCHECKS): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(basename $@) $(TESTFLAGS) --quickcheck

$(addsuffix .sync, $(ANDROID_BUILDS)): $$(basename $$@)
	@tools/android-sync.sh $(basename $@) $(OUTDIR) \
	                       $(shell pwd) $(ANDROID_V8)

$(addsuffix .check, $(ANDROID_BUILDS)): $$(basename $$@).sync
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	     --arch-and-mode=$(basename $@) \
	     --timeout=600 \
	     --command-prefix="tools/android-run.py" $(TESTFLAGS)

$(addsuffix .check, $(ANDROID_ARCHES)): \
                $(addprefix $$(basename $$@).,$(MODES)).check

$(addsuffix .check, $(NACL_BUILDS)): $$(basename $$@)
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	     --arch-and-mode=$(basename $@) \
	     --timeout=600 --nopresubmit --noi18n \
	     --command-prefix="tools/nacl-run.py"

$(addsuffix .check, $(NACL_ARCHES)): \
                $(addprefix $$(basename $$@).,$(MODES)).check

native.check: native
	@tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR)/native \
	    --arch-and-mode=. $(TESTFLAGS)

SUPERFASTTESTMODES = ia32.release
FASTTESTMODES = $(SUPERFASTTESTMODES),x64.release,ia32.optdebug,x64.optdebug,arm.optdebug,arm64.release
FASTCOMPILEMODES = $(FASTTESTMODES),arm64.optdebug

COMMA = ,
EMPTY =
SPACE = $(EMPTY) $(EMPTY)
quickcheck: $(subst $(COMMA),$(SPACE),$(FASTCOMPILEMODES))
	tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(SUPERFASTTESTMODES) $(TESTFLAGS) --quickcheck \
	    --download-data mozilla webkit
	tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(FASTTESTMODES) $(TESTFLAGS) --quickcheck
qc: quickcheck

turbocheck: $(subst $(COMMA),$(SPACE),$(FASTCOMPILEMODES))
	tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(SUPERFASTTESTMODES) $(TESTFLAGS) \
	    --quickcheck --variants=turbofan --download-data mozilla webkit
	tools/run-tests.py $(TESTJOBS) --outdir=$(OUTDIR) \
	    --arch-and-mode=$(FASTTESTMODES) $(TESTFLAGS) \
	    --quickcheck --variants=turbofan
tc: turbocheck

# Clean targets. You can clean each architecture individually, or everything.
$(addsuffix .clean, $(ARCHES) $(ANDROID_ARCHES) $(NACL_ARCHES)):
	rm -f $(OUTDIR)/Makefile.$(basename $@)*
	rm -rf $(OUTDIR)/$(basename $@).release
	rm -rf $(OUTDIR)/$(basename $@).debug
	rm -rf $(OUTDIR)/$(basename $@).optdebug
	find $(OUTDIR) -regex '.*\(host\|target\)\.$(basename $@).*\.mk' -delete

native.clean:
	rm -f $(OUTDIR)/Makefile.native
	rm -rf $(OUTDIR)/native
	find $(OUTDIR) -regex '.*\(host\|target\)\.native\.mk' -delete

clean: $(addsuffix .clean, $(ARCHES) $(ANDROID_ARCHES) $(NACL_ARCHES)) native.clean gtags.clean

# GYP file generation targets.
OUT_MAKEFILES = $(addprefix $(OUTDIR)/Makefile.,$(BUILDS))
$(OUT_MAKEFILES): $(GYPFILES) $(ENVFILE)
	$(eval CXX_TARGET_ARCH:=$(shell $(CXX) -v 2>&1 | grep ^Target: | \
	        cut -f 2 -d " " | cut -f 1 -d "-" ))
	$(eval CXX_TARGET_ARCH:=$(subst aarch64,arm64,$(CXX_TARGET_ARCH)))
	$(eval CXX_TARGET_ARCH:=$(subst x86_64,x64,$(CXX_TARGET_ARCH)))
	$(eval V8_TARGET_ARCH:=$(subst .,,$(suffix $(basename $@))))
	PYTHONPATH="$(shell pwd)/tools/generate_shim_headers:$(shell pwd)/build:$(PYTHONPATH):$(shell pwd)/build/gyp/pylib:$(PYTHONPATH)" \
	GYP_GENERATORS=make \
	build/gyp/gyp --generator-output="$(OUTDIR)" build/all.gyp \
	              -Ibuild/standalone.gypi --depth=. \
	              -Dv8_target_arch=$(V8_TARGET_ARCH) \
	              $(if $(findstring $(CXX_TARGET_ARCH),$(V8_TARGET_ARCH)), \
	              -Dtarget_arch=$(V8_TARGET_ARCH),) \
	              $(if $(findstring optdebug,$@),-Dv8_optimized_debug=1,) \
	              -S$(suffix $(basename $@))$(suffix $@) $(GYPFLAGS)

$(OUTDIR)/Makefile.native: $(GYPFILES) $(ENVFILE)
	PYTHONPATH="$(shell pwd)/tools/generate_shim_headers:$(shell pwd)/build:$(PYTHONPATH):$(shell pwd)/build/gyp/pylib:$(PYTHONPATH)" \
	GYP_GENERATORS=make \
	build/gyp/gyp --generator-output="$(OUTDIR)" build/all.gyp \
	              -Ibuild/standalone.gypi --depth=. -S.native $(GYPFLAGS)

# Note that NACL_SDK_ROOT must be set to point to an appropriate
# Native Client SDK before using this makefile. You can download
# an SDK here:
#   https://developers.google.com/native-client/sdk/download
# The path indicated by NACL_SDK_ROOT will typically end with
# a folder for a pepper version such as "pepper_25" that should
# have "tools" and "toolchain" subdirectories.
must-set-NACL_SDK_ROOT:
ifndef NACL_SDK_ROOT
	  $(error NACL_SDK_ROOT must be set)
endif

# Replaces the old with the new environment file if they're different, which
# will trigger GYP to regenerate Makefiles.
$(ENVFILE): $(ENVFILE).new
	@if test -r $(ENVFILE) && cmp $(ENVFILE).new $(ENVFILE) > /dev/null; \
	    then rm $(ENVFILE).new; \
	    else mv $(ENVFILE).new $(ENVFILE); fi

# Stores current GYPFLAGS in a file.
$(ENVFILE).new:
	$(eval CXX_TARGET_ARCH:=$(shell $(CXX) -v 2>&1 | grep ^Target: | \
	        cut -f 2 -d " " | cut -f 1 -d "-" ))
	$(eval CXX_TARGET_ARCH:=$(subst aarch64,arm64,$(CXX_TARGET_ARCH)))
	$(eval CXX_TARGET_ARCH:=$(subst x86_64,x64,$(CXX_TARGET_ARCH)))
	@mkdir -p $(OUTDIR); echo "GYPFLAGS=$(GYPFLAGS) -Dtarget_arch=$(CXX_TARGET_ARCH)" > $(ENVFILE).new;

# Heap constants for grokdump.
DUMP_FILE = tools/v8heapconst.py
grokdump: ia32.release
	@cat $(DUMP_FILE).tmpl > $(DUMP_FILE)
	@$(OUTDIR)/ia32.release/d8 --dump-heap-constants >> $(DUMP_FILE)

# Support for the GNU GLOBAL Source Code Tag System.
gtags.files: $(GYPFILES) $(ENVFILE)
	@find include src test -name '*.h' -o -name '*.cc' -o -name '*.c' > $@

# We need to manually set the stack limit here, to work around bugs in
# gmake-3.81 and global-5.7.1 on recent 64-bit Linux systems.
GPATH GRTAGS GSYMS GTAGS: gtags.files $(shell cat gtags.files 2> /dev/null)
	@bash -c 'ulimit -s 10240 && GTAGSFORCECPP=yes gtags -i -q -f $<'

gtags.clean:
	rm -f gtags.files GPATH GRTAGS GSYMS GTAGS

dependencies builddeps:
	$(error Use 'gclient sync' instead)
