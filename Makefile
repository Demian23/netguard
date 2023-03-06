all:
	$(MAKE) -C aux
	$(MAKE) -C events
	$(MAKE) -C net_guard
clean:
	$(MAKE) clean -C aux
	$(MAKE) clean -C events
	$(MAKE) clean -C net_guard
