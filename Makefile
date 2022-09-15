all: monitor talk

monitor:
	+$(MAKE) -C monitor

talk:
	+$(MAKE) -C talk

.PHONY: monitor talk
