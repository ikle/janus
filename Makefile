TARGETS = groups libjanus service utils

.PHONY: $(TARGETS)

all: $(TARGETS)

clean install:
	for i in $(TARGETS); do \
		make -C $$i $@; \
	done

$(TARGETS):
	make -C $@
