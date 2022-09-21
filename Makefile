DIRS=monitor serprog talk

all: $(DIRS)

$(DIRS):
	+$(MAKE) -C $@

.PHONY: $(DIRS)
