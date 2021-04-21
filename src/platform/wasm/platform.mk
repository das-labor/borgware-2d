TARGET := out/image.js

SRC += src/platform/wasm/platform.c

compile-main: bundle
	@ echo
	@ echo '#########################################################################################'
	@ echo "# You can now deploy the contents of the out/ directory to the webserver of your choice #"
	@ echo '#########################################################################################'
	@ echo

.PHONY: bundle

bundle: out src/obj/platform/wasm compile-sources
	@ cp $(wildcard src/platform/wasm/common/*) out/
	@ cp $(wildcard src/platform/wasm/$(TARGET_BORG)/*) out/

out:
	@ mkdir $@

src/obj/platform/wasm:
	@ mkdir -p $@
