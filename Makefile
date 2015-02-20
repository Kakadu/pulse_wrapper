OUT=main

PACKAGES = libpulse libpulse-mainloop-glib gtk+-3.0 gtkmm-3.0

CXXFLAGS += -g -std=c++0x $(shell pkg-config --cflags $(PACKAGES))
CFLAGS   += $(shell pkg-config --cflags $(PACKAGES))
LDFLAGS  += $(shell pkg-config --libs   $(PACKAGES))

$(OUT): mainwindow.o main.o
	$(CXX) $^ $(LDFLAGS) -o $@

main.o: mainwindow.h

mainwindow.o: mainwindow.h

mainwindow.h:

clean:
	$(RM)  $(OUT) *.o
