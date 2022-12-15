DIRS=monitor serprog talk talk2

all: $(DIRS)

$(DIRS):
	+$(MAKE) -C $@

.PHONY: $(DIRS)
