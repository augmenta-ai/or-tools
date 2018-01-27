BASE_ORTOOLS_DLL_NAME=Google.OrTools
FSHARP_ORTOOLS_DLL_NAME=$(BASE_ORTOOLS_DLL_NAME).FSharp
CLEAN_FILES=$(FSHARP_ORTOOLS_DLL_NAME).*

# Check for required build tools
ifeq ($(SYSTEM), win)
  FSHARP_COMPILER:=fsc
  FLAG_PREFIX:=/
else
  FSHARP_COMPILER := fsharpc
  FLAG_PREFIX:=--
endif

EXECUTABLES = mono $(FSHARP_COMPILER)
CHECK := $(foreach exec,$(EXECUTABLES),\
        $(if $(shell which $(exec)),some string,$(error "Cannot find '$(exec)' command which is needed for build)))

# Check whether to build Debug or Release version
ifeq (${FSHARP_DEBUG}, 1)
  FSHARP_DEBUG = $(FLAG_PREFIX)debug
endif

# Check for key pair for strong naming
ifdef CLR_KEYFILE
	SIGNING_FLAGS = $(FLAG_PREFIX)keyfile:$(CLR_KEYFILE)
endif

.PHONY: default
default: fsharp-help

.PHONY: fsharp-help # Generate list of targets with descriptions.
fsharp-help:
	$(info Use one of the following targets:)
	@grep "^.PHONY: .* #" $(CURDIR)/makefiles/Makefile.fsharp.mk | sed "s/\.PHONY: \(.*\) # \(.*\)/\1\t\2/" | expand -t20


.PHONY: fsharp # Build F# OR-Tools. Set environment variable FSHARP_DEBUG=1 for debug symbols.
fsharp:
	$(FSHARP_COMPILER) $(FLAG_PREFIX)target:library $(FLAG_PREFIX)out:bin$S$(FSHARP_ORTOOLS_DLL_NAME).dll $(FLAG_PREFIX)platform:anycpu $(FLAG_PREFIX)nocopyfsharpcore $(FLAG_PREFIX)lib:bin $(FLAG_PREFIX)reference:$(BASE_ORTOOLS_DLL_NAME).dll $(FSHARP_DEBUG) $(SIGNING_FLAGS) ortools$Sfsharp$S$(FSHARP_ORTOOLS_DLL_NAME).fsx


.PHONY: fsharp-clean # Clean output from previous build.
fsharp-clean:
	@rm bin$S$(CLEAN_FILES)