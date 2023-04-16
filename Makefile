all:
	$(MAKE) -C aux
	$(MAKE) -C events
	$(MAKE) -C authentication
	$(MAKE) -C net_guard
release:
	$(MAKE) release -C aux
	$(MAKE) release -C events
	$(MAKE) release -C authentication
	$(MAKE) release -C net_guard
net_bot:
	$(MAKE) -C aux
	$(MAKE) -C events
	$(MAKE) -C authentication
	$(MAKE) -C bot
	$(MAKE) net_bot -C net_guard
clean:
	$(MAKE) clean -C aux
	$(MAKE) clean -C events
	$(MAKE) clean -C authentication
	$(MAKE) clean -C bot
	$(MAKE) clean -C net_guard
