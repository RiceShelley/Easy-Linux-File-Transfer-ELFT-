CC=gcc
CFLAGS=
EXECUTABLE=elft
CFILES= main.c host.c recv.c
SRCS= $(CFILES) host.h recv.h

all: $(SRCS) 
	$(CC) $(CFLAGS) $(CFILES) -o $(EXECUTABLE)

$(SRCS):
	@echo "Missing files"
	@exit 1

update:
	@echo "Pulling latest update from repo"
	#@rm $(SRCS)

clean: $(EXECUTABLE)
	@rm $(EXECUTABLE)

