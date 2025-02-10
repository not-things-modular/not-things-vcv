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

DEPS += json
DEPS += json-schema-validator

json-3.11.3:
	$(WGET) "https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz" -O json-3.11.3.tar.gz
	$(SHA256) json-3.11.3.tar.gz 0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406
	$(UNTAR) json-3.11.3.tar.gz
	rm json-3.11.3.tar.gz

json-schema-validator-40af3ec:
	$(WGET) "https://github.com/pboettch/json-schema-validator/archive/40af3ec.tar.gz" -O json-schema-validator-40af3ec.tar.gz
	$(SHA256) json-schema-validator-40af3ec.tar.gz 36d7e99a73aa6076834736f0fb108fa8e232c4739aa4b3e2089fe96efb21fa8d
	$(UNTAR) json-schema-validator-40af3ec.tar.gz
	mv json-schema-validator-40af3ec39670e768fc3f01f935140af311d71024 json-schema-validator-40af3ec
	rm json-schema-validator-40af3ec.tar.gz

json: | json-3.11.3
	mkdir -p include
	cp -R json-3.11.3/include/* include

json-schema-validator: | json-schema-validator-40af3ec
	mkdir -p include
	mkdir -p src
	cp -R json-schema-validator-40af3ec/src/nlohmann include
	cp -R json-schema-validator-40af3ec/src .

dep: $(DEPS)

clean:
	rm -Rf json-3.11.3
	rm -Rf json-schema-validator-40af3ec
	rm -Rf include
	rm -Rf src
