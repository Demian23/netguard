all:
	$(MAKE) -C srcs
	$(MAKE) gui -C gui
	$(MAKE) -C gui 
	$(MAKE) -C build 

release:
	$(MAKE) release -C srcs
	$(MAKE) gui -C gui
	$(MAKE) release -C gui 
	$(MAKE) release -C build 

clean:
	$(MAKE) clean -C srcs
	$(MAKE) clean -C gui 
	$(MAKE) clean -C build 

tests: 
	$(MAKE) -C srcs
	$(MAKE) -C test

clean_test:
	$(MAKE) clean -C test
clean_gui:
	$(MAKE) clean -C gui 
	$(MAKE) clean -C build
	

clean_all: clean clean_test
