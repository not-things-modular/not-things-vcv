# Dependencies Makefile mainly based on the dep makefile of VCV Rack (https://github.com/rack/rack)
all: dep

RACK_DIR ?= ../../..
include $(RACK_DIR)/arch.mk

ifdef ARCH_MAC
	SHA256SUM := shasum -a 256
	SED := sed -i ''
else
	SHA256SUM := sha256sum
	SED := sed -i
endif
SHA256 := sha256check() { echo "$$2  $$1" | $(SHA256SUM) -c; }; sha256check

WGET := wget -c
UNTAR := tar xf
UNZIP := unzip -o

DEPS += googletest

googletest-1.12.1:
	$(WGET) "https://github.com/google/googletest/archive/refs/tags/release-1.12.1.tar.gz" -O googletest-1.12.1.tar.gz
	$(SHA256) googletest-1.12.1.tar.gz 81964fe578e9bd7c94dfdb09c8e4d6e6759e19967e397dbea48d1c10e45d0df2
	$(UNTAR) googletest-1.12.1.tar.gz
	rm googletest-1.12.1.tar.gz

googletest: | googletest-1.12.1

dep: $(DEPS)

clean:
	rm -rf $(filter-out Makefile, $(wildcard *))
