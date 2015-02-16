PACKAGES = libpulse libpulse-mainloop-glib gtk+-3.0 gtkmm-3.0

CXXFLAGS += $(shell pkg-config --cflags $(PACKAGES))
CFLAGS   += $(shell pkg-config --cflags $(PACKAGES))
LDFLAGS  += $(shell pkg-config --libs   $(PACKAGES))

main: main.o
	$(CXX) $< $(LDFLAGS) -o $@
