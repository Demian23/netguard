all:
	$(MAKE) -C net 
	$(MAKE) -C events
	$(MAKE) -C authentication
	$(MAKE) -C net_guard
test:
	$(MAKE) -C net 
	$(MAKE) -C events
	$(MAKE) -C authentication
	$(MAKE) -C tests
release:
	$(MAKE) release -C net 
	$(MAKE) release -C events
	$(MAKE) release -C authentication
	$(MAKE) release -C net_guard
clean:
	$(MAKE) clean -C net
	$(MAKE) clean -C events
	$(MAKE) clean -C authentication
	$(MAKE) clean -C tests
	$(MAKE) clean -C net_guard
