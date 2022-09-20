DIRS=monitor serprog

all: $(DIRS)

$(DIRS):
	+$(MAKE) -C $@

.PHONY: $(DIRS)
